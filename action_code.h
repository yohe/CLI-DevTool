
#ifndef ACTION_CODE_H
#define ACTION_CODE_H

class ActionCode {
public:
    enum Code {
        // ascii 0-127
        KEY_BEL = 7,
        KEY_BS = 8,
        KEY_TAB = 9,
        KEY_NL = 10,
        KEY_CR = 13,
        KEY_DEL = 127,

        // non ascii 1000 - 
        KEY_UP_ARROW = 1000,
        KEY_RIGHT_ARROW,
        KEY_DOWN_ARROW,
        KEY_LEFT_ARROW,
        KEY_F1,
        KEY_F2,
        KEY_F3,
        KEY_F4,
        KEY_F5,
        KEY_F6,
        KEY_F7,
        KEY_F8,
        KEY_F9,

        // SHIFT ON
        KEY_SHIFT_TAB,

        // CTRL ON
        KEY_CTRL_A,
        KEY_CTRL_B,
        KEY_CTRL_C,
        KEY_CTRL_D,
        KEY_CTRL_E,
        KEY_CTRL_F,
        KEY_CTRL_G,
        KEY_CTRL_H,
        KEY_CTRL_I,
        KEY_CTRL_J,
        KEY_CTRL_K,
        KEY_CTRL_L,
        KEY_CTRL_M,
        KEY_CTRL_N,
        KEY_CTRL_O,
        KEY_CTRL_P,
        KEY_CTRL_Q,
        KEY_CTRL_R,
        KEY_CTRL_S,
        KEY_CTRL_T,
        KEY_CTRL_U,
        KEY_CTRL_V,
        KEY_CTRL_W,
        KEY_CTRL_X,
        KEY_CTRL_Y,
        KEY_CTRL_Z,
        KEY_CTRL_UP_ARROW,
        KEY_CTRL_RIGHT_ARROW,
        KEY_CTRL_DOWN_ARROW,
        KEY_CTRL_LEFT_ARROW,
        KEY_CTRL_SPACE,

        // Option or Alt ON
        KEY_ALT_A,
        KEY_ALT_B,
        KEY_ALT_C,
        KEY_ALT_D,
        KEY_ALT_E,
        KEY_ALT_F,
        KEY_ALT_G,
        KEY_ALT_H,
        KEY_ALT_I,
        KEY_ALT_J,
        KEY_ALT_K,
        KEY_ALT_L,
        KEY_ALT_M,
        KEY_ALT_N,
        KEY_ALT_O,
        KEY_ALT_P,
        KEY_ALT_Q,
        KEY_ALT_R,
        KEY_ALT_S,
        KEY_ALT_T,
        KEY_ALT_U,
        KEY_ALT_V,
        KEY_ALT_W,
        KEY_ALT_X,
        KEY_ALT_Y,
        KEY_ALT_Z,
        KEY_ALT_UP_ARROW,
        KEY_ALT_RIGHT_ARROW,
        KEY_ALT_DOWN_ARROW,
        KEY_ALT_LEFT_ARROW,
        KEY_ALT_TAB,
        KEY_ALT_SPACE,
        KEY_ALT_RETURN

    };

};

#endif /* end of include guard */
