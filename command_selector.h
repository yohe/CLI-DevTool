#ifndef _COMMAND_SELECTOR_H_
#define _COMMAND_SELECTOR_H_

#include <map>
#include <string>
#include <list>

#include "console.h"

class CommandSelector {
public:
    typedef std::map<std::string, Command*> CommandSet;

    CommandSelector() {}
    virtual ~CommandSelector() {
        for(CommandSet::iterator ite = _commandSet.begin();
            ite != _commandSet.end();
            ++ite) {
            Command* cmd = ite->second;
            delete cmd;
        }
        _commandSet.clear();
    }

    virtual Command* getCommand(const std::string& input) = 0;
    virtual std::string errorCause() const { return _errorCause; }
    virtual bool registCommand(Command* command) = 0;
    virtual Command* unregistCommand(const std::string& key) = 0;
    virtual const CommandSet& getCommandSet() { return _commandSet; }

protected:
    std::string _errorCause;
    CommandSet _commandSet;
};

// デフォルトのコマンドセレクタ
// コマンド名を適切に入力
class DefaultCommandSelector : public CommandSelector {

public:
    DefaultCommandSelector() {}
    virtual ~DefaultCommandSelector() {}

    virtual Command* getCommand(const std::string& input) {
        if(_commandSet.count(input) == 0) {
            _errorCause = input + ": Command not found.";
            return NULL;
        }

        return _commandSet.find(input)->second;
    }

    virtual bool registCommand(Command* command) {
        if(_commandSet.count(command->getKey()) == 1) {
            _errorCause = command->getKey() + ": Command is already registed.";
            return false;
        }

        //command->setConsole(console);
        _commandSet.insert(std::pair<std::string, Command*>(command->getKey(), command));
        return true;
    }

    virtual Command* unregistCommand(const std::string& key) {
        if(_commandSet.count(key) == 0) {
            return NULL;
        }
        Command* cmd = _commandSet.find(key)->second;
        _commandSet.erase(key);

        return cmd;
    }

};

// 短縮形コマンド名に対応したコマンドセレクタ
class AbbreviatedCommandSelector : public DefaultCommandSelector {
public:
    AbbreviatedCommandSelector() {}
    virtual ~AbbreviatedCommandSelector() {}

    virtual Command* getCommand(const std::string& input) {
        std::list<Command*> cmdList;
        for(CommandSet::iterator ite = _commandSet.begin();
            ite != _commandSet.end();
            ++ite) {

            Command* cmd = (*ite).second;
            if(cmd->getKey().compare(0, input.size(), input) == 0) {
                cmdList.push_back(cmd);
            }
        }

        if(cmdList.size() == 1) {
            return cmdList.front();
        } else if(cmdList.empty()) {
            _errorCause = input + "* is not found.";
            return NULL;
        }

        std::stringstream ss;
        ss << input <<  " is not unique keyword. Command candidate is.." << std::endl;
        int i = 1;
        for(std::list<Command*>::iterator ite = cmdList.begin();
            ite != cmdList.end();
            ++ite) {
            ss << "[" << i << "] : " << (*ite)->getKey() << std::endl;
            i++;
        }
        _errorCause = ss.str();
        return NULL;
    }

};


#endif /* end of include guard */
