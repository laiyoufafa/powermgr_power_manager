/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef POWER_SHELL_COMMAND_H
#define POWER_SHELL_COMMAND_H

#include "shell_command.h"

#include "power_mgr_client.h"

namespace OHOS {
namespace PowerMgr {
class PowerShellCommand : public OHOS::AAFwk::ShellCommand {
public:
    PowerShellCommand(int argc, char *argv[]);
    ~PowerShellCommand() override {};

private:
    static constexpr const char * const TEMP_DUMP_LOG_PATH = "/data/test/power_dump.log";
    static constexpr int DUMP_BUFF_SIZE = 100;
    ErrCode CreateCommandMap() override;
    ErrCode CreateMessageMap() override;
    ErrCode init() override;
    ErrCode RunAsHelpCommand();
    ErrCode RunAsDumpCommand();
    ErrCode RunAsSetModeCommand();
    ErrCode RunAsWakeupCommand();
    ErrCode RunAsSuspendCommand();
    ErrCode RunAsDisplayCommand();
};
}  // namespace PowerMgr
}  // namespace OHOS
#endif  // POWER_SHELL_COMMAND_H