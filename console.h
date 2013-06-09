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
};
//namespace console {

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
    virtual std::string printHelp() const { return _helpBehavior->printHelp(); }
    virtual std::string execute(std::string param) const { 
        std::string cmd = _command + " " + _option + param;
        system(cmd.c_str()); 
        return "";
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
    virtual std::string printHelp() const {
        return "Usage : exe [command]";
    }
    virtual std::string execute(std::string param) const {
        system(param.c_str()); 
        return "";
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

};

class BuiltInScriptCommand;
class BuiltInScriptExitCommand;

class Console {
    typedef void (Console::*Action)();
    typedef CommandSelector::CommandSet CommandSet;
    typedef std::map<KeyCode::Code, Action> KeyBindMap;
    
    friend class BuiltInScriptCommand;
    friend class BuiltInScriptExitCommand;
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
        //_commandSelector = new DefaultCommandSelector();
        _commandSelector = new AbbreviatedCommandSelector();
        initialize();
    }
    virtual ~Console() {
        unInitialize();
    }
    
    bool registerKeyBinding(KeyCode::Code code, Action action) {
        if(_keyBindMap.count(code) == 1) {
            return false;
        }
        _keyBindMap.insert(std::pair<KeyCode::Code, Action>(code, action));
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
    Action getAction(KeyCode::Code keyCode) const {
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
    void executeCommand(const Command* cmd, const std::string& argument);
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
        return !_consoleExit;
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

    // エラー
    std::string getSystemError() {
        return strerror(_systemErrorNumber);
    }

    // ヒストリ機能
    void printAllHistory();
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

    void inputStringToTerminal(const std::string& str) {
        std::cout << str;
        _stringPos += str.length();
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
    virtual std::string printHelp() const { return "Usage help [command]:\n   Display [command] help"; }
    virtual std::string execute(std::string param) const {
        std::string str = param;
        str = str.erase(0, str.find_first_not_of(" "));
        str = str.erase(str.find_last_not_of(" ")+1);
        Command* cmd = _console->getCommand(str);
        if(cmd == NULL) {
            std::string ret = "help : Command[" + param + "] not found.";
            return ret;
        }
        return cmd->printHelp();
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _console->getCommandNameList(candidates);
    }
};

class BuiltInHistoryCommand : public Command {
    //Console* _console;
public:
    BuiltInHistoryCommand() {}
    virtual ~BuiltInHistoryCommand() {}

    virtual std::string getKey() const { return "history"; }
    virtual std::string printHelp() const { return "Usage history [History Number]"; }
    virtual std::string execute(std::string param) const {
        std::string str = param;
        str = str.erase(0, str.find_first_not_of(" "));
        str = str.erase(str.find_last_not_of(" ")+1);
        if(str.empty()) {
            printHistory();
            return printHelp();
        }

        std::stringstream ss(str);
        size_t num = 0;
        ss >> std::skipws >> num;
        std::string cmd = _console->getHistory(num);
        if(cmd.empty()) {
            return "history: Error.";
        } else {
            std::string key = cmd.substr(0, cmd.find(" "));
            if(key == "history") {
                return "history: History command can not execute oneself.";
            }
        }
        std::cout << "Execute : " << cmd << std::endl;
        std::cout << "-------------------";
        _console->execute(cmd);
        return "";
    }

    virtual void printHistory() const {
        _console->printAllHistory();
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        if(inputting.size() != 0 || !inputtedList.empty()) {
            return;
        }
        _console->printAllHistory();
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
    virtual std::string printHelp() const { return "make typescript end."; }
    virtual std::string execute(std::string param) const {
        _console->loggingMode(false);

        _console->uninstallCommand("exit", false);
        if(_exit == NULL) {
            return "";
        }
        _console->installCommand(_exit);
        
        return "";
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
    virtual std::string printHelp() const { return "make typescript of terminal session.\nUsage script [filename]:"; }
    virtual std::string execute(std::string param) const {

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
            
            return ("script " + filename + ": "+ _console->getSystemError());

        }
        
        return "";
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _fileListBehavior.getParamCandidates(inputtedList, inputting, candidates);
    }
};

void Console::keyMapInitialize() {

    KeyCode codeMaster;
    codeMaster.initialize();
    KeyBindMap::iterator ite = _keyBindMap.begin();
    KeyBindMap::iterator end = _keyBindMap.end();
    for(; ite != end; ite++) {
        std::stringstream ss;
        std::vector<char> stroke;
        std::string codeSeq = codeMaster.getCodeSeq(ite->first);
        ss << codeSeq;
        while(!ss.eof()) {
            int code;
            ss >> code;
            stroke.push_back(code);
        }
        _keyMap.addKeyCodeSeq(ite->first, stroke);
    }
    
}

bool Console::initialize() {
    if(setupterm(NULL, fileno(stdout), (int*)0) == ERR) {
        return false;
    }
    //char str[8] = "clear";
    //char* cmd;
    //if((cmd = tigetstr(str)) == NULL) {
    //    return false;
    //}
    //if(putp(cmd) == ERR) {
    //    return false;
    //}
    if(tcgetattr(fileno(stdin), &_save_term) == -1) {
        return false;
    } else {
        _term_setting = _save_term;
    }

    _term_setting.c_iflag &= IGNCR;
    _term_setting.c_lflag &= ~ICANON;
    _term_setting.c_lflag &= ~ECHO;
    _term_setting.c_lflag &= ~ISIG;
    _term_setting.c_cc[VMIN] = 1;
    _term_setting.c_cc[VTIME] = 0;

    if(tcsetattr(fileno(stdin), TCSANOW, &_term_setting) == -1) {
        return false;
    }

#ifdef DEBUG
    std::cout << " ------------------ KeyMapSetting Start" << std::endl;
#endif
    // キーストロークに対応するアクションの登録
    keyBindInitialize();
#ifdef DEBUG
    std::cout << " ------------------ KeyMapSetting End" << std::endl;
#endif
    // キーストロークの登録
    keyMapInitialize();

    installCommand(new BuiltInHelpCommand());
    installCommand(new BuiltInHistoryCommand());
    installCommand(new BuiltInScriptCommand());

#ifdef DEBUG
    std::cout << " ------------------ Load History Start" << std::endl;
#endif
    // load History
    loadHistory();
#ifdef DEBUG
    std::cout << " ------------------ Load History End" << std::endl;
#endif

    _consoleExit = false;
    struct passwd* userInfo = getpwnam(getlogin());
    _user_name = userInfo->pw_name;
    _user_uid = userInfo->pw_uid;
    _user_homeDir = userInfo->pw_dir;
    return true;
}

void Console::run() {
    printTitle();
    printPrompt();
    _inputString ="";
    _stringPos=0;
    _historyIndex = 0;
    bool isKeyStrokeBeginning = false;
#ifndef KEY_TRACE
    const KeySequenceEntry* entry = NULL;
#endif
    while(isEnd()) {
        char input = fgetc(stdin);
        //bool ret = userKeyHookProc(input);
#ifdef KEY_TRACE
        std::cout << (int)input << " "; // for key trace 
        if(input == 3) {
            return;
        }
#endif
        if( !isKeyStrokeBeginning ) {
            // space and symble or alphanumeric is output without do something.
            if( input >= 0x20 && input <= 0x7E) {
#ifndef KEY_TRACE
                _inputString.insert(_stringPos, 1, input);
                ++_stringPos;
                // 端末を挿入モードに設定することで、標準出力に入力するだけで、
                // 文字の追加処理を意識せずにカーソル位置に入力できる。
                char enterIns[8] = "smir";
                char exitIns[8] = "rmir";
                putp(tigetstr(enterIns));
                std::cout << input;
                putp(tigetstr(exitIns));
#endif
                continue;
            }
        }
        
#ifndef KEY_TRACE
        // Defined action execute if non alphanumeric charactor.
        if( entry == NULL ) {
            entry = _keyMap.getKeyEntry(input);
        } else {
            entry = entry->getKeySequenceEntry(input);
        }

        if(entry == NULL) {
            beep();
            isKeyStrokeBeginning = false;
            continue;
        }

        if(entry->isEntry()) {
            if(setupterm(NULL, fileno(stdout), (int*)0) == ERR) {
                continue;
            }
            KeyCode::Code actionCode = entry->getVirtualKeyCode();
            Action action = getAction(actionCode);
            if(action != NULL) {
                (this->*action)();
            }
            entry = NULL;
            isKeyStrokeBeginning = false;
        } else {
            isKeyStrokeBeginning = true;
            continue;
        }
#endif

    }
}


bool Console::completeCommand(std::string& key, std::vector<std::string>& candidates) {
    size_t min = -1;
    CommandSet::const_iterator candidate;
    candidates.clear();

    // trim white space
    std::string str = key;
    str = str.erase(0, str.find_first_not_of(" "));
    str = str.erase(str.find_last_not_of(" ")+1);

    // search candidate
    for(CommandSet::const_iterator ite = _commandSelector->getCommandSet().begin();
            ite != _commandSelector->getCommandSet().end();
            ++ite) {

        // find minimum size candidate
        if(ite->first.compare(0, str.size(), str) == 0) {
            if(min > ite->first.size()) {
                min = ite->first.size();
                candidate = ite;
            }
            candidates.push_back(ite->first);
        }
    }

    // ########### Complete longest common string

    if(candidates.size() == 1) {
        // full Complete
        key = candidates.front();
        return true;
    } else if(candidates.empty()) {
        //candidates.push_back("Candidate does not exist.");
        return false;
    }

    size_t pos = key.size();
    while(pos <= candidate->first.size()) {
        std::string complStr = candidate->first.substr(0, pos+1);
        for(std::vector<std::string>::iterator ite = candidates.begin();
                ite != candidates.end();
                ++ite) {

            if(ite->compare(0, complStr.size(), complStr) != 0){
                return false;
            }
        }
        key = complStr;
        ++pos;
    }

    return false;
}

template <class Iterator>
bool Console::completeStringList(std::string& inputStr, std::vector<std::string>& matchList, Iterator begin, Iterator end) {
    size_t min = -1;
    Iterator candidate;
    matchList.clear();

    // trim white space
    std::string str = inputStr;
    str = str.erase(0, str.find_first_not_of(" "));
    str = str.erase(str.find_last_not_of(" ")+1);

    // search candidate
    for(Iterator ite = begin; ite != end; ++ite) {
        // find minimum size candidate
        if(ite->compare(0, str.size(), str) == 0) {
            if(min > ite->size()) {
                min = ite->size();
                candidate = ite;
            }
            matchList.push_back(*ite);
        }
    }

    // ########### Complete longest common string

    if(matchList.size() == 1) {
        // full Complete
        inputStr = matchList.front();
        return true;
    } else if(matchList.empty()) {
        //matchList.push_back("Candidate does not exist.");
        return false;
    }

    size_t pos = inputStr.size();
    while(pos <= candidate->size()) {
        std::string complStr = candidate->substr(0, pos+1);
        // 最長一致検索
        for(std::vector<std::string>::iterator ite = matchList.begin();
                ite != matchList.end();
                ++ite) {

            if(ite->compare(0, complStr.size(), complStr) != 0){
                return false;
            }
        }
        inputStr = complStr;
        ++pos;
    }

    return false;
}

bool Console::selectHistory(bool up) {

    bool isGetHistory=false;

    if(up) {
        if(!_history.empty() && _historyIndex < _history.size()) {
            _historyIndex++;
            isGetHistory=true;
        }
    } else {
        if(_historyIndex > 0) {
            _historyIndex--;
            isGetHistory=true;
        }
    }

    if(isGetHistory) {
        //printf("\r");
        clearLine();
        std::string history = getHistory(_historyIndex-1);
        _inputString = history;
        char enterIns[8] = "smir";
        char endIns[8] = "rmir";
        putp(tigetstr(enterIns));
        std::cout << _inputString;
        putp(tigetstr(endIns));
        _stringPos = _inputString.size();
    }

    return true;
}

int Console::getTerminalColumnSize() const {
    char str[8] = "cols";
    int cols = tigetnum(str);
    return cols;
}
int Console::getTerminalLineSize() const {
    char str[8] = "lines";
    int cols = tigetnum(str);
    return cols;
}
bool Console::moveCursor(bool left) {

    if(!left) {
        char str[8] = "cuf1";
        if( _stringPos < _inputString.size() ) {
            char* cmd = tigetstr(str);
            _stringPos++;
            putp(cmd);
        }
    } else {
        char str[8] = "cub1";
        if( _stringPos > 0) {
            char* cmd = tigetstr(str);
            _stringPos--;
            putp(cmd);
        }
    }

    return true;
}

void Console::actionMoveCursorForwardParam() {
    size_t pos = getCursorPosOnString();
    size_t space = _inputString.find(" ", pos);
    if(space == std::string::npos) {
        setCursorPos(_inputString.length());
        return;
    }
    size_t nextParamPos = _inputString.find_first_not_of(" ", space);
    if(nextParamPos == std::string::npos) {
        setCursorPos(_inputString.length());
        return;
    }
    setCursorPos(nextParamPos);
    return;
}
void Console::actionMoveCursorBackwardParam() {
    size_t pos = getCursorPosOnString();
    size_t nextParamPos = 0;
    if(_inputString[pos] == ' ') {
        std::string::reverse_iterator ite = _inputString.rbegin();
        std::string::reverse_iterator end = _inputString.rend();
        ite += (_inputString.length() - (pos+1));
        std::string::reverse_iterator ret = std::find_if(ite, end, NotSpaceCampare());
        ret = std::find_if(ret, end, SpaceCampare());
        nextParamPos = std::distance(ret, end);
    } else {
        std::string::reverse_iterator ite = _inputString.rbegin();
        std::string::reverse_iterator end = _inputString.rend();
        ite += (_inputString.length() - (pos+1));
        ite++;
        std::string::reverse_iterator ret = std::find_if(ite, end, NotSpaceCampare());
        ret = std::find_if(ret, end, SpaceCampare());
        nextParamPos = std::distance(ret, end);
    }

    setCursorPos(nextParamPos);
    return;
}

void Console::actionDeleteBackwardCharacter() {
    if(_inputString.empty()) {
        // 入力文字列がない
        beep();
        return;
    }

    if(_stringPos > 0) {
        moveCursor(true);
        actionDeleteForwardCharacter();
        //_inputString.erase(_stringPos, 1);
        return;
    } else {
        beep();
    }
    return;
}

void Console::actionDeleteForwardCharacter() {
    char str[8] = "dch1";
    if(_inputString.empty() || _stringPos == _inputString.size()){
        beep();
        return;
    }
    
    // カーソルが文字列内にあるか判定
    if(_stringPos <= _inputString.size()) {
        char* cmd = tigetstr(str);
        putp(cmd);
        _inputString.erase(_stringPos, 1);
        return;
    } else {
        beep();
    }
    return;
}

void Console::actionDeleteParam() {
    size_t pos = getCursorPosOnString();
    size_t delSize = 0;
    if(_inputString[pos] == ' ') {
        return;
    }

    std::string::iterator ite = _inputString.begin();
    std::string::iterator end = _inputString.end();
    ite += pos;
    std::string::iterator ret = std::find_if(ite, end, NotSpaceCampare());
    ret = std::find_if(ret, end, SpaceCampare());
    if(ret == end) {
        ret = end;
    } else {
        ret++;
    }
    //std::cout << std::endl << "["<< *ite << "]:[" << *ret << "]" << std::endl;
    delSize = std::distance(ite, ret);
    while(delSize > 0) {
        actionDeleteForwardCharacter();
        delSize--;
    }

    return;
}

void Console::actionEnter() {
    execute(_inputString);
    clearStatus();
    printPrompt();

    return;
}

void Console::actionTerminate() {
    if(_isTerminatePermit) {
        std::cout << std::endl;
        _consoleExit = true;
        return;
    }
    return;
}

void Console::actionDeleteFromCursorToEnd() {
    size_t cursorPos = getCursorPosOnString();
    size_t strSize = _inputString.size();

    putp(tparm(parm_dch, strSize-cursorPos));
    _inputString.erase(cursorPos, strSize-cursorPos);

    return;
}
void Console::actionDeleteFromHeadToCursor() {
    size_t cursorPos = getCursorPosOnString();
    while(cursorPos > 0) {
        actionDeleteBackwardCharacter();
        cursorPos--;
    }

    return;
}

void Console::actionClearScreen() {
    char str[8] = "clear";
    char* cmd;
    if((cmd = tigetstr(str)) == NULL) {
        return;
    }
    if(putp(cmd) == ERR) {
        return;
    }

    clearLine();
    return;
}

void Console::execute(const std::string& inputString) {
    std::cout << std::endl;

    // コマンド名、引数に分離
    size_t sp = inputString.find(' ');
    std::string key = inputString.substr(0, sp);
    std::string argument = "";
    if(sp != std::string::npos) {
        argument = inputString.substr(sp);
    }

    if(key.empty()) {
        return;
    }

    if(_isLogging && _logFlag == false) {
        _stdinBackup = dup(1);
        _stderrBackup = dup(2);
        dup2(_typeLogFd, 1);
        dup2(_typeLogFd, 2);
        _logFlag = true;
        fprintf(_typeLog, "$ %s\n", inputString.c_str());
        fseek(_typeLog, 0, SEEK_END);
        _before_fpos = ftell(_typeLog);
    }

    Command* cmd = getCommand(key);
    executeCommand(cmd, argument);

    // 文字列が一文字以上であればヒストリに追加
    if(!inputString.empty()) {

        // 実行したコマンドがある場合は、そのコマンドをヒストリに残すべきかを判定する.
        //
        // history コマンド自体を ヒストリに残さない. これは [history 0] などで history から historyを呼び出す再帰を防ぐため
        if(cmd != NULL && cmd->isHistoryAdd()) {
            addHistory(inputString);
        }
    }

    if(_isLogging) {
        dup2(_stdinBackup, 1);
        dup2(_stderrBackup, 2);
        _logFlag = false;
        updateDisplayFromScriptLog();
    }
}

void Console::executeCommand(const Command* cmd, const std::string& argument) {
    // ターミナル状態をもとに戻す
    setTermIOS(_save_term);

    std::string ret("");
    if(cmd == NULL) {
        ret =  _commandSelector->errorCause();
    } else {
        ret = cmd->execute(argument);
    }

    std::cout << ret << std::endl;
    if(_isLogging) {
        std::cout << std::flush;
    }

    // ターミナル状態をもとに戻す
    setTermIOS(_term_setting);
}

Command* Console::getCommandFromInputString(std::string& inputString) {
   Command* cmd = NULL;
   size_t max = 0;
    for(CommandSet::const_iterator ite = _commandSelector->getCommandSet().begin();
            ite != _commandSelector->getCommandSet().end();
            ++ite) {
        size_t tmp = 0;
        tmp = inputString.find(ite->first);
        if(tmp != std::string::npos) {
            if(max < tmp) {
                cmd = ite->second;
                max = tmp;
            }
        }
    }
    return cmd;
}

void Console::actionComplete() {

    // コマンド名を入力中であればコマンド名を補完する
    // コマンド名が確定している場合はパラメータ補完

    // コマンド名が空の場合は全てのコマンド名を表示
    if(_inputString.empty()) {
        // 文字列が入力されていないので、全コマンドをリストアップ
        std::cout << std::endl;
        printAllCommandName();

        std::cout << std::endl;
        printPrompt();

        return;
    }

    if(_inputString.find(" ") == std::string::npos) {
        if(completeCommandName() == ERROR) {
            assert(false);
        }
        return;
    }

    // 以降ではコマンド名が確定している場合の処理
    Command* cmd = NULL;

    std::string fixedInput = _inputString.substr(0, _stringPos);
    std::list<std::string> delimiterList;
    delimiterList.push_back(" ");
    std::vector<std::string>* tokenList = divideStringToVector(fixedInput, delimiterList);

    cmd = getCommand((*tokenList)[0]);
    if(cmd == NULL) {
        delete tokenList;
        return;
    }

    // トークンリストが 1 つまりコマンド名のみである場合は、パラメータリストを表示して終了
    if(tokenList->size() == 1) {
        std::cout << std::endl;
        std::vector<std::string> argumentList;
        std::string param = "";
        std::vector<std::string> candidates;
        cmd->getParamCandidates(argumentList, param, candidates);
        cmd->afterCompletionHook(candidates);
        printStringList(candidates.begin(), candidates.end());
        size_t pos = _stringPos;
        clearLine(false);
        _stringPos = 0;
        inputStringToTerminal(_inputString);
        setCursorPos(pos);

        return;
    }


    std::vector<std::string> argumentList;
    std::string param = "";
    getInputParameter(fixedInput, tokenList, param, argumentList);

    delete tokenList; tokenList = NULL;

    // paramのサイズが0の場合はパラメータを一文字も入力していない, そもそもこっちまで来ない
    // paramのサイズが0以上の場合はパラメータ入力中

    // パラメータ補完を実施
    // 補完が完全補完の場合は表示して終了
    // 補完が一部補完である場合は、補完されている場合はそのまま表示して終了
    // 補完候補がない場合は beep
    std::vector<std::string> candidates;
    cmd->getParamCandidates(argumentList, param, candidates);

    std::string after = param;
    argumentList.clear();
    std::vector<std::string>::iterator begin = candidates.begin();
    std::vector<std::string>::iterator end = candidates.end();
    bool ret = completeStringList(after, argumentList, begin, end);

    // 補完結果をカーソル位置に反映させる
    if( ret ) {
        // 完全補完
        size_t pos = after.find(param);
        std::string sub = after.substr(pos+param.length());
        if(pos != std::string::npos) {
            _inputString.insert(_stringPos, sub);
            _stringPos += sub.length();
            size_t cursorPos = _stringPos;
            clearLine(false);
            _stringPos = 0;
            inputStringToTerminal(_inputString);
            setCursorPos(cursorPos);
            return;
        } else {
            beep();
            return;
        }
    } else {
        // 一部補完 or 補完候補なし
        // candidates.size() > 0 である場合、一部補完と判断
        if(!argumentList.empty()) {
            // 補完が行われた場合は、そのまま表示
            // 変更がない場合は、候補を表示する。
            if(param != after) {
                if(param.empty()) {
                    _inputString.insert(_stringPos, after);
                    _stringPos += after.length();
                    size_t cursorPos = _stringPos;
                    clearLine(false);
                    _stringPos = 0;
                    inputStringToTerminal(_inputString);
                    setCursorPos(cursorPos);
                    return;
                } else {
                    //size_t pos = _inputString.rfind(param);
                    size_t pos = after.find(param);
                    std::string sub = after.substr(pos+param.length());
                    if(pos != std::string::npos) {
                        _inputString.insert(_stringPos, sub);
                        _stringPos += sub.length();
                        size_t cursorPos = _stringPos;
                        clearLine(false);
                        _stringPos = 0;
                        inputStringToTerminal(_inputString);
                        setCursorPos(cursorPos);
                        return;
                    } else {
                        beep();
                        return;
                    }
                }
            } else {
                // 変更がないので候補表示
                std::cout << std::endl;
                cmd->afterCompletionHook(argumentList);
                printStringList(argumentList.begin(), argumentList.end());
                size_t cursorPos = _stringPos;
                clearLine(false);
                _stringPos = 0;
                inputStringToTerminal(_inputString);
                setCursorPos(cursorPos);
                return;
            }
        } else {
            // 補完候補なし
            beep();
            return;
        }
    }
    
    return;
}

Console::CompletionType Console::completeCommandName() {

    std::vector<std::string> candidates;
    std::string tmp = _inputString;
    // ret = true : コマンド名を完全補完
    // ret = false: コマンド名を一部補完 or 補完候補なし
    bool ret = completeCommand(tmp, candidates);
    if( ret ) {
        // 完全補完
        clearLine();
        _inputString = tmp + " ";
        _stringPos = tmp.size()+1;
        std::cout << _inputString;
        return FULL_COMPLETE;
    } else {
        // 一部補完 or 補完候補なし
        // candidates.size() > 0 である場合、一部補完と判断
        if(candidates.size() > 0) {
            // 補完が行われた場合は、そのまま表示
            // 変更がない場合は、候補を表示する。
            if(tmp != _inputString) {
                clearLine();
                _inputString = tmp;
                _stringPos = tmp.size();
                std::cout << _inputString;
                return PARTIAL_COMPLETE;
            } else {
                // 変更がないので候補表示
                std::cout << std::endl;
                printStringList(candidates.begin(), candidates.end());
                printPrompt();
                _inputString = tmp;
                _stringPos = tmp.size();
                std::cout << _inputString;
                return NO_CHANGE;
            }
        } else {
            // 補完候補なし
            beep();
            return NO_CANDIDATE;
        }
    }
    return ERROR;
}

void Console::getInputParameter(std::string& inputString, std::vector<std::string>* tokenList, std::string& inputting, std::vector<std::string>& paramList) {

    // パラメータは、スペースで区切られる。
    // パラメータリストで取得する際に指定する文字列は入力中のパラメータを含めない。 但し入力中の文字列として取得。
    // 上記についてコマンド名を含めず、最初のスペースで区切られたトークンから最後のスペースで区切られたトークンまでを引数リストに入れる
    // 例：Test ABC DEF G|    "|"をカーソル位置とすると Gは含めない
    // 例：Test ABC DEF G |   "|"をカーソル位置とすると Gは含む

    if(inputString[inputString.size()-1] == ' ') {
        // 最後のトークンもスペースで区切られている
        std::vector<std::string>::iterator ite = tokenList->begin();
        ite++;
        for(;
                ite != tokenList->end();
                ++ite) {
            paramList.push_back(*ite);
        }
    } else {
        // 最後のトークンはスペースで区切られていない
        std::vector<std::string>::iterator ite = tokenList->begin();
        ite++;
        for(;
            ite != tokenList->end();
            ++ite) {

            paramList.push_back(*ite);
        }
        inputting = paramList.back();
        paramList.pop_back();
    }
}

void Console::printAllHistory() {
    std::cout << std::endl;
    size_t count = _history.size()-1;
    for(std::deque<std::string>::reverse_iterator ite = _history.rbegin();
        ite != _history.rend();
        ++ite) {
        std::cout << "[" << count << "] : " << *ite << std::endl;
        --count;
    }
}

std::string Console::getHistory(size_t index) {
    if(!_history.empty() && _history.size() > index) {
        return _history.at(index);
    }
    return "";
}

void Console::addHistory(std::string str, bool save) {
    _history.push_front(str);
    if(_history.size() > _historyMax) {
        _history.pop_back();
    }
    if(save) {
        std::ofstream ofs(_historyFile.c_str(), std::ios::trunc | std::ios::out);
        for(std::deque<std::string>::reverse_iterator ite = _history.rbegin();
                ite != _history.rend();
                ++ite) {
            ofs << *ite << std::endl;
        }
    }

}

void Console::loadHistory() {
    std::ifstream ifs(_historyFile.c_str());
    if(!ifs.is_open()) {
        return;
    }

    std::string line;
    while(!ifs.eof()) {
        std::getline(ifs, line);
        if(line.empty()) {
            break;
        }
        addHistory(line, false);
    }

#ifdef DEBUG
    // history print
    for(std::deque<std::string>::iterator ite = _history.begin();
        ite != _history.end();
        ++ite) {
        std::cout << *ite << std::endl;
    }
#endif
}

void Console::printAllCommandName() {
    size_t max=0;
    for(CommandSet::const_iterator ite = _commandSelector->getCommandSet().begin();
            ite != _commandSelector->getCommandSet().end();
            ++ite) {
        if(max < ite->first.size()) {
            max = ite->first.size();
        }
    }
    max += 3;
    size_t num = getTerminalColumnSize()/max;
    num--;
    size_t i=0;
    for(CommandSet::const_iterator ite = _commandSelector->getCommandSet().begin();
            ite != _commandSelector->getCommandSet().end();
            ++ite) {
        std::cout << std::left << std::setw(max) << ite->first;
        ++i;
        if(i > num) {
            std::cout << std::endl;
            i = 0;
        }
    }
}
bool Console::loggingMode(bool flag, std::string filename) {
    if(_isLogging == flag) {
        std::cout << "Now Logging." << std::endl;
        return false;
    }

    // logging中はCTRL-Cは不可, 必ずexitを使用させる
    // logging終了後は以前の状態に戻す
    _isLogging = flag;
    if(_isLogging) {
        // ロギング開始
        _before_fpos = _after_fpos = 0;
        _typeLog = NULL;
        _typeLogFd = 0;
        _typeLog = fopen(filename.c_str(), "w+");
        if(_typeLog == NULL) {
            _systemErrorNumber = errno;
            _isLogging = false;
            return false;
        }
        _typeLogFd = fileno(_typeLog);
        _typeLogName = filename;


        // 標準出力 -> _typeLog
        _stdinBackup = dup(1);
        _stderrBackup = dup(2);
        dup2(_typeLogFd, 1);
        dup2(_typeLogFd, 2);
        _logFlag = true;

        std::cout << "Script started, output file is " << _typeLogName << "." << std::endl;
        system("date");

        _isTerminatePermit = false;
    } else {
        std::cout << "Script done, output file is " << _typeLogName << "." << std::endl;
        system("date");

        dup2(_stdinBackup, 1);
        dup2(_stderrBackup, 2);
        std::cout << std::endl << "Script done, output file is " << _typeLogName << "." << std::endl;
        //updateDisplayFromScriptLog();

        fclose(_typeLog);
        close(_stdinBackup);
        close(_stderrBackup);

        _isTerminatePermit = true;
    }

    return true;
}

template <class Iterator>
void Console::printStringList(Iterator lh, Iterator rh) {
    size_t max=0;
    Iterator begin = lh;
    Iterator end = rh;
    while(lh!= rh) {
        if(max < lh->size()) {
            max = lh->size();
        }
        lh++;
    }

    max += 2;
    size_t num = getTerminalColumnSize()/max;
    num--;
    size_t i=0;
    lh = begin;
    rh = end;
    while(lh!= rh) {
        std::cout << std::left << std::setw(max) << *lh;
        ++i;
        if(i > num) {
            std::cout << std::endl;
            i = 0;
        }
        lh++;
    }
    std::cout << std::endl;
}

std::vector<std::string>* Console::divideStringToVector(std::string& src, std::list<std::string>& delimiter) {

    std::vector<std::string>* result = new std::vector<std::string>();

    if(src.empty()){
        return result;
    }

    result->push_back(src);

    std::list<std::string>::iterator dite = delimiter.begin();

    for(; dite != delimiter.end(); ++dite){
        for(std::vector<std::string>::iterator ite = result->begin();
                ite != result->end();
                ++ite){

            size_t pos = (ite)->find(*dite);
            if(pos == std::string::npos){
                continue;
            }

            std::string substr = (ite)->substr(pos+dite->size());
            (ite)->erase(pos);

            if(*ite == ""){
                ite--;
                result->erase(ite+1);
            }

            if(substr != "") {
                ite = result->insert(ite+1, substr);
                ite--;
            }

        }
    }

    return result;
}

class EditorCommand : public Command {
    std::string _command;
    ParameterBehavior* _behavior;
    HelpBehavior* _helpBehavior;
public:
    EditorCommand(std::string commandName, ParameterBehavior* behavior, HelpBehavior* helpBehavior) :
        _command(commandName), _behavior(behavior), _helpBehavior(helpBehavior) {}

    virtual ~EditorCommand() { delete _behavior; delete _helpBehavior; }

    virtual std::string getKey() const { return _command; }
    virtual std::string printHelp() const { return _helpBehavior->printHelp(); }
    virtual std::string execute(std::string param) const { 
        std::string cmd = _command + " " + param;
        if(_console->isLogging()) {
            return _command + " : not supporting during \"script\" execution.";
        }
        system(cmd.c_str()); 
        return "";
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        _behavior->getParamCandidates(inputtedList, inputting, candidates);
    }
};

//}

#endif /* end of include guard */
