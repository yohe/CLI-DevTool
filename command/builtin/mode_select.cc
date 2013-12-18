
#include "command/builtin/mode_select.h"
#include "console.h"
#include "mode/mode.h"

namespace clidevt {

void BuiltInModeSelectCommand::execute(std::string param) {
    const ModeMap& modeMap = _console->getModeMap();
    if(modeMap.count(param) == 1) {
        Mode* mode = modeMap.at(param);
        Mode* cur_mode = _console->getCurrentMode();
        if(mode != cur_mode) {
            cur_mode->leave(_console);
            mode->enter(_console, cur_mode);
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

