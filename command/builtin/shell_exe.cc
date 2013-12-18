
#include <unistd.h>
#include <dirent.h>

#include "command/builtin/shell_exe.h"

namespace clidevt {

void ShellCommandExecutor::execute(std::string param) {
    if(param.empty()) {
        _historyAdd = false;
        return;
    }
    system(param.c_str()); 
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

