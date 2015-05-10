#include "command/param_comple/file_behavior.h"
#include "command/param_comple/tool/wildcard.h"
#include <iostream>

namespace clidevt {

void FileListBehavior::getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        FILE* in_pipe = NULL;
        CandidateType type;

        std::string path("");
        std::string file;
        if(inputting.rfind('/') == std::string::npos) {
            file = inputting;
        } else {
            path = inputting.substr(0,inputting.rfind('/')+1);
            file = inputting.substr(inputting.rfind('/')+1);
        }

        std::string cmd = "ls -1LFa " + path + " 2> /dev/null";

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
        inputting+="*";
        pclose(in_pipe);
        while(!paramList.eof()) {
            std::string str;
            std::getline(paramList, str);
            if(str.empty()) {
                continue;
            }

            if(!file.empty() and file.at(0) == '.') {
                if(str.at(0) != '.') continue;
            } else {
                if(str.at(0) == '.') continue;
            }
            
            SpecialCharEscaper e;
            std::string name = path;
            name += e(str);
            if (tool::wildcard_is_match(inputting, name) == false){
                continue;
            } else {
                switch (name[name.size()-1]) {
                case '*':
                    type = EXECUTABLE;
                    name.erase(name.size()-1);
                    name += ' ';
                    break;
                case '/':
                    type = DIRECTORY;
                    break;
                case '|':
                    type = NON_EXECUTABLE;
                    name.erase(name.size()-1);
                    break;
                case '@':
                    type = SYMBOLIC_LINK;
                    name[name.size()-1] = ' ';
                    break;
                default:
                    type = NON_EXECUTABLE;
                    name += " ";
                    break;
                }
            }
            if(candidateType_ & type) {
                candidates.push_back(name);
            }
        }
        return ;
}

void FileListBehavior::stripParentPath(std::vector<std::string>& candidates) const {
    std::vector<std::string>::iterator it;
    std::vector<std::string> after;
    for(it = candidates.begin(); it != candidates.end(); ++it) {
        size_t pos = it->find_last_of("/", it->length()-2);
        std::string name = it->substr(pos+1);
        if(!name.empty()) {
            after.push_back(name);
        }
    }
    candidates.swap(after);
}
void FileListBehavior::stripFile(std::vector<std::string>& candidates) const {
    std::vector<std::string>::iterator it;
    std::vector<std::string> after;
    for(it = candidates.begin(); it != candidates.end(); ++it) {
        if(it->at(it->length()-1) == '/') {
            size_t pos = it->find_last_of("/", it->length()-2);
            std::string name = it->substr(pos+1);
            if(name == "./" || name == "../") {
                continue;
            }
            after.push_back(name);
        }
    }
    candidates.swap(after);
}

}

