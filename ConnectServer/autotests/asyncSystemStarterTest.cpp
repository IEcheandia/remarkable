/**
* @file
* @copyright Precitec GmbH & Co. KG
* @author GM
* @date 2017
* @brief Test for AsyncSystemStarter of ConnectServer
*/
#include "../Mod_Grabber/autotests/testHelper.h"
#include "../AsyncSystemStarter.h"

#include "Poco/Activity.h"
#include "Poco/DateTime.h"
#include "Poco/File.h"
#include "Poco/Path.h"
#include "Poco/Util/PropertyFileConfiguration.h"

#include <string.h>
#include <functional>

#include <unistd.h>
#include <fcntl.h>

using Poco::DateTime;
using Poco::File;
using Poco::Path;
using Poco::Util::PropertyFileConfiguration;
using Precitec::Service::Discovery::AsyncSystemStarter;

class AsyncSystemStarterTest : public CppUnit::TestFixture, private AsyncSystemStarter
{

CPPUNIT_TEST_SUITE(AsyncSystemStarterTest);
CPPUNIT_TEST(testLoadNoConfig);
CPPUNIT_TEST(testOneArgument);
CPPUNIT_TEST(testArgumentTrailingWhitespaces);
CPPUNIT_TEST(testMultipleArguments);
CPPUNIT_TEST(testBatch);
CPPUNIT_TEST(testWeldMasterApplication);
CPPUNIT_TEST(testTerminalArguments);
CPPUNIT_TEST(testMultipleProcesses);
CPPUNIT_TEST(testMissingConfigSection);
CPPUNIT_TEST(testWaitNoFd);
CPPUNIT_TEST(testWaitFd);
CPPUNIT_TEST(testWaitTimeout);
CPPUNIT_TEST(testCreateArguments);
CPPUNIT_TEST(testCreateArgumentsNoPipe);
CPPUNIT_TEST(testCreatePipe);
CPPUNIT_TEST_SUITE_END();

public:
    void setUp() override;
    void tearDown() override;

    void testLoadNoConfig();
    void testOneArgument();
    void testArgumentTrailingWhitespaces();
    void testMultipleArguments();
    void testBatch();
    void testWeldMasterApplication();
    void testTerminalArguments();
    void testMultipleProcesses();
    void testMissingConfigSection();
    void testWaitNoFd();
    void testWaitFd();
    void testWaitTimeout();
    void testCreateArguments();
    void testCreateArgumentsNoPipe();
    void testCreatePipe();

private:
    void testWaitNoFd_helper()
    {
        waitForProcessStarted(-1, "test");
    }
    void testWaitFd_helper()
    {
        waitForProcessStarted(readFd, "test", 5000);
    }
    void createConfig(const std::string &argument);
    Poco::Path configDirPath;
    int readFd;
};

void AsyncSystemStarterTest::setUp()
{
    configDirPath = Path(Path::temp());
    configDirPath.pushDirectory(std::string("connectservertest"));
    configDirPath.makeDirectory();
    char *env = new char[13 + configDirPath.toString().size()];
    strcpy(env, std::string("WM_BASE_DIR=" + configDirPath.toString()).c_str());
    putenv(env);

    configDirPath.pushDirectory(std::string("system_config"));
    configDirPath.makeDirectory();

    File configDir(configDirPath);
    configDir.createDirectories();
    readFd = -1;
}

void AsyncSystemStarterTest::tearDown()
{
    // clean up the temp directory we had
    Path testDirPath(Path::temp());
    testDirPath.pushDirectory(std::string("connectservertest"));
    testDirPath.makeDirectory();

    File testDir(testDirPath);
    testDir.remove(true);
}

void AsyncSystemStarterTest::testLoadNoConfig()
{
    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(result.empty());
}

void AsyncSystemStarterTest::testOneArgument()
{
    // this test verifies that a config with one argument is parsed correctly
    createConfig(std::string("-arg"));

    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process = result.front();
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process.terminalPath);
    CPPUNIT_ASSERT(process.terminalArguments.empty());
    CPPUNIT_ASSERT_EQUAL(false, process.batch);
    CPPUNIT_ASSERT_EQUAL(true, process.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(true, process.enabled);
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, process.arguments.size());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg"), process.arguments.front());
    CPPUNIT_ASSERT(process.ldPreload.empty());
}

