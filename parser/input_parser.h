#ifndef CLI_DEV_COMMAND_INPUT_PARSER_H
#define CLI_DEV_COMMAND_INPUT_PARSER_H

#include <string>
#include <vector>
#include <unistd.h>
#include <dirent.h>
#include <sys/wait.h>
#include <iostream>
#include <stdexcept>

namespace clidevt {

enum SupportSemantic {
    InputRedirection = 1,
    OutputRedirection,
    Pipe,
    SentenceTerminator,
    OneSentence,
};

class SyntaxToken {
public:
    SyntaxToken(SupportSemantic type, const std::string value) : 
        tokenType_(type), value_(value)
    {
    }
    SyntaxToken(const SyntaxToken& rhs) : tokenType_(rhs.tokenType_), value_(rhs.value_) {
    }

    int type() const { return tokenType_; }
    std::string value() const { return value_; }

private:
    SupportSemantic tokenType_;
    std::string value_;
};

template <class RedirectionLexer, class PipeLexer, class SentenceLexer>
class Tokenizer {
    std::vector<SyntaxToken> token_;
public:
    Tokenizer() {}
    const std::vector<SyntaxToken>& tokenize(const std::string& input);
};

template <class RL, class PL, class SL>
const std::vector<SyntaxToken>&
Tokenizer<RL,PL,SL>::tokenize(const std::string& input) {
    token_.clear();
    typedef SL SentenceLexer;
    SentenceLexer sl;
    std::vector<SyntaxToken> v = sl.split(input);
    typedef PL PipeLexer;
#ifdef REDIRECTION_SUPPORT
    typedef RL RedirectionLexer;
#endif
    PipeLexer pl;
    typedef std::vector<SyntaxToken>::iterator Iterator;
    for (Iterator vIte = v.begin(); vIte != v.end(); vIte++ ) {
        if(vIte->type() == SentenceTerminator) {
            token_.push_back(*vIte);
            continue;
        }

        std::vector<SyntaxToken> vp = pl.split(vIte->value());
        for (Iterator vpIte = vp.begin(); vpIte != vp.end(); vpIte++ ) {
            if(vpIte->type() == Pipe) {
                token_.push_back(*vpIte);
                continue;
            }
#ifdef REDIRECTION_SUPPORT
            RedirectionLexer rl;
            std::vector<SyntaxToken> vr = rl.split(vpIte->value());
            token_.insert(token_.end(), vr.begin(), vr.end());
#else
            token_.push_back(*vpIte);
#endif
        }
    }

    return token_;
}

class Statement {
    std::string statement_;
    bool inputR_;
    std::string inputFile_;
    bool outputR_;
    std::string outputFile_;
    bool pipe_;
    int pipefd_[2];
    pid_t child_;
    int stdinBackup_;
    int stdoutBackup_;
    int stderrBackup_;
    bool hasChild_;
    bool isChild_;

    FILE* inputFileP_;
    FILE* outputFileP_;

public:
    void setupRedirection();
    void teardownRedirection();
    Statement(const std::string& st) : 
        statement_(st), inputR_(false),
        outputR_(false), pipe_(false), child_(0), hasChild_(false), isChild_(false),
        inputFileP_(NULL), outputFileP_(NULL)
    {
    }

    void setInputRedirection(bool flag, std::string file) {
        inputR_ = flag;
        inputFile_ = file;
    }
    void setOutputRedirection(bool flag, std::string file) {
        outputR_ = flag;
        outputFile_ = file;
    }
    void setPiped(bool flag) {
        pipe_ = flag;
    }
    bool isPipe() const {
        return pipe_;
    }
    const std::string& getString() const {
        return statement_;
    }

    bool isParent(pid_t pid) {
        if(pipe_ == false) {
            return true;
        }
        return child_ != 0;
    }
    bool isChild(pid_t pid) {
        if(pipe_ == false) {
            return true;
        }
        return child_ == 0;
    }

    pid_t setupOfExecution(); 
    void teardownOfExecution();
};

class SyntaxError : public std::runtime_error {
public:
    SyntaxError(const std::string& what) : std::runtime_error(what) {
    }
};

template <class Tokenizer>
class InputParser {
    enum state {
        INIT = 0,
        NORMAL,
        TERMINATED,
        PIPED,
        IN_REDIRECTED,
        OUT_REDIRECTED,
    };
public:
    InputParser() : tokenizer_() {}

