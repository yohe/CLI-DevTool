#include "console.h"
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <fstream>

class SampleCommand : public Command {
public:
    SampleCommand() {}
    virtual ~SampleCommand() {}

    virtual std::string getKey() const { return "sample"; }
    virtual std::string printHelp() const { return "sample command."; }
    virtual std::string execute(std::string parameter) const { return parameter; }
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        if(std::find(inputtedList.begin(), inputtedList.end(), "hoge") == inputtedList.end()) {
            matchList.push_back("hoge");
        }
        if(std::find(inputtedList.begin(), inputtedList.end(), "huga") == inputtedList.end()) {
            matchList.push_back("huga");
        }
        if(std::find(inputtedList.begin(), inputtedList.end(), "hogehoge") == inputtedList.end()) {
            matchList.push_back("hogehoge");
        }
        if(std::find(inputtedList.begin(), inputtedList.end(), "hogehuga") == inputtedList.end()) {
            matchList.push_back("hogehuga");
        }
    }

private:
};

class ChangeDirCommand : public Command {
    FileListBehavior _behavior;
public:
    ChangeDirCommand(){}
    virtual ~ChangeDirCommand() {}

    virtual std::string getKey() const { return "cd"; }
    virtual std::string printHelp() const { system("man cd"); return ""; }
    virtual std::string execute(std::string parameter) const { 
        std::string str = parameter;
        str = str.erase(0, str.find_first_not_of(" "));
        str = str.erase(str.find_last_not_of(" ")+1);
        if(str.size() == 0) {
            str = getenv("HOME");
        }
        if(chdir(str.c_str()) == -1) {
            perror("cd");
        }
        system("pwd");
        return "";
    }

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _behavior.getParamList(inputtedList, inputting, matchList);
    }
};

class ExitCommand : public Command {
public:
    ExitCommand() {}
    virtual ~ExitCommand() {}

    virtual std::string getKey() const { return "exit"; }
    virtual std::string printHelp() const { return "console exit."; }
    virtual std::string execute(std::string parameter) const { _console->unInitialize(); exit(1); return ""; }
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
    }

private:
};

class GitCommand : public Command {
    ManBehavior _manBehavior;
    FileListBehavior _fileBehavior;
public:
    GitCommand() : _manBehavior("git") {}
    virtual ~GitCommand() {}

    virtual std::string getKey() const { return "git"; }
    virtual std::string printHelp() const { return _manBehavior.printHelp(); }
    virtual std::string execute(std::string parameter) const {
        std::string cmd = "git " + parameter;
        system(cmd.c_str());
        return "";
    }
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        if(inputtedList.empty()) {
            matchList.push_back("add");
            matchList.push_back("branch");
            matchList.push_back("checkout");
            matchList.push_back("clone");
            matchList.push_back("commit");
            matchList.push_back("diff");
            matchList.push_back("status");
            matchList.push_back("pull");
            matchList.push_back("push");
            matchList.push_back("rm");

            return;
        }
        _fileBehavior.getParamList(inputtedList, inputting, matchList);
    }

private:
};

int main(int argc, char const* argv[])
{
    int historySize = 100;
    bool ctrl_c = true; // false : ignore <CTRL-C> key.
    std::string historyFileName = ".cli_history";

    // Console console;             // use default : historySize = 20, ctrl_c = true, historyFileName = ".cli_history"
    // Console console(history); 
    // Console console(history, ctrl_c);
    Console console(historySize, ctrl_c, historyFileName.c_str());

    if(console.installCommand(new ExitCommand()) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }
    if(console.installCommand(new ChangeDirCommand()) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }
    if(console.installCommand(new SampleCommand()) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }
    if(console.installCommand(new GitCommand()) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }

    // SystemFuncCommand(std::string commandName, std::string commandOption, ParameterBehavior* p, HelpBehavior* h)
    // 
    // -G : Enable colorized for Mac OS X.
    // --color=auto : Enable colorized for Linux.
    if(console.installCommand(new SystemFuncCommand("ls", "-G", new FileListBehavior(), new ManBehavior("ls"))) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }

    console.initialize();
    console.run();

    return 0;
}

std::string Console::printPromptImpl() {
    return std::string("$ ");
}
void Console::printTitle() {
    std::cout << "+------------------------------------+\n";
    std::cout << "|              Console               |\n";
    std::cout << "+------------------------------------+\n";
}

