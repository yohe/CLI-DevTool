#ifndef _CONSOLE_H_
#define _CONSOLE_H_

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

#include <errno.h>
#include <pwd.h>
#include <assert.h>

#include <termios.h>
#include <term.h>
#include <ncurses.h>
#include <unistd.h>
#include <dirent.h>

#include "action_code.h"
#include "key_map.h"
#include "command.h"
#include "command_selector.h"

//namespace console {

class ParameterBehavior {
public:
    ParameterBehavior() {}
    virtual ~ParameterBehavior() {}

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const = 0;
};

class NullParameterBehavior : public ParameterBehavior {
public:
    NullParameterBehavior() {}
    virtual ~NullParameterBehavior() {}

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const { return ; }
};

class FileListBehavior : public ParameterBehavior {
public:
    FileListBehavior() {}
    virtual ~FileListBehavior() {}

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        FILE* in_pipe = NULL;

        std::string path("");
        if(inputting.rfind('/') == std::string::npos) {
            // nop
        } else {
            path = inputting.substr(0,inputting.rfind('/')+1);
        }

        std::string cmd = "ls -Fa " + path;

        in_pipe = popen(cmd.c_str(), "r");
        std::stringstream paramList("");
        char buffer[512+1] = {0};
        if(in_pipe != NULL) {
            size_t readed_num = fread(buffer, sizeof(char), 512, in_pipe);
            while(readed_num > 0) {
                paramList.write(buffer, readed_num);
                readed_num = fread(buffer, sizeof(char), 512, in_pipe);
            }
        }
        pclose(in_pipe);
        while(!paramList.eof()) {
            std::string str;
            paramList >> std::skipws >> str;
            if(str.empty()) {
                continue;
            }
            std::string name = path+str;
            if(name.find(inputting) == std::string::npos) {
                continue;
            } else {
                if(name[name.size()-1] == '*') {
                    name.erase(name.size()-1);
                } else if (name[name.size()-1] == '/') {
                    // nop
                } else if (name[name.size()-1] == '|') {
                    name.erase(name.size()-1);
                } else if(name[name.size()-1] == '@') {
                    name[name.size()-1] = '/';
                } else {
                    // ファイルを補完する場合は スペースを追加することで、
                    // シェルライクなファイル名補完になる
                    name += " ";
                }
            }
            matchList.push_back(name);
        }
        return ;
    }
};

class HelpBehavior {
public:
    HelpBehavior() {}
    virtual ~HelpBehavior() {}

    virtual std::string printHelp() const = 0;
};

class ManBehavior : public HelpBehavior {
    std::string _commandName;
public:
    ManBehavior(std::string commandName) : _commandName(commandName) {}
    virtual ~ManBehavior() {}

    virtual std::string printHelp() const {
        std::string cmd = "man " + _commandName;
        system(cmd.c_str());
        return "";
    }
};

class SystemFuncHelpBehavior : public HelpBehavior {
    std::string _commandName;
    std::string _helpOption;
public:
    SystemFuncHelpBehavior(std::string commandName, std::string helpOption) : _commandName(commandName), _helpOption(helpOption) {}
    virtual ~SystemFuncHelpBehavior() {}

    virtual std::string printHelp() const {
        std::string cmd = _commandName + " " + _helpOption;
        system(cmd.c_str());
        return "";
    }
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
    virtual std::string printHelp() const { return _helpBehavior->printHelp(); }
    virtual std::string execute(std::string param) const { 
        std::string cmd = _command + " " + _option + param;
        system(cmd.c_str()); 
        return "";
    }
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _behavior->getParamList(inputtedList, inputting, matchList);
    }
};


class Console {
    typedef bool (Console::*Action)();
    typedef CommandSelector::CommandSet CommandSet;
    typedef std::map<int, Action> ActionMap;
    
public:

    enum ComplementType {
        FULL_COMPLEMENT = 1,
        PARTIAL_COMPLEMENT,
        NO_CHANGE,
        NO_CANDIDATE, 
        ERROR
    };

