
#include "command/builtin/mode_select.h"
#include "console.h"
#include "mode/mode.h"

namespace clidevt {

void BuiltInModeSelectCommand::printHelp() const {
    std::cout << "The command of mode selecting.\nUsage mode <mode-name>" << std::endl;
}

void BuiltInModeSelectCommand::execute(std::string param) {
    size_t pos = param.find(" ");
    std::string modeName;
    if(pos == std::string::npos) {
        modeName = param;
        param = "";
    } else {
        modeName = param.substr(0,pos);
        param = param.erase(0,param.find_first_not_of(" ", pos));
    }
    const ModeMap& modeMap = _console->getModeMap();
    if(modeMap.count(modeName) == 1) {
        Mode* mode = modeMap.at(modeName);
        Mode* cur_mode = _console->getCurrentMode();
        if(mode != cur_mode) {
            cur_mode->leave(_console);
            mode->enter(_console, cur_mode, param);
            _console->setMode(mode);
        }
    } else {
        std::cout << "Unkown mode." << std::endl;
    }
}
void BuiltInModeSelectCommand::getParamCandidates(
        std::vector<std::string>& inputtedList,
        std::string inputting,
        std::vector<std::string>& candidates) const 
{
    const ModeMap& modeMap = _console->getModeMap();
    Mode* cur_mode = _console->getCurrentMode();
    for(ModeMap::const_iterator ite = modeMap.begin();
        ite != modeMap.end();
        ++ite) {
        if(cur_mode->getName() == ite->first) {
            continue;
        }
        candidates.push_back(ite->first);
    }
}

}

