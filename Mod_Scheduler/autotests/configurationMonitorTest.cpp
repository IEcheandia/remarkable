#include <QTest>
#include <QObject>

#include "Scheduler/configurationMonitor.h"
#include "Scheduler/signalManagerWriter.h"
#include "Scheduler/taskScheduler.h"

#include <fstream>
#include <chrono>
#include <thread>

using namespace std::chrono_literals;

using precitec::scheduler::SignalManager;
using precitec::scheduler::TaskScheduler;
using precitec::scheduler::SignalManagerWriter;
using precitec::scheduler::SignalManagerFactory;

class ConfigurationMonitorTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
};

void ConfigurationMonitorTest::testCtor()
{
    QTemporaryDir m_dir;
    QVERIFY(m_dir.isValid());

    nlohmann::json json1 = R"({
                          "Task":
                          {
                                "Name": "TestTask",
                                "Settings": {"IdAddressSender": "192.168.6.62"}
                          },
                          "Trigger":
                          {
                                "Name": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ea975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;
    nlohmann::json json2 = R"({
                          "Task":
                          {
                                "Name": "TestTask",
                                "Settings": {"IdAddressSender": "192.168.6.62"}
                          },
                          "Trigger":
                          {
                                "Name": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ea975c22-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    SignalManagerWriter writer;
    std::string fileName = "scheduler.json";
    std::string fullFileName = m_dir.path().toStdString() + "/" + fileName;

    std::set<SignalManager> signalManagers1 = {SignalManagerFactory::make(json1)};
    std::ofstream oFile1;
    oFile1.open(fullFileName);
    writer.write(oFile1, signalManagers1);
    oFile1.flush();

    TaskScheduler taskScheduler;
    taskScheduler.rewriteSignalManagers(signalManagers1);
    precitec::scheduler::ConfigurationMonitor configurationMonitor(taskScheduler, m_dir.path().toStdString(), fileName);

    configurationMonitor.monitor();
    QVERIFY(taskScheduler.signalManagers().size() == 1);
    std::set<SignalManager> signalManagers2 = {SignalManagerFactory::make(json1), SignalManagerFactory::make(json2)};
    std::ofstream oFile2;
    oFile2.open(fullFileName);
    writer.write(oFile2, signalManagers2);
    oFile2.flush();

    //QTRY_COMPARE_WITH_TIMEOUT(taskScheduler.signalManagers().size(), 2, 60000);
}


QTEST_GUILESS_MAIN(ConfigurationMonitorTest)
#include "configurationMonitorTest.moc"
