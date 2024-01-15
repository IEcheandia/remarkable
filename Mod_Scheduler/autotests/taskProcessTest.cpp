
#include <QTest>
#include <QObject>

#include <sstream>
#include <memory>

#include "Scheduler/testTaskProcess.h"

class TestTaskProcessTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRun();
};

void TestTaskProcessTest::testCtor()
{
    precitec::scheduler::TestTaskProcess process;
}

void TestTaskProcessTest::testRun()
{
    precitec::scheduler::TestTaskProcess process;
    process.setPath("./../src/SchedulerHelper/");
    QVERIFY(process.run());
}

QTEST_GUILESS_MAIN(TestTaskProcessTest)
#include "taskProcessTest.moc"

