
#ifndef CLI_DEV_COMMAND_BUILTIN_SCRIPT_H
#define CLI_DEV_COMMAND_BUILTIN_SCRIPT_H

#include <vector>
#include <string>
#include <iostream>

#include "command/command.h"
#include "command/param_comple/file_behavior.h"


class BuiltInScriptExitCommand : public Command {
    FileListBehavior _fileListBehavior;
    Command* _exit;
public:
    BuiltInScriptExitCommand() : _exit(NULL) {}
    virtual ~BuiltInScriptExitCommand() {}

    virtual std::string getKey() const { return "exit"; }
    virtual void printHelp() const { std::cout << "make typescript end." << std::endl; }
    virtual void execute(std::string param) ;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
    }

    void setExitCommand(Command* exit) { _exit = exit; }
};

class BuiltInScriptCommand : public Command {
    FileListBehavior _fileListBehavior;
    BuiltInScriptExitCommand* _scriptExitCmd;
public:
    BuiltInScriptCommand() : _scriptExitCmd(new BuiltInScriptExitCommand())  {}
    virtual ~BuiltInScriptCommand() {}

    virtual std::string getKey() const { return "script"; }
    virtual void printHelp() const { std::cout << "make typescript of terminal session.\nUsage script [filename]:" << std::endl; }
    virtual void execute(std::string param) ;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
    }
};


#endif


