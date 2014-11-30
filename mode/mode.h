
#ifndef CLI_DEV_MODE_H
#define CLI_DEV_MODE_H

#include <utility>
#include <string>

namespace clidevt {

    enum ModeHookBit {
        INPUT_KEY_HOOK = (1 << 0),
        PROMPT_DISPLAY_HOOK = (1 << 1),
        EXECUTE_CMD_LINE_BEFORE = (1 << 2),
        EXECUTE_CMD_LINE_AFTER = (1 << 3),
        EXECUTE_CMD_BEFORE = (1 << 4),
        EXECUTE_CMD_AFTER = (1 << 5),
        PREPARE_INSERT_STR = (1 << 6),
        HOOK_ALL = ((1 << 7) -1)
    };

class Console;
class Action;
class Command;

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

    virtual void enter(Console* console, Mode* current, const std::string& param) {}
    virtual void leave(Console* console) {};

    /**
     * @brief hookinputkey
     *
     * @param[in] input     This is a character that is inputted by keyboard.
     * @param[in] console   Console class.
     *
     * If boolean is true, the input character insert to console.
     * If boolean is false and Action is NULL, System execute nothing.
     * If boolean is false and Action is specified, System execute a Action.
     */
    virtual std::pair<bool, Action*> hookInputKey(char input, Console* console) {
        return std::make_pair(true, (Action*)NULL);
    } 

    virtual void hookPromptDisplay(const std::string& prompt, Console* console) {};
    virtual void hookExecuteCommandLineBefore(const std::string& input, Console* console) {}
    virtual void hookExecuteCommandLineAfter(Console* console) {}
    virtual void hookExecuteCmdBefore(Command* cmd, Console* console) {};
    virtual void hookExecuteCmdAfter(Command* cmd, Console* console) {};
    virtual void hookPrepareInsert(Console* console) {};

private:
    std::string _name;
    int _flags;
};

}

#endif
