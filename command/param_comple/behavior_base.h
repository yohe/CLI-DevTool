
#ifndef CLI_DEV_COMMAND_PARAM_COMPLE_BEHAVIOR_BASE_H
#define CLI_DEV_COMMAND_PARAM_COMPLE_BEHAVIOR_BASE_H

#include <vector>
#include <string>

namespace clidevt {

class ParameterBehavior {
public:
    ParameterBehavior() {}
    virtual ~ParameterBehavior() {}

    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const = 0;

    virtual void afterCompletionHook(std::vector<std::string>& candidates) const {
    }
};

class NullParameterBehavior : public ParameterBehavior {
public:
    NullParameterBehavior() {}
    virtual ~NullParameterBehavior() {}

    virtual void getParamCandidates(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& candidates) const {
        return;
    }
};

}

#endif /* end of include guard */
