/*
 * Copyright (c) 2022 Huawei Device Co., Ltd.
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

#include "powermgr_service_mock_parcel_test.h"

#include <datetime_ex.h>

#include "power_log.h"
#include "power_mode_callback_proxy.h"
#include "power_mgr_service.h"
#include "power_state_callback_proxy.h"
#include "power_state_machine_info.h"
#include "powermgr_service_test_proxy.h"
#include "running_lock_token_stub.h"
#include "work_trigger.h"

using namespace testing::ext;
using namespace OHOS::PowerMgr;
using namespace OHOS;
using namespace std;

namespace {
static sptr<PowerMgrService> g_powerMgrService = nullptr;
static std::shared_ptr<PowerMgrServiceTestProxy> g_powerMgrServiceProxy = nullptr;
} // namespace

void PowerMgrServiceMockParcelTest::SetUpTestCase()
{
    g_powerMgrService = DelayedSpSingleton<PowerMgrService>::GetInstance();
    g_powerMgrService->OnStart();

    if (g_powerMgrServiceProxy == nullptr) {
        g_powerMgrServiceProxy = std::make_shared<PowerMgrServiceTestProxy>(g_powerMgrService);
    }
}

void PowerMgrServiceMockParcelTest::TearDownTestCase()
{
    g_powerMgrService->OnStop();
    DelayedSpSingleton<PowerMgrService>::DestroyInstance();
}

void PowerMgrServiceMockParcelTest::PowerModeTestCallback::OnPowerModeChanged(PowerMode mode)
{
    POWER_HILOGD(LABEL_TEST, "PowerModeTestCallback::OnPowerModeChanged.");
}

void PowerMgrServiceMockParcelTest::PowerShutdownTestCallback::ShutdownCallback()
{
    POWER_HILOGD(LABEL_TEST, "PowerShutdownTestCallback::ShutdownCallback.");
}

void PowerMgrServiceMockParcelTest::PowerStateTestCallback::OnPowerStateChanged(PowerState state)
{
    POWER_HILOGD(LABEL_TEST, "PowerStateTestCallback::OnPowerStateChanged.");
}

namespace {
/**
 * @tc.name: PowerMgrServiceMockParcelTest001
 * @tc.desc: PowerMgr service test when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceMockParcelTest, PowerMgrServiceMockParcelTest001, TestSize.Level2)
{
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    sptr<IRemoteObject> token = new RunningLockTokenStub();
    RunningLockInfo runningLockInfo;
    runningLockInfo.name = "runninglock";
    runningLockInfo.type = RunningLockType::RUNNINGLOCK_SCREEN;
    pid_t uid = 0;
    pid_t pid = 0;
    uint32_t timeOutMs = 0;
    EXPECT_FALSE(g_powerMgrServiceProxy->CreateRunningLock(token, runningLockInfo) == PowerErrors::ERR_OK);
    EXPECT_FALSE(g_powerMgrServiceProxy->ReleaseRunningLock(token));
    auto type = static_cast<uint32_t>(RunningLockType::RUNNINGLOCK_BUTT);
    EXPECT_FALSE(g_powerMgrServiceProxy->IsRunningLockTypeSupported(type));
    EXPECT_FALSE(g_powerMgrServiceProxy->Lock(token, runningLockInfo, timeOutMs));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnLock(token));
    EXPECT_FALSE(g_powerMgrServiceProxy->IsUsed(token));
    WorkTriggerList workList;
    workList.push_back(nullptr);
    workList.push_back(nullptr);
    workList.push_back(nullptr);
    EXPECT_FALSE(g_powerMgrServiceProxy->SetWorkTriggerList(token, workList));
    EXPECT_FALSE(g_powerMgrServiceProxy->ProxyRunningLock(true, uid, pid));
}

/**
 * @tc.name: PowerMgrServiceMockParcelTest002
 * @tc.desc: PowerMgr service test when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceMockParcelTest, PowerMgrServiceMockParcelTest002, TestSize.Level2)
{
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    int32_t suspendReason = (static_cast<int32_t>(SuspendDeviceType::SUSPEND_DEVICE_REASON_MAX)) + 1;
    SuspendDeviceType abnormaltype = SuspendDeviceType(suspendReason);
    EXPECT_EQ(g_powerMgrServiceProxy->SuspendDevice(0, abnormaltype, false), PowerErrors::ERR_CONNECTION_FAIL);
    auto error = g_powerMgrServiceProxy->WakeupDevice(GetTickCount(),
        WakeupDeviceType::WAKEUP_DEVICE_APPLICATION, std::string("app call"));
    EXPECT_EQ(error, PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_FALSE(g_powerMgrServiceProxy->RefreshActivity(GetTickCount(),
        UserActivityType::USER_ACTIVITY_TYPE_ATTENTION, true));
    EXPECT_FALSE(g_powerMgrServiceProxy->OverrideScreenOffTime(200));
    EXPECT_FALSE(g_powerMgrServiceProxy->RestoreScreenOffTime());
    EXPECT_EQ(g_powerMgrServiceProxy->GetState(), PowerState::UNKNOWN);
    EXPECT_FALSE(g_powerMgrServiceProxy->IsScreenOn());
    g_powerMgrServiceProxy->SetDisplaySuspend(true);
    PowerMode setMode = PowerMode::NORMAL_MODE;
    EXPECT_EQ(g_powerMgrServiceProxy->SetDeviceMode(setMode), PowerErrors::ERR_CONNECTION_FAIL);
    PowerMode getMode = g_powerMgrServiceProxy->GetDeviceMode();
    EXPECT_TRUE(getMode == setMode);
}

/**
 * @tc.name: PowerMgrServiceMockParcelTest003
 * @tc.desc: PowerMgr service test when the Parcel is mock
 * @tc.type: FUNC
 * @tc.require: issueI67Z62
 */
