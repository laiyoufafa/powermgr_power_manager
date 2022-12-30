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

#ifndef POWERMGR_SERVICE_TEST_PROXY_H
#define POWERMGR_SERVICE_TEST_PROXY_H

#include <cstdint>
#include <sys/types.h>
#include <functional>
#include <iosfwd>
#include <vector>

#include "iremote_object.h"
#include "ishutdown_callback.h"
#include "ipower_mode_callback.h"
#include "ipower_state_callback.h"
#include "ipower_mgr.h"
#include "running_lock_info.h"
#include "power_errors.h"
#include "power_mgr_service.h"
#include "power_state_machine_info.h"

namespace OHOS {
namespace PowerMgr {
class PowerMgrServiceTestProxy {
public:
    PowerMgrServiceTestProxy(const sptr<PowerMgrService>& service);
    ~PowerMgrServiceTestProxy() = default;

    PowerErrors CreateRunningLock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo);
    bool ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj);
    bool IsRunningLockTypeSupported(uint32_t type);
    bool Lock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo, uint32_t timeOutMs);
    bool UnLock(const sptr<IRemoteObject>& remoteObj);
    bool SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj, const WorkTriggerList& workTriggerList);
    bool ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid);
    bool IsUsed(const sptr<IRemoteObject>& remoteObj);
    PowerErrors SuspendDevice(int64_t callTimeMs,
        SuspendDeviceType reason = SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION,
        bool suspendImmed = false);
    PowerErrors WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason = WakeupDeviceType::WAKEUP_DEVICE_APPLICATION,
        const std::string& detail = std::string("app call"));
    bool RefreshActivity(int64_t callTimeMs, UserActivityType type = UserActivityType::USER_ACTIVITY_TYPE_OTHER,
        bool needChangeBacklight = true);
    bool OverrideScreenOffTime(int64_t timeout);
    bool RestoreScreenOffTime();
    PowerState GetState();
    bool IsScreenOn();
    bool ForceSuspendDevice(int64_t callTimeMs);
    PowerErrors RebootDevice(const std::string& reason);
    PowerErrors ShutDownDevice(const std::string& reason);
    bool RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    bool UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback);
    bool RegisterShutdownCallback(const sptr<IShutdownCallback>& callback,
        IShutdownCallback::ShutdownPriority priority
            = IShutdownCallback::ShutdownPriority::POWER_SHUTDOWN_PRIORITY_DEFAULT);
    bool UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback);
    bool RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    bool UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback);
    bool SetDisplaySuspend(bool enable);
    PowerErrors SetDeviceMode(const PowerMode& mode);
    PowerMode GetDeviceMode();
    std::string ShellDump(const std::vector<std::string>& args, uint32_t argc);
private:
    sptr<PowerMgrStub> stub_ {nullptr};
};
} // namespace PowerMgr
} // namespace OHOS
#endif // POWERMGR_SERVICE_TEST_PROXY_H
