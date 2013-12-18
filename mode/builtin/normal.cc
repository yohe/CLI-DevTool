
#include "mode/builtin/normal.h"
#include "key/key_map.h"
#include "key/key_seq.h"

#include "console.h"

namespace clidevt {

std::pair<bool, Action*>
NormalMode::hookInputKey(char input, Console* console) {
    const KeyMap& keyMap = console->getKeyMap();

    if( !_isKeyStrokeBeginning ) {
        // space and symble or alphanumeric is output without do something.
        if( input >= 0x20 && input <= 0x7E) {
            return std::make_pair(true, (Action*)NULL);
        }
    }
    // Defined action execute if non alphanumeric charactor.
    if( _keyEntry == NULL ) {
        _keyEntry = keyMap.getKeyEntry(input);
    } else {
        _keyEntry = _keyEntry->getKeySequenceEntry(input);
    }

    if(_keyEntry == NULL) {
        beep();
        _isKeyStrokeBeginning = false;
        return std::make_pair(false, (Action*)NULL);
    }

    if(_keyEntry->isEntry()) {
        if(setupterm(NULL, fileno(stdout), (int*)0) == ERR) {
            return std::make_pair(false, (Action*)NULL);
        }
        KeyCode::Code actionCode = _keyEntry->getVirtualKeyCode();
        Action* action = console->getAction(actionCode);
        _keyEntry = NULL;
        _isKeyStrokeBeginning = false;
        return std::make_pair(false, action);
    } else {
        _isKeyStrokeBeginning = true;
        return std::make_pair(false, (Action*)NULL);
    }
}

}