void AsyncSystemStarterTest::testArgumentTrailingWhitespaces()
{
    // this test verifies that a config with one argument
    // and trailing whitespaces is parsed correctly
    createConfig(std::string("-arg  "));

    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process = result.front();
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process.terminalPath);
    CPPUNIT_ASSERT(process.terminalArguments.empty());
    CPPUNIT_ASSERT_EQUAL(false, process.batch);
    CPPUNIT_ASSERT_EQUAL(true, process.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, process.arguments.size());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg"), process.arguments.front());
    CPPUNIT_ASSERT(process.ldPreload.empty());
}

void AsyncSystemStarterTest::testMultipleArguments()
{
    // this test verifies that a config with multiple arguments is parsed correctly
    createConfig(std::string("-arg    -arg1  -arg2    -arg3"));

    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process = result.front();
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process.terminalPath);
    CPPUNIT_ASSERT(process.terminalArguments.empty());
    CPPUNIT_ASSERT_EQUAL(false, process.batch);
    CPPUNIT_ASSERT_EQUAL(true, process.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(std::size_t{4}, process.arguments.size());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg"), process.arguments.at(0));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg1"), process.arguments.at(1));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg2"), process.arguments.at(2));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg3"), process.arguments.at(3));
    CPPUNIT_ASSERT(process.ldPreload.empty());
}

void AsyncSystemStarterTest::testBatch()
{
    // this test verifies that the batch argument is read correctly
    Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
    pConf->setString(std::string("Startup.00"), std::string("Foo"));
    pConf->setString(std::string("Foo.Name"), std::string("Bar"));
    pConf->setString(std::string("Foo.Arguments"), std::string());
    pConf->setBool(std::string("Foo.Batch"), true);
    pConf->setString(std::string("TerminalApplication.Path"), std::string("/usr/local/bin/sh"));

    Path configFilePath = configDirPath;
    configFilePath.setFileName("startup.config");

    pConf->save(configFilePath.toString());

    auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process = result.front();
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process.terminalPath);
    CPPUNIT_ASSERT(process.terminalArguments.empty());
    CPPUNIT_ASSERT_EQUAL(true, process.batch);
    CPPUNIT_ASSERT(process.arguments.empty());

    pConf->setBool(std::string("Foo.Batch"), false);
    pConf->save(configFilePath.toString());

    result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process2 = result.front();
    CPPUNIT_ASSERT_EQUAL(false, process2.batch);
}

void AsyncSystemStarterTest::testWeldMasterApplication()
{
    // this test verifies that the weldMasterApplication property is read correctly
    Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
    pConf->setString(std::string("Startup.00"), std::string("Foo"));
    pConf->setString(std::string("Foo.Name"), std::string("Bar"));
    pConf->setBool(std::string("Foo.WeldMasterApplication"), true);
    pConf->setString(std::string("TerminalApplication.Path"), std::string("/usr/local/bin/sh"));

    Path configFilePath = configDirPath;
    configFilePath.setFileName("startup.config");

    pConf->save(configFilePath.toString());

    auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process = result.front();
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process.terminalPath);
    CPPUNIT_ASSERT(process.terminalArguments.empty());
    CPPUNIT_ASSERT_EQUAL(true, process.weldMasterApplication);
    CPPUNIT_ASSERT(process.arguments.empty());
    CPPUNIT_ASSERT_EQUAL(true, process.enabled);
    CPPUNIT_ASSERT_EQUAL(false, process.autoRestart);
    CPPUNIT_ASSERT(process.ldPreload.empty());

    pConf->setBool(std::string("Foo.WeldMasterApplication"), false);
    pConf->setBool(std::string("Foo.Enabled"), false);
    pConf->setBool(std::string("Foo.AutoRestart"), true);
    pConf->save(configFilePath.toString());

    result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process2 = result.front();
    CPPUNIT_ASSERT_EQUAL(false, process2.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(false, process2.enabled);
    CPPUNIT_ASSERT_EQUAL(true, process2.autoRestart);
}

