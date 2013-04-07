
#ifndef CLI_DEV_KEY_STROKE_H
#define CLI_DEV_KEY_STROKE_H

#include <string>
#include <vector>
#include <map>

class KeySequenceEntry {
public:
    KeySequenceEntry(char keyCode, std::string strokeName, int actionCode);
    virtual ~KeySequenceEntry();

    char getKeyCode() const { return _keyCode; }
    std::string getStrokeName() const { return _strokeName; }
    int getActionCode() const { return _actionCode; }

    virtual bool isEntry() const { return true; };
    virtual const KeySequenceEntry* getKeySequenceEntry(char keyCode) const {
        if(_keyCode != keyCode) {
            return NULL;
        }
        return this;
    };

protected:
    char _keyCode;
    std::string _strokeName;
    int _actionCode;
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

#endif /* end of include guard */
