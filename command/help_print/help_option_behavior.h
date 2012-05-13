
#ifndef CLI_DEV_COMMAND_HELP_PRINT_HELP_OPTION_BEHAVIOR_H
#define CLI_DEV_COMMAND_HELP_PRINT_HELP_OPTION__BEHAVIOR_H

#include <stdlib.h>
#include "command/help_print/behavior_base.h"

class ShellHelpOptionBehavior : public HelpBehavior {
    std::string _commandName;
    std::string _helpOption;
public:
    ShellHelpOptionBehavior(std::string commandName, std::string helpOption) : _commandName(commandName), _helpOption(helpOption) {}
    virtual ~ShellHelpOptionBehavior() {}

    virtual std::string printHelp() const {
        std::string cmd = _commandName + " " + _helpOption;
        system(cmd.c_str());
        return "";
    }
};

#endif /* end of include guard */

