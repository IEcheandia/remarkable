/**
*
* @defgroup testing
*
* @file
* @brief  Header file for some unit test helping
* @copyright    Precitec GmbH & Co. KG
* @author GM
*/
#pragma once
// cppunit includes needed in all tests
#include <cppunit/TestCase.h>
#include <cppunit/extensions/HelperMacros.h>
#include <cppunit/ui/text/TestRunner.h>

#define TEST_MAIN( CLASSNAME ) \
int main(int /*argc*/, char **/*argv*/) \
{ \
    CppUnit::TextUi::TestRunner runner; \
    runner.addTest(CLASSNAME::suite()); \
    return !runner.run(); \
}
