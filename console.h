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

#include "key_code.h"
#include "key_map.h"
#include "command/command.h"
#include "command/command_selector.h"

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
//namespace console {

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

class SystemFuncCommand : public Command {
    std::string _command;
    std::string _option;
    ParameterBehavior* _behavior;
    HelpBehavior* _helpBehavior;
public:
    SystemFuncCommand(std::string commandName, std::string option, ParameterBehavior* behavior, HelpBehavior* helpBehavior) :
        _command(commandName), _option(option), _behavior(behavior), _helpBehavior(helpBehavior) {}

    virtual ~SystemFuncCommand() { delete _behavior; delete _helpBehavior; }

    virtual std::string getKey() const { return _command; }
    virtual void printHelp() const { _helpBehavior->printHelp(); return; }
    virtual void execute(std::string param) { 
        std::string cmd = _command + " " + _option + " " + param;
        system(cmd.c_str()); 
        return;
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _behavior->getParamCandidates(inputtedList, inputting, candidates);
    }
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const {
        FileListBehavior fileListBehavior;
        fileListBehavior.stripParentPath(candidates);
    }
};

class ShellCommandExecutor : public Command {
    std::string _command;
    // 遅延評価させるための措置
    mutable bool _initFlag;
    mutable std::set<std::string> _commandList;
    FileListBehavior _fileListBehavior;
public:
    ShellCommandExecutor(const std::string& commandName ) : _command(commandName), _initFlag(false) {
    }

    virtual ~ShellCommandExecutor() {
    }

    virtual std::string getKey() const { return _command; }
    virtual void printHelp() const {
        std::cout << "Usage : exe [command]" << std::endl;
    }
    virtual void execute(std::string param) {
        system(param.c_str()); 
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        if(_initFlag == false) {
            const char* c_env = getenv("PATH");
            std::string env(c_env);
            std::string::size_type pos = 0;
            while(true) {
                std::string::size_type findpos = env.find(':', pos);
                std::string path = env.substr(pos, (findpos-pos));
                if(findpos == std::string::npos || path.empty()) {
                    //std::cout << "--------------------- END!!!!!!!!!!" << std::endl;
                    break;
                }
                //std::cout << "---------------------" << path << std::endl;
                pos = findpos+1;
                struct dirent** namelist;
                int entrySize = scandir(path.c_str(), &namelist, NULL, alphasort);
                if( entrySize == -1 ) {
                    continue;
                }
                //std::cout << "---------------------" << entrySize << std::endl;
                if(path.at(path.length()-1) != '/') {
                    path+='/';
                }
                for(int i=0; i < entrySize; ++i) {
                    std::string fullPath = path + namelist[i]->d_name;
                    //std::cout << "---------------------" << fullPath.c_str() << std::endl;
                    int ret = access(fullPath.c_str(), X_OK);
                    if(ret == 0) {
                        _commandList.insert(namelist[i]->d_name);
                    }
                    free(namelist[i]);
                }
                free(namelist);
            }
            _initFlag = true;
        }

        if(inputting.find("./") != std::string::npos) {
            _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
            return;
        }
        if(inputtedList.empty()) {
            candidates.insert(candidates.begin(), _commandList.begin(), _commandList.end());
        } else {
            if(inputtedList[0] == "man") {
                candidates.insert(candidates.begin(), _commandList.begin(), _commandList.end());
            } else {
                _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
            }
        }
    }
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const {
        FileListBehavior fileListBehavior;
        fileListBehavior.stripParentPath(candidates);
    }
};

class BuiltInScriptCommand;
class BuiltInScriptExitCommand;

class Console {
    typedef void (Console::*ConsoleMethod)();
    typedef CommandSelector::CommandSet CommandSet;
    typedef std::map<KeyCode::Code, Action*> KeyBindMap;
    
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

