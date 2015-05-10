
#include <fcntl.h>
#include <functional>

#include "mode/builtin/vi.h"
#include "key/key_map.h"
#include "key/key_seq.h"
#include "key/key_code.h"

#include "console.h"

namespace {
    const int ESC = 27;
    const int CTRL_R = 18;
    const int UNDO_BUFFER_SIZE = 100;
}

namespace clidevt {

void ViMode::initialize() {
    initializeTransitionAction();
    initializeMotionAction();
    initializeOperatorAction();
}

void ViMode::initializeTransitionAction() {
    _transitionKeyMap.insert(std::make_pair(ESC,TO_NORMAL));
    _transitionKeyMap.insert(std::make_pair('i', INSERT_CURRENT));
    _transitionKeyMap.insert(std::make_pair('I', INSERT_TOP));
    _transitionKeyMap.insert(std::make_pair('a', INSERT_NEXT));
    _transitionKeyMap.insert(std::make_pair('A', INSERT_BOTTOM));
    _transitionKeyMap.insert(std::make_pair('o', INSERT_NEW_LINE));
    _transitionKeyMap.insert(std::make_pair('O', INSERT_NEW_LINE_NO_EXE));


    _transitionFuncMap.insert(std::make_pair(INSERT_CURRENT, &ViMode::insertTransition_current));
    _transitionFuncMap.insert(std::make_pair(INSERT_TOP, &ViMode::insertTransition_top));
    _transitionFuncMap.insert(std::make_pair(INSERT_NEXT, &ViMode::insertTransition_next));
    _transitionFuncMap.insert(std::make_pair(INSERT_BOTTOM, &ViMode::insertTransition_bottom));
    _transitionFuncMap.insert(std::make_pair(INSERT_NEW_LINE, &ViMode::insertTransition_newLine));
    _transitionFuncMap.insert(std::make_pair(INSERT_NEW_LINE_NO_EXE, &ViMode::insertTransition_beforeLine));
    _transitionFuncMap.insert(std::make_pair(TO_NORMAL, &ViMode::normalTransition));
}
void ViMode::initializeMotionAction() {
}
void ViMode::initializeOperatorAction() {
    _oneKeyOperatorFuncMap.insert(std::make_pair('x', std::make_pair(&ViMode::executeDeleteOperator, RIGHT)));
    _oneKeyOperatorFuncMap.insert(std::make_pair('X', std::make_pair(&ViMode::executeDeleteOperator, LEFT)));
    _oneKeyOperatorFuncMap.insert(std::make_pair('s', std::make_pair(&ViMode::executeChangeOperator, RIGHT)));
    _oneKeyOperatorFuncMap.insert(std::make_pair('S', std::make_pair(&ViMode::executeChangeOperator, MOTION_ALIAS)));
    _oneKeyOperatorFuncMap.insert(std::make_pair('G', std::make_pair(&ViMode::executeSelectHistory, BOTTOM)));
    _oneKeyOperatorFuncMap.insert(std::make_pair('D', std::make_pair(&ViMode::executeDeleteOperator, BOTTOM)));
    _oneKeyOperatorFuncMap.insert(std::make_pair('u', std::make_pair(&ViMode::executeUndo, UP)));
    _oneKeyOperatorFuncMap.insert(std::make_pair(CTRL_R, std::make_pair(&ViMode::executeUndo, DOWN)));


    _changeOperatorFuncMap.insert(std::make_pair(TOP, &ViMode::changeOperator_top));
    _changeOperatorFuncMap.insert(std::make_pair(BOTTOM, &ViMode::changeOperator_bottom));
    _changeOperatorFuncMap.insert(std::make_pair(WORD, &ViMode::changeOperator_word));
    _changeOperatorFuncMap.insert(std::make_pair(BACK_WORD, &ViMode::changeOperator_backWord));
    _changeOperatorFuncMap.insert(std::make_pair(RIGHT, &ViMode::changeOperator_right));
    _changeOperatorFuncMap.insert(std::make_pair(LEFT, &ViMode::changeOperator_left));
    _changeOperatorFuncMap.insert(std::make_pair(UP, &ViMode::changeOperator_up));
    _changeOperatorFuncMap.insert(std::make_pair(DOWN, &ViMode::changeOperator_down));
}

std::pair<bool, Action*>
ViMode::hookInputKey(char input, Console* console) {
    if(modeTransition(input, console) == true) {
        return std::make_pair(false, (Action*)NULL);
    }

    if(state_ == INSERT) {
        return normal_.hookInputKey(input, console);
    } else {
        // NORMAL mode operation
        // support 
        //   * d[w|$|b|^|0|] : delete characters an specified range.
        //   * c[w|$|b|^|0|] : change characters an specified range.
        if(state_ == NORMAL) {
            Operator op = checkOperatorCommand(input);
            if(op != UNKNOWN_OP && op != ONE_KEY_CMD) {
                acceptOperatorCommand(op, console);
                return std::make_pair(false, (Action*)NULL);
            } else if(op == ONE_KEY_CMD) {
                executeOneKeyCommand(input, console);
                return std::make_pair(false, (Action*)NULL);
            }
            Motion motion = checkMotionCommand(input);
            if(motion != UNKNOWN_MOTION) {
                executeMotionCommand(motion, console);
                return std::make_pair(false, (Action*)NULL);
            }
            return std::make_pair(false, (Action*)NULL);
        } else if(state_ == MOTION_WAIT) {
            Motion motion = checkMotionCommand(input);
            if(motion != UNKNOWN_MOTION) {
                executeOperatorCommand(op_, motion, console);
                return std::make_pair(false, (Action*)NULL);
            }
        }
        return std::make_pair(false, (Action*)NULL);
    }
}

void ViMode::hookPromptDisplay(const std::string& prompt, Console* console) {
    if(state_ == NORMAL) {
        std::cout << "\x1b[32mN " << prompt << "\x1b[39m";
    } else if(state_ == INSERT) {
        std::cout << "\x1b[31mI " << prompt << "\x1b[39m";
    }
}

bool ViMode::modeTransition(char input, Console* console) {
    if(state_ == INSERT) {
        // INSERT mode -> NORMAL mode
        ModeTransition type = getModeTransitionType(input);
        if(type == TO_NORMAL) {
            addUndoBuffer(console->getInputtingString());
            return executeModeTransition(type, console);
        }
    } else if( state_ == NORMAL){
        ModeTransition type = getModeTransitionType(input);
        if(type != UNKNOWN_TRANSITION && type != TO_NORMAL) {
            addUndoBuffer(console->getInputtingString());
            return executeModeTransition(type, console);
        }
    }
    return false;
}

bool ViMode::checkReadableStdInput() const {

    int flags = fcntl(fileno(stdin), F_GETFL);
    int new_flags = flags | O_NONBLOCK;
    fcntl(fileno(stdin), F_SETFL, new_flags);

    int c = fgetc(stdin);
    fcntl(fileno(stdin), F_SETFL, flags);

    if(c == EOF) {
        return false;
    }

    ungetc(c, stdin);

    return true;
}


void ViMode::transitionNormalMode(Console* console) {
    state_ = NORMAL;
    console->redraw();
    console->actionMoveCursorLeft();
}

bool ViMode::executeModeTransition(ModeTransition method, Console* console) {
    TransitionFunc func = getTransitionFunc(method);
    if(func == NULL) {
        return false;
    }
    return (this->*func)(console);
}
bool ViMode::insertTransition_current(Console* console) {
    state_ = INSERT;
    console->redraw();
    return true;
}
bool ViMode::insertTransition_top(Console* console) {
    state_ = INSERT;
    console->redraw();
    console->actionMoveCursorTop();
    return true;
}
bool ViMode::insertTransition_next(Console* console) {
    state_ = INSERT;
    console->redraw();
    console->actionMoveCursorRight();
    return true;
}
bool ViMode::insertTransition_bottom(Console* console) {
    state_ = INSERT;
    console->redraw();
    console->actionMoveCursorBottom();
    return true;
}
bool ViMode::insertTransition_newLine(Console* console) {
    state_ = INSERT;
    console->actionExecuteCommandLine();
    return true;
}
bool ViMode::insertTransition_beforeLine(Console* console) {
    state_ = INSERT;
    console->actionClearLine();
    return true;
}
bool ViMode::normalTransition(Console* console) {
    if(checkReadableStdInput() == false) {
        transitionNormalMode(console);
        return true;
    }
    return false;
}

ViMode::Operator ViMode::checkOperatorCommand(char input) const {
    switch (input) {
    case 'd':
        return DELETE;
    case 'c':
        return CHANGE;
    case 'y':
        return YANK;
    default:
        if(isOneKeyCommand(input) == true) {
            return ONE_KEY_CMD;
        }
        return UNKNOWN_OP;
    }
}

bool ViMode::isOneKeyCommand(char input) const {
    if(_oneKeyOperatorFuncMap.count(input) == 1) {
        return true;
    }
    return false;
}
void ViMode::executeOneKeyCommand(char input, Console* console) {
    std::pair<OperatorFunc, Motion> funcInfo = getOneKeyOperatorFunc(input);
    if(funcInfo.first == NULL) {
        return;
    }
    return (this->*funcInfo.first)(funcInfo.second, console);
}

void ViMode::acceptOperatorCommand(Operator op, Console* console) {
    switch (op) {
    case DELETE: 
        state_ = MOTION_WAIT;
        op_ = DELETE;
        break;
    case CHANGE:
        state_ = MOTION_WAIT;
        op_ = CHANGE;
        break;
    case YANK:
        state_ = MOTION_WAIT;
        op_ = YANK;
        break;
    default:
        assert(false);
    }
}
void ViMode::executeOperatorCommand(Operator op, Motion motion, Console* console) {
    switch (op) {
    case DELETE: 
        executeDeleteOperator(motion, console);
        break;
    case CHANGE:
        executeChangeOperator(motion, console);
        break;
    case YANK:
        // not support
        //executeYankOperator(motion, console);
        break;
    default:
        assert(false);
    }

}

void ViMode::executeDeleteOperator(Motion motion, Console* console) {
    addUndoBuffer(console->getInputtingString());
    state_ = NORMAL;
    switch (motion) {
    case TOP: 
        console->actionDeleteFromHeadToCursor();
        break;
    case BOTTOM:
        console->actionDeleteFromCursorToEnd();
        break;
    case WORD:
        console->actionDeleteParam();
        break;
    case BACK_WORD:
        console->actionMoveCursorBackwardParam();
        console->actionDeleteParam();
        break;
    case RIGHT:
        console->actionDeleteForwardCharacter();
        break;
    case LEFT:
        console->actionDeleteBackwardCharacter();
        break;
    case UP:
        console->beep();
        break;
    case DOWN:
        console->beep();
        break;
    case MOTION_ALIAS:
        console->actionClearLine();
        break;
    default:
        assert(false);
    }
}

void ViMode::changeOperator_top(Console* console) {
    state_ = INSERT;
    console->actionDeleteFromHeadToCursor();
    console->redraw();
}
void ViMode::changeOperator_bottom(Console* console) {
    state_ = INSERT;
    console->actionDeleteFromCursorToEnd();
    console->redraw();
}
void ViMode::changeOperator_word(Console* console) {
    state_ = INSERT;
    console->actionDeleteParam();
    console->redraw();
}
void ViMode::changeOperator_backWord(Console* console) {
    state_ = INSERT;
    console->actionMoveCursorBackwardParam();
    console->actionDeleteParam();
    console->redraw();
}
void ViMode::changeOperator_right(Console* console) {
    console->actionDeleteForwardCharacter();
    state_ = INSERT;
    console->redraw();
}
void ViMode::changeOperator_left(Console* console) {
    state_ = INSERT;
    console->actionDeleteBackwardCharacter();
    console->redraw();
}
void ViMode::changeOperator_up(Console* console) {
    state_ = NORMAL;
    console->beep();
    console->redraw();
}
void ViMode::changeOperator_down(Console* console) {
    state_ = NORMAL;
    console->beep();
    console->redraw();
}
void ViMode::changeOperator_line(Console* console) {
    state_ = INSERT;
    console->actionClearLine();
    console->redraw();
}

void ViMode::executeChangeOperator(Motion motion, Console* console) {
    addUndoBuffer(console->getInputtingString());
    ChangeOperatorFunc func = getChangeOperatorFunc(motion);
    if(func == NULL) {
        return;
    }
    return (this->*func)(console);
}

void ViMode::executeSelectHistory(Motion motion, Console* console) {
    console->actionLastHistory();
}
void ViMode::executeUndo(Motion motion, Console* console) {
    if(motion == DOWN) {
        // Down : Redo
        if(_undoIndex >= _undoBuffer.size()) {
            console->beep();
            return;
        }
        std::string tmp = _undoBuffer.at(_undoIndex);
        _undoIndex++;

        console->actionClearLine();
        console->insertStringToTerminal(tmp);
    } else {
        // UP : Undo
        if(_undoIndex == 0) {
            console->beep();
            return;
        }
        _undoIndex--;
        std::string tmp = _undoBuffer.at(_undoIndex);

        console->actionClearLine();
        console->insertStringToTerminal(tmp);
    }
}

ViMode::Motion ViMode::checkMotionCommand(char input) const {
    switch (input) {
    case 'w':
        return WORD;
    case 'b':
        return BACK_WORD;
    case '$':
        return BOTTOM;
    case '^':
        return TOP;
    case '0':
        return TOP;
    case 'l':
        return RIGHT;
    case 'h':
        return LEFT;
    case 'j':
        return DOWN;
    case 'k':
        return UP;
    default:
        if(isMotionAlias(input) == true) {
            return MOTION_ALIAS;
        }
        return UNKNOWN_MOTION;
    }
}

bool ViMode::isMotionAlias(char input) const {
    switch (op_) {
    case DELETE:
        if(input == 'd') {
            return true;
        }
        break;
    case CHANGE:
        if(input == 'c') {
            return true;
        }
    case YANK:
        if(input == 'y') {
            return true;
        }
    default:
        return false;
    }
    return false;
}

void ViMode::executeMotionCommand(Motion motion, Console* console) const {
    switch (motion) {
    case TOP: 
        console->actionMoveCursorTop();
        break;
    case BOTTOM:
        console->actionMoveCursorBottom();
        break;
    case WORD:
        console->actionMoveCursorForwardParam();
        break;
    case BACK_WORD:
        console->actionMoveCursorBackwardParam();
        break;
    case RIGHT:
        console->actionMoveCursorRight();
        break;
    case LEFT:
        console->actionMoveCursorLeft();
        break;
    case UP:
        console->actionBackwardHistory();
        break;
    case DOWN:
        console->actionForwardHistory();
        break;
    default:
        assert(false);
    }

}

void ViMode::addUndoBuffer(const std::string& input) {
    _undoBuffer.push_back(input);
    if(_undoBuffer.size() > UNDO_BUFFER_SIZE) {
        _undoBuffer.pop_front();
    }
    _undoIndex = _undoBuffer.size();

#ifdef DEBUG
    for(int i=0; i < _undoBuffer.size(); i++) {
        std::cout << _undoBuffer.at(i) << std::endl;
    }
#endif
}

}

