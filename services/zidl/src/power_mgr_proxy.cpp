/*
 * Copyright (c) 2021-2022 Huawei Device Co., Ltd.
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

#include "power_mgr_proxy.h"

#include <ipc_types.h>
#include <message_parcel.h>
#include <string_ex.h>

#include "power_common.h"

namespace OHOS {
namespace PowerMgr {
void PowerMgrProxy::CreateRunningLock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, remoteObj.GetRefPtr());
    WRITE_PARCEL_NO_RET(data, Parcelable, &runningLockInfo);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::CREATE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::ReleaseRunningLock(const sptr<IRemoteObject>& remoteObj)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, remoteObj.GetRefPtr());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RELEASE_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

bool PowerMgrProxy::IsRunningLockTypeSupported(uint32_t type)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Uint32, static_cast<uint32_t>(type), false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::IS_RUNNINGLOCK_TYPE_SUPPORTED), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }

    if (!reply.ReadBool(result)) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "ReadBool fail");
    }

    return result;
}

void PowerMgrProxy::Lock(const sptr<IRemoteObject>& remoteObj, const RunningLockInfo& runningLockInfo,
    uint32_t timeOutMs)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, remoteObj.GetRefPtr());
    WRITE_PARCEL_NO_RET(data, Parcelable, &runningLockInfo);
    WRITE_PARCEL_NO_RET(data, Uint32, timeOutMs);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_LOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::UnLock(const sptr<IRemoteObject>& remoteObj)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, remoteObj.GetRefPtr());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_UNLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

bool PowerMgrProxy::IsUsed(const sptr<IRemoteObject>& remoteObj)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return false;
    }

    WRITE_PARCEL_WITH_RET(data, RemoteObject, remoteObj.GetRefPtr(), false);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_ISUSED),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return false;
    }
    bool used = false;
    READ_PARCEL_WITH_RET(reply, Bool, used, false);
    return used;
}

void PowerMgrProxy::SetWorkTriggerList(const sptr<IRemoteObject>& remoteObj, const WorkTriggerList& workTriggerList)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, remoteObj.GetRefPtr());
    RETURN_IF(!RunningLockInfo::MarshallingWorkTriggerList(data, workTriggerList));
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::RUNNINGLOCK_SET_WORK_TRIGGER_LIST),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::ProxyRunningLock(bool proxyLock, pid_t uid, pid_t pid)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Bool, proxyLock);
    WRITE_PARCEL_NO_RET(data, Int32, uid);
    WRITE_PARCEL_NO_RET(data, Int32, pid);
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::PROXY_RUNNINGLOCK),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_RUNNING_LOCK, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::RebootDevice(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, String16, Str8ToStr16(reason));

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REBOOT_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
    }
}

void PowerMgrProxy::ShutDownDevice(const std::string& reason)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, String16, Str8ToStr16(reason));

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SHUTDOWN_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
    }
}

void PowerMgrProxy::SuspendDevice(int64_t callTimeMs, SuspendDeviceType reason, bool suspendImmed)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Int64, callTimeMs);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(reason));
    WRITE_PARCEL_NO_RET(data, Bool, suspendImmed);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SUSPEND_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
    }
}

void PowerMgrProxy::WakeupDevice(int64_t callTimeMs, WakeupDeviceType reason, const std::string& details)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_WAKEUP, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Int64, callTimeMs);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(reason));
    WRITE_PARCEL_NO_RET(data, String16, Str8ToStr16(details));

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::WAKEUP_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_WAKEUP, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::RefreshActivity(int64_t callTimeMs, UserActivityType type, bool needChangeBacklight)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_ACTIVITY, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Int64, callTimeMs);
    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(type));
    WRITE_PARCEL_NO_RET(data, Bool, needChangeBacklight);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REFRESH_ACTIVITY), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_ACTIVITY, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

bool PowerMgrProxy::ForceSuspendDevice(int64_t callTimeMs)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return result;
    }

    WRITE_PARCEL_WITH_RET(data, Int64, callTimeMs, false);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::FORCE_DEVICE_SUSPEND), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(FEATURE_SUSPEND, "ReadBool fail");
    }

    return result;
}

PowerState PowerMgrProxy::GetState()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, PowerState::UNKNOWN);

    uint32_t result = 0;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return PowerState::UNKNOWN;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::GET_STATE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return PowerState::UNKNOWN;
    }
    if (!reply.ReadUint32(result)) {
        POWER_HILOGE(FEATURE_POWER_STATE, "ReadUint32 failed");
        return PowerState::UNKNOWN;
    }

    return static_cast<PowerState>(result);
}

bool PowerMgrProxy::IsScreenOn()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    bool result = false;
    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::IS_SCREEN_ON), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    if (!reply.ReadBool(result)) {
        POWER_HILOGE(COMP_FWK, "ReadBool fail");
    }

    return result;
}

void PowerMgrProxy::RegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_POWER_STATE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::UnRegisterPowerStateCallback(const sptr<IPowerStateCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_STATE, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_POWER_STATE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_STATE, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::RegisterShutdownCallback(IShutdownCallback::ShutdownPriority priority,
    const sptr<IShutdownCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Uint32, static_cast<uint32_t>(priority));
    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_SHUTDOWN_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::UnRegisterShutdownCallback(const sptr<IShutdownCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_SHUTDOWN_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SHUTDOWN, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::RegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::REG_POWER_MODE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::UnRegisterPowerModeCallback(const sptr<IPowerModeCallback>& callback)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF((remote == nullptr) || (callback == nullptr));

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, RemoteObject, callback->AsObject());

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::UNREG_POWER_MODE_CALLBACK), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return;
    }
}

void PowerMgrProxy::SetDisplaySuspend(bool enable)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_SUSPEND, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Bool, enable);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SET_DISPLAY_SUSPEND), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_SUSPEND, "SendRequest is failed, ret: %{public}d", ret);
    }
}

void PowerMgrProxy::SetDeviceMode(const uint32_t& mode)
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF(remote == nullptr);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return;
    }

    WRITE_PARCEL_NO_RET(data, Uint32, mode);

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SETMODE_DEVICE), data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
    }
}

uint32_t PowerMgrProxy::GetDeviceMode()
{
    sptr<IRemoteObject> remote = Remote();
    RETURN_IF_WITH_RET(remote == nullptr, false);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(FEATURE_POWER_MODE, "Write descriptor failed");
        return 0;
    }

    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::GETMODE_DEVICE),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(FEATURE_POWER_MODE, "SendRequest is failed, ret: %{public}d", ret);
        return 0;
    }

    uint32_t used = 0;
    if (!reply.ReadUint32(used)) {
        POWER_HILOGE(FEATURE_POWER_MODE, "ReadUint32 fail");
    }
    return used;
}

std::string PowerMgrProxy::ShellDump(const std::vector<std::string>& args, uint32_t argc)
{
    sptr<IRemoteObject> remote = Remote();
    std::string result = "remote error";
    RETURN_IF_WITH_RET(remote == nullptr, result);

    MessageParcel data;
    MessageParcel reply;
    MessageOption option;

    if (!data.WriteInterfaceToken(PowerMgrProxy::GetDescriptor())) {
        POWER_HILOGE(COMP_FWK, "Write descriptor failed");
        return result;
    }

    data.WriteUint32(argc);
    for (uint32_t i = 0; i < argc; i++) {
        data.WriteString(args[i]);
    }
    int ret = remote->SendRequest(static_cast<int>(IPowerMgr::SHELL_DUMP),
        data, reply, option);
    if (ret != ERR_OK) {
        POWER_HILOGE(COMP_FWK, "SendRequest is failed, ret: %{public}d", ret);
        return result;
    }
    result = reply.ReadString();

    return result;
}
} // namespace PowerMgr
} // namespace OHOS
