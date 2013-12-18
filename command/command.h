#ifndef CLI_DEV_COMMAND_H
#define CLI_DEV_COMMAND_H

#include <vector>
#include <string>

#include "command/param_comple/param_comple.h"
#include "command/help_print/help_print.h"

namespace clidevt {

class Console;

class Command {
public:
    Command(){}
    virtual ~Command() {}

    virtual std::string getKey() const = 0;
    virtual void printHelp() const = 0;
    virtual void execute(std::string param) = 0;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const = 0;
    virtual bool isHistoryAdd() const { return true; }
    virtual void setConsole(Console* console) { _console = console; }
    
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const {
    }
protected:
    Console* _console;

private:

};

class CommandAlias : public Command {
public:
    CommandAlias(std::string aliasName, std::string commandName, std::string option = "") : 
        Command(),
        _aliasName(aliasName),
        _commandName(commandName), _option(option) {}

    virtual std::string getKey() const { return _aliasName; }
    virtual void printHelp() const;
    virtual void execute(std::string param);
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const;
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const;
protected:
    std::string _aliasName;
    std::string _commandName;
    std::string _option;
};

}

#endif /* end of include guard */

