/*
 * Copyright (c) 2021-2023 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "wakeup_controller.h"
#include "suspend_controller.h"
#include <datetime_ex.h>
#include <hisysevent.h>
#include <input_manager.h>
#include <ipc_skeleton.h>
#include <securec.h>

#include "permission.h"
#include "power_errors.h"
#include "power_log.h"
#include "power_mgr_service.h"
#include "power_state_callback_stub.h"
#include "powerms_event_handler.h"
#include "setting_helper.h"
#include "system_suspend_controller.h"

namespace OHOS {
namespace PowerMgr {
using namespace OHOS::MMI;
namespace {
sptr<SettingObserver> g_wakeupSourcesKeyObserver = nullptr;
}

/** WakeupController Implement */

WakeupController::WakeupController(
    std::shared_ptr<PowerStateMachine>& stateMachine, std::shared_ptr<AppExecFwk::EventRunner>& runner)
{
    stateMachine_ = stateMachine;
    runner_ = runner;
    std::shared_ptr<InputCallback> callback = std::make_shared<InputCallback>();
    if (monitorId_ < 0) {
        monitorId_ = InputManager::GetInstance()->AddMonitor(std::static_pointer_cast<IInputEventConsumer>(callback));
    }
}

WakeupController::~WakeupController()
{
    InputManager* inputManager = InputManager::GetInstance();
    if (monitorId_ >= 0) {
        inputManager->RemoveMonitor(monitorId_);
    }

    if (g_wakeupSourcesKeyObserver) {
        SettingHelper::UnregisterSettingWakeupSourcesObserver(g_wakeupSourcesKeyObserver);
    }
}

void WakeupController::Init()
{
    std::shared_ptr<WakeupSources> sources = WakeupSourceParser::ParseSources();
    handler_ = std::make_shared<WakeupEventHandler>(runner_, shared_from_this());
    sourceList_ = sources->GetSourceList();
    if (sourceList_.empty()) {
        POWER_HILOGE(FEATURE_WAKEUP, "InputManager is null");
        return;
    }

    for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
        POWER_HILOGI(FEATURE_WAKEUP, "registered type=%{public}u", (*source).GetReason());
        std::shared_ptr<WakeupMonitor> monitor = WakeupMonitor::CreateMonitor(*source);
        if (monitor != nullptr && monitor->Init()) {
            POWER_HILOGI(FEATURE_WAKEUP, "register type=%{public}u", (*source).GetReason());
            monitor->RegisterListener(std::bind(&WakeupController::ControlListener, this, std::placeholders::_1));
            monitorMap_.emplace(monitor->GetReason(), monitor);
        }
    }
    RegisterSettingsObserver();
}

void WakeupController::Cancel()
{
    for (auto monitor = monitorMap_.begin(); monitor != monitorMap_.end(); monitor++) {
        monitor->second->Cancel();
    }
    monitorMap_.clear();
}

void WakeupController::RegisterSettingsObserver()
{
    if (g_wakeupSourcesKeyObserver) {
        POWER_HILOGE(FEATURE_POWER_STATE, "wakeup sources key observer is already registered");
        return;
    }
    SettingObserver::UpdateFunc updateFunc = [&](const std::string&) {
        std::lock_guard lock(monitorMutex_);
        POWER_HILOGI(COMP_SVC, "start setting string update");
        std::string jsonStr = SettingHelper::GetSettingWakeupSources();
        std::shared_ptr<WakeupSources> sources = WakeupSourceParser::ParseSources(jsonStr);
        std::vector<WakeupSource> updateSourceList = sources->GetSourceList();
        if (updateSourceList.size() == 0) {
            return;
        }
        sourceList_ = updateSourceList;
        POWER_HILOGI(COMP_SVC, "start updateListener");
        Cancel();
        for (auto source = sourceList_.begin(); source != sourceList_.end(); source++) {
            std::shared_ptr<WakeupMonitor> monitor = WakeupMonitor::CreateMonitor(*source);
            if (monitor != nullptr && monitor->Init()) {
                monitor->RegisterListener(std::bind(&WakeupController::ControlListener, this, std::placeholders::_1));
                monitorMap_.emplace(monitor->GetReason(), monitor);
            }
        }
    };
    g_wakeupSourcesKeyObserver = SettingHelper::RegisterSettingWakeupSourcesObserver(updateFunc);
    POWER_HILOGI(FEATURE_POWER_STATE, "register setting observer fin");
}

void WakeupController::ExecWakeupMonitorByReason(WakeupDeviceType reason)
{
    std::lock_guard lock(monitorMutex_);
    if (monitorMap_.find(reason) != monitorMap_.end()) {
        auto monitor = monitorMap_[reason];
        monitor->Notify();
    }
}

void WakeupController::Wakeup()
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get powerMgrService instance error");
        return;
    }
    auto suspendController = pms->GetSuspendController();
    if (suspendController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get suspendController instance error");
        return;
    }
    suspendController->StopSleep();
}