    Console(size_t histroySize = 20, bool isCTRL_CPermit = true, std::string filename = ".cli_history") :
        _commandSelector(NULL),
        _historyMax(histroySize),
        _historyFile(filename),
        _isCTRL_CPermit(isCTRL_CPermit),
        _tmpCTRL_CPermit(isCTRL_CPermit),
        _isLogging(false)
    {
        //_commandSelector = new DefaultCommandSelector();
        _commandSelector = new AbbreviatedCommandSelector();
    }
    virtual ~Console() {
        unInitialize();
    }

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
    void keyActionInitialize();


    // エラー
    std::string getSystemError() {
        return strerror(_systemErrorNumber);
    }

    // ヒストリ機能
    void printAllHistory();
    std::string getHistory(size_t index); 
    void addHistory(std::string str, bool save = true);

    // 画面フォーマット出力
    std::string printPromptImpl();
    void printTitle(); 

    void printAllCommandName();
    template <class Iterator>
    void printStringList(Iterator begin, Iterator end);
    void beep() { printf("\a"); }
    std::string getUserName() const { return _user_name; }
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
    Action getAction(int actionCode) const {
        ActionMap::const_iterator ite = _actionMap.find(actionCode);
        if(ite == _actionMap.end()) {
            return NULL;
        }

        return ite->second;
    }

    // CTRL-C を受け付けるか
    bool isCTRL_CPermit() {
        return _isCTRL_CPermit;
    }
    void setCTRL_CPermit(bool isPermit) {
        _isCTRL_CPermit = isPermit;
    }

    bool isLogging() { return _isLogging; }
    bool loggingMode(bool flag, std::string filename = "typescript") {
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
                setCTRL_CPermit(_tmpCTRL_CPermit);
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

            setCTRL_CPermit(false);
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

            setCTRL_CPermit(_tmpCTRL_CPermit);
        }

        return true;
    }
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

protected:
    // 定義済みアクション
    bool actionBackwardHistory() { return selectHistory(true); }
    bool actionForwarddHistory(){ return selectHistory(false); }
    bool actionMoveCursorRight() { return moveCursor(false); }
    bool actionMoveCursorLeft() { return moveCursor(true); }
    bool actionDeleteForwardCharacter();
    bool actionDeleteBackwardCharacter();
    bool actionEnter();
    bool actionComplement();
    bool actionMoveCursorTop() { setCursorPos(0); _stringPos = 0; return true;}
    bool actionTerminate() { _consoleExit = true; return true;}
    bool actionMoveCursorBottom() { setCursorPos(_inputString.size()); _stringPos = _inputString.size(); return true;}

    // 文字列分割
    std::vector<std::string>* divideStringToVector(std::string& src, std::list<std::string>& delimiter); 

    // ターミナル機能
    void  printPrompt() {
        printf("\r");
        std::string str = printPromptImpl();
        std::cout << str;
    }
    void clearLine(bool clearString = true) {
        char str[8] = "dl1";
        char* cmd = tigetstr(str);
        putp(cmd);
        if(clearString) {
            clearInputString();
        }
        printPrompt();
    }
    void clearInputString() {
        _stringPos = 0;
        _inputString.clear();
    }
    void setCursorPos(int pos) {
        printf("\r");
        pos += 2;
        putp(tparm(parm_right_cursor, pos));
    }
    void clearStatus() {
        _stringPos = 0;
        _inputString.clear();
        _historyIndex = 0;
    }
    bool moveCursor(bool left);

    // 補完機能
    ComplementType complementCommandName();
    void getInputParameter(std::string& inputString, std::vector<std::string>* tokenList, std::string& lastParam, std::vector<std::string>& paramList);
    bool completeCommand(std::string& key, std::vector<std::string>& matchList);
    template <class Iterator>
    bool completeStringList(std::string& key, std::vector<std::string>& matchList, Iterator begin, Iterator end);

    // コマンド実行機能
    void executeCommand(const Command* cmd, const std::string& argument);

    // コマンド取得機能
    Command* getCommandFromInputString(std::string& inputString);

    // History
    void loadHistory();
    bool selectHistory(bool up);

    bool isEnd() {
        return !_consoleExit;
    }
