
#include <iostream>
#include <test_case.hpp>
#include <test_suite.hpp>
#include <test_text_outputter.hpp>
#include <test_junit_outputter.hpp>
#include <test_macros.hpp>

#include <fstream>

using namespace iunit;

int main(int argc, char const* argv[])
{
    CppTestResultCollector collector;
    CppTestSuite suite("TestSuite", &collector);
    suite.start();

    std::ofstream xml_ofs("result.xml");
    JUnitOutputter outputter2(xml_ofs);
    collector.write(&outputter2);
    return (collector.isSuccessful() ? 0 : -1);
}

