
#ifndef CLI_DEV_KEY_MAP_H
#define CLI_DEV_KEY_MAP_H

#include <map>

#include "key_seq.h"

class KeyMap {
public:
    typedef std::map<char, KeySequenceEntry*> GroupMap;
    typedef std::map<std::string, std::vector<char> > StrokeNameMap;

    KeyMap() ;
    ~KeyMap();

    void addKeyCodeSeq(const std::string& strokeName, const std::vector<char> keyStroke, int keyCode);
    void deleteKeyCodeSeq(const std::string& strokeName);

    KeySequenceEntry* getKeyEntry(char keyCode) const;
    KeySequenceEntry* getKeyEntry(const std::string& strokeName) const;
protected:
    std::vector<char> getKeySequence(const std::string& strokeName) const;

    GroupMap _keyMap;
    StrokeNameMap _nameMap;
};

#endif /* end of include guard */
