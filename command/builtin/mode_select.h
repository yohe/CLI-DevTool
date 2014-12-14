
#ifndef CLI_DEV_COMMAND_BUILTIN_MODE_SELECT_H
#define CLI_DEV_COMMAND_BUILTIN_MODE_SELECT_H

#include <vector>
#include <string>
#include <iostream>
#include <map>

#include "command/command.h"

namespace clidevt {

class Mode;

class BuiltInModeSelectCommand: public Command {
    typedef std::map<std::string, Mode*> ModeMap;
public:
    BuiltInModeSelectCommand() {}
    virtual ~BuiltInModeSelectCommand() {}

    virtual std::string getKey() const { return "mode"; }
    virtual void printHelp() const;
    virtual void execute(std::string param); 

    virtual void getParamCandidates(std::vector<std::string>& inputtedList,
                                    std::string inputting,
                                    std::vector<std::string>& candidates) const ;

    virtual bool isHistoryAdd() const { return true; }
};

}


#endif
