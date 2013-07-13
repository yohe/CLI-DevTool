#include "console.h"
#include <stdio.h>
#include <unistd.h>
#include <algorithm>
#include <sstream>
#include <fstream>

class InsertExeAction : public UserAction {
public:
    InsertExeAction() {}
    virtual ~InsertExeAction() {}

    virtual void operator()() {
        Console* console = get();
        console->actionClearLine();
        console->insertStringToTerminal("exe ");
    }
};

class ExportCommand : public Command {
public:
    ExportCommand() {}
    virtual ~ExportCommand() {}

    virtual std::string getKey() const { return "export"; }
    virtual std::string printHelp() const { return "setting the environment variable.\n   Usage: export VAR=VALUE"; }
    virtual std::string execute(std::string parameter) const {
        parameter = parameter.erase(0, parameter.find_first_not_of(" "));
        parameter = parameter.erase(parameter.find_last_not_of(" ")+1);
        std::string var = parameter.substr(0, parameter.find("="));
        std::string val;
        if(parameter.find("=") != std::string::npos) {
            val = parameter.substr(parameter.find("=")+1);
        }
        if(setenv(var.c_str(), val.c_str(), 1) != 0) {
            return "NG.";
        }
        return "OK.";
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        if(inputting.find("=") == std::string::npos) {
            getEnvironmentVariable(candidates);
            return;
        }
        _behavior.getParamCandidates(inputtedList, inputting, candidates);
    }
    virtual void afterCompletionHook(std::vector<std::string>& candidates) const {
        _behavior.stripParentPath(candidates);
    }

private:

    void getEnvironmentVariable(std::vector<std::string>& candidates) const {
        FILE* in_pipe = NULL;

        std::string cmd = "env";

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
            if(str.find("=") == std::string::npos) {
                continue;
            }
            std::string envVar = str.substr(0, str.find("="));
            candidates.push_back(envVar);
        }
        return ;
    }

    FileListBehavior _behavior;

};

class SampleCommand : public Command {
public:
    SampleCommand() {}
    virtual ~SampleCommand() {}

    virtual std::string getKey() const { return "sample"; }
    virtual std::string printHelp() const { return "sample command."; }
    virtual std::string execute(std::string parameter) const { return parameter; }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
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

    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
        _behavior.getParamCandidates(inputtedList, inputting, matchList);
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
    virtual std::string execute(std::string parameter) const { _console->actionTerminate(); return "end."; }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
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
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
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
        _fileBehavior.getParamCandidates(inputtedList, inputting, matchList);
    }
    
    virtual void afterCompletionHook(std::vector<std::string>& matchList) const {
        _fileBehavior.stripParentPath(matchList);
    }

private:
};

int main(int argc, char const* argv[])
{
    int historySize = 100;
    std::string historyFileName = ".cli_history";

    // Console console;             // use default : historySize = 20, ctrl_c = true, historyFileName = ".cli_history"
    // Console console(history); 
    // Console console(history, ctrl_c);
    Console console(historySize, historyFileName.c_str());

    if(console.installCommand(new ExitCommand()) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }
    if(console.installCommand(new ChangeDirCommand()) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }
    if(console.installCommand(new ExportCommand()) == false) {
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
    if(console.installCommand(new SystemFuncCommand("env", "", new FileListBehavior(), new ManBehavior("env"))) == false) {
        std::cout << "install false" << std::endl;
        return -1;
    }

    console.run();

    return 0;
}

std::string Console::printPromptImpl() const {
    std::string dir = getCurrentDirectory();
    //std::string prompt = "$ ";
    std::string prompt = "[" + _user_name + ":" + dir.substr(dir.rfind("/")+1) + "]$ ";
    return prompt;
}
void Console::printTitle() {
    std::cout << "+------------------------------------+\n";
    std::cout << "|              Console               |\n";
    std::cout << "+------------------------------------+\n";
}

// キー入力発生時に実行する関数を登録
void Console::keyBindInitialize() {

    // Delete Character
    registerKeyBinding(KeyCode::KEY_BS, &Console::actionDeleteBackwardCharacter);
    registerKeyBinding(KeyCode::KEY_BS, &Console::actionDeleteBackwardCharacter);
    registerKeyBinding(KeyCode::KEY_DEL, &Console::actionDeleteForwardCharacter);
    registerKeyBinding(KeyCode::KEY_CTRL_K, &Console::actionDeleteFromCursorToEnd);
    registerKeyBinding(KeyCode::KEY_CTRL_U, &Console::actionDeleteFromHeadToCursor);
    registerKeyBinding(KeyCode::KEY_CTRL_L, &Console::actionClearScreen);
    registerKeyBinding(KeyCode::KEY_CTRL_S, &Console::actionClearLine);
    registerKeyBinding(KeyCode::KEY_CTRL_G, &Console::actionDeleteParam);

    // Complete
    registerKeyBinding(KeyCode::KEY_TAB, &Console::actionComplete);
    registerKeyBinding(KeyCode::KEY_CTRL_SPACE, &Console::actionComplete);

    // History select
    registerKeyBinding(KeyCode::KEY_UP_ARROW, &Console::actionBackwardHistory);
    registerKeyBinding(KeyCode::KEY_ALT_K, &Console::actionBackwardHistory);
    registerKeyBinding(KeyCode::KEY_DOWN_ARROW, &Console::actionForwardHistory);
    registerKeyBinding(KeyCode::KEY_ALT_J, &Console::actionForwardHistory);

    // cursor move
    registerKeyBinding(KeyCode::KEY_RIGHT_ARROW, &Console::actionMoveCursorRight);
    registerKeyBinding(KeyCode::KEY_CTRL_F, &Console::actionMoveCursorRight);
    registerKeyBinding(KeyCode::KEY_LEFT_ARROW, &Console::actionMoveCursorLeft);
    registerKeyBinding(KeyCode::KEY_CTRL_B, &Console::actionMoveCursorLeft);
    registerKeyBinding(KeyCode::KEY_CTRL_A, &Console::actionMoveCursorTop);
    registerKeyBinding(KeyCode::KEY_CTRL_E, &Console::actionMoveCursorBottom);

    UserAction* action = new InsertExeAction();
    registerKeyBinding(KeyCode::KEY_CTRL_W, action);

    // command execute
    registerKeyBinding(KeyCode::KEY_CR, &Console::actionEnter);
    registerKeyBinding(KeyCode::KEY_CTRL_N, &Console::actionMoveCursorForwardParam);
    registerKeyBinding(KeyCode::KEY_CTRL_P, &Console::actionMoveCursorBackwardParam);

    // CTRL-C
    registerKeyBinding(KeyCode::KEY_CTRL_C, &Console::actionTerminate);
}

