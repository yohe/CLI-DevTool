
#include "command/command.h"
#include "console.h"

namespace clidevt {

void CommandAlias::printHelp() const {
    Command* cmd = _console->getCommand(_commandName);
    if(cmd != NULL) {
        cmd->printHelp();
    } else {
        std::cout << _commandName << "is not found." << std::endl;
    }
}

void CommandAlias::execute(std::string param) {
    std::string cmdParam;
    if(_option.empty()) {
        cmdParam = param;
    } else {
        cmdParam = " " + _option + " " + param;
    }
    Command* cmd = _console->getCommand(_commandName);
    if(cmd != NULL) {
        cmd->execute(cmdParam);
    } else {
        std::cout << _commandName << "is not found." << std::endl;
    }
}

void CommandAlias::getParamCandidates(std::vector<std::string>& inputtedList,
                                      std::string inputting,
                                      std::vector<std::string>& candidates) const
{
    Command* cmd = _console->getCommand(_commandName);
    if(cmd != NULL) {
        cmd->getParamCandidates(inputtedList, inputting, candidates);
    } else {
        std::cout << _commandName << "is not found." << std::endl;
    }
}

void CommandAlias::afterCompletionHook(std::vector<std::string>& inputtedList) const 
{
    Command* cmd = _console->getCommand(_commandName);
    if(cmd != NULL) {
        cmd->afterCompletionHook(inputtedList);
    } else {
        std::cout << _commandName << "is not found." << std::endl;
    }
}

}