    Console(size_t histroySize = 20, std::string filename = ".cli_history") :
        _commandSelector(NULL),
        _historyMax(histroySize),
        _historyFile(filename),
        _isLogging(false),
        _isTerminatePermit(true)
    {
        _commandSelector = new DefaultCommandSelector();
        //_commandSelector = new AbbreviatedCommandSelector();
        initialize();
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
    void actionEnter();
    void actionComplete();
    void actionMoveCursorTop() { setCursorPos(0); return;}
    void actionTerminate();
    void actionMoveCursorBottom() { setCursorPos(_inputString.size()); return;}
    void actionClearLine() { clearLine(); return; }
    void actionDeleteFromCursorToEnd();
    void actionDeleteFromHeadToCursor();
    void actionClearScreen();

    void insertStringToTerminal(const std::string& str) {
        _inputString.insert(_stringPos, str);
        _stringPos += str.length();
        std::string tmpStr = _inputString;
        size_t pos = _stringPos;
        actionClearLine();
        std::cout << tmpStr;
        _inputString = tmpStr;
        _stringPos = pos;
        setCursorPos(_stringPos);
    }
    void printStringOnTerminal(const std::string& str) {
        std::cout << str;
        _stringPos += str.length();
    }
    std::string getInputtingString() {
        return _inputString;
    }

private:
    // 初期化
    bool initialize();
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

    // アクション
    Action* getAction(KeyCode::Code keyCode) const {
        KeyBindMap::const_iterator ite = _keyBindMap.find(keyCode);
        if(ite == _keyBindMap.end()) {
            return NULL;
        }

        return ite->second;
    }

    // 補完機能
    CompletionType completeCommandName();
    void getInputParameter(std::string& inputString, std::vector<std::string>* tokenList, std::string& lastParam, std::vector<std::string>& paramList);
    bool completeCommand(std::string& key, std::vector<std::string>& candidates);
    template <class Iterator>
    bool completeStringList(std::string& key, std::vector<std::string>& candidates, Iterator begin, Iterator end);

    // コマンド機能
    void executeCommand(Command* cmd, const std::string& argument);
    Command* getCommandFromInputString(std::string& inputString);

    // History
    void loadHistory();
    bool selectHistory(bool up);
    void addHistory(std::string str, bool save = true);

     // 画面フォーマット出力
    void printPrompt() {
        printf("\r");
        std::string str = printPromptImpl();
        std::cout << "\x1b[36m" << str << "\x1b[39m";
        _inputString = "";
        _stringPos = 0;
    }
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

    // script コマンド対応
    bool loggingMode(bool flag, std::string filename = "typescript"); 
    void updateDisplayFromScriptLog() {

        fseek(_typeLog, 0, SEEK_END);
        _after_fpos = ftell(_typeLog);
        if(_before_fpos >= _after_fpos) {
            return;
        }
 
        char* buf = new char[_after_fpos - _before_fpos + 1];
        size_t size = _after_fpos - _before_fpos;
        if(fseek(_typeLog, _before_fpos, SEEK_SET) != 0) {
            assert(false);
        }
        if(fread(buf, size, 1, _typeLog) == 0) {
            assert(false);
        }
        _before_fpos = ftell(_typeLog);
        buf[size] = '\0';
        dup2(_stdinBackup, 1);
        dup2(_stderrBackup, 2);

        std::cout << buf;
    }

public:
    class Filter {
    public:
        virtual ~Filter() {}
        virtual bool isSkip(const std::string& value) = 0;
    };
    class NullFilter : public Filter {
    public:
        virtual bool isSkip(const std::string& value) {
            return false;
        }
    };

    // エラー
    std::string getSystemError() {
        return strerror(_systemErrorNumber);
    }

    // ヒストリ機能
    void printAllHistory(Filter* fileter);
    std::string getHistory(size_t index); 

    void printTitle(); 
    void printAllCommandName();

    void beep() { printf("\a"); }
    std::string getUserName() const { return _user_name; }
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

    // 文字列分割
    std::vector<std::string>* divideStringToVector(std::string& src, std::list<std::string>& delimiter); 

    int getTerminalColumnSize() const ;
    int getTerminalLineSize() const ;
    int getCursorPosOnTerminal() const {
        return _stringPos + printPromptImpl().length();
    }
    int getCursorPosOnString() const {
        return _stringPos;
    }

    bool isLogging() { return _isLogging; }

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
    bool _isLogging;
    bool _isTerminatePermit;
    bool _logFlag;
    FILE* _typeLog;
    size_t _before_fpos, _after_fpos;
    int _typeLogFd, _stdinBackup, _stderrBackup;
    std::string _typeLogName;

    std::string _inputString;
    size_t _stringPos;
    
    bool _consoleExit;
    std::string _user_name;
    long _user_uid;
    std::string _user_homeDir;
    int _systemErrorNumber;
};

class BuiltInHelpCommand : public Command {
    //Console* _console;
public:
    BuiltInHelpCommand() {}
    virtual ~BuiltInHelpCommand() {}

    virtual std::string getKey() const { return "help"; }
    virtual void printHelp() const { std::cout << "Usage help [command]:\n   Display [command] help" << std::endl;; }
    virtual void execute(std::string param) {
        std::string str = param;
        str = str.erase(0, str.find_first_not_of(" "));
        str = str.erase(str.find_last_not_of(" ")+1);
        Command* cmd = _console->getCommand(str);
        if(cmd == NULL) {
            std::string ret = "help : Command[" + param + "] not found.";
            std::cout << ret << std::endl;
            return;
        }
        cmd->printHelp();
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _console->getCommandNameList(candidates);
    }
};

class BuiltInHistoryCommand : public Command {
    class HistoryFilter : public Console::Filter {
        const std::vector<std::string>& _filterList;
    public:
        HistoryFilter(const std::vector<std::string>& filterList) : _filterList(filterList) {
        };

        virtual bool isSkip(const std::string& value) {
            std::vector<std::string>::const_iterator ite = _filterList.begin();
            std::vector<std::string>::const_iterator end = _filterList.end();
            for(; ite != end; ++ite) {
                if(value.find(*ite) == std::string::npos) {
                    return true;
                }
            }
            return false;
        }
    };
public:
    BuiltInHistoryCommand() {}
    virtual ~BuiltInHistoryCommand() {}

