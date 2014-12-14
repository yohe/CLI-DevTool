
#ifndef CLI_DEV_MODE_BUILTIN_VI_H
#define CLI_DEV_MODE_BUILTIN_VI_H

#include <map>
#include <deque>

#include "mode/mode.h"
#include "mode/builtin/normal.h"

namespace clidevt {

    class KeySequenceEntry;

class ViMode : public Mode {
    enum State {
        NORMAL = 0,
        INSERT,
        MOTION_WAIT,
    };

    enum ModeTransition {
        INSERT_TOP = 0,
        INSERT_BOTTOM,
        INSERT_CURRENT,
        INSERT_NEXT,
        INSERT_NEW_LINE,
        INSERT_NEW_LINE_NO_EXE,
        TO_NORMAL,
        UNKNOWN_TRANSITION,
    };
    enum Motion {
        TOP = 0,
        BOTTOM,
        WORD,
        BACK_WORD,
        RIGHT,
        LEFT,
        UP,
        DOWN,

        MOTION_ALIAS,
        UNKNOWN_MOTION,
    };

    enum Operator {
        DELETE = 0,
        CHANGE,
        YANK,

        ONE_KEY_CMD,
        UNKNOWN_OP,
    };
    bool modeTransition(char input, Console* console);
    ModeTransition getModeTransitionType(char input) {
        if(_transitionKeyMap.count(input) == 0) {
            return UNKNOWN_TRANSITION;
        }
        return _transitionKeyMap[input];
    }

    bool checkReadableStdInput() const;
    void transitionNormalMode(Console* console);
    bool executeModeTransition(ModeTransition method, Console* console);

    Operator checkOperatorCommand(char input) const;
    bool isOneKeyCommand(char input) const;
    void executeOneKeyCommand(char input, Console* console);
    void acceptOperatorCommand(Operator op, Console* console) ;
    void executeOperatorCommand(Operator op, Motion motion, Console* console);
    void executeDeleteOperator(Motion motion, Console* console);
    void executeChangeOperator(Motion motion, Console* console);

    void executeSelectHistory(Motion motion, Console* console);
    void executeUndo(Motion motion, Console* console);
    void addUndoBuffer(const std::string& input);

    bool isMotionAlias(char input) const;
    Motion checkMotionCommand(char input) const;
    void executeMotionCommand(Motion motion, Console* console) const;

    void initialize();
    void initializeTransitionAction();
    void initializeMotionAction();
    void initializeOperatorAction();

    typedef bool (ViMode::*TransitionFunc)(Console*);
    bool insertTransition_current(Console* console);
    bool insertTransition_top(Console* console);
    bool insertTransition_next(Console* console);
    bool insertTransition_bottom(Console* console);
    bool insertTransition_newLine(Console* console);
    bool insertTransition_beforeLine(Console* console);
    bool normalTransition(Console* console);

    TransitionFunc getTransitionFunc(ModeTransition trans) const {
        if(_transitionFuncMap.count(trans) == 0) {
            return NULL;
        }
        return _transitionFuncMap.at(trans);
    }

    typedef void (ViMode::*OperatorFunc)(Motion,Console*);

    std::pair<OperatorFunc, Motion> getOneKeyOperatorFunc(char input) const {
        if(_oneKeyOperatorFuncMap.count(input) == 0) {
            return std::make_pair((OperatorFunc)NULL, UNKNOWN_MOTION);
        }
        return _oneKeyOperatorFuncMap.at(input);
    }

    typedef void (ViMode::*ChangeOperatorFunc)(Console*);
    ChangeOperatorFunc getChangeOperatorFunc(Motion motion) const {
        if(_changeOperatorFuncMap.count(motion) == 0) {
            return NULL;
        }
        return _changeOperatorFuncMap.at(motion);
    }
    void changeOperator_top(Console* console);
    void changeOperator_bottom(Console* console);
    void changeOperator_word(Console* console);
    void changeOperator_backWord(Console* console);
    void changeOperator_right(Console* console);
    void changeOperator_left(Console* console);
    void changeOperator_up(Console* console);
    void changeOperator_down(Console* console);
    void changeOperator_line(Console* console);
public:
    ViMode() : 
        Mode("vi", INPUT_KEY_HOOK | PROMPT_DISPLAY_HOOK),
        state_(0),
        _undoBuffer(),
        _undoIndex(0)
    {
        initialize();
    }

    virtual std::pair<bool, Action*>
        hookInputKey(char input, Console* console);
    virtual void hookPromptDisplay(const std::string& prompt, Console* console);

private:
    //const KeySequenceEntry* _keyEntry;
    //bool _isKeyStrokeBeginning;
    int state_;
    Operator op_;
    NormalMode normal_;
    std::map<char, ModeTransition> _transitionKeyMap;
    std::map<ModeTransition, TransitionFunc> _transitionFuncMap;
    std::map<char, std::pair<OperatorFunc, Motion> > _oneKeyOperatorFuncMap;
    std::map<Motion, ChangeOperatorFunc> _changeOperatorFuncMap;
    std::deque<std::string> _undoBuffer;
    size_t _undoIndex;
};

}

#endif

