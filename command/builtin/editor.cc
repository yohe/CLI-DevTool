
#include <iostream>

#include "command/builtin/editor.h"
#include "console.h"

namespace clidevt {

void EditorCommand::execute(std::string param) { 
    std::string cmd = _command + " " + param;
    if(_console->isLogging()) {
        std::cout << _command + " : not supporting during \"script\" execution." << std::endl;
        return;
    }
    system(cmd.c_str()); 
}
void EditorCommand::getParamCandidates(std::vector<std::string>& inputtedList,
                                       std::string inputting,
                                       std::vector<std::string>& candidates) const {

    _behavior->getParamCandidates(inputtedList, inputting, candidates);
}
void EditorCommand::afterCompletionHook(std::vector<std::string>& candidates) const {
    FileListBehavior fileListBehavior;
    fileListBehavior.stripParentPath(candidates);
}

}
