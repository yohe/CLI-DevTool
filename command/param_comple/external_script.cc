
#include "command/param_comple/external_script.h"
#include <fstream>
#include <sstream>
#include <iostream>

namespace clidevt {

void ExternalScriptBehavior::getParamCandidates(std::vector<std::string>& inputtedList,
                                                std::string inputting,
                                                std::vector<std::string>& candidates) const {
    FILE* in_pipe = NULL;
    std::ifstream ifs(candidates_.c_str());
    if(!ifs) {
        std::cout << candidates_ + ": script not found." << std::endl;
        return;
    }

    in_pipe = popen(candidates_.c_str(), "r");
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
        std::getline(paramList, str);
        if(str.empty()) {
            continue;
        }
        candidates.push_back(str+" ");
    }
    return ;

}

void ExternalScriptBehavior::afterCompletionHook(std::vector<std::string>& candidates) const {
    if(output_.length() == 0) {
        return;
    }
    FILE* in_pipe = NULL;
    std::ifstream ifs(output_.c_str());
    if(!ifs) {
        std::cout << output_ + ": script not found." << std::endl;
        return;
    }

    in_pipe = popen(output_.c_str(), "r");
    std::stringstream paramList("");
    char buffer[512+1] = {0};
    if(in_pipe != NULL) {
        size_t readed_num = fread(buffer, sizeof(char), 512, in_pipe);
        while(readed_num > 0) {
            paramList.write(buffer, readed_num);
            readed_num = fread(buffer, sizeof(char), 512, in_pipe);
        }
    }

    candidates.clear();
    pclose(in_pipe);
    while(!paramList.eof()) {
        std::string str;
        std::getline(paramList, str);
        if(str.empty()) {
            continue;
        }
        candidates.push_back(str+" ");
    }
    return ;
}

}
