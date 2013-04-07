
#include <iostream>
#include <assert.h>

#include "key_map.h"

KeyMap::KeyMap() {

}

KeyMap::~KeyMap() {
    for(GroupMap::iterator ite = _keyMap.begin(); ite != _keyMap.end(); ++ite) {
        KeySequenceEntry* entry = ite->second;
        delete entry;
    }

    _keyMap.clear();
    
}


void KeyMap::addKeyCodeSeq(const std::string& strokeName, std::vector<char> keyStroke, int keyCode) {
#ifdef DEBUG
    std::cout << "[ \"" << strokeName << "\" ]" << "install start." << std::endl;
#endif

    if(keyStroke.size() == 0) {
        return;
    }

    KeySequenceEntry* entry = NULL;

    std::vector<char>::iterator ite = keyStroke.begin(); 
    entry = getKeyEntry(*ite);

    if(entry == NULL) {
        if(keyStroke.size() == 1) {
            _keyMap.insert(std::pair<char, KeySequenceEntry*>(*ite, new KeySequenceEntry(*ite, strokeName, keyCode)));
#ifdef DEBUG
            std::cout << "[ \"" << strokeName << "\" ]" << "installed." << std::endl;
            std::cout << "[ \"" << strokeName << "\" ]" << "install end." << std::endl;
#endif
            _nameMap.insert(std::pair<std::string, std::vector<char> >(strokeName, keyStroke));
            return;
        } else {
            entry = new KeySequenceGroup(*ite);
            _keyMap.insert(std::pair<char, KeySequenceEntry*>(*ite, entry));
#ifdef DEBUG
            std::cout << "group add " << "keyCode = " << (int)*ite << std::endl;
#endif
        }
    }

    ++ite;

    for(;ite != keyStroke.end();++ite) {

        if(entry->isEntry()) {
            assert(false);
        }

        KeySequenceGroup* group = dynamic_cast<KeySequenceGroup*>(entry);
        const KeySequenceEntry* innerEntry;
        innerEntry = group->getKeySequenceEntry(*ite);
        if(innerEntry == NULL) {
            std::vector<char>::iterator endIte = keyStroke.end(); 
            std::advance(endIte, -1);

            // 最後のキーコードであれば KeySequenceEntry
            // 途中のキーコードであれば KeySequenceGroupe
            if(ite == endIte) {
                group->addKeySequence(new KeySequenceEntry(*ite, strokeName, keyCode));
#ifdef DEBUG
                std::cout << "[ \"" << strokeName << "\" ]" << "installed." << std::endl;
#endif
                break;
            } else {
                entry = new KeySequenceGroup(*ite);
                group->addKeySequence(entry);
#ifdef DEBUG
                std::cout << "group add " << "keyCode = " << (int)*ite << std::endl;
#endif
            }
        } else {
            std::vector<char>::iterator next = ite;
            next++;
            if(next == keyStroke.end()) {
                std::cout << "\"" << strokeName << "\"" << " is install error. : conflict keystroke" << std::endl;
                return;
            }
        }
    }

#ifdef DEBUG
    std::cout << "[ \"" << strokeName << "\" ]" << "install end." << std::endl;
#endif
    _nameMap.insert(std::pair<std::string, std::vector<char> >(strokeName, keyStroke));
    return;
}

void KeyMap::deleteKeyCodeSeq(const std::string& strokeName) {
    StrokeNameMap::iterator ite = _nameMap.find(strokeName);
    if( ite == _nameMap.end() ) {
        return;
    }
    std::vector<char> keyStroke(ite->second);
    _nameMap.erase(strokeName);

    std::vector<char>::iterator keyIte = keyStroke.begin();
    KeySequenceEntry* entry = NULL;
    entry = getKeyEntry(*keyIte);
    if(entry->isEntry()) {
        delete entry;
        _keyMap.erase(*keyIte);
        return;
    }
    keyIte++;
    for(; keyIte != keyStroke.end(); ++keyIte) {
        KeySequenceGroup* group = dynamic_cast<KeySequenceGroup*>(entry);
        entry = group->getKeySequenceEntry(*keyIte);
        if(entry->isEntry()) {
            group->deleteKeySequence(entry->getKeyCode());
            break;
        }
    }
}

KeySequenceEntry* KeyMap::getKeyEntry(char keyCode) const {
    GroupMap::const_iterator ite = _keyMap.find(keyCode);
    if(ite == _keyMap.end()) {
        return NULL;
    }

    return ite->second;
}
KeySequenceEntry* KeyMap::getKeyEntry(const std::string& strokeName) const {
    StrokeNameMap::const_iterator ite = _nameMap.find(strokeName);
    if( ite == _nameMap.end() ) {
        return NULL;
    }
    
    std::vector<char> keyStroke(ite->second);
    std::vector<char>::iterator keyIte = keyStroke.begin();
    KeySequenceEntry* entry = getKeyEntry(*keyIte);
    ++keyIte;
    for(; keyIte != keyStroke.end(); ++keyIte) {
        KeySequenceGroup* group = dynamic_cast<KeySequenceGroup*>(entry);
        entry = group->getKeySequenceEntry(*keyIte);
    }
    return entry;
}
