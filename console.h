#ifndef CLI_DEV_CONSOLE_H
#define CLI_DEV_CONSOLE_H

#include <iostream>
#include <iomanip>
#include <map>
#include <string>
#include <vector>
#include <list>
#include <queue>
#include <sstream>
#include <fstream>
#include <cstdio>
#include <set>
#include <algorithm>

#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pwd.h>
#include <assert.h>

#include <termios.h>
#include <term.h>
#include <ncurses.h>
#include <unistd.h>
#include <dirent.h>

#include "key/key_code.h"
#include "key/key_map.h"
#include "command/command.h"
#include "command/command_selector.h"
#include "command/builtin/builtin.h"

namespace {
    class NotSpaceCampare {
    public:
        bool operator()(char c) {
            return c != ' ';
        }
    };
    class SpaceCampare {
    public:
        bool operator()(char c) {
            return c == ' ';
        }
    };
}

namespace clidevt {

// 文字列分割
std::vector<std::string>* divideStringToVector(std::string& src, std::list<std::string>& delimiter); 

class Mode;
class Statement;

class Action {
public:
    Action() {}
    virtual ~Action() {};
    virtual void operator()() = 0;
};

class Console;
class UserAction : public Action {
public:
    UserAction() {}
    virtual ~UserAction() {}

    void set(Console* console) { console_ = console; }
    Console* get() const { return console_; }
private:
    Console* console_;
};


class BuiltInScriptCommand;
class BuiltInScriptExitCommand;

class Console {
    typedef void (Console::*ConsoleMethod)();
    typedef CommandSelector::CommandSet CommandSet;
    typedef std::map<KeyCode::Code, Action*> KeyBindMap;
    typedef std::map<std::string, Mode*> ModeMap;
    
    friend class BuiltInScriptCommand;
    friend class BuiltInScriptExitCommand;

    class SystemAction : public Action {
    public:
        SystemAction(Console* console, ConsoleMethod method) : console_(console), method_(method) {}
        virtual ~SystemAction() {}

        virtual void operator()() { (console_->*method_)(); }
    private:
        Console* console_;
        ConsoleMethod method_;
    };

public:

    enum CompletionType {
        FULL_COMPLETE = 1,
        PARTIAL_COMPLETE,
        NO_CHANGE,
        NO_CANDIDATE, 
        ERROR
    };

    Console(int argc, char const* argv[], size_t histroySize = 20, std::string filename = ".cli_history") :
        _commandSelector(NULL),
        _historyMax(histroySize),
        _historyFile(filename),
        _isTerminatePermit(true)
    {
        _commandSelector = new DefaultCommandSelector();
        //_commandSelector = new AbbreviatedCommandSelector();
        initialize(argc, argv);
    }
    virtual ~Console() {
        unInitialize();
    }
    
    bool registerKeyBinding(KeyCode::Code code, ConsoleMethod method) {
        if(_keyBindMap.count(code) == 1) {
            return false;
        }
        SystemAction* action = new SystemAction(this, method);
        _keyBindMap.insert(std::pair<KeyCode::Code, Action*>(code, action));
        return true;
    }
    bool registerKeyBinding(KeyCode::Code code, UserAction* action) {
        if(_keyBindMap.count(code) == 1) {
            return false;
        }
        action->set(this);
        _keyBindMap.insert(std::pair<KeyCode::Code, Action*>(code, action));
        return true;
    }
    bool unregisterKeyBinding(KeyCode::Code code) {
        if(_keyBindMap.count(code) == 0) {
            return false;
        }
        _keyBindMap.erase(code);
        return true;
    }

    Mode* getCurrentMode() const {
        return _currentMode;
    }

    Mode* setMode(Mode* new_mode) {
        Mode* cur = _currentMode;
        _currentMode = new_mode;
        return cur;
    }
    const ModeMap& getModeMap() const {
        return _modeMap;
    }
    bool registMode(Mode* mode) ;
    Mode* unregistMode(std::string name) ;

    const KeyMap& getKeyMap() const {
        return _keyMap;
    }

    // アクション
    Action* getAction(KeyCode::Code keyCode) const {
        KeyBindMap::const_iterator ite = _keyBindMap.find(keyCode);
        if(ite == _keyBindMap.end()) {
            return NULL;
        }

        return ite->second;
    }


    // 定義済みアクション
    void actionBackwardHistory() { selectHistory(true); }
    void actionForwardHistory(){ selectHistory(false); }
    void actionMoveCursorRight() { moveCursor(false); }
    void actionMoveCursorLeft() { moveCursor(true); }
    void actionMoveCursorForwardParam();
    void actionMoveCursorBackwardParam();
    void actionDeleteForwardCharacter();
    void actionDeleteBackwardCharacter();
    void actionDeleteParam();
    void actionExecuteCommandLine();
    void actionComplete();
    void actionMoveCursorTop() { setCursorPos(0); return;}
    void actionTerminate();
    void actionMoveCursorBottom() { setCursorPos(_inputString.size()); return;}
    void actionClearLine() { clearLine(); return; }
    void actionDeleteFromCursorToEnd();
    void actionDeleteFromHeadToCursor();
    void actionClearScreen();

    void redraw() {
        clearLine(false);
        std::cout << _inputString;
    }
    void insertStringToTerminal(const std::string& str) ;
    void printStringOnTerminal(const std::string& str) ;
    std::string getInputtingString() {
        return _inputString;
    }

