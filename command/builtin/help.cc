
#include "command/builtin/help.h"
#include "console.h"

namespace clidevt {

void BuiltInHelpCommand::execute(std::string param) {
    std::string str = param;
    str = str.erase(0, str.find_first_not_of(" "));
    str = str.erase(str.find_last_not_of(" ")+1);
    Command* cmd = _console->getCommand(str);
    if(cmd == NULL) {
        std::string ret = "help : Command[" + param + "] not found.";
        std::cout << ret << std::endl;
        return;
    }
    cmd->printHelp();
}

void BuiltInHelpCommand::getParamCandidates(std::vector<std::string>& inputtedList,
                                            std::string inputting,
                                            std::vector<std::string>& candidates) const {
    _console->getCommandNameList(candidates);
}

}
