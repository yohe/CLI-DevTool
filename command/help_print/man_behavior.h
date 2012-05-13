
#ifndef CLI_DEV_COMMAND_HELP_PRINT_MAN_BEHAVIOR_H
#define CLI_DEV_COMMAND_HELP_PRINT_MAN_BEHAVIOR_H

#include <stdlib.h>
#include "command/help_print/behavior_base.h"

class ManBehavior : public HelpBehavior {
    std::string _commandName;
public:
    ManBehavior(std::string commandName) : _commandName(commandName) {}
    virtual ~ManBehavior() {}

    virtual std::string printHelp() const {
        std::string cmd = "man " + _commandName;
        system(cmd.c_str());
        return "";
    }
};

#endif /* end of include guard */

