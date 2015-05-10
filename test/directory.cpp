#include <test_case.hpp>
#include <test_macros.hpp>
#include <set>

#include "command/param_comple/tool/directory.h"

using namespace iunit;
using namespace clidevt::tool;

class DirectoryScanTest : public CppTestCase {
    DirectoryScan* scan_;
public:
    DirectoryScanTest() : CppTestCase("DirectoryScanTest") {
    }

    virtual void setup() {
        scan_ = new DirectoryScan("dir_test/");
    }
    virtual void teardown() {
        delete scan_;
    }

    void checkResult(std::set<std::string>& answer, DirectoryScan::iterator& ite) {
        size_t i=0;
        for(; ite != scan_->end(); ite++, i++) {
            //std::cout << ite->getPath() << std::endl;
            IUNIT_TRUE(answer.count((*ite).getPath()) == 1);
        }
        IUNIT_EQ(i, answer.size());
    }

    void onlyAsterTest() {
        //IUNIT_MESSAGE("------------[*]-------------");
        std::set<std::string> answer;
        answer.insert("dir_test/ABBBCBBC");
        answer.insert("dir_test/ABCAABCBABC");
        answer.insert("dir_test/ABCDDBCBABC");
        answer.insert("dir_test/a.txt");
        answer.insert("dir_test/a.txt.dat");
        answer.insert("dir_test/a.txt.txt");
        answer.insert("dir_test/a.txt.txt.dat");
        answer.insert("dir_test/a.txt.dat.txt.dat");
        answer.insert("dir_test/abc");
        answer.insert("dir_test/abefgdc");
        answer.insert("dir_test/abcefg");
        answer.insert("dir_test/abcefgc");
        answer.insert("dir_test/abcefgca");
        answer.insert("dir_test/efg");
        answer.insert("dir_test/dec");
        answer.insert("dir_test/def");
        answer.insert("dir_test/efgefg");
        answer.insert("dir_test/ewaaaglkjgd");
        answer.insert("dir_test/ewrtefglkj");
        answer.insert("dir_test/..");
        answer.insert("dir_test/.");
        DirectoryScan::iterator ite = scan_->begin("*");
        checkResult(answer, ite);
    }
    void middleAsterTest() {
        //IUNIT_MESSAGE("--------------[a*c]---------------");
        std::set<std::string> answer;
        answer.insert("dir_test/abc");
        answer.insert("dir_test/abefgdc");
        answer.insert("dir_test/abcefgc");
        DirectoryScan::iterator ite = scan_->begin("a*c");
        checkResult(answer, ite);
    }

    void firstAsterTest() {
        //IUNIT_MESSAGE("-------------[*efg]------------");
        std::set<std::string> answer;
        answer.insert("dir_test/abcefg");
        answer.insert("dir_test/efg");
        answer.insert("dir_test/efgefg");
        DirectoryScan::iterator ite = scan_->begin("*efg");
        checkResult(answer, ite);
    }
    void endAsterTest() {
        //IUNIT_MESSAGE("-------------[a.txt*]------------");
        std::set<std::string> answer;
        answer.insert("dir_test/a.txt");
        answer.insert("dir_test/a.txt.txt");
        answer.insert("dir_test/a.txt.dat");
        answer.insert("dir_test/a.txt.txt.dat");
        answer.insert("dir_test/a.txt.dat.txt.dat");
        DirectoryScan::iterator ite = scan_->begin("a.txt*");
        checkResult(answer, ite);
    }

    void bothEndsAsterTest() {
        //IUNIT_MESSAGE("-------------[*efg*]------------");
        std::set<std::string> answer;
        answer.insert("dir_test/efg");
        answer.insert("dir_test/abefgdc");
        answer.insert("dir_test/abcefg");
        answer.insert("dir_test/abcefgc");
        answer.insert("dir_test/abcefgca");
        answer.insert("dir_test/ewrtefglkj");
        answer.insert("dir_test/efgefg");
        DirectoryScan::iterator ite = scan_->begin("*efg*");
        checkResult(answer, ite);
    }
    void middleAndEndAsterTest() {
        //IUNIT_MESSAGE("--------------[ew*glkj*]---------------");
        std::set<std::string> answer;
        answer.insert("dir_test/ewrtefglkj");
        answer.insert("dir_test/ewaaaglkjgd");
        DirectoryScan::iterator ite = scan_->begin("ew*glkj*");
        checkResult(answer, ite);
    }

    void longestMatchTest() {
        //IUNIT_MESSAGE("--------------[*.txt]---------------");
        std::set<std::string> answer;
        answer.insert("dir_test/a.txt");
        answer.insert("dir_test/a.txt.txt");
        DirectoryScan::iterator ite = scan_->begin("*.txt");
        checkResult(answer, ite);

        //IUNIT_MESSAGE("------------[*.txt.dat]---------------");
        answer.clear();
        answer.insert("dir_test/a.txt.dat");
        answer.insert("dir_test/a.txt.txt.dat");
        answer.insert("dir_test/a.txt.dat.txt.dat");
        ite = scan_->begin("*.txt.dat");
        checkResult(answer, ite);
    }

    void multiMiddleAstersTest() {
        //IUNIT_MESSAGE("--------------[A*BC*A*C]---------------");
        std::set<std::string> answer;
        answer.insert("dir_test/ABCAABCBABC");
        answer.insert("dir_test/ABCDDBCBABC");
        DirectoryScan::iterator ite = scan_->begin("A*BC*A*C");
        checkResult(answer, ite);
    }

    virtual void init() {
        //IUNIT_ADD_TEST( DirectoryScanTest, test);
        IUNIT_ADD_TEST( DirectoryScanTest, onlyAsterTest);
        IUNIT_ADD_TEST( DirectoryScanTest, firstAsterTest);
        IUNIT_ADD_TEST( DirectoryScanTest, middleAsterTest);
        IUNIT_ADD_TEST( DirectoryScanTest, endAsterTest);
        IUNIT_ADD_TEST( DirectoryScanTest, bothEndsAsterTest);
        IUNIT_ADD_TEST( DirectoryScanTest, middleAndEndAsterTest);
        IUNIT_ADD_TEST( DirectoryScanTest, longestMatchTest);
        IUNIT_ADD_TEST( DirectoryScanTest, multiMiddleAstersTest);
    }
};

