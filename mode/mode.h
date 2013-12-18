
#ifndef CLI_DEV_MODE_H
#define CLI_DEV_MODE_H

#include <utility>
#include <string>

namespace clidevt {

    enum ModeHookBit {
        INPUT_KEY_HOOK = 1,
        PROMPT_DISPLAY_HOOK = 2,
        EXECUTE_CMD_BEFORE = 4,
        EXECUTE_CMD_AFTER = 8,
        HOOK_ALL = 15,
    };

    class Console;
    class Action;

class Mode {
public:

    Mode(std::string name, int flags) : 
        _name(name),
        _flags(flags)
    {
    }

    std::string getName() {
        return _name;
    }
    virtual int getFlags() const {
        return _flags;
    }

    virtual void enter(Console* console, Mode* current) = 0;
    virtual void leave(Console* console) = 0;
    virtual std::pair<bool, Action*> hookInputKey(char input, Console* console) = 0;
    virtual void hookPromptDisplay(const std::string& prompt, Console* console) = 0;
    virtual void hookExecuteCmdBefore(Console* console) = 0;
    virtual void hookExecuteCmdAfter(Console* console) = 0;

private:
    std::string _name;
    int _flags;
};

}

#endif
