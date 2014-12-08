
#ifndef CLI_DEV_ACTION_CODE_H
#define CLI_DEV_ACTION_CODE_H

#include <map>

namespace clidevt {

class KeyCode{
public:
    enum Code {
        // CTRL ON
        KEY_CTRL_A = 1,
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

        // ascii 0-127
        KEY_BEL,
        KEY_BS,
        KEY_TAB,
        KEY_NL,
        KEY_CR,
        KEY_DEL,
        KEY_ESCAPE,

        KEY_UP_ARROW,
        KEY_RIGHT_ARROW,
        KEY_DOWN_ARROW,
        KEY_LEFT_ARROW,
        KEY_CTRL_UP_ARROW,
        KEY_CTRL_RIGHT_ARROW,
        KEY_CTRL_DOWN_ARROW,
        KEY_CTRL_LEFT_ARROW,
        KEY_CTRL_SPACE,
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
        KEY_ALT_RETURN,

        KEY_NONE
    };

    void initialize() {
        _code[KEY_CTRL_A] = "1";
        _code[KEY_CTRL_B] = "2";
        _code[KEY_CTRL_C] = "3";
        _code[KEY_CTRL_D] = "4";
        _code[KEY_CTRL_E] = "5";
        _code[KEY_CTRL_F] = "6";
        _code[KEY_CTRL_G] = "7";
        _code[KEY_CTRL_H] = "8";
        _code[KEY_CTRL_I] = "9";
        _code[KEY_CTRL_J] = "10";
        _code[KEY_CTRL_K] = "11";
        _code[KEY_CTRL_L] = "12";
        _code[KEY_CTRL_M] = "13";
        _code[KEY_CTRL_N] = "14";
        _code[KEY_CTRL_O] = "15";
        _code[KEY_CTRL_P] = "16";
        _code[KEY_CTRL_Q] = "17";
        _code[KEY_CTRL_R] = "18";
        _code[KEY_CTRL_S] = "19";
        _code[KEY_CTRL_T] = "20";
        _code[KEY_CTRL_U] = "21";
        _code[KEY_CTRL_V] = "22";
        _code[KEY_CTRL_W] = "23";
        _code[KEY_CTRL_X] = "24";
        _code[KEY_CTRL_Y] = "25";
        _code[KEY_CTRL_Z] = "26";
        _code[KEY_BEL]    = "7";
#ifdef USE_KEY_BS_AS_ASCII_DEL
        _code[KEY_BS]     = "8";
#else
        _code[KEY_BS]     = "127";
#endif
        _code[KEY_TAB]    = "9";
        _code[KEY_NL]     = "10";
        _code[KEY_CR]     = "13";
#ifdef USE_KEY_DEL_AS_ASCII_DEL
        _code[KEY_DEL]    = "127";
#else
        _code[KEY_DEL]    = "27 91 51 126";
#endif
        _code[KEY_ESCAPE] = "27";

        _code[KEY_UP_ARROW] = "27 91 65";
        _code[KEY_DOWN_ARROW] = "27 91 66";
        _code[KEY_RIGHT_ARROW] = "27 91 67";
        _code[KEY_LEFT_ARROW] = "27 91 68";
        _code[KEY_CTRL_UP_ARROW] = "";
        _code[KEY_CTRL_RIGHT_ARROW] = "27 91 53 68";
        _code[KEY_CTRL_DOWN_ARROW] = "";
        _code[KEY_CTRL_LEFT_ARROW] = "27 91 53 67";
        _code[KEY_CTRL_SPACE] = "0";
        _code[KEY_F1] = "27 79 80";
        _code[KEY_F2] = "27 79 81";
        _code[KEY_F3] = "27 79 82";
        _code[KEY_F4] = "27 79 83";
        _code[KEY_F5] = "27 91 49 53 126";
        _code[KEY_F6] = "27 91 49 55 126";
        _code[KEY_F7] = "27 91 49 56 126";
        _code[KEY_F8] = "27 91 49 57 126";
        _code[KEY_F9] = "27 91 49 58 126";
        _code[KEY_SHIFT_TAB] = "27 91 90";
        _code[KEY_ALT_A] = "27 97";
        _code[KEY_ALT_B] = "27 98";
        _code[KEY_ALT_C] = "27 99";
        _code[KEY_ALT_D] = "27 100";
        _code[KEY_ALT_E] = "27 101";
        _code[KEY_ALT_F] = "27 102";
        _code[KEY_ALT_G] = "27 103";
        _code[KEY_ALT_H] = "27 104";
        _code[KEY_ALT_I] = "27 105";
        _code[KEY_ALT_J] = "27 106";
        _code[KEY_ALT_K] = "27 107";
        _code[KEY_ALT_L] = "27 108";
        _code[KEY_ALT_M] = "27 109";
        _code[KEY_ALT_N] = "27 110";
        _code[KEY_ALT_O] = "27 111";
        _code[KEY_ALT_P] = "27 112";
        _code[KEY_ALT_Q] = "27 113";
        _code[KEY_ALT_R] = "27 114";
        _code[KEY_ALT_S] = "27 115";
        _code[KEY_ALT_T] = "27 116";
        _code[KEY_ALT_U] = "27 117";
        _code[KEY_ALT_V] = "27 118";
        _code[KEY_ALT_W] = "27 119";
        _code[KEY_ALT_X] = "27 120";
        _code[KEY_ALT_Y] = "27 121";
        _code[KEY_ALT_Z] = "27 122";
        _code[KEY_ALT_UP_ARROW]    = "27 27 91 65";
        _code[KEY_ALT_RIGHT_ARROW] = "27 27 91 66";
        _code[KEY_ALT_DOWN_ARROW]  = "27 27 91 67";
        _code[KEY_ALT_LEFT_ARROW]  = "27 27 91 68";
        _code[KEY_ALT_TAB]         = "27 9";
        _code[KEY_ALT_SPACE]       = "27 32";
        _code[KEY_ALT_RETURN]      = "27 13";
        _code[KEY_NONE]             = "";
    }

    std::string getCodeSeq(Code code) {
        if(_code.count(code) == 0) {
            return "";
        }
        return _code[code];
    }
private:
    std::map<Code, std::string> _code;
};

}

#endif /* end of include guard */
