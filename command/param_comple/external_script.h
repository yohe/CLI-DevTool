
#ifndef CLI_DEV_COMMAND_PARAM_COMPLE_EXTERNAL_SCRIPT_H
#define CLI_DEV_COMMAND_PARAM_COMPLE_EXTERNAL_SCRIPT_H

#include <vector>
#include <string>

#include "command/param_comple/behavior_base.h"

namespace clidevt {

class ExternalScriptBehavior : public ParameterBehavior {
    std::string candidates_;
    std::string output_;
public:
    ExternalScriptBehavior(const std::string& candidates, const std::string output = "") : candidates_(candidates), output_(output) {}
    virtual ~ExternalScriptBehavior() {}

    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const;

    virtual void afterCompletionHook(std::vector<std::string>& candidates) const;
};


}

#endif /* end of include guard */
