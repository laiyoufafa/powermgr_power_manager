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

#include "running_lock_action.h"

#include "system_suspend_controller.h"

using namespace std;

namespace OHOS {
namespace PowerMgr {
void RunningLockAction::Lock(const RunningLockParam& param)
{
    SystemSuspendController::GetInstance().AcquireRunningLock(param);
}

void RunningLockAction::Unlock(const RunningLockParam& param)
{
    SystemSuspendController::GetInstance().ReleaseRunningLock(param);
}
} // namespace PowerMgr
} // namespace OHOS
