
#ifndef CLI_DEV_COMMAND_TOKENIZE_ARGUMENT_H
#define CLI_DEV_COMMAND_TOKENIZE_ARGUMENT_H

#include <string>
#include <vector>

namespace clidevt {
    std::vector<std::string>* divideArgumentList(std::string& src);
}

#endif