protected:

    KeyMap _keyMap;
    ActionMap _actionMap;
    CommandSelector* _commandSelector;
    struct termios _save_term;
    struct termios _term_setting;
    std::deque<std::string> _history;
    size_t _historyIndex;
    size_t _historyMax;
    std::string _historyFile;
    bool _isCTRL_CPermit;
    bool _tmpCTRL_CPermit;
    bool _isLogging;
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
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _console->getCommandNameList(matchList);
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
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
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
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _fileListBehavior.getParamList(inputtedList, inputting, matchList);
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
        std::string filename;
        if(param.empty()) {
            filename = "typescript";
        } else {
            std::string str = param;
            str = str.erase(0, str.find_first_not_of(" "));
            str = str.erase(str.find_last_not_of(" ")+1);
            filename = str.substr(0, str.find(' '));
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
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _fileListBehavior.getParamList(inputtedList, inputting, matchList);
    }
};


bool Console::initialize() {
    if(setupterm(NULL, fileno(stdout), (int*)0) == ERR) {
        return false;
    }
    char str[8] = "clear";
    char* cmd;
    if((cmd = tigetstr(str)) == NULL) {
        return false;
    }
    if(putp(cmd) == ERR) {
        return false;
    }
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

    // キーストロークの登録
    keyMapInitialize();
    // キーストロークに対応するアクションの登録
    keyActionInitialize();

    installCommand(new BuiltInHelpCommand());
    installCommand(new BuiltInHistoryCommand());
    installCommand(new BuiltInScriptCommand());

    // load History
    loadHistory();

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
    const KeyStrokeEntry* entry = NULL;
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
                printPrompt();
                std::cout << _inputString;
                setCursorPos(_stringPos);
#endif
                continue;
            }
        }
        
