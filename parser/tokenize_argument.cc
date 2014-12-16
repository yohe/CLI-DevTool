
#include "parser/tokenize_argument.h"
#include <iostream>

namespace clidevt {

std::vector<std::string>* divideArgumentList(std::string& src) {
    std::vector<std::string>* result = new std::vector<std::string>();

    if(src.empty()){
        return result;
    }

    std::string::iterator begin = src.begin();
    std::string::iterator end = src.end();
    std::string tmp;
    while(begin != end) {
        if(*begin == '"') {
            begin++;
            while(true) {
                if(*begin == '"') {
                    begin++;
                    break;
                }
                if(begin == end) {
                    delete result;
                    throw std::runtime_error("Double quote error");
                }
                if(*begin == '\\') {
                    tmp.append(1,*begin);
                    begin++;
                    if(begin == end) {
                        delete result;
                        throw std::runtime_error("Escape sequence error");
                    }
                }
                tmp.append(1,*begin);
                begin++;
            }
        }
        if(*begin == ' ') {
            result->push_back(tmp);
            tmp.clear();
            while(*begin == ' ') {
                begin++;
            }
            begin--;
        } else {
            tmp.append(1,*begin);
        }
        begin++;
    }
    if(tmp.length() != 0) {
        result->push_back(tmp);
    }

#ifdef DEBUG
    {
        std::cout << src << std::endl;
        std::vector<std::string>::iterator begin = result->begin();
        std::vector<std::string>::iterator end = result->end();
        for(; begin != end; begin++) {
            std::cout << *begin << std::endl;
        }
    }
#endif

    return result;
}

}