HWTEST_F(PowerMgrServiceMockParcelTest, PowerMgrServiceMockParcelTest003, TestSize.Level2)
{
    ASSERT_NE(g_powerMgrServiceProxy, nullptr);
    sptr<IPowerStateCallback> stateCb = new PowerStateTestCallback();
    sptr<IShutdownCallback> shutdownCb = new PowerShutdownTestCallback();
    sptr<IPowerModeCallback> modeCb = new PowerModeTestCallback();
    auto priority = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_LOW;
    EXPECT_FALSE(g_powerMgrServiceProxy->RegisterPowerStateCallback(stateCb));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnRegisterPowerStateCallback(stateCb));
    EXPECT_FALSE(g_powerMgrServiceProxy->RegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnRegisterPowerStateCallback(nullptr));
    EXPECT_FALSE(g_powerMgrServiceProxy->RegisterShutdownCallback(shutdownCb, priority));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnRegisterShutdownCallback(shutdownCb));
    EXPECT_FALSE(g_powerMgrServiceProxy->RegisterShutdownCallback(nullptr, priority));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnRegisterShutdownCallback(nullptr));
    EXPECT_FALSE(g_powerMgrServiceProxy->RegisterPowerModeCallback(modeCb));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnRegisterPowerModeCallback(modeCb));
    EXPECT_FALSE(g_powerMgrServiceProxy->RegisterPowerModeCallback(nullptr));
    EXPECT_FALSE(g_powerMgrServiceProxy->UnRegisterPowerModeCallback(nullptr));
    EXPECT_EQ(g_powerMgrServiceProxy->RebootDevice(""), PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_EQ(g_powerMgrServiceProxy->ShutDownDevice(""), PowerErrors::ERR_CONNECTION_FAIL);
    EXPECT_FALSE(g_powerMgrServiceProxy->ForceSuspendDevice(0));
    static std::vector<std::string> dumpArgs;
    dumpArgs.push_back("-a");
    std::string errorCode = "remote error";
    std::string actualDebugInfo = g_powerMgrServiceProxy->ShellDump(dumpArgs, dumpArgs.size());
    EXPECT_EQ(errorCode, actualDebugInfo);
}
} // namespace