    virtual std::string getKey() const { return "history"; }
    virtual void printHelp() const { std::cout << "Usage history [filter string] ... [History Number]" << std::endl; }
    virtual void execute(std::string param) {
        std::string str = param;
        str = str.erase(0, str.find_first_not_of(" "));
        str = str.erase(str.find_last_not_of(" ")+1);
        if(str.empty()) {
            printHistory();
            printHelp();
            return;
        }

        std::list<std::string> delimiterList;
        delimiterList.push_back(" ");
        std::vector<std::string>* filterList = _console->divideStringToVector(param, delimiterList);
        std::string endStr = filterList->back();
        bool isExecute = true;;
        for(std::string::iterator ite = endStr.begin(); ite != endStr.end(); ++ite) {
            if(std::isxdigit(*ite) == false) {
                isExecute = false;
            }
        }

        if(isExecute) {
            std::stringstream ss(endStr);
            size_t num = 0;
            ss >> std::skipws >> num;
            std::string cmd = _console->getHistory(num);
            if(cmd.empty()) {
                delete filterList;
                std::cout << "history: Error." << std::endl;
                return;
            } else {
                std::string key = cmd.substr(0, cmd.find(" "));
                if(key == "history") {
                    delete filterList;
                    std::cout << "history: History command can not execute oneself." << std::endl;
                    return;
                }
            }
            std::cout << "Execute : " << cmd << std::endl;
            std::cout << "-------------------";
            _console->execute(cmd);
            delete filterList;
            std::cout << "end" << std::endl;
        } else {
            HistoryFilter filter(*filterList);
            _console->printAllHistory(&filter);
            delete filterList;
            printHelp();
        }
    }

    virtual void printHistory() const {
        Console::NullFilter nonFilter;
        _console->printAllHistory(&nonFilter);
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        if(inputting.size() != 0 || !inputtedList.empty()) {
            std::cout << std::endl;
            std::vector<std::string> filterList = inputtedList;
            filterList.push_back(inputting);
            HistoryFilter filter(filterList);
            _console->printAllHistory(&filter);
            std::cout << std::endl;
            _console->insertStringToTerminal("");
            return;
        }

        Console::NullFilter nonFilter;
        _console->printAllHistory(&nonFilter);
    }

    virtual bool isHistoryAdd() { return false; }
};

class BuiltInScriptExitCommand : public Command {
    FileListBehavior _fileListBehavior;
    Command* _exit;
public:
    BuiltInScriptExitCommand() : _exit(NULL) {}
    virtual ~BuiltInScriptExitCommand() {}

    virtual std::string getKey() const { return "exit"; }
    virtual void printHelp() const { std::cout << "make typescript end." << std::endl; }
    virtual void execute(std::string param) {
        _console->loggingMode(false);

        _console->uninstallCommand("exit", false);
        if(_exit == NULL) {
            return;
        }
        _console->installCommand(_exit);
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
    }

    void setExitCommand(Command* exit) { _exit = exit; }
};

class BuiltInScriptCommand : public Command {
    FileListBehavior _fileListBehavior;
    BuiltInScriptExitCommand* _scriptExitCmd;
public:
    BuiltInScriptCommand() : _scriptExitCmd(new BuiltInScriptExitCommand())  {}
    virtual ~BuiltInScriptCommand() {}

    virtual std::string getKey() const { return "script"; }
    virtual void printHelp() const { std::cout << "make typescript of terminal session.\nUsage script [filename]:" << std::endl; }
    virtual void execute(std::string param) {

        bool ret = false;
        std::string filename = param;
        filename = filename.erase(0, filename.find_first_not_of(" "));
        filename = filename.erase(filename.find_last_not_of(" ")+1);

        if(filename.empty()) {
            filename = "typescript";
        }

        ret = _console->loggingMode(true, filename);

        if(ret) {
            Command* cmd = _console->getCommand("exit");
            _scriptExitCmd->setExitCommand(cmd);
            _console->uninstallCommand("exit", false);
            _console->installCommand(_scriptExitCmd);
        } else {
            std::cout << "script " << filename << ": " <<  _console->getSystemError() << std::endl;
        }
        return;
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
    }
};


class EditorCommand : public Command {
    std::string _command;
    ParameterBehavior* _behavior;
    HelpBehavior* _helpBehavior;
public:
    EditorCommand(std::string commandName, ParameterBehavior* behavior, HelpBehavior* helpBehavior) :
        _command(commandName), _behavior(behavior), _helpBehavior(helpBehavior) {}

    virtual ~EditorCommand() { delete _behavior; delete _helpBehavior; }

    virtual std::string getKey() const { return _command; }
    virtual void printHelp() const { _helpBehavior->printHelp(); }
    virtual void execute(std::string param) { 
        std::string cmd = _command + " " + param;
        if(_console->isLogging()) {
            std::cout << _command + " : not supporting during \"script\" execution." << std::endl;
        }
        system(cmd.c_str()); 
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _behavior->getParamCandidates(inputtedList, inputting, candidates);
    }
};

//}

#endif /* end of include guard */
