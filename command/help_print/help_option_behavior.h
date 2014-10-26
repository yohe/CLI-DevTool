
#ifndef CLI_DEV_COMMAND_HELP_PRINT_HELP_OPTION_BEHAVIOR_H
#define CLI_DEV_COMMAND_HELP_PRINT_HELP_OPTION_BEHAVIOR_H

#include <stdlib.h>
#include "command/help_print/behavior_base.h"
#include "command/builtin/shell_exe.h"

namespace clidevt {

class ShellHelpOptionBehavior : public HelpBehavior {
    std::string _commandName;
    std::string _helpOption;
public:
    ShellHelpOptionBehavior(std::string commandName, std::string helpOption) : _commandName(commandName), _helpOption(helpOption) {}
    virtual ~ShellHelpOptionBehavior() {}

    virtual std::string printHelp() const {
        std::string cmd = _commandName + " " + _helpOption;
        std::string dummy;
        ShellCommandExecutor executor(dummy);
        executor.execute(cmd);
        return "";
    }
};

}

#endif /* end of include guard */

