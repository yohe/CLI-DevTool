#ifndef _COMMAND_H_
#define _COMMAND_H_

#include <vector>
#include <string>

#include "command/param_comple/param_comple.h"
#include "command/help_print/help_print.h"

class Console;

class Command {
public:
    Command(){}
    virtual ~Command() {}

    virtual std::string getKey() const = 0;
    virtual std::string printHelp() const = 0;
    virtual std::string execute(std::string param) const = 0;
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const = 0;
    virtual bool isHistoryAdd() { return true; }
    virtual void setConsole(Console* console) { _console = console; }
    
    virtual void afterCompletionHook(std::vector<std::string>& matchList) const {
    }
protected:
    Console* _console;

private:

};




#endif /* end of include guard */