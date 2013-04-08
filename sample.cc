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

    virtual void afterCompletionHook(std::vector<std::string>& matchList) const {
        _behavior.stripParentPath(matchList);
    }
};

class ExitCommand : public Command {
public:
    ExitCommand() {}
    virtual ~ExitCommand() {}

    virtual std::string getKey() const { return "exit"; }
    virtual std::string printHelp() const { return "console exit."; }
    virtual std::string execute(std::string parameter) const { _console->actionTerminate(); return ""; }
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
            matchList.push_back("add ");
            matchList.push_back("branch ");
            matchList.push_back("checkout ");
            matchList.push_back("clone ");
            matchList.push_back("commit ");
            matchList.push_back("diff ");
            matchList.push_back("status ");
            matchList.push_back("pull ");
            matchList.push_back("push ");
            matchList.push_back("rm ");

            return;
        }
        _fileBehavior.getParamList(inputtedList, inputting, matchList);
    }
    
    virtual void afterCompletionHook(std::vector<std::string>& matchList) const {
        _fileBehavior.stripParentPath(matchList);
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

    if(console.installCommand(new ShellCommandExecutor("exe")) == false) {
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

    console.run();

    return 0;
}

std::string Console::printPromptImpl() const {
    std::string dir = getCurrentDirectory();
    std::string prompt = "$ ";
    //std::string prompt = "[" + _user_name + ":" + dir.substr(dir.rfind("/")+1) + "]$ ";
    return prompt;
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
// 0x20 - 0x7F の間のAsciiコード( alphanumeric symbol )で始まるコード列は使用できません
#define KEY_STROKE_DEF(Num, Seq) \
    SEMICOLON_SPLIT_##Num Seq;

// strokeListは KEY_STROKE_DEF を使用
#define ADD_KEY_MAP(name, code, strokeList) \
    strokeList; \
    _keyMap.addKeyCodeSeq(name, stroke, code); \
    stroke.clear();

// Asciiコード列に対応するキーを設定する
void Console::keyMapInitialize() {

    std::vector<char> stroke;
    ADD_KEY_MAP("BS", KeyCode::KEY_BS, KEY_STROKE_DEF(1, (127)));
    ADD_KEY_MAP("DEL", KeyCode::KEY_DEL, KEY_STROKE_DEF(4, (27) (91) (51) (126)));

    ADD_KEY_MAP("CTRL-SPACE", KeyCode::KEY_CTRL_SPACE, KEY_STROKE_DEF(1, (0)));
    ADD_KEY_MAP("CTRL-A", KeyCode::KEY_CTRL_A, KEY_STROKE_DEF(1, (1)));
    ADD_KEY_MAP("CTRL-B", KeyCode::KEY_CTRL_B, KEY_STROKE_DEF(1, (2)));
    ADD_KEY_MAP("CTRL-C", KeyCode::KEY_CTRL_C, KEY_STROKE_DEF(1, (3)));
    ADD_KEY_MAP("CTRL-E", KeyCode::KEY_CTRL_E, KEY_STROKE_DEF(1, (5)));
    ADD_KEY_MAP("CTRL-F", KeyCode::KEY_CTRL_F, KEY_STROKE_DEF(1, (6)));
    ADD_KEY_MAP("CTRL-H", KeyCode::KEY_CTRL_H, KEY_STROKE_DEF(1, (8)));
    ADD_KEY_MAP("CTRL-J", KeyCode::KEY_CTRL_J, KEY_STROKE_DEF(1, (10) ));
    ADD_KEY_MAP("CTRL-K", KeyCode::KEY_CTRL_K, KEY_STROKE_DEF(1, (11) ));
    ADD_KEY_MAP("CTRL-N", KeyCode::KEY_CTRL_N, KEY_STROKE_DEF(1, (14) ));
    ADD_KEY_MAP("CTRL-P", KeyCode::KEY_CTRL_P, KEY_STROKE_DEF(1, (16) ));

    ADD_KEY_MAP("TAB", KeyCode::KEY_TAB, KEY_STROKE_DEF(1, (9)));
    ADD_KEY_MAP("RETURN", KeyCode::KEY_CR, KEY_STROKE_DEF(1, (13)));
    ADD_KEY_MAP("UP", KeyCode::KEY_UP_ARROW, KEY_STROKE_DEF(3, (27) (91) (65)));
    ADD_KEY_MAP("DOWN", KeyCode::KEY_DOWN_ARROW, KEY_STROKE_DEF(3, (27) (91) (66) ));
    ADD_KEY_MAP("RIGHT", KeyCode::KEY_RIGHT_ARROW, KEY_STROKE_DEF(3, (27) (91) (67) ));
    ADD_KEY_MAP("LEFT", KeyCode::KEY_LEFT_ARROW, KEY_STROKE_DEF(3, (27) (91) (68) ));

}

// キー入力発生時に実行する関数を登録
void Console::keyBindInitialize() {

    // Delete Character
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_BS, &Console::actionDeleteBackwardCharacter));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_DEL, &Console::actionDeleteForwardCharacter));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_K, &Console::actionClearFromCursorToEnd));

    // Complete
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_TAB, &Console::actionComplete));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_SPACE, &Console::actionComplete));

    // History select
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_UP_ARROW, &Console::actionBackwardHistory));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_P, &Console::actionBackwardHistory));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_DOWN_ARROW, &Console::actionForwardHistory));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_N, &Console::actionForwardHistory));

    // cursor move
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_RIGHT_ARROW, &Console::actionMoveCursorRight));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_F, &Console::actionMoveCursorRight));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_LEFT_ARROW, &Console::actionMoveCursorLeft));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_B, &Console::actionMoveCursorLeft));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_A, &Console::actionMoveCursorTop));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_E, &Console::actionMoveCursorBottom));


    // command execute
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CR, &Console::actionEnter));
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_J, &Console::actionEnter));

    // CTRL-C
    _keyBindMap.insert(std::pair<int, Action>(KeyCode::KEY_CTRL_C, &Console::actionTerminate));
}