    std::string getPromptString() const {
        return printPromptImpl();
    }
    std::string getDateString() const;

private:
    // 初期化
    bool initialize(int argc, char const* argv[]);
    bool setTermIOS(struct termios& setting) {
        if(tcsetattr(fileno(stdin), TCSANOW, &setting) == -1) {
            return false;
        }
        return true;
    }
    void unInitialize() {
        delete _commandSelector;
        _history.clear();
        _stringPos = 0;
        _inputString.clear();
        tcsetattr(fileno(stdin), TCSANOW, &_save_term);
    }
    void keyMapInitialize();
    void keyBindInitialize();

    // 補完機能
    void executeComplete(std::string input);
    CompletionType completeCommandName(const std::string& input);
    void getInputParameter(std::string& inputString, std::vector<std::string>* tokenList,
                           std::string& lastParam, std::vector<std::string>& paramList);
    bool completeCommand(std::string& key, std::vector<std::string>& candidates);
    template <class Iterator>
    bool completeStringList(std::string& key, std::vector<std::string>& candidates, Iterator begin, Iterator end);

    typedef std::vector<Statement>::iterator StatementIterator;
    // コマンド機能
    void execute(StatementIterator begin, StatementIterator end);
    void executeStatement(const Statement& statement);
    void executeShellCommand(const std::string& string);
    void executeCommand(Command* cmd, const std::string& argument);
    Command* getCommandFromInputString(std::string& inputString);

    // History
    void loadHistory();
    bool selectHistory(bool up);
    void addHistory(std::string str, bool save = true);

     // 画面フォーマット出力
    void printPrompt();
    std::string printPromptImpl() const;

    // ターミナル操作機能
    bool moveCursor(bool left);
    void clearLine(bool clearString = true) {

        std::string tmp = _inputString;
        size_t pos = _stringPos;

        size_t length = _inputString.length();
        actionMoveCursorBottom();
        while(length > 0) {
            actionDeleteBackwardCharacter();
            length--;
        }

        printPrompt();

        if(clearString) {
            clearInputString();
        } else {
            _inputString = tmp;
            _stringPos = pos;
        }
    }
    void clearInputString() {
        _stringPos = 0;
        _inputString.clear();
    }
    void setCursorPos(int pos) {
        int sub = _stringPos - pos;
        //sub -= printPromptImpl().length();
        if(sub > 0) {
            putp(tparm(parm_left_cursor, sub));
        } else if(sub < 0) {
            sub *= -1;
            putp(tparm(parm_right_cursor, sub));
        }
        _stringPos = pos;
    }
    void clearStatus() {
        _stringPos = 0;
        _inputString.clear();
        _historyIndex = 0;
    }
    bool isEnd() {
        return _consoleExit;
    }

    template <class Iterator>
    void printStringList(Iterator begin, Iterator end);

public:
    // エラー
    std::string getSystemError() {
        return strerror(_systemErrorNumber);
    }

    // ヒストリ機能
    void printAllHistory(HistoryFilter* fileter);
    std::string getHistory(size_t index); 

    void printTitle(); 
    void printAllCommandName();

    void beep() { printf("\a"); }
    std::string getUserName() const { return getpwuid(geteuid())->pw_name; }
    std::string getHostName() const {
        char buf[128];
        gethostname(buf, 128);
        return buf;
    }
    std::string getHomeDirectory() const {
        return _user_homeDir;
    }
    std::string getCurrentDirectory() const {
        size_t size = pathconf(".", _PC_PATH_MAX);
        char* buf = new char[size];

        if(getcwd(buf, size) == NULL) {
            return "ERANGE";
        }
        std::string ret(buf);
        delete[] buf;
        return ret;
    }

    // コマンド登録, 登録解除, 取得
    void execute(const std::string& inputStr);
    bool installCommand(Command* cmd) {
        if(!_commandSelector->registCommand(cmd)) {
            return false;
        }

        cmd->setConsole(this);
        return true;
    }
    bool uninstallCommand(std::string key, bool isDelete = true) {
        Command* cmd = _commandSelector->unregistCommand(key);
        if(cmd == NULL) {
            return false;
        }

        if(isDelete) {
            delete cmd;
        }
        return true;
    }
    Command* getCommand(std::string key) const {
        return _commandSelector->getCommand(key);
    }
    void getCommandNameList(std::vector<std::string>& nameList) {
        for(CommandSet::const_iterator ite = _commandSelector->getCommandSet().begin();
            ite != _commandSelector->getCommandSet().end();
            ++ite) {
            nameList.push_back(ite->first);
        }
    }

    // 処理ループ
    void run();

    int getTerminalColumnSize() const ;
    int getTerminalLineSize() const ;
    int getCursorPosOnTerminal() const {
        return _stringPos + printPromptImpl().length();
    }
    int getCursorPosOnString() const {
        return _stringPos;
    }

protected:

    KeyMap _keyMap;
    KeyBindMap _keyBindMap;
    CommandSelector* _commandSelector;
    struct termios _save_term;
    struct termios _term_setting;
    std::deque<std::string> _history;
    size_t _historyIndex;
    size_t _historyMax;
    std::string _historyFile;
    bool _isTerminatePermit;

    std::string _inputString;
    size_t _stringPos;
    
    bool _consoleExit;
    std::string _user_name;
    long _user_uid;
    std::string _user_homeDir;
    int _systemErrorNumber;
    Mode* _currentMode;
    ModeMap _modeMap;

    std::string _consolePath;
};

}

#endif /* end of include guard */
