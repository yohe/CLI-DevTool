
#ifndef CLI_DEV_MODE_BUILTIN_VI_H
#define CLI_DEV_MODE_BUILTIN_VI_H


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

    enum InsertMode {
        INSERT_TOP = 0,
        INSERT_BOTTOM,
        INSERT_CURRENT,
        INSERT_NEXT,
        NEW_LINE,
        NEW_LINE_NO_EXE,
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

        OP_ALIAS,
        UNKNOWN_MOTION,
    };

    enum Operator {
        DELETE = 0,
        CHANGE,
        YANK,
        UNKNOWN_OP,
    };
    bool modeTransition(char input, Console* console);
    bool checkReadableStdInput() const;
    void transitionNormalMode(Console* console);
    void transitionInsertMode(InsertMode method, Console* console);

    Operator checkOperatorCommand(char input) const;
    bool isOperatorAlias(char input) const;
    void acceptOperatorCommand(Operator op, Console* console) ;
    void executeOperatorCommand(Operator op, Motion motion, Console* console);
    Motion checkMotionCommand(char input) const;
    void executeMotionCommand(Motion motion, Console* console) const;

    void executeDeleteOperator(Motion motion, Console* console);
    void executeChangeOperator(Motion motion, Console* console);
public:
    ViMode() : 
        Mode("vi", INPUT_KEY_HOOK | PROMPT_DISPLAY_HOOK),
        state_(0)
    {
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
};

}

#endif

