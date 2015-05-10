
#include "command/param_comple/tool/directory.h"
#include "command/param_comple/tool/wildcard.h"

#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <stdlib.h>

namespace clidevt {
namespace tool {

DirectoryData::DirectoryData(const char* path) {
    struct dirent **namelist;
    int n = scandir(path, &namelist, NULL, alphasort);

    while(n--) {
        std::string str;
        std::string tmp(path);
        if(tmp.at(tmp.length()-1) != '/') {
            tmp += "/";
        }
        str = tmp + namelist[n]->d_name;
        struct stat st;
        stat(str.c_str(), &st);
        DirectoryEntry entry(str, S_ISDIR(st.st_mode));
        entries_.push_back(entry);
        free(namelist[n]);
    }
    free(namelist);
}
DirectoryData::~DirectoryData() {
}

DirectoryScan::DirectoryScan(const char* path) : directory_(path) {
}

DirectoryScan::iterator DirectoryScan::begin(const char* filter) {
    return iterator(directory_.begin(), directory_.end(), filter);
}

DirectoryScan::iterator DirectoryScan::end() {
    return iterator(directory_.end(), directory_.end(), "");
}

bool DirectoryScan::iterator::isMatch(DirectoryEntry& entry) {
    return wildcard_is_match(filter_, entry.basename());
    //if(filter_.length() == 0) {
    //    return true;
    //}

    //std::string entryName = entry.basename();

    ////if(filter_.length() > entryName.length()) {
    ////    return false;
    ////}

    //size_t filterPos = 0;
    //size_t entryPos = 0;
    ////std::cout << "          -----------"+entryName+"--------------" << std::endl;
    //while(true) {
    //    size_t asterPos = filter_.find("*", filterPos);
    //    //std::cout << "          asterPos = " << asterPos << " filterPos = " << filterPos << " entryPos = " << entryPos << std::endl;
    //    if(asterPos != std::string::npos) {
    //        std::string pertial = filter_.substr(filterPos, asterPos-filterPos);
    //        //std::cout << "         0 "+ pertial << std::endl;
    //        size_t offset = entryName.find(pertial, entryPos);
    //        if(offset == std::string::npos) {
    //            //std::cout << "!        1 "+ entryName << std::endl;
    //            return false;
    //        }
    //        offset += pertial.length();
    //        entryPos+=offset;
    //        filterPos=asterPos+1;
    //    } else {
    //        //std::cout << filter_ << std::endl;
    //        if(filter_.at(filter_.length()-1) == '*') {
    //            //std::cout << "*       2 "+ entryName << std::endl;
    //            return true;
    //        } else {
    //            std::string pertial = filter_.substr(filterPos);
    //            //std::cout << pertial << std::endl;
    //            if(entryName.length() >= pertial.length() &&
    //               entryName.compare(entryName.length()-pertial.length(), pertial.length(), pertial) == 0) {
    //                //std::cout << "*       3 "+ entryName << std::endl;
    //                return true;
    //            } else {
    //                //std::cout << "!       4 " + entryName << std::endl;
    //                return false;
    //            }
    //        }
    //    }
    //}
}

void DirectoryScan::scan(const char* path) {
    
}

}

}

