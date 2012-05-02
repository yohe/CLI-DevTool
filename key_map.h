
#ifndef KEY_MAP_H
#define KEY_MAP_H

#include <map>

#include "key_stroke.h"

class KeyMap {
public:
    typedef std::map<char, KeyStrokeEntry*> GroupMap;
    typedef std::map<std::string, std::vector<char> > StrokeNameMap;

    KeyMap() ;
    ~KeyMap();

    void addKeyStroke(const std::string strokeName, const std::vector<char> keyStroke, int actionCode);
    void deleteKeyStroke(const std::string strokeName);

    KeyStrokeEntry* getKeyStroke(char keyCode) const;
protected:

    GroupMap _keyMap;
    StrokeNameMap _nameMap;
};

#endif /* end of include guard */
