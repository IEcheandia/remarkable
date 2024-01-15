#include <QTest>
#include <QObject>

#include <memory>

#include "Scheduler/signalManager.h"
#include "Scheduler/testTask.h"
#include "Scheduler/testTrigger.h"

using precitec::scheduler::AbstractTask;
using precitec::scheduler::AbstractTrigger;
using precitec::scheduler::SignalManager;
using precitec::scheduler::SignalManagerFactory;
using precitec::scheduler::TestTask;
using precitec::scheduler::TestTrigger;

class SignalManagerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testInitializeSignalManagerWithTestTaskAndTrigger();
    void testSignalManagerFromJson();
    void testTaskErrorNameSignalManagerFromJson();
    void testErrorErrorNameSignalManagerFromJson();
    void testTaskErrorNameValueSignalManagerFromJson();
    void testErrorErrorNameValueSignalManagerFromJson();
};

void SignalManagerTest::testCtor()
{
    SignalManager signalManager;
}

void SignalManagerTest::testInitializeSignalManagerWithTestTaskAndTrigger()
{
    SignalManagerFactory signalManagerFactory;
    auto manager = signalManagerFactory.make(precitec::scheduler::TaskFactory().make("TestTask",{{"IdAddressSender","192.168.6.62"},
                                                                                                 {"IdAddressReceiver","192.168.63.12"}}),
                                             precitec::scheduler::TriggerFactory().make("TestTrigger", {{"test","!!!!!"}}),
                                             Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    std::stringstream ss;
    ss << manager.toJson();
    QCOMPARE(QString::fromStdString(ss.str()),"{\"Task\":"
                                              "{\"Name\":\"TestTask\","
                                              "\"Settings\":{\"IdAddressReceiver\":\"192.168.63.12\","
                                                            "\"IdAddressSender\":\"192.168.6.62\"}},"
                                              "\"Trigger\":"
                                              "{\"Name\":\"TestTrigger\","
                                              "\"Settings\":{}},"
                                              "\"Uuid\":\"ea975c11-8acc-4c30-b5cc-85f1b34dc13b\"}");
}

void SignalManagerTest::testSignalManagerFromJson()
{
    nlohmann::json json = R"({
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

    auto signalManager = SignalManagerFactory::make(json);
    AbstractTask *task = const_cast<AbstractTask *>(signalManager.task());
    QVERIFY(dynamic_cast<TestTask *>(task) != nullptr);
}

void SignalManagerTest::testTaskErrorNameSignalManagerFromJson()
{
    nlohmann::json json = R"({
                          "Task":
                          {
                                "Namefsdfsa": "TestTask",
                                "Settings": {"IdAddressSender"   : "192.168.6.62",
                                             "IdAddressReceiver" : "192.168.63.12"}
                          },
                          "Trigger":
                          {
                                "Name": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ea975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    auto signalManager = SignalManagerFactory::make(json);
    AbstractTask *task = const_cast<AbstractTask *>(signalManager.task());
    QVERIFY(dynamic_cast<TestTask *>(task) == nullptr);
}

void SignalManagerTest::testErrorErrorNameSignalManagerFromJson()
{
    nlohmann::json json = R"({
                          "Task":
                          {
                                "Name": "TestTask",
                                "Settings": {"IdAddressSender"   : "192.168.6.62",
                                             "IdAddressReceiver" : "192.168.63.12"}
                          },
                          "Trigger":
                          {
                                "Nameasdf": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ea975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    auto signalManager = SignalManagerFactory::make(json);
    AbstractTask *task = const_cast<AbstractTask *>(signalManager.task());
    QVERIFY(dynamic_cast<TestTask *>(task) == nullptr);
}

void SignalManagerTest::testTaskErrorNameValueSignalManagerFromJson()
{
    nlohmann::json json = R"({
                          "Task":
                          {
                                "Name": "ErrorTask!!!!!!!",
                                "Settings": {"IdAddressSender"   : "192.168.6.62",
                                             "IdAddressReceiver" : "192.168.63.12"}
                          },
                          "Trigger":
                          {
                                "Name": "TestTrigger",
                                "Settings": {}
                          },
                          "Uuid": "ea975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    auto signalManager = SignalManagerFactory::make(json);
    AbstractTask *task = const_cast<AbstractTask *>(signalManager.task());
    QVERIFY(dynamic_cast<TestTask *>(task) == nullptr);
}

void SignalManagerTest::testErrorErrorNameValueSignalManagerFromJson()
{
    nlohmann::json json = R"({
                          "Task":
                          {
                                "Name": "TestTask",
                                "Settings": {"IdAddressSender"   : "192.168.6.62",
                                             "IdAddressReceiver" : "192.168.63.12"}
                          },
                          "Trigger":
                          {
                                "Name": "ErrorTrigger!!!!!",
                                "Settings": {}
                          },
                          "Uuid": "ea975c11-8acc-4c30-b5cc-85f1b34dc13b"})"_json;

    auto signalManager = SignalManagerFactory::make(json);
    AbstractTask *task = const_cast<AbstractTask *>(signalManager.task());
    QVERIFY(dynamic_cast<TestTask *>(task) == nullptr);
}

QTEST_GUILESS_MAIN(SignalManagerTest)
#include "signalManagerTest.moc"
