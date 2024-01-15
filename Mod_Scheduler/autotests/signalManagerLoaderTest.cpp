#include <QTest>
#include <QObject>

#include <memory>

#include "Scheduler/signalManagerLoader.h"
#include "Scheduler/testTask.h"
#include "Scheduler/testTrigger.h"

using precitec::scheduler::SignalManager;
using precitec::scheduler::SignalManagerFactory;
using precitec::scheduler::SignalManagerLoader;
using precitec::scheduler::TestTask;
using precitec::scheduler::TestTrigger;

class SignalManagerLoaderTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testInitializedLoader();
    void testTwoSignalManagersLoader();
};

void SignalManagerLoaderTest::testInitializedLoader()
{
    SignalManagerLoader loader;
    std::stringstream ss;
    loader.load(ss);
}

void SignalManagerLoaderTest::testTwoSignalManagersLoader()
{
    nlohmann::json json1 = R"({
                          "Task":
                          {
                                "Name": "TestTask",
                                "Settings": {"IdAddressSender"   : "192.168.6.62",
                                             "IdAddressReceiver" : "192.168.63.12"}
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
                                "Settings": {"IdAddressSender"   : "192.168.6.62",
                                             "IdAddressReceiver" : "192.168.63.12"}
                          },
                          "Trigger":
                          {
                                "Name": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ba975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    nlohmann::json mutualJson;
    mutualJson.push_back(json1);
    mutualJson.push_back(json2);

    std::stringstream stringStream;
    stringStream << mutualJson;

    SignalManagerLoader loader;
    auto resultSignalManagers = loader.load(stringStream);
    QCOMPARE(resultSignalManagers.size(), 2);
}

QTEST_GUILESS_MAIN(SignalManagerLoaderTest)
#include "signalManagerLoaderTest.moc"