void WakeupController::ControlListener(WakeupDeviceType reason)
{
    std::lock_guard lock(mutex_);
    pid_t pid = IPCSkeleton::GetCallingPid();
    auto uid = IPCSkeleton::GetCallingUid();
    if (!Permission::IsSystem()) {
        return;
    }
    POWER_HILOGI(FEATURE_WAKEUP, "Try to wakeup device, pid=%{public}d, uid=%{public}d", pid, uid);
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    int64_t now = static_cast<int64_t>(time(nullptr));
    pms->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    if (pms->IsScreenOn()) {
        return;
    }

    if (stateMachine_->GetState() != PowerState::AWAKE) {
        Wakeup();
        StartWakeupTimer();
        SystemSuspendController::GetInstance().Wakeup();
        POWER_HILOGI(FEATURE_WAKEUP, "wakeup Request: %{public}d", reason);
        bool ret = stateMachine_->SetState(
            PowerState::AWAKE, stateMachine_->GetReasonByWakeType(static_cast<WakeupDeviceType>(reason)), true);
        if (ret != true) {
            POWER_HILOGI(FEATURE_WAKEUP, "setstate wakeup error");
        }
    } else {
        POWER_HILOGI(FEATURE_WAKEUP, "state=%{public}u no transitor", stateMachine_->GetState());
    }
}

void WakeupController::StartWakeupTimer()
{
    if (handler_ == nullptr) {
        return;
    }
    handler_->SendEvent(PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG, 0, WakeupMonitor::POWER_KEY_PRESS_DELAY_MS);
}

void WakeupController::NotifyDisplayActionDone(uint32_t event)
{
    handler_->RemoveEvent(PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG);
}

/** WakeupEventHandler Implement */

void WakeupEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event)
{
    std::shared_ptr<WakeupController> controller = controller_.lock();
    if (controller == nullptr) {
        POWER_HILOGI(FEATURE_WAKEUP, "ProcessEvent: No controller");
        return;
    }
    POWER_HILOGI(FEATURE_WAKEUP, "recv event=%{public}d", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case PowermsEventHandler::SCREEN_ON_TIMEOUT_MSG: {
            std::string message = "POWER KEY TIMEOUT BUT DISPLAY NOT FINISHED";
            HiSysEventWrite(HiviewDFX::HiSysEvent::Domain::POWER, "SCREEN_ON_TIMEOUT",
                HiviewDFX::HiSysEvent::EventType::FAULT, "PID", IPCSkeleton::GetCallingPid(), "UID",
                IPCSkeleton::GetCallingUid(), "PACKAGE_NAME", "", "PROCESS_NAME", "", "MSG", message.c_str());
            break;
        }
        default:
            break;
    }
}

/* InputCallback achieve */
void InputCallback::OnInputEvent(std::shared_ptr<KeyEvent> keyEvent) const
{
    int32_t keyCode = keyEvent->GetKeyCode();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "get powerMgrService instance error");
        return;
    }

    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    if (wakeupController == nullptr) {
        POWER_HILOGE(FEATURE_WAKEUP, "wakeupController is not init");
        return;
    }

    if (keyCode == KeyEvent::KEYCODE_F1) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK;
    }

    if (keyCode >= KeyEvent::KEYCODE_0 && keyCode <= KeyEvent::KEYCODE_NUMPAD_RIGHT_PAREN) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD;
    }

    POWER_HILOGI(FEATURE_WAKEUP, "KeyEvent wakeupType=%{public}u, keyCode=%{public}d", wakeupType, keyCode);
    if (wakeupType != WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        wakeupController->ExecWakeupMonitorByReason(wakeupType);
    }
}

void InputCallback::OnInputEvent(std::shared_ptr<PointerEvent> pointerEvent) const
{
    auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
    if (pms == nullptr) {
        return;
    }
    std::shared_ptr<WakeupController> wakeupController = pms->GetWakeupController();
    WakeupDeviceType wakeupType = WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN;
    PointerEvent::PointerItem pointerItem;
    if (!pointerEvent->GetPointerItem(pointerEvent->GetPointerId(), pointerItem)) {
        POWER_HILOGI(FEATURE_WAKEUP, "GetPointerItem false");
    }
    int32_t deviceType = pointerItem.GetToolType();
    int32_t sourceType = pointerEvent->GetSourceType();
    POWER_HILOGI(FEATURE_WAKEUP, "deviceType=%{public}d", deviceType);
    if (deviceType == PointerEvent::TOOL_TYPE_PEN) {
        wakeupType = WakeupDeviceType::WAKEUP_DEVICE_PEN;
        POWER_HILOGI(FEATURE_WAKEUP, "current wakeup reason=%{public}u", wakeupType);
        wakeupController->ExecWakeupMonitorByReason(wakeupType);
        return;
    }

    switch (sourceType) {
        case PointerEvent::SOURCE_TYPE_MOUSE:
            wakeupType = WakeupDeviceType::WAKEUP_DEVICE_MOUSE;
            break;
        case PointerEvent::SOURCE_TYPE_TOUCHPAD:
            wakeupType = WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD;
            break;
        case PointerEvent::SOURCE_TYPE_TOUCHSCREEN:
            wakeupType = WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK;
            break;
        default:
            break;
    }

    if (wakeupType == WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK) {
        int64_t now = static_cast<int64_t>(time(nullptr));
        pms->RefreshActivity(now, UserActivityType::USER_ACTIVITY_TYPE_BUTTON, false);
    }

    if (wakeupType != WakeupDeviceType::WAKEUP_DEVICE_UNKNOWN) {
        POWER_HILOGI(FEATURE_WAKEUP, "current wakeup reason=%{public}u", wakeupType);
        wakeupController->ExecWakeupMonitorByReason(wakeupType);
    }
}

