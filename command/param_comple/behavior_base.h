
#ifndef CLI_DEV_COMMAND_PARAM_COMPLE_BEHAVIOR_BASE_H
#define CLI_DEV_COMMAND_PARAM_COMPLE_BEHAVIOR_BASE_H

#include <vector>
#include <string>

class ParameterBehavior {
public:
    ParameterBehavior() {}
    virtual ~ParameterBehavior() {}

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const = 0;
};

class NullParameterBehavior : public ParameterBehavior {
public:
    NullParameterBehavior() {}
    virtual ~NullParameterBehavior() {}

    virtual void getParamList(std::vector<std::string>& inputtedList, std::string inputting, std::vector<std::string>& matchList) const { return ; }
};


#endif /* end of include guard */
