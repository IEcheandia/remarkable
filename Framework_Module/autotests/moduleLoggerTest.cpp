/**
* @file
* @copyright Precitec GmbH & Co. KG
* @author Daniel Feist (Ft)
* @date 2017
* @brief Test for moduleLogger of Framework_Module
*/

#include "../../Mod_Grabber/autotests/testHelper.h"
#include "../include/module/moduleLogger.h"
#include "../include/module/baseModule.h"

#include <common/logMessage.h>

#include <iostream>
#include <memory>
#include <regex>

class ModuleLoggerTest : public CppUnit::TestFixture
{
    CPPUNIT_TEST_SUITE(ModuleLoggerTest);
    CPPUNIT_TEST(testWMLog);
    CPPUNIT_TEST(testWMLogTr);
    CPPUNIT_TEST(testWMFatal);
    CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void testWMLog();
    void testWMLogTr();
    void testWMFatal();

private:
    int i = -7;
    unsigned int u = 10;
    std::string testStr = "test";
};

void ModuleLoggerTest::setUp()
{
    unsetenv("WM_LOG_STDOUT");
}

void ModuleLoggerTest::testWMLog()
{
    precitec::framework::module::BaseModule::m_pBaseModuleLogger = std::make_unique<precitec::ModuleLogger>("LoggerConsole");
    precitec::LogMessage* tmpLogMsg;

    // without argument(s).
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "");
    std::regex emptyExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| $");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), emptyExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "no arguments");
    std::regex noArgExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| no arguments$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), noArgExp), true);

    // with argument(s).
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "%s", testStr.c_str());
    std::regex singleArgExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), singleArgExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "one conversion specifier %s", testStr.c_str());
    std::regex oneArgExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| one conversion specifier test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), oneArgExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "%i, multiple conversion specifiers %u and %s", i, u, testStr.c_str());
    std::regex twoArgsExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), twoArgsExp), true);

    // special case: number of arguments does not match number of %.
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "%i, multiple conversion specifiers %u and %s, %u", i, u, testStr.c_str()); // #% > #parameter
    std::regex plusOneArgsExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and test, 0$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), plusOneArgsExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLog(precitec::eDebug, "%i, multiple conversion specifiers %u and", i, u, testStr.c_str()); // #% < #parameter
    std::regex minusOneArgsExp("^DEBUG\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), minusOneArgsExp), true);

    // with invalid argument(s).
    //
    //std::initializer_list<double> dlist = {0.0, 1.1, 2.2};
    //wmLog(precitec::eDebug, "no conversion possible: %s\n", dlist);
    //wmLog(precitec::eDebug, "no conversion possible: %i, %d\n", std::make_pair(i, u), dlist);

    precitec::framework::module::BaseModule::m_pBaseModuleLogger.reset();
}

void ModuleLoggerTest::testWMLogTr()
{
    precitec::framework::module::BaseModule::m_pBaseModuleLogger = std::make_unique<precitec::ModuleLogger>("LoggerConsole");
    precitec::LogMessage* tmpLogMsg;

    // without argument(s).
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", "");
    std::regex emptyExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| $");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), emptyExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", "no arguments");
    std::regex noArgExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| no arguments$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), noArgExp), true);

    // with argument(s).
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", testStr.c_str());
    std::regex singleArgExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), singleArgExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", "one conversion specifier %s", testStr.c_str());
    std::regex oneArgExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| one conversion specifier test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), oneArgExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", "%i, multiple conversion specifiers %u and %s", i, u, testStr.c_str());
    std::regex twoArgsExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), twoArgsExp), true);

    // special case: number of arguments does not match number of %.
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", "%i, multiple conversion specifiers %u and %s, %u", i, u, testStr.c_str()); // #% > #parameter
    std::regex plusOneArgsExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and test, 0$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), plusOneArgsExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmLogTr(precitec::eError, "UnitTest.testWMLogTr", "%i, multiple conversion specifiers %u and", i, u, testStr.c_str()); // #% < #parameter
    std::regex minusOneArgsExp("^ERR  \\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), minusOneArgsExp), true);


    // with invalid argument(s).
    //
    //std::initializer_list<double> dlist = {0.0, 1.1, 2.2};
    //wmLogTr(precitec::eError, "UnitTest.testWMLoTr", "no conversion possible: %s\n", dlist);
    //wmLogTr(precitec::eError, "UnitTest.testWMLoTr", "no conversion possible: %i, %d\n", std::make_pair(i, u), dlist);

    precitec::framework::module::BaseModule::m_pBaseModuleLogger.reset();
}

void ModuleLoggerTest::testWMFatal()
{
    precitec::framework::module::BaseModule::m_pBaseModuleLogger = std::make_unique<precitec::ModuleLogger>("LoggerConsole");
    precitec::LogMessage* tmpLogMsg;

    // without argument(s).
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", "");
    std::regex emptyExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| $");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), emptyExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", "no arguments");
    std::regex noArgExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| no arguments$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), noArgExp), true);

    // with argument(s).
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", testStr.c_str());
    std::regex singleArgExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), singleArgExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", "one conversion specifier %s", testStr.c_str());
    std::regex oneArgExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| one conversion specifier test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), oneArgExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", "%i, multiple conversion specifiers %u and %s", i, u, testStr.c_str());
    std::regex twoArgsExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and test$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), twoArgsExp), true);

    // special case: number of arguments does not match number of %.
    //
    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", "%i, multiple conversion specifiers %u and %s, %u", i, u, testStr.c_str()); // #% > #parameter
    std::regex plusOneArgsExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and test, 0$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), plusOneArgsExp), true);

    tmpLogMsg = precitec::framework::module::BaseModule::m_pBaseModuleLogger->getMessage();
    wmFatal(precitec::eAxis, "UnitTest.testWMFatal", "%i, multiple conversion specifiers %u and", i, u, testStr.c_str()); // #% < #parameter
    std::regex minusOneArgsExp("^FATAL\\| \\d{2}\\.\\d{2}\\.\\d{4} - \\d{2}\\:\\d{2}\\:\\d{2}\\.\\d{3} \\| LoggerCons \\| -7, multiple conversion specifiers 10 and$");
    CPPUNIT_ASSERT_EQUAL(std::regex_match(tmpLogMsg->format(), minusOneArgsExp), true);

    // with invalid argument(s).
    //
    //std::initializer_list<double> dlist = {0.0, 1.1, 2.2};
    //wmFatal(precitec::eError, "UnitTest.testWMLoTr", "no conversion possible: %s\n", dlist);
    //wmFatal(precitec::eError, "UnitTest.testWMLoTr", "no conversion possible: %i, %d\n", std::make_pair(i, u), dlist);

    precitec::framework::module::BaseModule::m_pBaseModuleLogger.reset();
}

TEST_MAIN(ModuleLoggerTest)