void AsyncSystemStarterTest::testTerminalArguments()
{
    // this test verifies that terminal arguments are parsed correctly
    Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
    pConf->setString(std::string("Startup.00"), std::string("Foo"));
    pConf->setString(std::string("Foo.Name"), std::string("Bar"));
    pConf->setString(std::string("Foo.Arguments"), std::string());
    pConf->setString(std::string("Foo.TermArguments"), std::string(" -arg  -arg1    -arg2 -arg3 -arg4  "));
    pConf->setString(std::string("TerminalApplication.Path"), std::string("/usr/local/bin/sh"));

    Path configFilePath = configDirPath;
    configFilePath.setFileName("startup.config");

    pConf->save(configFilePath.toString());

    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, result.size());
    const auto &process = result.front();
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process.terminalPath);
    CPPUNIT_ASSERT_EQUAL(true, process.weldMasterApplication);
    CPPUNIT_ASSERT(process.arguments.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{5}, process.terminalArguments.size());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg"), process.terminalArguments.at(0));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg1"), process.terminalArguments.at(1));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg2"), process.terminalArguments.at(2));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg3"), process.terminalArguments.at(3));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg4"), process.terminalArguments.at(4));
}

void AsyncSystemStarterTest::testMultipleProcesses()
{
    // this test verifies that multiple processes are read correctly from configuration
    Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
    pConf->setString(std::string("Startup.00"), std::string("Foo"));
    pConf->setString(std::string("Startup.01"), std::string("FooBar"));
    pConf->setString(std::string("Startup.10"), std::string("FooBarBaz"));

    pConf->setString(std::string("Foo.Name"), std::string("Bar"));
    pConf->setString(std::string("Foo.Arguments"), std::string());
    pConf->setString(std::string("Foo.TermArguments"), std::string(" -arg  -arg1    -arg2 -arg3 -arg4  "));
    pConf->setString(std::string("Foo.LD_PRELOAD"), std::string("lib/libTest.so"));

    pConf->setString(std::string("FooBar.Name"), std::string("FirstProcess"));
    pConf->setString(std::string("FooBar.Arguments"), std::string("-arg1"));
    pConf->setString(std::string("FooBar.TermArguments"), std::string(" -arg"));
    pConf->setBool(std::string("FooBar.WeldMasterApplication"), false);

    pConf->setString(std::string("FooBarBaz.Name"), std::string("SecondProcess"));
    pConf->setString(std::string("FooBarBaz.Arguments"), std::string("-arg2"));
    pConf->setString(std::string("FooBarBaz.TermArguments"), std::string());
    pConf->setBool(std::string("FooBarBaz.Batch"), true);

    pConf->setString(std::string("TerminalApplication.Path"), std::string("/usr/local/bin/sh"));
    Path configFilePath = configDirPath;
    configFilePath.setFileName("startup.config");

    pConf->save(configFilePath.toString());

    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(!result.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{3}, result.size());

    const auto &process0 = result.at(0);
    CPPUNIT_ASSERT_EQUAL(std::string("Bar"), process0.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process0.terminalPath);
    CPPUNIT_ASSERT_EQUAL(true, process0.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(false, process0.batch);
    CPPUNIT_ASSERT(process0.arguments.empty());
    CPPUNIT_ASSERT_EQUAL(std::size_t{5}, process0.terminalArguments.size());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg"), process0.terminalArguments.at(0));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg1"), process0.terminalArguments.at(1));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg2"), process0.terminalArguments.at(2));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg3"), process0.terminalArguments.at(3));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg4"), process0.terminalArguments.at(4));
    CPPUNIT_ASSERT_EQUAL(std::string("lib/libTest.so"), process0.ldPreload);

    const auto &process1 = result.at(1);
    CPPUNIT_ASSERT_EQUAL(std::string("FirstProcess"), process1.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process1.terminalPath);
    CPPUNIT_ASSERT_EQUAL(false, process1.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(false, process1.batch);
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, process1.arguments.size());
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, process1.terminalArguments.size());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg1"), process1.arguments.at(0));
    CPPUNIT_ASSERT_EQUAL(std::string("-arg"), process1.terminalArguments.at(0));
    CPPUNIT_ASSERT(process1.ldPreload.empty());

    const auto &process2 = result.at(2);
    CPPUNIT_ASSERT_EQUAL(std::string("SecondProcess"), process2.processName);
    CPPUNIT_ASSERT_EQUAL(std::string("/usr/local/bin/sh"), process2.terminalPath);
    CPPUNIT_ASSERT_EQUAL(true, process2.weldMasterApplication);
    CPPUNIT_ASSERT_EQUAL(true, process2.batch);
    CPPUNIT_ASSERT_EQUAL(std::size_t{1}, process2.arguments.size());
    CPPUNIT_ASSERT_EQUAL(true, process2.terminalArguments.empty());
    CPPUNIT_ASSERT_EQUAL(std::string("-arg2"), process2.arguments.at(0));
    CPPUNIT_ASSERT(process2.ldPreload.empty());
}

