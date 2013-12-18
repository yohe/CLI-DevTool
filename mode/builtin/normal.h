
#ifndef CLI_DEV_MODE_BUILTIN_NORMAL_H
#define CLI_DEV_MODE_BUILTIN_NORMAL_H


#include "mode/mode.h"

namespace clidevt {

    class KeySequenceEntry;

class NormalMode : public Mode {
public:
    NormalMode() : 
        Mode("normal", INPUT_KEY_HOOK),
        _keyEntry(NULL),
        _isKeyStrokeBeginning(false)
    {
    }

    virtual void enter(Console* console, Mode* current) {}
    virtual void leave(Console* console) {}
    virtual std::pair<bool, Action*>
        hookInputKey(char input, Console* console);
    virtual void hookPromptDisplay(const std::string& prompt, Console* console) {}
    virtual void hookExecuteCmdBefore(Console* console) {}
    virtual void hookExecuteCmdAfter(Console* console) {}

private:
    const KeySequenceEntry* _keyEntry;
    bool _isKeyStrokeBeginning;
};

}

#endif
