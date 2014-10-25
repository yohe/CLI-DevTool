
#include <unistd.h>
#include <dirent.h>

#include <list>
#include <console.h>

#include "command/builtin/shell_exe.h"

namespace clidevt {

void ShellCommandExecutor::execute(std::string param) {
    if(param.empty()) {
        _historyAdd = false;
        return;
    }
    std::string cmd = param.substr(0, param.find(" "));
    const char* argv[64];
    std::list<std::string> delimiterList;
    delimiterList.push_back(" ");
    std::vector<char*> data;
    std::vector<std::string>* paramList = divideStringToVector(param, delimiterList);
    std::vector<std::string>::iterator ite = paramList->begin();
    int i=0;
    for(; ite != paramList->end() && i < 64; ite++, i++) {
        argv[i] = ite->c_str();
    }
    argv[i] = NULL;
    pid_t pid = fork();
    if(pid < 0) {
        perror("fork");
        return;
    } else if(pid == 0) {
        execvp(cmd.c_str(), (char* const *)argv);
        exit(-1);
    }

    int status;
    pid_t r = waitpid(pid, &status, 0);
    delete paramList;
    if(r < 0) {
        perror("waitpid");
        return;
    }
    _historyAdd = true;
}
void ShellCommandExecutor::getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
    if(_initFlag == false) {
        const char* c_env = getenv("PATH");
        std::string env(c_env);
        std::string::size_type pos = 0;
        while(true) {
            std::string::size_type findpos = env.find(':', pos);
            std::string path = env.substr(pos, (findpos-pos));
            if(findpos == std::string::npos || path.empty()) {
                break;
            }
            pos = findpos+1;
            struct dirent** namelist;
            int entrySize = scandir(path.c_str(), &namelist, NULL, alphasort);
            if( entrySize == -1 ) {
                continue;
            }
            if(path.at(path.length()-1) != '/') {
                path+='/';
            }
            for(int i=0; i < entrySize; ++i) {
                std::string fullPath = path + namelist[i]->d_name;
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
void ShellCommandExecutor::afterCompletionHook(std::vector<std::string>& candidates) const {
    FileListBehavior fileListBehavior;
    fileListBehavior.stripParentPath(candidates);
}

}

