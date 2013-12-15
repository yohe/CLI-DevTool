
#ifndef CLI_DEV_COMMAND_BUILTIN_HELP_H
#define CLI_DEV_COMMAND_BUILTIN_HELP_H

#include <string>
#include <vector>
#include <iostream>

#include "command/command.h"

class BuiltInHelpCommand : public Command {
public:
    BuiltInHelpCommand() {}
    virtual ~BuiltInHelpCommand() {}

    virtual std::string getKey() const { return "help"; }
    virtual void printHelp() const { std::cout << "Usage help [command]:\n   Display [command] help" << std::endl; }
    virtual void execute(std::string param);
    virtual void getParamCandidates(std::vector<std::string>& inputtedList,
                                    std::string inputting,
                                    std::vector<std::string>& candidates) const;
};

#endif

