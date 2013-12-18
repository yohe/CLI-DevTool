
#ifndef CLI_DEV_COMMAND_BUILTIN_SHELL_EXE_H
#define CLI_DEV_COMMAND_BUILTIN_SHELL_EXE_H

#include <iostream>
#include <string>
#include <vector>
#include <set>

#include "command/command.h"
#include "command/param_comple/file_behavior.h"

namespace clidevt {

class ShellCommandExecutor : public Command {
    std::string _command;
    // 遅延評価させるための措置
    mutable bool _initFlag;
    mutable std::set<std::string> _commandList;
    FileListBehavior _fileListBehavior;
    bool _historyAdd;
public:
    ShellCommandExecutor(const std::string& commandName ) : _command(commandName), _initFlag(false) {
    }

    virtual ~ShellCommandExecutor() {
    }

    virtual std::string getKey() const { return _command; }
    virtual void printHelp() const {
        std::cout << "Usage : exe [command]" << std::endl;
    }
    virtual void execute(std::string param) ;
    virtual void getParamCandidates(std::vector<std::string>& inputtedList,
                                    std::string inputting,
                                    std::vector<std::string>& candidates) const ;
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const ;

    virtual bool isHistoryAdd() const {
        return _historyAdd;
    }
};

}

#endif
