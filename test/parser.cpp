#include <test_case.hpp>
#include <test_macros.hpp>

#include "parser/input_parser.h"
#include "parser/tokenize_argument.h"

using namespace iunit;
using namespace clidevt;

class ParserTest : public CppTestCase {
    DefaultParser parser;
    DefaultPipeLex pipeLex;
    std::string oneScentence;
    std::string oneScentence_b;
    std::string twoScentence;
    std::string pipeString;
    std::string in_redirection;
    std::string out_redirection;
public:
    ParserTest() : CppTestCase("ParserTest") {
        oneScentence = "aaaaaaaa";
        oneScentence_b       =  "bbbbbbb";
        twoScentence = "aaaaaaaa;bbbbbbb";
        pipeString =   "aaaaaaaa|bbbbbbb";
        in_redirection = "aa < bb";
        out_redirection = "aa < bb";
    }

    virtual void setup() {
    }
    virtual void teardown() {
    }

    virtual void test();

    virtual void init() {
        IUNIT_ADD_TEST( ParserTest, test);
    }
};


void ParserTest::test() {
    {
        std::vector<Statement> ret = parser.parse(oneScentence);
        IUNIT_EQ(1, ret.size());
        IUNIT_EQ(oneScentence, ret[0].getString());
    }
    {
        std::vector<Statement> ret = parser.parse(twoScentence);
        IUNIT_EQ(2, ret.size());
        IUNIT_EQ(oneScentence, ret[0].getString());
        IUNIT_EQ(oneScentence_b, ret[1].getString());
    }
    {
        std::vector<Statement> ret = parser.parse(pipeString);
        IUNIT_EQ(2, ret.size());
        IUNIT_EQ(oneScentence, ret[0].getString());
        IUNIT_EQ(true, ret[0].isPipe());
        IUNIT_EQ(oneScentence_b, ret[1].getString());
    }
    {
        //std::vector<Statement> ret = parser.parse(in_redirection);
        //IUNIT_EQ(1, ret.size());
        //IUNIT_EQ("aa ", ret[0].getString());
        //IUNIT_EQ("bb", ret[1].getString());
    }
    {
        std::vector<Statement> ret = parser.parse("ls|cat|cat");
        //IUNIT_EQ(3, ret.size());
        //IUNIT_EQ("ls", ret[0].getString());
        //IUNIT_EQ("cat", ret[1].getString());
        //IUNIT_EQ("cat", ret[2].getString());
    }
    {
        std::vector<SyntaxToken> ret = pipeLex.split("ls|cat|cat");
        IUNIT_EQ(5, ret.size());
        IUNIT_EQ("ls", ret[0].value());
        IUNIT_EQ("|", ret[1].value());
        IUNIT_EQ("cat", ret[2].value());
        IUNIT_EQ("|", ret[3].value());
        IUNIT_EQ("cat", ret[4].value());
    }
}


class DivideArgumentTest : public CppTestCase {
    std::string argument_1;
public:
    DivideArgumentTest() : CppTestCase("DivideArgumentTest") {
        argument_1 = "grep \"Console\" console.h";
    }

    virtual void setup() {
    }
    virtual void teardown() {
    }

    virtual void test() {
        std::vector<std::string>* ret = clidevt::divideArgumentList(argument_1);
        IUNIT_EQ(std::string("grep"), ret->at(0));
        IUNIT_EQ(std::string("Console"), ret->at(1));
        IUNIT_EQ(std::string("console.h"), ret->at(2));
        IUNIT_EQ(size_t(3), ret->size());
    }

    virtual void init() {
        IUNIT_ADD_TEST(DivideArgumentTest, test);
    }
};

