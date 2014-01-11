
#include "command/builtin/history.h"
#include "console.h"

namespace clidevt {

void BuiltInHistoryCommand::execute(std::string param) {
    std::string str = param;
    str = str.erase(0, str.find_first_not_of(" "));
    str = str.erase(str.find_last_not_of(" ")+1);
    if(str.empty()) {
        printHistory();
        printHelp();
        return;
    }

    std::list<std::string> delimiterList;
    delimiterList.push_back(" ");
    std::vector<std::string>* filterList = _console->divideStringToVector(param, delimiterList);
    std::string endStr = filterList->back();
    bool isExecute = true;;
    for(std::string::iterator ite = endStr.begin(); ite != endStr.end(); ++ite) {
        if(std::isxdigit(*ite) == false) {
            isExecute = false;
        }
    }

    if(isExecute) {
        std::stringstream ss(endStr);
        size_t num = 0;
        ss >> std::skipws >> num;
        std::string cmd = _console->getHistory(num);
        if(cmd.empty()) {
            delete filterList;
            std::cout << "history: Error." << std::endl;
            return;
        } else {
            std::string key = cmd.substr(0, cmd.find(" "));
            if(key == "history") {
                delete filterList;
                std::cout << "history: History command can not execute oneself." << std::endl;
                return;
            }
        }
        std::cout << "Execute : " << cmd << std::endl;
        std::cout << "-------------------";
        _console->execute(cmd);
        delete filterList;
    } else {
        BuiltInHistoryFilter filter(*filterList);
        _console->printAllHistory(&filter);
        delete filterList;
        printHelp();
    }
}

void BuiltInHistoryCommand::getParamCandidates(std::vector<std::string>& inputtedList,
                                               std::string inputting,
                                               std::vector<std::string>& candidates) const {
    if(inputting.size() != 0 || !inputtedList.empty()) {
        std::cout << std::endl;
        std::vector<std::string> filterList = inputtedList;
        filterList.push_back(inputting);
        BuiltInHistoryFilter filter(filterList);
        _console->printAllHistory(&filter);
        std::cout << std::endl;
        _console->redraw();
        return;
    }

    NonHistoryFilter nonFilter;
    _console->printAllHistory(&nonFilter);
}

void BuiltInHistoryCommand::printHistory() const {
    NonHistoryFilter nonFilter;
    _console->printAllHistory(&nonFilter);
}

}
