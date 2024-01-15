#include <QTest>
#include <QObject>

#include "Scheduler/taskFactory.h"
#include "Scheduler/transferDirectoryTask.h"

using precitec::scheduler::TaskFactory;
using precitec::scheduler::TransferDirectoryTask;
using precitec::scheduler::TransferDirectoryTaskFactory;

class TransferDirectoryTaskTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testJsonTransferDirectoryTaskFactory();
    void testCheckSettings();
};

void TransferDirectoryTaskTest::testCtor()
{
    TransferDirectoryTaskTest task;
}

void TransferDirectoryTaskTest::testJsonTransferDirectoryTaskFactory()
{
    TaskFactory taskFactory;

    auto json = R"({"Name": "TransferDirectoryTask",
                    "Settings": {}})"_json;
    auto task = taskFactory.make(json);

    QVERIFY(dynamic_cast<precitec::scheduler::TransferDirectoryTask *>(task.get()) != nullptr);
}

void TransferDirectoryTaskTest::testCheckSettings()
{
    TaskFactory taskFactory;
    auto json = R"({"Name": "TransferDirectoryTask",
                    "Settings": {"TargetIpAddress" : "192.168.6.62",
                                 "TargetUserName" : "sftp_user",
                                 "TargetPassword" : "Precitec01&",
                                 "SourceDirectoryPath" : "/home/alexander/Temp/SFTP_Data/Folder1/",
                                 "SourceDirectoryName" : "Directory1.txt",
                                 "TargetDirectoryPath" : "DirectoryFolder/",
                                 "TargetDirectoryName"  : "Datei1.txt",
                                 "DebugOptionStatus" : "true"}})"_json;
    auto task = taskFactory.make(json);
    QVERIFY(dynamic_cast<precitec::scheduler::TransferDirectoryTask *>(task.get())->checkSettings());
}

QTEST_GUILESS_MAIN(TransferDirectoryTaskTest)
#include "transferDirectoryTaskTest.moc"