#ifndef KEY_TRACE
        // Defined action execute if non alphanumeric charactor.
        if( entry == NULL ) {
            entry = _keyMap.getKeyStroke(input);
        } else {
            entry = entry->getKeyStroke(input);
        }

        if(entry == NULL) {
            beep();
            isKeyStrokeBeginning = false;
            continue;
        }

        if(entry->isEntry()) {

            int actionCode = entry->getActionCode();
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


bool Console::completeCommand(std::string& key, std::vector<std::string>& matchList) {
    size_t min = -1;
    CommandSet::const_iterator candidate;
    matchList.clear();

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
            matchList.push_back(ite->first);
        }
    }

    // ########### Complete longest common string

    if(matchList.size() == 1) {
        // full Complete
        key = matchList.front();
        return true;
    } else if(matchList.empty()) {
        //matchList.push_back("Candidate does not exist.");
        return false;
    }

    size_t pos = key.size();
    while(pos <= candidate->first.size()) {
        std::string complStr = candidate->first.substr(0, pos+1);
        for(std::vector<std::string>::iterator ite = matchList.begin();
                ite != matchList.end();
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
        std::cout << _inputString;
        _stringPos = _inputString.size();
    }

    return true;
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

bool Console::actionDeleteBackwardCharacter() {
    if(_inputString.empty()) {
        // 入力文字列がない
        beep();
        return false;
    }

    if(_stringPos > 0) {
        moveCursor(true);
        actionDeleteForwardCharacter();
        _inputString.erase(_stringPos, 1);
        return true;
    } else {
        beep();
    }
    return false;
}

bool Console::actionDeleteForwardCharacter() {
    char str[8] = "dch1";
    if(_inputString.empty() || _stringPos == _inputString.size()){
        beep();
        return false;
    }
    
    // カーソルが文字列内にあるか判定
    if(_stringPos <= _inputString.size()) {
        char* cmd = tigetstr(str);
        putp(cmd);
        _inputString.erase(_stringPos, 1);
        return true;
    } else {
        beep();
    }
    return false;
}

bool Console::actionEnter() {
    execute(_inputString);
    std::cout << std::endl;
    clearStatus();
    printPrompt();

    return true;
}

void Console::execute(const std::string& inputString) {

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
        } else {
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
    std::cout << std::endl;
    // ターミナル状態をもとに戻す
    setTermIOS(_save_term);

    std::string ret("");
    if(cmd == NULL) {
        ret =  _commandSelector->errorCause();
    } else {
        ret = cmd->execute(argument);
    }

    std::cout << ret;
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

bool Console::actionComplement() {

    // コマンド名を入力中であればコマンド名を補完する
    // コマンド名が確定している場合はパラメータ補完

    // コマンド名が空の場合は全てのコマンド名を表示
    if(_inputString.empty()) {
        // 文字列が入力されていないので、全コマンドをリストアップ
        std::cout << std::endl;
        printAllCommandName();

        std::cout << std::endl;
        printPrompt();

        // 入力されていないはずなので不要？
        _inputString.clear();
        _stringPos = 0;
        return true;
    }

    if(_inputString.find(" ") == std::string::npos) {
        if(complementCommandName() == ERROR) {
            assert(false);
        }
        return true;
    }

    // 以降ではコマンド名が確定している場合の処理
    Command* cmd = NULL;
    std::list<std::string> delimiterList;
    delimiterList.push_back(" ");
    std::vector<std::string>* tokenList = divideStringToVector(_inputString, delimiterList);

    cmd = getCommand((*tokenList)[0]);
    if(cmd == NULL) {
        delete tokenList;
        return true;
    }

    // トークンリストが 1 つまりコマンド名のみである場合は、パラメータリストを表示して終了
    if(tokenList->size() == 1) {
        std::cout << std::endl;

        std::vector<std::string> argumentList;
        std::string param = "";
        std::vector<std::string> matchList;
        cmd->getParamList(argumentList, param, matchList);
        printStringList(matchList.begin(), matchList.end());
        std::cout << std::endl;
        printPrompt();
        std::cout << _inputString;
        return true;
    }


    std::vector<std::string> argumentList;
    std::string param = "";
    getInputParameter(_inputString, tokenList, param, argumentList);

    delete tokenList; tokenList = NULL;

    // paramのサイズが0の場合はパラメータを一文字も入力していない, そもそもこっちまで来ない
    // paramのサイズが0以上の場合はパラメータ入力中

    // パラメータ補完を実施
    // 補完が完全補完の場合は表示して終了
    // 補完が一部補完である場合は、補完されている場合はそのまま表示して終了
    // 補完候補がない場合は beep
    std::vector<std::string> matchList;
    cmd->getParamList(argumentList, param, matchList);

    std::string after = param;
    argumentList.clear();
    std::vector<std::string>::iterator begin = matchList.begin();
    std::vector<std::string>::iterator end = matchList.end();
    bool ret = completeStringList(after, argumentList, begin, end);

    if( ret ) {
        // 完全補完
        // 完全補完と判定された場合に, param.size()==0 であれば追加して表示
        // param.empty()==true の場合上書き
        if(param.empty()) {
            _inputString += after;// + " ";
            printPrompt();
            std::cout << _inputString;
            _stringPos = _inputString.size();
            return true;
        } else {
            size_t pos = _inputString.rfind(param);
            if(pos != std::string::npos) {
                _inputString.replace(pos, param.size(), after);
                printPrompt();
                //_inputString += " ";
                //_stringPos += 1;
                _stringPos = _inputString.size(); 
                std::cout << _inputString;
                return true;
            } else {
                beep();
                return true;
            }
        }
    } else {
        // 一部補完 or 補完候補なし
        // matchList.size() > 0 である場合、一部補完と判断
        if(!argumentList.empty()) {
            // 補完が行われた場合は、そのまま表示
            // 変更がない場合は、候補を表示する。
            if(param != after) {
                if(param.empty()) {
                    _inputString += after;
                    clearLine(false);
                    std::cout << _inputString;
                    _stringPos = _inputString.size();
                    return true;
                } else {
                    size_t pos = _inputString.rfind(param);
                    if(pos != std::string::npos) {
                        _inputString.replace(pos, param.size(), after);
                        clearLine(false);
                        std::cout << _inputString;
                        _stringPos = _inputString.size();
                        return true;
                    } else {
                        beep();
                        return true;
                    }
                }
            } else {
                // 変更がないので候補表示
                std::cout << std::endl;
                printStringList(argumentList.begin(), argumentList.end());
                std::cout << std::endl;
                if(param.empty()) {
                    _inputString += after;
                    _stringPos = _inputString.size();
                    clearLine(false);       
                    std::cout << _inputString;
                    return true;
                } else {
                    size_t pos = _inputString.rfind(param);
                    if(pos != std::string::npos) {
                        _inputString.replace(pos, param.size(), after);
                        _stringPos = _inputString.size();
                        clearLine(false);       
                        std::cout << _inputString;
                        return true;
                    } else {
                        beep();
                        printPrompt();
                        return true;
                    }
                }
            }
        } else {
            // 補完候補なし
            beep();
            return true;
        }
    }
    
    return true;
}

Console::ComplementType Console::complementCommandName() {

    std::vector<std::string> matchList;
    std::string tmp = _inputString;
    // ret = true : コマンド名を完全補完
    // ret = false: コマンド名を一部補完 or 補完候補なし
    bool ret = completeCommand(tmp, matchList);
    if( ret ) {
        // 完全補完
        clearLine();       
        _inputString = tmp + " ";
        _stringPos = tmp.size()+1;
        std::cout << _inputString;
        return FULL_COMPLEMENT;
    } else {
        // 一部補完 or 補完候補なし
        // matchList.size() > 0 である場合、一部補完と判断
        if(matchList.size() > 0) {
            // 補完が行われた場合は、そのまま表示
            // 変更がない場合は、候補を表示する。
            if(tmp != _inputString) {
                clearLine();       
                _inputString = tmp;
                _stringPos = tmp.size();
                std::cout << _inputString;
                return PARTIAL_COMPLEMENT;
            } else {
                // 変更がないので候補表示
                std::cout << std::endl;
                printStringList(matchList.begin(), matchList.end());
                std::cout << std::endl;
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

void Console::getInputParameter(std::string& inputString, std::vector<std::string>* tokenList, std::string& lastParam, std::vector<std::string>& paramList) {

    // パラメータは、スペースで区切られる。
    // パラメータリストで取得する際に指定する文字列は入力中のパラメータを含めない。 但し入力中の文字列として取得。
    // 上記についてコマンド名を含めず、最初のスペースで区切られたトークンから最後のスペースで区切られたトークンまでを引数リストに入れる
    // 例：Test ABC DEF G|    "|"をカーソル位置とすると Gは含めない

    if(inputString[inputString.size()-1] == ' ') {
        // 最後のトークンもスペースで区切られている
        std::vector<std::string>::iterator ite = tokenList->begin();
        ite++;
        for(;
                ite != tokenList->end();
                ++ite) {
            paramList.push_back(*ite);
        }
        //lastParam = paramList.back();
    } else {
        // 最後のトークンはスペースで区切られていない
        std::vector<std::string>::iterator ite = tokenList->begin();
        ite++;
        for(;
                ite != tokenList->end();
                ++ite) {
            paramList.push_back(*ite);
        }
        lastParam = paramList.back();
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
    std::cout << std::endl;
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
    size_t num = 80/max;
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

    max += 3;
    size_t num = 80/max;
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
    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _behavior->getParamList(inputtedList, inputting, matchList);
    }
};

//}

#endif /* end of include guard */
