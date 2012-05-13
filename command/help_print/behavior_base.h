
#ifndef CLI_DEV_COMMAND_HELP_PRINT_BEHAVIOR_BASE_H
#define CLI_DEV_COMMAND_HELP_PRINT_BEHAVIOR_BASE_H

#include <string>

class HelpBehavior {
public:
    HelpBehavior() {}
    virtual ~HelpBehavior() {}

    virtual std::string printHelp() const = 0;
};

#endif /* end of include guard */
