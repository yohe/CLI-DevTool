
#ifndef CLI_DEV_KEY_MAP_H
#define CLI_DEV_KEY_MAP_H

#include <map>

#include "key_seq.h"

class KeyMap {
public:
    typedef std::map<char, KeySequenceEntry*> GroupMap;
    typedef std::map<KeyCode::Code, std::vector<char> > RegisteredKeyMap;

    KeyMap() ;
    ~KeyMap();

    void addKeyCodeSeq(KeyCode::Code keyCode,const std::vector<char> keySeq);
    void deleteKeyCodeSeq(KeyCode::Code keyCode);

    KeySequenceEntry* getKeyEntry(char seqCode) const;
    KeySequenceEntry* getKeyEntry(const KeyCode::Code keyCode) const;
protected:
    std::vector<char> getKeySequence(const std::string& strokeName) const;

    GroupMap _keyMap;
    RegisteredKeyMap _registeredMap;
};

#endif /* end of include guard */