void AsyncSystemStarterTest::testMissingConfigSection()
{
    // this test verifies that missing a config section is handled correctly
    Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
    pConf->setString(std::string("Startup.00"), std::string("Foo"));
    pConf->setString(std::string("TerminalApplication.Path"), std::string("/usr/local/bin/sh"));
    Path configFilePath = configDirPath;
    configFilePath.setFileName("startup.config");

    pConf->save(configFilePath.toString());
    const auto result = loadConfiguration();
    CPPUNIT_ASSERT(result.empty());
}

void AsyncSystemStarterTest::testWaitNoFd()
{
    // this test verifies that testWaitForProcessStarted times out correctly in case of -1 fd
    Poco::Activity<AsyncSystemStarterTest> activity(this, &AsyncSystemStarterTest::testWaitNoFd_helper);
    const Poco::DateTime startTime;
    activity.start();
    activity.wait();
    auto timespan = Poco::DateTime() - startTime;
    CPPUNIT_ASSERT(timespan.totalMilliseconds() >= 4900);
    CPPUNIT_ASSERT(timespan.totalMilliseconds() < 6000);
}

void AsyncSystemStarterTest::testWaitFd()
{
    // this test verifies that testWaitForProcessStarted can read back correctly with a pipe

    // create pipe
    int pipeFds[2];
    CPPUNIT_ASSERT(pipe(pipeFds) == 0);
    readFd = pipeFds[0];
    CPPUNIT_ASSERT(readFd != -1);
    CPPUNIT_ASSERT_EQUAL(0, fcntl(readFd, F_SETFL, O_NONBLOCK));

    Poco::Activity<AsyncSystemStarterTest> activity(this, &AsyncSystemStarterTest::testWaitFd_helper);
    const Poco::DateTime startTime;
    activity.start();

    Poco::Thread::sleep(1000);
    write(pipeFds[1], "1", 1);
    close(pipeFds[1]);

    activity.wait();

    // should be below timeout
    auto timespan = Poco::DateTime() - startTime;
    CPPUNIT_ASSERT(timespan.totalMilliseconds() < 5000);
}

void AsyncSystemStarterTest::testWaitTimeout()
{
    // this test verifies that testWaitForProcessStarted can read back correctly with a pipe

    // create pipe
    int pipeFds[2];
    CPPUNIT_ASSERT(pipe(pipeFds) == 0);
    readFd = pipeFds[0];
    CPPUNIT_ASSERT(readFd != -1);
    CPPUNIT_ASSERT_EQUAL(0, fcntl(readFd, F_SETFL, O_NONBLOCK));

    Poco::Activity<AsyncSystemStarterTest> activity(this, &AsyncSystemStarterTest::testWaitFd_helper);
    const Poco::DateTime startTime;
    activity.start();
    activity.wait();

    write(pipeFds[1], "1", 1);
    close(pipeFds[1]);

    // should be below timeout
    auto timespan = Poco::DateTime() - startTime;
    CPPUNIT_ASSERT(timespan.totalMilliseconds() >= 5000);
    CPPUNIT_ASSERT(timespan.totalMilliseconds() <= 10000);
}

