
#include "command/param_comple/tool/wildcard.h"

namespace clidevt {
namespace tool {

bool wildcard_is_match(std::string condition, const std::string& target) {
    if(condition.length() == 0) {
        return true;
    }

    //if(condition.length() > target.length()) {
    //    return false;
    //}

    size_t filterPos = 0;
    size_t entryPos = 0;
    //std::cout << "          -----------"+target+"--------------" << std::endl;
    while(true) {
        size_t asterPos = condition.find("*", filterPos);
        //std::cout << "          asterPos = " << asterPos << " filterPos = " << filterPos << " entryPos = " << entryPos << std::endl;
        if(asterPos != std::string::npos) {
            std::string pertial = condition.substr(filterPos, asterPos-filterPos);
            //std::cout << "         0 "+ pertial << std::endl;
            size_t offset = target.find(pertial, entryPos);
            if(offset == std::string::npos) {
                //std::cout << "!        1 "+ target << std::endl;
                return false;
            }
            offset += pertial.length();
            entryPos+=offset;
            filterPos=asterPos+1;
        } else {
            //std::cout << condition << std::endl;
            if(condition.at(condition.length()-1) == '*') {
                //std::cout << "*       2 "+ target << std::endl;
                return true;
            } else {
                std::string pertial = condition.substr(filterPos);
                //std::cout << pertial << std::endl;
                if(target.length() >= pertial.length() &&
                   target.compare(target.length()-pertial.length(), pertial.length(), pertial) == 0) {
                    //std::cout << "*       3 "+ target << std::endl;
                    return true;
                } else {
                    //std::cout << "!       4 " + target << std::endl;
                    return false;
                }
            }
        }
    }
}

}
}
