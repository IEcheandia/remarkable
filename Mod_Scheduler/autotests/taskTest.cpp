#include <QTest>
#include <QObject>

#include <memory>

#include "Scheduler/testTask.h"
#include "Scheduler/taskFactory.h"

using precitec::scheduler::TaskFactory;
using precitec::scheduler::TestTask;
using precitec::scheduler::TestTaskFactory;

class TaskTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testInitializeOneTask();
    void testTestTaskFactoryType();
    void testTaskFactoryType();
    void testJson();
    void testJsonFactory();
};

void TaskTest::testCtor()
{
    TestTask task;
}

void TaskTest::testInitializeOneTask()
{
    TestTask task;

    task.setSettings({{"settings", "*****"}});

    QCOMPARE(task.settings()["settings"], QStringLiteral("*****").toStdString());
}

void TaskTest::testTestTaskFactoryType()
{
    TestTaskFactory taskFactory;

    auto task = taskFactory.make({{"", ""}});

    QCOMPARE(task->name(), "TestTask");
}

void TaskTest::testTaskFactoryType()
{
    TaskFactory taskFactory;

    auto task = taskFactory.make("TestTask", {{"",""}});

    QVERIFY(dynamic_cast<precitec::scheduler::TestTask *>(task.get()) != nullptr);
}

void TaskTest::testJson()
{
    TaskFactory taskFactory;

    auto task = taskFactory.make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}});
    std::stringstream ss;
    ss << task->toJson();

    QCOMPARE(QString::fromStdString(ss.str()), "{\"Name\":\"TestTask\",\"Settings\":{\"IpAddressReceiver\":\"192.168.6.80\",\"IpAddressSender\":\"192.168.6.62\"}}");
}

void TaskTest::testJsonFactory()
{
    TaskFactory taskFactory;

    auto json = R"({"Name": "TestTask",
                    "Settings": {"IdAddressSender": "192.168.6.62",
                                 "IpAddressReceiver": "192.168.6.62"}})"_json;
    auto task = taskFactory.make(json);

    QVERIFY(dynamic_cast<precitec::scheduler::TestTask *>(task.get()) != nullptr);
}

QTEST_GUILESS_MAIN(TaskTest)
#include "taskTest.moc"
