
#ifndef CLI_DEV_COMMAND_BUILTIN_SYSTEM_FUNC_H
#define CLI_DEV_COMMAND_BUILTIN_SYSTEM_FUNC_H

#include <string>
#include <vector>

#include "command/command.h"

namespace clidevt {

class ParameterBehavior;
class HelpBehavior;

class SystemFuncCommand : public Command {
    std::string _command;
    std::string _option;
    ParameterBehavior* _behavior;
    HelpBehavior* _helpBehavior;
public:
    SystemFuncCommand(std::string commandName, std::string option, ParameterBehavior* behavior, HelpBehavior* helpBehavior) :
        _command(commandName), _option(option), _behavior(behavior), _helpBehavior(helpBehavior) {}

    virtual ~SystemFuncCommand() { delete _behavior; delete _helpBehavior; }

    virtual std::string getKey() const { return _command; }
    virtual void printHelp() const ;
    virtual void execute(std::string param) ;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList,
                                    std::string inputting,
                                    std::vector<std::string>& candidates) const ;
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const ;
};

}

#endif