void InputCallback::OnInputEvent(std::shared_ptr<AxisEvent> axisEvent) const
{
    POWER_HILOGD(FEATURE_WAKEUP, "AxisEvent");
}

/* WakeupMonitor Implement */

std::shared_ptr<WakeupMonitor> WakeupMonitor::CreateMonitor(WakeupSource& source)
{
    WakeupDeviceType reason = source.GetReason();
    POWER_HILOGE(FEATURE_WAKEUP, "CreateMonitor reason=%{public}d", reason);
    std::shared_ptr<WakeupMonitor> monitor = nullptr;
    switch (reason) {
        case WakeupDeviceType::WAKEUP_DEVICE_POWER_BUTTON:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<PowerkeyWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_MOUSE:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<MousekeyWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_KEYBOARD:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<KeyboardWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_PEN:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<PenWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_TOUCHPAD:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<TouchpadWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SINGLE_CLICK:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<SingleClickWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_DOUBLE_CLICK:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<DoubleClickWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_LID:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<LidWakeupMonitor>(source));
            break;
        case WakeupDeviceType::WAKEUP_DEVICE_SWITCH:
            monitor = std::static_pointer_cast<WakeupMonitor>(std::make_shared<SwitchWakeupMonitor>(source));
            break;
        default:
            POWER_HILOGE(FEATURE_WAKEUP, "CreateMonitor : Invalid reason=%{public}d", reason);
            break;
    }
    return monitor;
}

/** PowerkeyWakeupMonitor Implement */

bool PowerkeyWakeupMonitor::Init()
{
    if (powerkeyShortPressId_ >= 0) {
        return true;
    }
    std::shared_ptr<OHOS::MMI::KeyOption> keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    std::set<int32_t> preKeys;
    keyOption.reset();
    keyOption = std::make_shared<OHOS::MMI::KeyOption>();
    keyOption->SetPreKeys(preKeys);
    keyOption->SetFinalKey(OHOS::MMI::KeyEvent::KEYCODE_POWER);
    keyOption->SetFinalKeyDown(true);
    keyOption->SetFinalKeyDownDuration(0);
    powerkeyShortPressId_ = InputManager::GetInstance()->SubscribeKeyEvent(
        keyOption, [this](std::shared_ptr<OHOS::MMI::KeyEvent> keyEvent) {
            POWER_HILOGI(FEATURE_WAKEUP, "receive wakeup controller key down");
            auto pms = DelayedSpSingleton<PowerMgrService>::GetInstance();
            if (pms == nullptr) {
                return;
            }
            std::shared_ptr<SuspendController> suspendController = pms->GetSuspendController();
            suspendController->RecordPowerKeyDown();
            Notify();
        });

    POWER_HILOGI(FEATURE_WAKEUP, "powerkey register powerkeyShortPressId_=%{public}d", powerkeyShortPressId_);
    return powerkeyShortPressId_ >= 0 ? true : false;
}

void PowerkeyWakeupMonitor::Cancel()
{
    if (powerkeyShortPressId_ >= 0) {
        InputManager::GetInstance()->UnsubscribeKeyEvent(powerkeyShortPressId_);
    }
}

/** Keyboard Implement */

bool KeyboardWakeupMonitor::Init()
{
    return true;
}

void KeyboardWakeupMonitor::Cancel() {}

/** Mouse Implement */

bool MousekeyWakeupMonitor::Init()
{
    return true;
}

void MousekeyWakeupMonitor::Cancel() {}

/** Mouse Implement */

bool TouchpadWakeupMonitor::Init()
{
    return true;
}

void TouchpadWakeupMonitor::Cancel() {}

/** Pen Implement */

bool PenWakeupMonitor::Init()
{
    return true;
}

void PenWakeupMonitor::Cancel() {}

/** SingleClickWakeupMonitor Implement */

bool SingleClickWakeupMonitor::Init()
{
    return true;
}

void SingleClickWakeupMonitor::Cancel() {}

/** DoubleClickWakeupMonitor Implement */

bool DoubleClickWakeupMonitor::Init()
{
    return true;
}

void DoubleClickWakeupMonitor::Cancel() {}

/** SwitchWakeupMonitor Implement */

bool SwitchWakeupMonitor::Init()
{
    return true;
}

void SwitchWakeupMonitor::Cancel() {}

/** LidWakeupMonitor Implement */

bool LidWakeupMonitor::Init()
{
    return true;
}

void LidWakeupMonitor::Cancel() {}

} // namespace PowerMgr
} // namespace OHOS
