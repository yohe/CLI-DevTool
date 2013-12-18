
#include "command/builtin/system_func.h"

namespace clidevt {

void SystemFuncCommand::execute(std::string param) { 
    std::string cmd = _command + " " + _option + " " + param;
    system(cmd.c_str()); 
    return;
}
void SystemFuncCommand::getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
    _behavior->getParamCandidates(inputtedList, inputting, candidates);
}
void SystemFuncCommand::afterCompletionHook(std::vector<std::string>& candidates) const {
    FileListBehavior fileListBehavior;
    fileListBehavior.stripParentPath(candidates);
}
void SystemFuncCommand::printHelp() const {
    _helpBehavior->printHelp(); 
}

}

