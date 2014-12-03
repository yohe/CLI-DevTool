
#include <unistd.h>

#include "console.h"
#include <iostream>
#include "mode/builtin/logging.h"

namespace clidevt {

    void LoggingMode::enter(Console* console, Mode* mode, const std::string& param) {
        if(param.length() == 0) {
            file_ = "typescript";
        } else {
            file_ = param.substr(0,param.find(" "));
        }
        ofs_.open(file_.c_str());
        if(!ofs_) {
            std::cerr << "file could not open" << std::endl;
            return;
        }
        std::cout << "Script started, output file is " << file_ << "." << std::endl;
        std::cout << console->getDateString() << std::endl;
        ofs_ << "Script started, output file is " << file_ << "." << std::endl;
        ofs_ << console->getDateString() << std::endl;
    }
    void LoggingMode::leave(Console* console) {
        std::cout << "Script done, output file is " << file_ << "." << std::endl;
        std::cout << console->getDateString() << std::endl;
        ofs_.close();
    }
    void LoggingMode::hookPromptDisplay(const std::string& prompt, Console* console) {
        std::cout << "\x1b[33m" "Logging:" << prompt << "\x1b[39m";
    }
    void LoggingMode::hookExecuteCommandLineBefore(const std::string& input, Console* console) {
        ofs_ << console->getPromptString() << console->getInputtingString() << std::endl;
    }
    void LoggingMode::hookExecuteCmdBefore(Command* cmd, Console* console) {
        if(pipe(pipefd_) == -1) {
            std::cerr << "Enter failed : pipe could not create." << std::endl;
            return;
        }

        child_ = fork();
        if(child_ == -1) {
            std::cerr << "fork error" << std::endl;
        }

        if(child_ == 0) {
            // child process
            close(pipefd_[1]);

            char buf[128];
            size_t size = 0;
            while((size = read(pipefd_[0], &buf, 128)) != 0) {
                buf[size] = 0;
                write(STDOUT_FILENO, &buf, size);
                ofs_ << buf;
            }
            //write(STDOUT_FILENO, "\n", 1);
            //ofs_ << std::endl;
            ofs_ << std::flush;

            close(pipefd_[0]);
            _exit(1);
        } else {

            // 標準出力,標準エラーをpipeへ出力
            // parent process
            stdoutBackup_ = dup(1);
            stderrBackup_ = dup(2);
            dup2(pipefd_[1], 1);
            dup2(pipefd_[1], 2);
        }
    } 
    void LoggingMode::hookExecuteCmdAfter(Command* cmd, Console* console) {

        close(pipefd_[0]);
        close(pipefd_[1]);
        dup2(stdoutBackup_, 1);
        dup2(stderrBackup_, 2);
        close(stdoutBackup_);
        close(stderrBackup_);
        int status;

        waitpid(child_, &status, 0);
    }
}
