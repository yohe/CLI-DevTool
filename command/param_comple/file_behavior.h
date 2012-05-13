
#ifndef CLI_DEV_PCB_H
#define CLI_DEV_PCB_H

#include <stdio.h>
#include <sstream>

#include "command/param_comple/behavior_base.h"

class FileListBehavior : public ParameterBehavior {
public:
    FileListBehavior() {}
    virtual ~FileListBehavior() {}

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const ;
};

void FileListBehavior::getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const {
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

#endif /* end of include guard */
