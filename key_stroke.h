
#ifndef CLI_DEV_KEY_STROKE_H
#define CLI_DEV_KEY_STROKE_H

#include <string>
#include <vector>
#include <map>

class KeyStrokeEntry {
public:
    KeyStrokeEntry(char keyCode, std::string strokeName, int actionCode);
    virtual ~KeyStrokeEntry();

    char getKeyCode() const { return _keyCode; }
    std::string getStrokeName() const { return _strokeName; }
    int getActionCode() const { return _actionCode; }

    virtual bool isEntry() const { return true; };
    virtual const KeyStrokeEntry* getKeyStroke(char keyCode) const {
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

class KeyStrokeGroup : public KeyStrokeEntry {
    typedef std::map<char, KeyStrokeEntry*> GroupMap;
public:
    KeyStrokeGroup(char groupCode);
    virtual ~KeyStrokeGroup();

    virtual bool isEntry() const { return false; };
    virtual KeyStrokeEntry* getKeyStroke(char keyCode) const;

    void addKeyStroke(KeyStrokeEntry* entry);
    void deleteKeyStroke(char keyCode);
protected:
     GroupMap _group;
};



#endif /* end of include guard */
