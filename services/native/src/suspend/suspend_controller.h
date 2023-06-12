/*
 * Copyright (c) 2023 Huawei Device Co., Ltd.
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

#ifndef POWERMGR_SUSPEND_CONTROLLER_H
#define POWERMGR_SUSPEND_CONTROLLER_H

#include "event_handler.h"
#include "power_state_machine.h"
#include "sensor_agent.h"
#include "shutdown_service.h"
#include "suspend_source_parser.h"
#include "suspend_sources.h"
#include <functional>
#include <memory>
#include <vector>

namespace OHOS {
namespace PowerMgr {

using SuspendListener = std::function<void(SuspendDeviceType, uint32_t, uint32_t)>;

class SuspendMonitor;
class SuspendEventHandler;
class SuspendController : public std::enable_shared_from_this<SuspendController> {
public:
    SuspendController(ShutdownService* shutdownService, std::shared_ptr<PowerStateMachine>& stateMachine,
        std::shared_ptr<AppExecFwk::EventRunner>& runner);
    ~SuspendController();
    void Init();
    void ExecSuspendMonitorByReason(SuspendDeviceType reason);
    void RegisterSettingsObserver();
    void Execute();
    void Cancel();
    void StopSleep();
    void HandleEvent(uint32_t eventId);
    void HandleAction(SuspendDeviceType reason, uint32_t action);
    void RecordPowerKeyDown();
    bool GetPowerkeyDownWhenScreenOff();
    std::shared_ptr<PowerStateMachine> GetStateMachine()
    {
        return stateMachine_;
    }
    SuspendDeviceType GetLastReason()
    {
        return sleepReason_;
    }
    uint32_t GetLastAction()
    {
        return sleepAction_;
    }

private:
    void ControlListener(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    void StartSleepTimer(SuspendDeviceType reason, uint32_t action, uint32_t delay);
    void HandleAutoSleep(SuspendDeviceType reason);
    void HandleForceSleep(SuspendDeviceType reason);
    void HandleHibernate(SuspendDeviceType reason);
    void HandleShutdown(SuspendDeviceType reason);
    std::mutex monitorMutex_;
    std::vector<SuspendSource> sourceList_;
    std::map<SuspendDeviceType, std::shared_ptr<SuspendMonitor>> monitorMap_;
    ShutdownService* shutdownService_;
    std::shared_ptr<PowerStateMachine> stateMachine_;
    std::shared_ptr<AppExecFwk::EventRunner> runner_;
    std::shared_ptr<SuspendEventHandler> handler_ = nullptr;
    uint32_t sleepDuration_ {0};
    int64_t sleepTime_ {-1};
    SuspendDeviceType sleepReason_ {0};
    uint32_t sleepAction_ {0};
    bool powerkeyDownWhenScreenOff_ = false;
};

class SuspendEventHandler : public AppExecFwk::EventHandler {
public:
    enum {
        SLEEP_TIMEOUT_MSG = 100,
        SCREEN_OFF_TIMEOUT_MSG,
        SUSPEND_MAX_MSG,
    };
    SuspendEventHandler(
        const std::shared_ptr<AppExecFwk::EventRunner>& runner, std::shared_ptr<SuspendController> controller)
        : AppExecFwk::EventHandler(runner),
        controller_(controller)
    {
    }
    ~SuspendEventHandler() = default;
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer& event) override;

private:
    std::weak_ptr<SuspendController> controller_;
};

class SuspendMonitor {
public:
    static std::shared_ptr<SuspendMonitor> CreateMonitor(SuspendSource& source);

    virtual ~SuspendMonitor() = default;
    virtual bool Init() = 0;
    virtual void Cancel() = 0;
    virtual void HandleEvent(uint32_t eventId)
    {
        // do nothing in base class
    }
    SuspendDeviceType GetReason()
    {
        return reason_;
    }
    uint32_t GetAction()
    {
        return action_;
    }
    uint32_t GetDelay()
    {
        return delayMs_;
    }
    void RegisterListener(SuspendListener listener)
    {
        listener_ = listener;
    }

    void Notify()
    {
        listener_(reason_, action_, delayMs_);
    }
protected:
    explicit SuspendMonitor(SuspendSource& source)
    {
        reason_ = source.GetReason();
        action_ = source.GetAction();
        delayMs_ = source.GetDelay();
    }

    SuspendDeviceType reason_;
    uint32_t action_;
    uint32_t delayMs_;
    SuspendListener listener_;
};

class PowerKeySuspendMonitor : public SuspendMonitor {
public:
    explicit PowerKeySuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~PowerKeySuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;

private:
    static constexpr int32_t LONG_PRESS_DELAY_MS = 3000;
    static constexpr int32_t POWER_KEY_PRESS_DELAY_MS = 10000;
    int32_t powerkeyReleaseId_ {-1};
};

class TimeoutSuspendMonitor : public SuspendMonitor {
public:
    explicit TimeoutSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~TimeoutSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
    void HandleEvent(uint32_t eventId) override;
};

class LidSuspendMonitor : public SuspendMonitor {
public:
    explicit LidSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~LidSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

class SwitchSuspendMonitor : public SuspendMonitor {
public:
    explicit SwitchSuspendMonitor(SuspendSource& source) : SuspendMonitor(source) {}
    ~SwitchSuspendMonitor() override = default;
    bool Init() override;
    void Cancel() override;
};

} // namespace PowerMgr
} // namespace OHOS

#endif // POWERMGR_SUSPEND_CONTROLLER_H