void AsyncSystemStarterTest::testCreateArguments()
{
    Process process;
    process.useTerminal = true;
    process.terminalPath = std::string("/bin/sh");
    process.processName = std::string("foo");

    process.terminalArguments = std::vector<std::string>{
        std::string{"-arg1"},
        std::string{"-arg2"},
        std::string{"-arg3"}
    };
    process.batch = false;
    process.weldMasterApplication = true;
    process.arguments = std::vector<std::string>{
        std::string{"-parg1"},
        std::string{"-parg2"},
        std::string{"-parg3"},
        std::string{"-parg4"}

    };

    const std::vector<char*> arguments = createProcessArguments(process, std::string("myPipe"));
    CPPUNIT_ASSERT_EQUAL(std::size_t{12}, arguments.size());
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(0), process.terminalPath.c_str()));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(1), "-arg1"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(2), "-arg2"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(3), "-arg3"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(4), "/tmp/connectservertest//bin/foo"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(5), "-pipePath"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(6), "myPipe"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(7), "-parg1"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(8), "-parg2"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(9), "-parg3"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(10), "-parg4"));
    CPPUNIT_ASSERT(arguments.at(11) == nullptr);
    for (auto it = arguments.begin(); it != arguments.end(); ++it)
    {
        delete[] *it;
    }
}

void AsyncSystemStarterTest::testCreateArgumentsNoPipe()
{
    Process process;
    process.useTerminal = true;
    process.terminalPath = std::string("/bin/sh");
    process.processName = std::string("foo");

    process.terminalArguments = std::vector<std::string>{
        std::string{"-arg1"},
        std::string{"-arg2"},
        std::string{"-arg3"}
    };
    process.batch = false;
    process.weldMasterApplication = true;
    process.arguments = std::vector<std::string>{
        std::string{"-parg1"},
        std::string{"-parg2"},
        std::string{"-parg3"},
        std::string{"-parg4"}

    };

    const std::vector<char*> arguments = createProcessArguments(process, std::string());
    CPPUNIT_ASSERT_EQUAL(std::size_t{10}, arguments.size());
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(0), process.terminalPath.c_str()));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(1), "-arg1"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(2), "-arg2"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(3), "-arg3"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(4), "/tmp/connectservertest//bin/foo"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(5), "-parg1"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(6), "-parg2"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(7), "-parg3"));
    CPPUNIT_ASSERT_EQUAL(0, strcmp(arguments.at(8), "-parg4"));
    CPPUNIT_ASSERT(arguments.at(9) == nullptr);
    for (auto it = arguments.begin(); it != arguments.end(); ++it)
    {
        delete[] *it;
    }
}

void AsyncSystemStarterTest::testCreatePipe()
{
    Poco::Path pipePath = configDirPath;
    pipePath.setFileName(std::string("testPipe"));
    int fd = createPipe(pipePath.toString());
    CPPUNIT_ASSERT(fd != -1);
    close(fd);
    // for the path again - should fail
    fd = createPipe(pipePath.toString());
    CPPUNIT_ASSERT_EQUAL(-1, fd);
    // and a pipe path which doesn't exist
    fd = createPipe(std::string("/this/path/does/not/exist"));
    CPPUNIT_ASSERT_EQUAL(-1, fd);
}

void AsyncSystemStarterTest::createConfig(const std::string &argument)
{
    Poco::AutoPtr<PropertyFileConfiguration> pConf{new PropertyFileConfiguration()};
    pConf->setString(std::string("Startup.00"), std::string("Foo"));
    pConf->setString(std::string("Foo.Name"), std::string("Bar"));
    pConf->setString(std::string("Foo.Arguments"), argument);
    pConf->setString(std::string("TerminalApplication.Path"), std::string("/usr/local/bin/sh"));

    Path configFilePath = configDirPath;
    configFilePath.setFileName("startup.config");

    pConf->save(configFilePath.toString());
}

TEST_MAIN(AsyncSystemStarterTest)
