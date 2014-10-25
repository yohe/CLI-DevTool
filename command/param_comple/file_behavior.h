
#ifndef CLI_DEV_PCB_H
#define CLI_DEV_PCB_H

#include <stdio.h>
#include <sstream>

#include "command/param_comple/behavior_base.h"

namespace clidevt {

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
    enum CandidateType {
        DIRECTORY = 1,
        EXECUTABLE = 2,
        NON_EXECUTABLE = 4,
        SYMBOLIC_LINK = 8,
        ALL = 15
    };

    FileListBehavior() : candidateType_(ALL) {}
    virtual ~FileListBehavior() {}

    void setCandidatesType(int type) {
        candidateType_ = type;
    }
    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const;

    virtual void stripParentPath(std::vector<std::string>& candidates) const;
    virtual void stripFile(std::vector<std::string>& candidates) const;
private:
    int candidateType_;

};

}

#endif /* end of include guard */

