#include "console.h"
#include "mode/mode.h"
#include "mode/builtin/builtin.h"
#include "parser/input_parser.h"

#include "command/param_comple/file_behavior.h"

namespace clidevt {

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

bool Console::initialize(int argc, char const* argv[]) {
    _consolePath = argv[0];
    if(setupterm(NULL, fileno(stdout), (int*)0) == ERR) {
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
    //installCommand(new BuiltInScriptCommand());
    installCommand(new BuiltInModeSelectCommand());

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


    setMode(new NormalMode());
    registMode(getCurrentMode());
    registMode(new LoggingMode());

    return true;
}

void Console::printPrompt() {
    printf("\r");
    std::string str = printPromptImpl();
    Mode* mode = getCurrentMode();
    if(mode->getFlags() & PROMPT_DISPLAY_HOOK) {
        mode->hookPromptDisplay(str, this);
    } else {
        std::cout << "\x1b[36m" << str << "\x1b[39m";
    }
    _inputString = "";
    _stringPos = 0;
}

void Console::run() {
    printTitle();
    printPrompt();
    _inputString ="";
    _stringPos=0;
    _historyIndex = 0;

    std::pair<bool, Action*> ret;
    NormalMode normal_mode;

    while(!isEnd()) {
        Mode* mode = getCurrentMode();
        char input = fgetc(stdin);
        if(mode->getFlags() & INPUT_KEY_HOOK) {
            ret = mode->hookInputKey(input, this);
        } else {
            ret = normal_mode.hookInputKey(input, this);
        }
#ifdef KEY_TRACE
        std::cout << (int)input << " "; // for key trace 
        if(input == 3) {
            return;
        }
#endif
        if(ret.first) {
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
        
#ifndef KEY_TRACE

        if(ret.second != NULL) {
            Action* action = ret.second;
            (*action)();
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

void Console::actionExecuteCommandLine() {
    if(getCurrentMode()->getFlags() & EXECUTE_CMD_LINE_BEFORE) {
        getCurrentMode()->hookExecuteCommandLineBefore(getInputtingString(), this);
    }
    execute(getInputtingString());
    if(getCurrentMode()->getFlags() & EXECUTE_CMD_LINE_AFTER) {
        getCurrentMode()->hookExecuteCommandLineAfter(this);
    }
    clearStatus();
    if(!isEnd()) {
        printPrompt();
    }
    if(getCurrentMode()->getFlags() & PREPARE_INSERT_STR) {
        getCurrentMode()->hookPrepareInsert(this);
    }
    return;
}

void Console::actionTerminate() {
    Mode* mode = getCurrentMode();
    if(mode->getName() != "normal") {
        BuiltInModeSelectCommand cmd;
        cmd.setConsole(this);
        cmd.execute("normal");
        return;
    }
    if(_isTerminatePermit) {
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

std::string Console::getDateString() const {
    time_t sec;
    time(&sec);
    return ctime(&sec);
}

void Console::insertStringToTerminal(const std::string& str) {
    clearLine(false);
    _inputString.insert(_stringPos, str);
    _stringPos += str.length();
    std::cout << _inputString;
    setCursorPos(_stringPos);
}
void Console::printStringOnTerminal(const std::string& str) {
    std::cout << str;
    _stringPos += str.length();
}

void Console::executeStatement(const Statement& statement) {

    const std::string& inputString = statement.getString();
    bool isSystem = false;
    std::string key;
    std::string argument;

    if(inputString.compare(0, 2, "./") == 0) {
        isSystem = true;
    } else {
        // コマンド名、引数に分離
        size_t sp = inputString.find(' ');
        key = inputString.substr(0, sp);
        argument = "";
        if(sp != std::string::npos) {
            argument = inputString.substr(sp);
            argument = argument.erase(0, argument.find_first_not_of(" "));
            argument = argument.erase(argument.find_last_not_of(" ")+1);
        }

        if(key.empty()) {
            return;
        }
    }

    if(isSystem) {
#ifdef SHELL_SUPPORT
        executeShellCommand(inputString);
        addHistory(inputString);
#endif
    } else {
        Command* cmd = getCommand(key);
        Mode* mode = getCurrentMode();
        if(mode->getFlags() & EXECUTE_CMD_BEFORE) {
            mode->hookExecuteCmdBefore(cmd, this);
        }
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
        if(mode->getFlags() & EXECUTE_CMD_AFTER) {
            mode->hookExecuteCmdAfter(cmd, this);
        }
    }

}

void Console::execute(const std::string& inputString) {
    std::cout << std::endl;

    // ターミナル状態をもとに戻す
    setTermIOS(_save_term);

    DefaultParser parser;
    try {
        std::vector<Statement> statements = parser.parse(inputString);
#ifdef DEBUG
        typedef std::vector<Statement>::iterator Iterator;
        Iterator begin = statements.begin();
        Iterator end = statements.end();
        std::cout << "-----------------------------" << std::endl;
        for(; begin != end; begin++) {
            std::cout << begin->getString() << std::endl;
        }
        std::cout << "-----------------------------" << std::endl;
#endif
        execute(statements.begin(), statements.end());
        if(statements.size() > 1) {
            addHistory(inputString);
        }
    } catch (SyntaxError& e) {
        std::cout << e.what() << std::endl;
    }

    // ターミナル状態をもとに戻す
    setTermIOS(_term_setting);
}
void Console::execute(StatementIterator begin, StatementIterator end) {
    if(begin == end) {
        return;
    }
    pid_t pid = begin->setupOfExecution();
    if(begin->isParent(pid) == true) {
        begin->setupRedirection();
        executeStatement(*begin);
        begin->teardownRedirection();
        //std::cout << "executeStatement end" << std::endl;
    }
    if(begin->isChild(pid) == true) {
        // child process
        StatementIterator next = begin + 1;
        execute(next , end);
    }
    begin->teardownOfExecution();
}

void Console::executeShellCommand(const std::string& inputString) {

    std::string tmp;
    ShellCommandExecutor executor(tmp);
    executor.execute(inputString);
    addHistory(inputString);

}
void Console::executeCommand(Command* cmd, const std::string& argument) {

    std::string ret("");
    if(cmd == NULL) {
        ret =  _commandSelector->errorCause();
        std::cout << ret << std::endl;
    } else {
        cmd->execute(argument);
    }

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

    std::string fixedInput = _inputString.substr(0,_stringPos);

    DefaultSentenceTerminatorLex stLex;
    DefaultPipeLex pipeLex;

    std::vector<SyntaxToken> ret = stLex.split(fixedInput);
    if(ret.size() > 0) {
        if(ret[ret.size()-1].type() != SentenceTerminator) {
            std::string tmp = ret[ret.size()-1].value();
            ret.clear();
            ret = pipeLex.split(tmp);
        }
        if(ret[ret.size()-1].type() == Pipe || ret[ret.size()-1].type() == SentenceTerminator) {
            std::string tmp = _inputString;
            executeComplete("");
            _inputString = tmp;
            _stringPos = tmp.size();
            std::cout << tmp;
        } else {
            //std::cout << ret[ret.size()-1].value() + ", type=" << ret[ret.size()-1].type() << std::endl;
            executeComplete(ret[ret.size()-1].value());
        }
    } else {
        executeComplete("");
    }


}

void Console::executeComplete(std::string input) {
    // コマンド名を入力中であればコマンド名を補完する
    // コマンド名が確定している場合はパラメータ補完

    //std::cout << "--------- input = [" + input + "]" << std::endl;
    // コマンド名が空の場合は全てのコマンド名を表示
    if(input.empty()) {
        std::cout << std::endl;
        // 文字列が入力されていないので、全コマンドをリストアップ
        printAllCommandName();
        std::cout << std::endl;
        printPrompt();

        return;
    }

    bool ret = false;
    bool cmdUse = true;
    std::string param = "";
    std::string after = "";
    Command* cmd = NULL;
    std::vector<std::string> argumentList;
    FileListBehavior fileCompl;
    if(input.compare(0, 2, "./") == 0) {
#ifdef SHELL_SUPPORT
        cmdUse = false;
        //std::string fixedInput = input.substr(0, _stringPos);
        std::string fixedInput = input;
        std::list<std::string> delimiterList;
        delimiterList.push_back(" ");
        std::vector<std::string>* tokenList = divideStringToVector(fixedInput, delimiterList);

        std::vector<std::string> candidates;

        if(fixedInput[fixedInput.size()-1] == ' ') {
            fileCompl.setCandidatesType(FileListBehavior::ALL);
        } else {
            param = tokenList->back();
            tokenList->pop_back();
            if(tokenList->size() == 0) {
                fileCompl.setCandidatesType(
                        FileListBehavior::EXECUTABLE |
                        FileListBehavior::DIRECTORY |
                        FileListBehavior::SYMBOLIC_LINK
                        );
            } else {
                fileCompl.setCandidatesType(FileListBehavior::ALL);
            }
        }
        after = param;
        fileCompl.getParamCandidates(*tokenList, param, candidates);
        std::vector<std::string>::iterator begin = candidates.begin();
        std::vector<std::string>::iterator end = candidates.end();
        ret = completeStringList(after, argumentList, begin, end);
        delete tokenList;
#endif
    } else {
        //std::cout << "point A" << std::endl;
        if(input.find(" ") == std::string::npos) {
            //std::cout << "point C" << std::endl;
            if(completeCommandName(input) == ERROR) {
                assert(false);
            }
            return;
        }

        // 以降ではコマンド名が確定している場合の処理

        std::string fixedInput = input;
        //std::string fixedInput = input.substr(0, _stringPos);
        std::list<std::string> delimiterList;
        delimiterList.push_back(" ");
        std::vector<std::string>* tokenList = divideStringToVector(fixedInput, delimiterList);

        cmd = getCommand((*tokenList)[0]);
        if(cmd == NULL) {
            delete tokenList;
            return;
        }

        //std::cout << "point B" << std::endl;
        //std::cout << tokenList->size() << std::endl;
        // トークンリストが 1 つまりコマンド名のみである場合は、パラメータリストを表示して終了
        if(tokenList->size() == 1) {
            std::cout << std::endl;
            std::vector<std::string> argumentList_;
            std::string param = "";
            std::vector<std::string> candidates;
            cmd->getParamCandidates(argumentList_, param, candidates);
            cmd->afterCompletionHook(candidates);
            printStringList(candidates.begin(), candidates.end());
            size_t pos = _stringPos;
            clearLine(false);
            _stringPos = 0;
            printStringOnTerminal(_inputString);
            setCursorPos(pos);
            delete tokenList; tokenList = NULL;
            return;
        }


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

        after = param;
        argumentList.clear();
        std::vector<std::string>::iterator begin = candidates.begin();
        std::vector<std::string>::iterator end = candidates.end();
        ret = completeStringList(after, argumentList, begin, end);
    }


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
            printStringOnTerminal(_inputString);
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
                    printStringOnTerminal(_inputString);
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
                        printStringOnTerminal(_inputString);
                        setCursorPos(cursorPos);
                        return;
                    } else {
                        //beep();
                        return;
                    }
                }
            } else {
                // 変更がないので候補表示
                std::cout << std::endl;
                if(cmdUse) {
                    cmd->afterCompletionHook(argumentList);
                } else {
                    fileCompl.stripParentPath(argumentList);
                }
                printStringList(argumentList.begin(), argumentList.end());
                size_t cursorPos = _stringPos;
                clearLine(false);
                _stringPos = 0;
                printStringOnTerminal(_inputString);
                setCursorPos(cursorPos);
                return;
            }
        } else {
            // 補完候補なし
            //beep();
            return;
        }
    }
    
    return;
}

Console::CompletionType Console::completeCommandName(const std::string& input) {

    std::vector<std::string> candidates;
    std::string tmp = input;
    // ret = true : コマンド名を完全補完
    // ret = false: コマンド名を一部補完 or 補完候補なし
    bool ret = completeCommand(tmp, candidates);
    std::string diff = tmp.substr(input.size());
    //std::cout << "[" + diff + "]" << std::endl;
    if( ret ) {
        // 完全補完
        redraw();
        //clearLine(false);
        //std::cout << _inputString;
        insertStringToTerminal(diff+" ");
        return FULL_COMPLETE;
    } else {
        // 一部補完 or 補完候補なし
        // candidates.size() > 0 である場合、一部補完と判断
        if(candidates.size() > 0) {
            // 補完が行われた場合は、そのまま表示
            // 変更がない場合は、候補を表示する。
            if(tmp != input) {
                redraw();
                //clearLine(false);
                //std::cout << "["+_inputString+"]" << std::endl;;
                insertStringToTerminal(diff);
                //_inputString = tmp;
                //_stringPos = tmp.size();
                return PARTIAL_COMPLETE;
            } else {
                // 変更がないので候補表示
                std::cout << std::endl;
                printStringList(candidates.begin(), candidates.end());
                //printPrompt();
                clearLine(false);
                std::cout << _inputString;
                //_inputString = tmp;
                //_stringPos = tmp.size();
                //std::cout << _inputString;
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

void Console::printAllHistory(HistoryFilter* filter) {
    size_t count = _history.size()-1;
    for(std::deque<std::string>::reverse_iterator ite = _history.rbegin();
        ite != _history.rend();
        ++ite) {
        if(filter->isSkip(*ite) == false) {
            std::cout << "[" << count << "] : " << *ite << std::endl;
        }
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
    while(lh != rh) {
        std::cout << std::left << std::setw(max) << *lh;
        ++i;
        ++lh;
        if(i > num && lh != rh) {
            std::cout << std::endl;
            i = 0;
        }
    }
    std::cout << std::endl;
}

std::vector<std::string>* divideStringToVector(std::string& src, std::list<std::string>& delimiter) {

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

bool Console::registMode(Mode* mode) {
    if(_modeMap.count(mode->getName()) != 0) {
        return false;
    }

    _modeMap.insert(std::make_pair(mode->getName(), mode));
    return true;
}
Mode* Console::unregistMode(std::string name) {
    if(_modeMap.count(name) == 0) {
        return NULL;
    }

    Mode* mode = _modeMap[name];
    _modeMap.erase(name);
    return mode;
}

}
