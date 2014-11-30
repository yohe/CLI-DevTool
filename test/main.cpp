
#include <iostream>
#include <test_case.hpp>
#include <test_suite.hpp>
#include <test_text_outputter.hpp>
#include <test_junit_outputter.hpp>
#include <test_macros.hpp>
#include <test_config.hpp>

#include <fstream>

#include "key_seq.cpp"
#include "key_map.cpp"
#include "parser.cpp"

using namespace iunit;
using namespace clidevt;

int main(int argc, char const* argv[])
{
    TestConfig config;
    config.init(argc, argv);
    CppTestResultCollector collector;
    CppTestSuite suite("TestSuite", collector);
    suite.addTest(new KeySequenceEntryTest());
    suite.addTest(new KeySequenceGroupTest());
    suite.addTest(new KeyMapTest());
    suite.addTest(new ParserTest());
    suite.config(config);

    suite.start();

    std::ofstream xml_ofs("result.xml");
    JUnitOutputter outputter2(xml_ofs);
    collector.write(&outputter2);
    return (collector.isSuccessful() ? 0 : -1);
}