#define SEMICOLON_SPLIT_1(x) stroke.push_back(x);
#define SEMICOLON_SPLIT_2(x) stroke.push_back(x); SEMICOLON_SPLIT_1
#define SEMICOLON_SPLIT_3(x) stroke.push_back(x); SEMICOLON_SPLIT_2
#define SEMICOLON_SPLIT_4(x) stroke.push_back(x); SEMICOLON_SPLIT_3

// KEY_STROKE_DEF は Num に Asciiコード列長, Seq に Asciiコード列 を記載
// Asciiコード列は、 (first) (second) (therd) のように各コードを () で括り記載する
// Asciiコード列の調査は、make 時に作成される key_trace を使用する
#define KEY_STROKE_DEF(Num, Seq) \
    SEMICOLON_SPLIT_##Num Seq;

// strokeListは KEY_STROKE_DEF を使用
#define ADD_KEY_MAP(name, code, strokeList) \
    strokeList; \
    _keyMap.addKeyStroke(name, stroke, code); \
    stroke.clear();

// Asciiコード列に対応するキーイベントを設定する
void Console::keyMapInitialize() {

    std::vector<char> stroke;
    ADD_KEY_MAP("BS", ActionCode::KEY_BS, KEY_STROKE_DEF(1, (127)));
    ADD_KEY_MAP("DEL", ActionCode::KEY_DEL, KEY_STROKE_DEF(4, (27) (91) (51) (126)));

    ADD_KEY_MAP("CTRL-SPACE", ActionCode::KEY_CTRL_SPACE, KEY_STROKE_DEF(1, (0)));
    ADD_KEY_MAP("CTRL-A", ActionCode::KEY_CTRL_A, KEY_STROKE_DEF(1, (1)));
    ADD_KEY_MAP("CTRL-C", ActionCode::KEY_CTRL_C, KEY_STROKE_DEF(1, (3)));
    ADD_KEY_MAP("CTRL-E", ActionCode::KEY_CTRL_E, KEY_STROKE_DEF(1, (5)));
    ADD_KEY_MAP("CTRL-H", ActionCode::KEY_CTRL_H, KEY_STROKE_DEF(1, (8)));
    ADD_KEY_MAP("CTRL-J", ActionCode::KEY_CTRL_J, KEY_STROKE_DEF(1, (10) ));

    ADD_KEY_MAP("TAB", ActionCode::KEY_TAB, KEY_STROKE_DEF(1, (9)));
    ADD_KEY_MAP("RETURN", ActionCode::KEY_CR, KEY_STROKE_DEF(1, (13)));
    ADD_KEY_MAP("UP", ActionCode::KEY_UP_ARROW, KEY_STROKE_DEF(3, (27) (91) (65)));
    ADD_KEY_MAP("DOWN", ActionCode::KEY_DOWN_ARROW, KEY_STROKE_DEF(3, (27) (91) (66) ));
    ADD_KEY_MAP("RIGHT", ActionCode::KEY_RIGHT_ARROW, KEY_STROKE_DEF(3, (27) (91) (67) ));
    ADD_KEY_MAP("LEFT", ActionCode::KEY_LEFT_ARROW, KEY_STROKE_DEF(3, (27) (91) (68) ));

}

// キーイベント発生時に実行する関数を登録
void Console::keyActionInitialize() {

    // BS, DEL
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_BS, &Console::actionDeleteBackwardCharacter));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_DEL, &Console::actionDeleteForwardCharacter));

    // Complement
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_TAB, &Console::actionComplement));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_CTRL_SPACE, &Console::actionComplement));

    // History select
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_UP_ARROW, &Console::actionBackwardHistory));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_DOWN_ARROW, &Console::actionForwarddHistory));

    // cursor move
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_RIGHT_ARROW, &Console::actionMoveCursorRight));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_LEFT_ARROW, &Console::actionMoveCursorLeft));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_CTRL_A, &Console::actionMoveCursorTop));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_CTRL_E, &Console::actionMoveCursorBottom));

    // command execute
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_CTRL_J, &Console::actionEnter));
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_CR, &Console::actionEnter));

    // CTRL-C
    _actionMap.insert(std::pair<int, Action>(ActionCode::KEY_CTRL_C, &Console::actionTerminate));
}

