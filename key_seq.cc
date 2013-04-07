
#include "assert.h"

#include "key_seq.h"

KeySequenceEntry::KeySequenceEntry(char keyCode, std::string strokeName, int actionCode) :
    _keyCode(keyCode), _strokeName(strokeName), _actionCode(actionCode)
{

}

KeySequenceEntry::~KeySequenceEntry() {

}

KeySequenceGroup::KeySequenceGroup(char groupCode) : KeySequenceEntry(groupCode, "null", -1) {

} 

KeySequenceGroup::~KeySequenceGroup() {

    for(GroupMap::iterator ite = _group.begin(); ite != _group.end(); ++ite) {
        KeySequenceEntry* entry = ite->second;
        delete entry;
    }

    _group.clear();
}


KeySequenceEntry* KeySequenceGroup::getKeySequenceEntry(char keyCode) const {

    GroupMap::const_iterator ite = _group.find(keyCode);
    if(ite == _group.end()) {
        return NULL;
    }

    return ite->second;
}

void KeySequenceGroup::addKeySequence(KeySequenceEntry* entry) {

    GroupMap::iterator ite = _group.find(entry->getKeyCode());
    if(ite == _group.end()) {
        _group.insert(std::pair<char, KeySequenceEntry*>(entry->getKeyCode(), entry));
        return;
    }

    assert(false);
    //KeySequenceEntry* entry = ite->second;

    //// KeySequenceが存在する場合はエラー
    //if(entry->isEntry()) {
    //    assert(false);
    //}

    //KeySequenceGroup* group = dynamic_cast<KeySequenceGroup*>(entry);
}

void KeySequenceGroup::deleteKeySequence(char keyCode) {

    GroupMap::iterator ite = _group.find(keyCode);
    if(ite == _group.end()) {
        return;
    }

    _group.erase(ite);
    KeySequenceEntry* entry = ite->second;
    delete entry;
}

