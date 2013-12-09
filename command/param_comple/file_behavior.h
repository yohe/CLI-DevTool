
#ifndef CLI_DEV_PCB_H
#define CLI_DEV_PCB_H

#include <stdio.h>
#include <sstream>

#include "command/param_comple/behavior_base.h"

class FileListBehavior : public ParameterBehavior {
    class SpecialCharEscaper {
    public:
        std::string operator()(std::string str) {
            std::string ret;
            std::string::iterator ite = str.begin();
            for(; ite != str.end(); ++ite) {
                switch(*ite) {
                    case ' ':
                        ret += "\\ ";
                        break;
                    case '\\':
                        ret += "\\\\";
                        break;
                    case '|':
                        ret += "\\|";
                        break;
                    case '<':
                        ret += "\\<";
                        break;
                    case '>':
                        ret += "\\>";
                        break;
                    case '\"':
                        ret += "\\\"";
                        break;
                    case '\'':
                        ret += "\\\'";
                        break;
                    default:
                        ret += *ite;
                        break;
                }
            }
            return ret;
        }
    };
public:
    FileListBehavior() {}
    virtual ~FileListBehavior() {}

    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const ;
    
    virtual void stripParentPath(std::vector<std::string>& candidates) const;
    virtual void stripFile(std::vector<std::string>& candidates) const;
};

#endif /* end of include guard */

