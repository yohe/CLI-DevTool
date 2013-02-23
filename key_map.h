
#ifndef CLI_DEV_KEY_MAP_H
#define CLI_DEV_KEY_MAP_H

#include <map>

#include "key_stroke.h"

class KeyMap {
public:
    typedef std::map<char, KeyStrokeEntry*> GroupMap;
    typedef std::map<std::string, std::vector<char> > StrokeNameMap;

    KeyMap() ;
    ~KeyMap();

    void addKeyStroke(const std::string& strokeName, const std::vector<char> keyStroke, int actionCode);
    void deleteKeyStroke(const std::string& strokeName);

    KeyStrokeEntry* getKeyEntry(char keyCode) const;
    KeyStrokeEntry* getKeyEntry(const std::string& strokeName) const;
protected:
    std::vector<char> getKeyStroke(const std::string& strokeName) const;

    GroupMap _keyMap;
    StrokeNameMap _nameMap;
};

#endif /* end of include guard */