    std::vector<Statement> parse(std::string input) {
        std::vector<Statement> ret;

        const std::vector<SyntaxToken>& tokens = tokenizer_.tokenize(input);
        typedef std::vector<SyntaxToken> SytaxTokens;
        SytaxTokens::const_iterator ite = tokens.begin();
        int state = INIT;
        for(; ite != tokens.end(); ite++) {
            switch (state) {
            case INIT:
                if(ite->type() == OneSentence) {
                    state = NORMAL;
                    Statement st(ite->value());
                    ret.push_back(st);
                } else if(ite->type() == SentenceTerminator) {
                    state = TERMINATED;
                    continue;
                } else {
                    //std::cout << "point a" << std::endl;
                    throw SyntaxError("SyntaxError");
                }
                break;
            case NORMAL:
                if(ite->type() == OneSentence) {
                    state = NORMAL;
                    Statement st(ite->value());
                    ret.push_back(st);
                } else if(ite->type() == InputRedirection) {
                    state = IN_REDIRECTED;
                } else if(ite->type() == OutputRedirection) {
                    state = OUT_REDIRECTED;
                } else if(ite->type() == Pipe) {
                    state = PIPED;
                    ret[ret.size()-1].setPiped(true);
                } else{
                    state = TERMINATED;
                    continue;
                }
                break;
            case TERMINATED:
                if(ite->type() == OneSentence) {
                    std::string token = ite->value();
                    token = token.erase(0, token.find_first_not_of(" "));
                    state = NORMAL;
                    Statement st(token);
                    ret.push_back(st);
                } else if(ite->type() == SentenceTerminator) {
                } else {
                    //std::cout << "point b" << std::endl;
                    throw SyntaxError("SyntaxError");
                }
                break;
            case PIPED:
                if(ite->type() == OneSentence) {
                    std::string token = ite->value();
                    token = token.erase(0, token.find_first_not_of(" "));
                    state = NORMAL;
                    Statement st(token);
                    ret.push_back(st);
                } else {
                    //std::cout << "point c" << std::endl;
                    throw SyntaxError("SyntaxError");
                }
                break;
            case IN_REDIRECTED:
                if(ite->type() == OneSentence) {
                    state = NORMAL;
                    ret[ret.size()-1].setInputRedirection(true, ite->value());
                } else {
                    //std::cout << "point d" << std::endl;
                    throw SyntaxError("SyntaxError");
                }
            case OUT_REDIRECTED:
                if(ite->type() == OneSentence) {
                    state = NORMAL;
                    ret[ret.size()-1].setOutputRedirection(true, ite->value());
                } else {
                    //std::cout << "point e" << std::endl;
                    throw SyntaxError("SyntaxError");
                }
                break;
            }
        }
        if(state != INIT && state != NORMAL && state != TERMINATED) {
            std::cout << "point f" << std::endl;
            throw SyntaxError("SyntaxError");
        }

#ifdef DEBUG
        std::vector<Statement>::iterator begin = ret.begin();
        std::vector<Statement>::iterator end = ret.end();
        std::cout << "-------------------------------" << std::endl;
        for(; begin != end ; begin++) {
            std::cout << begin->getString() << std::endl;
        }
        std::cout << "-------------------------------" << std::endl;
#endif

        return ret;
    }
private:

    Tokenizer tokenizer_;
};

class LexBase {
protected:
    void addSyntaxTokenFromString(std::vector<SyntaxToken>& ret, const std::string& input,
            size_t pos, size_t pos2, SupportSemantic type) const;

    void addSyntaxToken(std::vector<SyntaxToken>& ret, SupportSemantic type, const std::string& value) const;

    size_t skipWhiteSpace(const std::string& input, size_t pos) const;
};

class DefaultRedrectionLex : public LexBase {
public:
    std::string getInputKeyword() const {
        return "<";
    }
    std::string getOutputKeyword() const {
        return ">";
    }
    std::vector<SyntaxToken> split(const std::string& input);
};
class DefaultPipeLex : public LexBase {
public:
    std::string getPipeKeyword() const {
        return "|";
    }
    std::vector<SyntaxToken> split(const std::string& input);
};
class DefaultSentenceTerminatorLex : public LexBase {
public:
    std::string getTerminatorKeyword() const {
        return ";";
    }
    std::vector<SyntaxToken> split(const std::string& input);
};

typedef Tokenizer<DefaultRedrectionLex,
                  DefaultPipeLex,
                  DefaultSentenceTerminatorLex> DefaultTokenizer;

typedef InputParser<DefaultTokenizer> DefaultParser;

}

#endif /* end of include guard */

