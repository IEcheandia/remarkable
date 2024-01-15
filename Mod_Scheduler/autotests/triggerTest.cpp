#include <QTest>
#include <QObject>

#include <sstream>
#include <memory>

#include "Scheduler/testTrigger.h"
#include "Scheduler/triggerFactory.h"

using precitec::scheduler::TestTrigger;
using precitec::scheduler::TestTriggerFactory;
using precitec::scheduler::TriggerFactory;

class TriggerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testInitializeOneTrigger();
    void testTestTriggerFactoryType();
    void testTriggerFactoryType();
    void testJson();
    void testJsonFactory();
};

void TriggerTest::testCtor()
{
    TestTrigger task;
}

void TriggerTest::testInitializeOneTrigger()
{
    TestTrigger trigger;
    trigger.setSettings({{"test", "*****"}});
    QCOMPARE(QString::fromStdString(trigger.settings().at("test")), QStringLiteral("*****"));
}

void TriggerTest::testTestTriggerFactoryType()
{
    TestTriggerFactory triggerFactory;
    auto trigger = triggerFactory.make({{"asd", "*****"}});
    QCOMPARE(trigger->name(), "TestTrigger");
}

void TriggerTest::testTriggerFactoryType()
{
    TriggerFactory triggerFactory;
    auto task = triggerFactory.make("TestTrigger", {{"test", "*****"}});

    QVERIFY(dynamic_cast<precitec::scheduler::TestTrigger *>(task.get()) != nullptr);
}

void TriggerTest::testJson()
{
    TriggerFactory triggerFactory;
    auto trigger = triggerFactory.make("TestTrigger", {{"test", "*****"}});
    std::stringstream ss;
    ss << trigger->toJson();
    QCOMPARE(QString::fromStdString(ss.str()), "{\"Name\":\"TestTrigger\",\"Settings\":{}}");
}

void TriggerTest::testJsonFactory()
{
    nlohmann::json j = R"({"Name": "TestTrigger",
                           "Settings": {"sdfsdf" : "-----"}})"_json;
    TriggerFactory taskFactory;
    auto task = taskFactory.make(j);
    QVERIFY(dynamic_cast<precitec::scheduler::TestTrigger *>(task.get()) != nullptr);
}

QTEST_GUILESS_MAIN(TriggerTest)
#include "triggerTest.moc"
