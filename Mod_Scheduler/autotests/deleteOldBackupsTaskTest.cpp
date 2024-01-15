#include <QTest>
#include <QObject>

#include "Scheduler/deleteOldBackupsTask.h"
#include "Scheduler/taskFactory.h"

using precitec::scheduler::DeleteOldBackupsTask;
using precitec::scheduler::DeleteOldBackupsTaskFactory;
using precitec::scheduler::TaskFactory;

class DeleteOldBackupsTaskTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testDeleteOldBackupsTaskFactory();
    void testCheckSettings();
};

void DeleteOldBackupsTaskTest::testCtor()
{
    DeleteOldBackupsTask task;
}

void DeleteOldBackupsTaskTest::testDeleteOldBackupsTaskFactory()
{
    TaskFactory taskFactory;

    auto json = R"({"Name": "DeleteOldBackupsTask",
                    "Settings": {}})"_json;
    auto task = taskFactory.make(json);

    QVERIFY(dynamic_cast<precitec::scheduler::DeleteOldBackupsTask *>(task.get()) != nullptr);
}

void DeleteOldBackupsTaskTest::testCheckSettings()
{
    TaskFactory taskFactory;
    auto json = R"({"Name": "DeleteOldBackupsTask",
                    "Settings": {"backupPath" : "/tmp",
                                 "TimeToLiveDays" : "20"}})"_json;
    auto task = taskFactory.make(json);
    QVERIFY(dynamic_cast<precitec::scheduler::DeleteOldBackupsTask *>(task.get())->checkSettings());
}

QTEST_GUILESS_MAIN(DeleteOldBackupsTaskTest)
#include "deleteOldBackupsTaskTest.moc"
