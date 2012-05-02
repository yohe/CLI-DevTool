
#include "assert.h"

#include "key_stroke.h"

KeyStrokeEntry::KeyStrokeEntry(char keyCode, std::string strokeName, int actionCode) :
    _keyCode(keyCode), _strokeName(strokeName), _actionCode(actionCode)
{

}

KeyStrokeEntry::~KeyStrokeEntry() {

}

KeyStrokeGroup::KeyStrokeGroup(char groupCode) : KeyStrokeEntry(groupCode, "null", -1) {

} 

KeyStrokeGroup::~KeyStrokeGroup() {

    for(GroupMap::iterator ite = _group.begin(); ite != _group.end(); ++ite) {
        KeyStrokeEntry* entry = ite->second;
        delete entry;
    }

    _group.clear();
}


KeyStrokeEntry* KeyStrokeGroup::getKeyStroke(char keyCode) const {

    GroupMap::const_iterator ite = _group.find(keyCode);
    if(ite == _group.end()) {
        return NULL;
    }

    return ite->second;
}

void KeyStrokeGroup::addKeyStroke(KeyStrokeEntry* entry) {

    GroupMap::iterator ite = _group.find(entry->getKeyCode());
    if(ite == _group.end()) {
        _group.insert(std::pair<char, KeyStrokeEntry*>(entry->getKeyCode(), entry));
        return;
    }

    assert(false);
    //KeyStrokeEntry* entry = ite->second;

    //// KeyStrokeが存在する場合はエラー
    //if(entry->isEntry()) {
    //    assert(false);
    //}

    //KeyStrokeGroup* group = dynamic_cast<KeyStrokeGroup*>(entry);
}

void KeyStrokeGroup::deleteKeyStroke(char keyCode) {

    GroupMap::iterator ite = _group.find(keyCode);
    if(ite == _group.end()) {
        return;
    }

    _group.erase(ite);
    KeyStrokeEntry* entry = ite->second;
    delete entry;
}

