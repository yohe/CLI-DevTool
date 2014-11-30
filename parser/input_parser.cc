
#include <cassert>
#include <iostream>
#include <exception>
#include "parser/input_parser.h"

using namespace clidevt;

void LexBase::addSyntaxTokenFromString(std::vector<SyntaxToken>& ret,
        const std::string& input,
        size_t pos, size_t pos2,
        SupportSemantic type) const
{
    if((pos2-pos) == 0) {
        return;
    }

    std::string token = input.substr(pos, pos2-pos);
    SyntaxToken t(type, token);
    ret.push_back(t);
}

void LexBase::addSyntaxToken(std::vector<SyntaxToken>& ret, SupportSemantic type, const std::string& value) const {
    std::string tmp = value;
    SyntaxToken t(type, tmp);
    //std::cout << tmp + ", type=" << type << std::endl;
    ret.push_back(t);
}

size_t LexBase::skipWhiteSpace(const std::string& input, size_t pos) const {
    return input.find_first_not_of(" ",pos);
}

std::vector<SyntaxToken> DefaultRedrectionLex::split(const std::string& input) {
    std::vector<SyntaxToken> ret;

    size_t pos = 0;
    while(pos < input.size()) {
        size_t in = input.find(getInputKeyword(),pos);
        size_t out = input.find(getOutputKeyword(),pos);
        //std::cout << in << ":" << out << std::endl;
        if(in == std::string::npos && in == out) {
            addSyntaxTokenFromString(ret, input, pos, in, OneSentence);
            pos = input.size();
            continue;
        }
        if(in < out) {
            addSyntaxTokenFromString(ret, input, pos, in, OneSentence);
            addSyntaxToken(ret, InputRedirection, getInputKeyword());
            pos = in + 1;
        } else {
            addSyntaxTokenFromString(ret, input, pos, out, OneSentence);
            addSyntaxToken(ret, OutputRedirection, getOutputKeyword());
            pos = out + 1;
        }
    }

    return ret;
}

std::vector<SyntaxToken> DefaultPipeLex::split(const std::string& input) {
    std::vector<SyntaxToken> ret;

    size_t pos = 0;
    while(pos < input.size()) {
        size_t pipe = input.find(getPipeKeyword(),pos);
        if(pipe == std::string::npos) {
            pos = skipWhiteSpace(input, pos);
            addSyntaxTokenFromString(ret, input, pos, pipe, OneSentence);
            pos = input.size();
        } else {
            pos = skipWhiteSpace(input, pos);
            addSyntaxTokenFromString(ret, input, pos, pipe, OneSentence);
            addSyntaxToken(ret, Pipe, getPipeKeyword());
            pos = pipe + 1;
        }
    }

    return ret;

}

std::vector<SyntaxToken> DefaultSentenceTerminatorLex::split(const std::string& input) {
    std::vector<SyntaxToken> ret;

    size_t pos = 0;
    while(pos < input.size()) {
        size_t term = input.find(getTerminatorKeyword(),pos);
        if(term == std::string::npos) {
            pos = skipWhiteSpace(input, pos);
            addSyntaxTokenFromString(ret, input, pos, term, OneSentence);
            pos = input.size();
        } else {
            pos = skipWhiteSpace(input, pos);
            addSyntaxTokenFromString(ret, input, pos, term, OneSentence);
            addSyntaxToken(ret, SentenceTerminator, getTerminatorKeyword());
            pos = term + 1;
        }
    }

    return ret;

}

pid_t Statement::setupOfExecution() {
    if(pipe_ == false) {
        return 0;
    }
    if(pipe(pipefd_) == -1) {
        std::cerr << "Enter failed : pipe could not create." << std::endl;
        throw std::runtime_error("pipe could not create.");
    }
    pid_t child = fork();
    if(child < 0) {
        perror("waitpid");
        throw std::runtime_error("fork error.");
    }
    if(child != 0) {
        if(isChild_) {
            assert(false);
        }
        // parent process
        // parent process transfer to input pipe for child process.
        stdoutBackup_ = dup(1);
        stderrBackup_ = dup(2);
        dup2(pipefd_[1], 1);
        dup2(pipefd_[1], 2);
        child_ = child;
        hasChild_ = true;

    } else {
        // child process
        // child process don't use write pipe.
        //stdinBackup_ = dup(0);
        close(pipefd_[1]);
        dup2(pipefd_[0], 0);
        child_ = 0;
        isChild_ = true;
        inputR_ = false;
        outputR_ = false;
    }

    return child ;
}
void Statement::teardownOfExecution() {
    if(pipe_ == false) {
        return;
    }
    if(hasChild_) {

        dup2(stdoutBackup_, 1);
        dup2(stderrBackup_, 2);
        close(pipefd_[0]);
        close(pipefd_[1]);
        close(stdoutBackup_);
        close(stderrBackup_);
        int status;
        waitpid(child_, &status, 0);
    }
    if(isChild_) {
        //close(pipefd_[0]);
        close(pipefd_[1]);
        //dup2(stdinBackup_, 0);
        _exit(0);
    }
}

// Redirectionはここで処理させるのではなく、コマンド実行する直前、shell_exe 側でしなければならない
void Statement::setupRedirection() {
    if(inputR_) {
        inputFileP_ = fopen(inputFile_.c_str(), "r");
        stdinBackup_ = dup2(fileno(inputFileP_), 0);
        //dup2(stdinBackup_, 0);
    }
    //if(outputR_) {
    //    outputFileP_ = fopen(outputFile_.c_str(), "w");
    //    dup2(fileno(inputFileP_), 1);
    //}
}
void Statement::teardownRedirection() {
    if(inputR_) {
        dup2(stdinBackup_, 0);
        close(stdinBackup_);
        fclose(inputFileP_);
    }
    //fclose(outputFileP_);
}

