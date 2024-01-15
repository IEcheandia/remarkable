#include <QTest>
#include <QObject>

#include "Scheduler/taskFactory.h"
#include "Scheduler/transferFileTask.h"

using precitec::scheduler::TaskFactory;
using precitec::scheduler::TransferFileTask;
using precitec::scheduler::TransferFileTaskFactory;

class TransferFileTaskTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testJsonTransferFileTaskFactory();
    void testCheckSettings();
};

void TransferFileTaskTest::testCtor()
{
    TransferFileTaskTest task;
}

void TransferFileTaskTest::testJsonTransferFileTaskFactory()
{
    TaskFactory taskFactory;

    auto json = R"({"Name": "TransferFileTask",
                    "Settings": {}})"_json;
    auto task = taskFactory.make(json);

    QVERIFY(dynamic_cast<precitec::scheduler::TransferFileTask *>(task.get()) != nullptr);
}

void TransferFileTaskTest::testCheckSettings()
{
    TaskFactory taskFactory;
    auto json = R"({"Name": "TransferFileTask",
                    "Settings": {"TargetIpAddress" : "192.168.6.62",
                                 "TargetUserName" : "sftp_user",
                                 "TargetPassword" : "Precitec01&",
                                 "SourceDirectoryPath" : "/home/alexander/Temp/SFTP_Data/Folder1/",
                                 "SourceFileName" : "File1.txt",
                                 "TargetDirectoryPath" : "FileFolder/",
                                 "TargetFileName"  : "Datei1.txt",
                                 "DebugOptionStatus" : "true"}})"_json;
    auto task = taskFactory.make(json);
    QVERIFY(dynamic_cast<precitec::scheduler::TransferFileTask *>(task.get())->checkSettings());
}

QTEST_GUILESS_MAIN(TransferFileTaskTest)
#include "transferFileTaskTest.moc"
