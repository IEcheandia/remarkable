#include <QTest>
#include <QObject>

#include "Scheduler/taskFactory.h"
#include "Scheduler/exportProductTask.h"

using precitec::scheduler::TaskFactory;
using precitec::scheduler::ExportProductTask;
using precitec::scheduler::ExportProductTaskFactory;

class ExportProductTaskTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testJsonExportProductTaskFactory();
    void testCheckSettings();
};

void ExportProductTaskTest::testCtor()
{
    ExportProductTaskTest task;
}

void ExportProductTaskTest::testJsonExportProductTaskFactory()
{
    TaskFactory taskFactory;

    auto json = R"({"Name": "ExportProductTask",
                    "Settings": {}})"_json;
    auto task = taskFactory.make(json);

    QVERIFY(dynamic_cast<precitec::scheduler::ExportProductTask *>(task.get()) != nullptr);
}

void ExportProductTaskTest::testCheckSettings()
{
    TaskFactory taskFactory;
    auto json = R"({"Name": "ExportProductTask",
                    "Settings": {"TargetIpAddress" : "192.168.6.62",
                                 "TargetUserName" : "sftp_user",
                                 "TargetPassword" : "Precitec01&",
                                 "TargetDirectoryPath" : "FileFolder/",
                                 "uuid" : "c2fd2292-260b-11ee-be56-0242ac120002"}})"_json;
    auto task = taskFactory.make(json);
    QVERIFY(dynamic_cast<precitec::scheduler::ExportProductTask *>(task.get())->checkSettings());
}

QTEST_GUILESS_MAIN(ExportProductTaskTest)
#include "exportProductTaskTest.moc"

