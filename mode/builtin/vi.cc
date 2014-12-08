
#include <fcntl.h>

#include "mode/builtin/vi.h"
#include "key/key_map.h"
#include "key/key_seq.h"
#include "key/key_code.h"

#include "console.h"

namespace clidevt {

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
        //   * o : execute inputted string, and start insert mode at new line.
        //   * O : Not execute inputted string, and start insert mode at new line.
        if(state_ == NORMAL) {
            Operator op = checkOperatorCommand(input);
            if(op != UNKNOWN_OP) {
                acceptOperatorCommand(op, console);
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
        std::cout << "\x1b[31m" << prompt << "\x1b[39m";
    } else if(state_ == INSERT) {
        std::cout << "\x1b[32m" << prompt << "\x1b[39m";
    }
}

bool ViMode::modeTransition(char input, Console* console) {
    if(state_ == INSERT) {
        // INSERT mode -> NORMAL mode
        if(input == 27) {
            if(checkReadableStdInput() == false) {
                transitionNormalMode(console);
                return true;
            }
        }
    } else {
        // NORMAL mode -> INSERT mode
        if(input == 'i') {
            transitionInsertMode(INSERT_CURRENT, console);
            return true;
        } else if(input == 'I') {
            transitionInsertMode(INSERT_TOP, console);
            return true;
        } else if(input == 'a') {
            transitionInsertMode(INSERT_NEXT, console);
            return true;
        } else if(input == 'A') {
            transitionInsertMode(INSERT_BOTTOM, console);
            return true;
        } else if(input == 'o') {
            transitionInsertMode(NEW_LINE, console);
            return true;
        } else if(input == 'O') {
            transitionInsertMode(NEW_LINE_NO_EXE, console);
            return true;
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

void ViMode::transitionInsertMode(InsertMode method, Console* console) {
    switch (method) {
    case INSERT_CURRENT:
        state_ = INSERT;
        console->redraw();
        break;
    case INSERT_TOP:
        state_ = INSERT;
        console->redraw();
        console->actionMoveCursorTop();
        break;
    case INSERT_NEXT:
        state_ = INSERT;
        console->redraw();
        console->actionMoveCursorRight();
        break;
    case INSERT_BOTTOM:
        state_ = INSERT;
        console->redraw();
        console->actionMoveCursorBottom();
        break;
    case NEW_LINE:
        state_ = INSERT;
        console->actionExecuteCommandLine();
        break;
    case NEW_LINE_NO_EXE:
        console->actionClearLine();
        state_ = INSERT;
        break;
    }
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
        return UNKNOWN_OP;
    }
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
    default:
        assert(false);
    }
}
void ViMode::executeChangeOperator(Motion motion, Console* console) {
    switch (motion) {
    case TOP: 
        console->actionDeleteFromHeadToCursor();
        state_ = INSERT;
        break;
    case BOTTOM:
        console->actionDeleteFromCursorToEnd();
        state_ = INSERT;
        break;
    case WORD:
        console->actionDeleteParam();
        state_ = INSERT;
        break;
    case BACK_WORD:
        console->actionMoveCursorBackwardParam();
        console->actionDeleteParam();
        state_ = INSERT;
        break;
    case RIGHT:
        console->actionDeleteForwardCharacter();
        state_ = INSERT;
        break;
    case LEFT:
        console->actionDeleteBackwardCharacter();
        state_ = INSERT;
        break;
    case UP:
        state_ = NORMAL;
        console->beep();
        break;
    case DOWN:
        state_ = NORMAL;
        console->beep();
        break;
    default:
        assert(false);
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
        if(isOperatorAlias(input) == true) {
            return OP_ALIAS;
        }
        return UNKNOWN_MOTION;
    }
}

bool ViMode::isOperatorAlias(char input) const {
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

}

