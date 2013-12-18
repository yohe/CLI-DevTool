
#ifndef CLI_DEV_KEY_STROKE_H
#define CLI_DEV_KEY_STROKE_H

#include <string>
#include <vector>
#include <map>

#include "key_code.h"

namespace clidevt {

class KeySequenceEntry {
public:
    KeySequenceEntry(char endSeqCode, KeyCode::Code keyCode);
    virtual ~KeySequenceEntry();

    char getSequenceCode() const { return _sequenceCode; }
    //std::string getStrokeName() const { return _strokeName; }
    KeyCode::Code getVirtualKeyCode() const { return _virtualKeyCode; }

    virtual bool isEntry() const { return true; };
    virtual const KeySequenceEntry* getKeySequenceEntry(char sequenceCode) const {
        if(_sequenceCode != sequenceCode) {
            return NULL;
        }
        return this;
    };

protected:
    char _sequenceCode;
    //std::string _strokeName;
    KeyCode::Code _virtualKeyCode;
};

class KeySequenceGroup: public KeySequenceEntry {
    typedef std::map<char, KeySequenceEntry*> GroupMap;
public:
    KeySequenceGroup(char groupCode);
    virtual ~KeySequenceGroup();

    virtual bool isEntry() const { return false; };
    virtual KeySequenceEntry* getKeySequenceEntry(char keyCode) const;

    void addKeySequence(KeySequenceEntry* entry);
    void deleteKeySequence(char keyCode);
protected:
     GroupMap _group;
};

}

#endif /* end of include guard */
