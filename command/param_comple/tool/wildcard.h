
#ifndef CLI_DEV_COMMAND_PARAM_COMPLE_TOOL_WILDCARD_H
#define CLI_DEV_COMMAND_PARAM_COMPLE_TOOL_WILDCARD_H

#include <string>

namespace clidevt {
    namespace tool {
        bool wildcard_is_match(std::string condition, const std::string& target);
    }
}

#endif
