#include <QTest>
#include <QObject>

#include "Scheduler/taskFactory.h"
#include "Scheduler/resultExcelFileFromProductInstanceTask.h"

using precitec::scheduler::TaskFactory;

class ResultExcelFileFromProductInstanceTaskTest : public QObject
{
Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testJsonTransferDirectoryTaskFactory();
    void testCheckSettings();
};

void ResultExcelFileFromProductInstanceTaskTest::testCtor()
{
    precitec::scheduler::ResultExcelFileFromProductInstanceTask task;
}

void ResultExcelFileFromProductInstanceTaskTest::testJsonTransferDirectoryTaskFactory()
{
    TaskFactory taskFactory;

    auto json = R"({"Name": "ResultExcelFileFromProductInstanceTask",
                    "Settings": {}})"_json;
    auto task = taskFactory.make(json);

    QVERIFY(dynamic_cast<precitec::scheduler::ResultExcelFileFromProductInstanceTask *>(task.get()) != nullptr);
}

void ResultExcelFileFromProductInstanceTaskTest::testCheckSettings()
{
    TaskFactory taskFactory;
    auto json = R"({"Name": "ResultExcelFileFromProductInstanceTask",
                    "Settings": {"TargetIpAddress" : "192.168.6.62",
                                 "TargetUserName" : "sftp_user",
                                 "TargetPassword" : "Precitec01&",
                                 "TargetDirectoryPath" : "FileFolder/",
                                 "TargetFileName"  : "Datei1.txt",
                                 "ProductInstanceDirectory" : "/home/mlebid/wm_inst/opt/wm_inst/data/results/274a7eca-7654-4893-9d6a-351e6c688a95/991a2170-93f7-11ec-97be-e0d4643b6870-SN-1234/",
                                 "ProductStorageDirectory" : "/home/mlebid/wm_inst/opt/wm_inst/config/products/"}})"_json;

    auto task = taskFactory.make(json);
    QVERIFY(dynamic_cast<precitec::scheduler::ResultExcelFileFromProductInstanceTask *>(task.get())->checkSettings());
}

QTEST_GUILESS_MAIN(ResultExcelFileFromProductInstanceTaskTest)
#include "resultExcelFileFromProductInstanceTaskTest.moc"

