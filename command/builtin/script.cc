
#include "command/builtin/script.h"

#include "console.h"

namespace clidevt {

void BuiltInScriptExitCommand::execute(std::string param) {
    _console->loggingMode(false);

    _console->uninstallCommand("exit", false);
    if(_exit == NULL) {
        return;
    }
    _console->installCommand(_exit);
}

void BuiltInScriptCommand::execute(std::string param) {
    bool ret = false;
    std::string filename = param;
    filename = filename.erase(0, filename.find_first_not_of(" "));
    filename = filename.erase(filename.find_last_not_of(" ")+1);

    if(filename.empty()) {
        filename = "typescript";
    }

    ret = _console->loggingMode(true, filename);

    if(ret) {
        Command* cmd = _console->getCommand("exit");
        _scriptExitCmd->setExitCommand(cmd);
        _console->uninstallCommand("exit", false);
        _console->installCommand(_scriptExitCmd);
    } else {
        std::cout << "script " << filename << ": " <<  _console->getSystemError() << std::endl;
    }
    return;
}

}
