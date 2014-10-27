
#ifndef CLI_DEV_MODE_BUILTIN_LOGGING_H
#define CLI_DEV_MODE_BUILTIN_LOGGING_H

#include "mode/mode.h"
#include <fstream>

namespace clidevt {

class LoggingMode : public Mode {
public:
    LoggingMode() : Mode("logger",
            PROMPT_DISPLAY_HOOK |
            EXECUTE_CMD_BEFORE |
            EXECUTE_CMD_AFTER
            )
    {
    }

    virtual void enter(Console* console, Mode* current, const std::string& param);
    virtual void leave(Console* console);
    virtual void hookPromptDisplay(const std::string& prompt, Console* console);
    virtual void hookExecuteCmdBefore(Command* cmd, Console* console); 
    virtual void hookExecuteCmdAfter(Command* cmd, Console* console);

private:
    int pipefd_[2];
    int stdoutBackup_, stderrBackup_;
    std::ofstream ofs_;
    std::string file_;
    pid_t child_;
};

}

#endif
