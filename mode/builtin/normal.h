
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

    virtual std::pair<bool, Action*>
        hookInputKey(char input, Console* console);

private:
    const KeySequenceEntry* _keyEntry;
    bool _isKeyStrokeBeginning;
};

}

#endif
