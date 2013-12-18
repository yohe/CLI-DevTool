
#ifndef CLI_DEV_COMMAND_BUILTIN_EDITOR_H
#define CLI_DEV_COMMAND_BUILTIN_EDITOR_H

#include <vector>
#include <string>

#include "command/command.h"
#include "command/param_comple/behavior_base.h"
#include "command/help_print/behavior_base.h"

namespace clidevt {

class EditorCommand : public Command {
    std::string _command;
    ParameterBehavior* _behavior;
    HelpBehavior* _helpBehavior;
public:
    EditorCommand(std::string commandName, ParameterBehavior* behavior, HelpBehavior* helpBehavior) :
        _command(commandName), _behavior(behavior), _helpBehavior(helpBehavior) {}

    virtual ~EditorCommand() { delete _behavior; delete _helpBehavior; }

    virtual std::string getKey() const { return _command; }
    virtual void printHelp() const { _helpBehavior->printHelp(); }
    virtual void execute(std::string param) ;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList,
                                    std::string inputting,
                                    std::vector<std::string>& candidates) const; 
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const ;
};

}

#endif

