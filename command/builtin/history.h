
#ifndef CLI_DEV_COMMAND_BUILTIN_HISTORY_H
#define CLI_DEV_COMMAND_BUILTIN_HISTORY_H

#include <vector>
#include <string>
#include <iostream>

#include "command/command.h"

namespace clidevt {

class HistoryFilter {
public:
    virtual ~HistoryFilter() {}
    virtual bool isSkip(const std::string& value) = 0;
};
class NonHistoryFilter : public HistoryFilter {
public:
    virtual bool isSkip(const std::string& value) {
        return false;
    }
};


class BuiltInHistoryCommand : public Command {
    class BuiltInHistoryFilter : public HistoryFilter {
        const std::vector<std::string>& _filterList;
    public:
        BuiltInHistoryFilter(const std::vector<std::string>& filterList) : _filterList(filterList) {
        };

        virtual bool isSkip(const std::string& value) {
            std::vector<std::string>::const_iterator ite = _filterList.begin();
            std::vector<std::string>::const_iterator end = _filterList.end();
            for(; ite != end; ++ite) {
                if(value.find(*ite) == std::string::npos) {
                    return true;
                }
            }
            return false;
        }
    };
public:
    BuiltInHistoryCommand() {}
    virtual ~BuiltInHistoryCommand() {}

    virtual std::string getKey() const { return "history"; }
    virtual void printHelp() const { std::cout << "Usage history [filter string] ... [History Number]" << std::endl; }
    virtual void execute(std::string param); 

    virtual void printHistory() const;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList,
                                    std::string inputting,
                                    std::vector<std::string>& candidates) const ;

    virtual bool isHistoryAdd() const { return false; }
};

}

#endif

