
#include "assert.h"

#include "key_seq.h"

KeySequenceEntry::KeySequenceEntry(char endSeqCode, KeyCode::Code keyCode) :
    _sequenceCode(endSeqCode), _virtualKeyCode(keyCode)
{

}

KeySequenceEntry::~KeySequenceEntry() {

}

KeySequenceGroup::KeySequenceGroup(char groupCode) : KeySequenceEntry(groupCode, KeyCode::KEY_NONE) {

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

    GroupMap::iterator ite = _group.find(entry->getSequenceCode());
    if(ite == _group.end()) {
        _group.insert(std::pair<char, KeySequenceEntry*>(entry->getSequenceCode(), entry));
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

