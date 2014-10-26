
#include "command/builtin/shell_exe.h"
#include "command/builtin/system_func.h"
#include "command/help_print/help_print.h"

namespace clidevt {

SystemFuncCommand::~SystemFuncCommand() {
    delete _behavior; delete _helpBehavior;
}

void SystemFuncCommand::execute(std::string param) { 
    std::string cmd = _command + " " + _option + " " + param;
    std::string dummy;
    ShellCommandExecutor executor(dummy);
    executor.execute(cmd);
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

