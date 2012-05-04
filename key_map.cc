
#include <iostream>
#include <assert.h>

#include "key_map.h"

KeyMap::KeyMap() {

}

KeyMap::~KeyMap() {
    for(GroupMap::iterator ite = _keyMap.begin(); ite != _keyMap.end(); ++ite) {
        KeyStrokeEntry* entry = ite->second;
        delete entry;
    }

    _keyMap.clear();
    
}


void KeyMap::addKeyStroke(std::string strokeName, std::vector<char> keyStroke, int actionCode) {
#ifdef DEBUG
    std::cout << "[" << strokeName << "]" << "install start." << std::endl;
#endif

    if(keyStroke.size() == 0) {
        return;
    }

    KeyStrokeEntry* entry = NULL;

    std::vector<char>::iterator ite = keyStroke.begin(); 
    entry = getKeyStroke(*ite);

    if(entry == NULL) {
        if(keyStroke.size() == 1) {
            _keyMap.insert(std::pair<char, KeyStrokeEntry*>(*ite, new KeyStrokeEntry(*ite, strokeName, actionCode)));
#ifdef DEBUG
            std::cout << "[" << strokeName << "]" << "installed." << std::endl;
            std::cout << "[" << strokeName << "]" << "install end." << std::endl;
#endif
            return;
        } else {
            entry = new KeyStrokeGroup(*ite);
            _keyMap.insert(std::pair<char, KeyStrokeEntry*>(*ite, entry));
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

        KeyStrokeGroup* group = dynamic_cast<KeyStrokeGroup*>(entry);
        entry = group->getKeyStroke(*ite);
        if(entry == NULL) {
            std::vector<char>::iterator endIte = keyStroke.end(); 
            std::advance(endIte, -1);

            // 最後のキーコードであれば KeyStrokeEntry
            // 途中のキーコードであれば KeyStrokeGroupe
            if(ite == endIte) {
                group->addKeyStroke(new KeyStrokeEntry(*ite, strokeName, actionCode));
#ifdef DEBUG
                std::cout << "[" << strokeName << "]" << "installed." << std::endl;
#endif
                break;
            } else {
                entry = new KeyStrokeGroup(*ite);
                group->addKeyStroke(entry);
#ifdef DEBUG
                std::cout << "group add " << "keyCode = " << (int)*ite << std::endl;
#endif
            }
        }
    }

#ifdef DEBUG
    std::cout << "[" << strokeName << "]" << "install end." << std::endl;
#endif
    _nameMap.insert(std::pair<std::string, std::vector<char> >(strokeName, keyStroke));
    return;
}

void KeyMap::deleteKeyStroke(std::string) {

}

KeyStrokeEntry* KeyMap::getKeyStroke(char keyCode) const {
    GroupMap::const_iterator ite = _keyMap.find(keyCode);
    if(ite == _keyMap.end()) {
        return NULL;
    }

    return ite->second;
}
