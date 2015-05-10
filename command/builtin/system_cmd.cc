
#include "command/builtin/shell_exe.h"
#include "command/builtin/system_cmd.h"
#include "command/help_print/help_print.h"
#include "console.h"

namespace clidevt {

SystemFuncCommand::~SystemFuncCommand() {
    delete _behavior; delete _helpBehavior;
}

void SystemFuncCommand::execute(std::string param) { 
    param = _console->expandWordExp(param);
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
    _behavior->afterCompletionHook(candidates);
}
void SystemFuncCommand::printHelp() const {
    _helpBehavior->printHelp(); 
}

}

