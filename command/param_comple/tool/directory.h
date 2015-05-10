
#ifndef CLI_DEV_COMMAND_PARAM_COMPLE_TOOL_DIRECTORY_H
#define CLI_DEV_COMMAND_PARAM_COMPLE_TOOL_DIRECTORY_H

#include <vector>
#include <string>
#include <dirent.h>
#include <iostream>

namespace clidevt {
    namespace tool {
        class DirectoryEntry {
        public:
            DirectoryEntry(const std::string& path, bool dir) : filepath_(path), dir_(dir) {
            }

            std::string getPath() const {
                return filepath_;
            }

            std::string basename() const {
                return filepath_.substr(filepath_.find_last_of("/")+1);
            }
            std::string dirname() const {
                return filepath_.substr(0, filepath_.find_last_of("/", filepath_.length()-2));
            }

            bool isDir() const {
                return dir_;
            }
        private:
            std::string filepath_;
            bool dir_;
        };

        class DirectoryData {
            typedef std::vector<DirectoryEntry>::iterator iterator;
            typedef std::vector<DirectoryEntry>::const_iterator const_iterator;
        public:
            DirectoryData(const char* path);
            ~DirectoryData();

            void reRead(const char* path);

            iterator begin() {
                return entries_.begin();
            }
            const_iterator begin() const {
                return entries_.begin();
            }

            iterator end() {
                return entries_.end();
            }
            const_iterator end() const {
                return entries_.end();
            }
        private:
            std::vector<DirectoryEntry> entries_;
        };

        class DirectoryScan {
        public:
            class iterator : public std::iterator<std::input_iterator_tag, std::vector<DirectoryEntry>::iterator> {
                iterator::value_type cur_;
                iterator::value_type end_;
                std::string filter_;
            public:
                iterator() : cur_(), end_(), filter_("") {}
                iterator(iterator::value_type begin, iterator::value_type end, const char* filter) :
                    cur_(begin),
                    end_(end),
                    filter_(filter)
                {
                    if(isMatch(*cur_) == false) {
                        (*this)++;
                    }
                }
                iterator(const iterator& rhs) :
                    cur_(rhs.cur_),
                    end_(rhs.end_),
                    filter_(rhs.filter_) {
                }

                iterator& operator++() {
                    // prefix ++
                    ++cur_;
                    while(cur_ != end_ && isMatch(*cur_) == false) {
                        ++cur_;
                    }
                    return *this;
                }

                bool operator==(const iterator& rhs) const {
                    return cur_ == rhs.cur_;
                }
                bool operator!=(const iterator& rhs) const {
                    return !(this->operator==(rhs));
                }

                iterator operator++(int) {
                    iterator tmp(*this);
                    this->operator++();
                    return tmp;
                }

                DirectoryEntry& operator*() {
                    return *cur_;
                }
                iterator::value_type operator->() {
                    return cur_;
                }

            private:
                // filter条件にマッチするかチェック
                // filter条件はファイルパスのbasenameに対するものとする
                bool isMatch(DirectoryEntry& entry);
            };
            DirectoryScan(const char* path);

            iterator begin(const char* filter = "");
            iterator end();

        private:
            void scan(const char* path);

            DirectoryData directory_;
            //std::vector<DirectoryEntry> entries_;
        };
    }
}

#endif
