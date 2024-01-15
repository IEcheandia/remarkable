#include <QTest>
#include <QObject>
#include <QSignalSpy>

#include "Scheduler/signalManager.h"
#include "Scheduler/taskScheduler.h"
#include "Scheduler/cronTrigger.h"
#include "Scheduler/testTask.h"
#include "Scheduler/testTrigger.h"

#include <chrono>
#include <thread>

using namespace std::chrono_literals;

using precitec::scheduler::AbstractTask;
using precitec::scheduler::AbstractTrigger;
using precitec::scheduler::CronTrigger;
using precitec::scheduler::SignalManager;
using precitec::scheduler::SignalManagerFactory;
using precitec::scheduler::TaskScheduler;
using precitec::scheduler::TestTask;
using precitec::scheduler::TestTrigger;

class TaskSchedulerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testAddOneSignalManager();
    void testAddTwoSignalManagersWithTheSameId();
    void testAddTwoSignalManagersWithDifferentIds();
    void testAddSignalManagerVector();
    void testRemoveSignalManager();
    void testStartOneSignalManagerJob();
    void testStartTwoSignalManagerJobs();
    void testCronTrigger();
};

void TaskSchedulerTest::testCtor()
{
    TaskScheduler scheduler;
}

void TaskSchedulerTest::testAddOneSignalManager()
{
    TaskScheduler scheduler;

    auto signalManager = SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                                    precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                                    Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    scheduler.addSignalManager(signalManager);

    QCOMPARE(scheduler.signalManagers().size(), 1);
}

void TaskSchedulerTest::testAddTwoSignalManagersWithTheSameId()
{
    TaskScheduler scheduler;

    auto signalManager = SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                                    precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                                    Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    scheduler.addSignalManager(signalManager);
    scheduler.addSignalManager(signalManager);

    QCOMPARE(scheduler.signalManagers().size(), 1);
}

void TaskSchedulerTest::testAddTwoSignalManagersWithDifferentIds()
{
    TaskScheduler scheduler;

    auto signalManager = SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                                    precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                                    Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto secondSignalManager =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ba975c11-8acc-4c30-b5cc-85f1b34dc13b"));

    scheduler.addSignalManager(signalManager);
    scheduler.addSignalManager(secondSignalManager);

    QCOMPARE(scheduler.signalManagers().size(), 2);
}

void TaskSchedulerTest::testAddSignalManagerVector()
{
    TaskScheduler scheduler;

    auto signalManager = SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                                    precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                                    Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto secondSignalManager =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ba975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto thirdSignalManager =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ba975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    std::set<SignalManager> signalManagers;
    signalManagers.insert(signalManager);
    signalManagers.insert(secondSignalManager);
    signalManagers.insert(thirdSignalManager);

    scheduler.addSignalManagers(signalManagers);

    QCOMPARE(scheduler.signalManagers().size(), 2);
}

void TaskSchedulerTest::testRemoveSignalManager()
{
    TaskScheduler scheduler;

    auto signalManager = SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask",{{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                                    precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                                    Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto secondSignalManager =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask",{{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ba975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto thirdSignalManager =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ba975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    std::set<SignalManager> signalManagers;
    signalManagers.insert(signalManager);
    signalManagers.insert(secondSignalManager);

    scheduler.addSignalManagers(signalManagers);
    scheduler.removeSignalManager(thirdSignalManager);
    QCOMPARE(scheduler.signalManagers().size(), 1);
}

void TaskSchedulerTest::testStartOneSignalManagerJob()
{

    TaskScheduler scheduler;

    auto signalManager = SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                                    precitec::scheduler::TriggerFactory().make("TestTrigger", {{"hey ho", "!!!!!"}}),
                                                    Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    scheduler.addSignalManager(signalManager);
    scheduler.setUpdateTime(0);
    scheduler.start();
    while (TestTask::callNumber < 1)
    {
        std::this_thread::sleep_for(1ms);
    }
    scheduler.stop();

    QCOMPARE(TestTask::callNumber, 3);
    TestTask::callNumber = 0;
}

void TaskSchedulerTest::testStartTwoSignalManagerJobs()
{
    TaskScheduler scheduler;
    auto signalManagerFirst =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto signalManagerSecond =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("TestTrigger", {{"","!!!!!"}}),
                                   Poco::UUID("ba975c11-8acc-4c30-b5cc-85f1b34dc13b"));

    scheduler.addSignalManager(signalManagerFirst);
    scheduler.addSignalManager(signalManagerSecond);
    scheduler.setUpdateTime(1);

    scheduler.start();
    while (TestTask::callNumber < 2)
    {
        std::this_thread::sleep_for(1ms);
    }
    scheduler.stop();

    TestTask::callNumber = 0;
}

void TaskSchedulerTest::testCronTrigger()
{
    TaskScheduler scheduler;
    auto signalManager =
        SignalManagerFactory::make(precitec::scheduler::TaskFactory().make("TestTask", {{"IpAddressSender","192.168.6.62"},{"IpAddressReceiver","192.168.6.80"}}),
                                   precitec::scheduler::TriggerFactory().make("CronTrigger", {{"cron", "* * * * * *"}}),
                                   Poco::UUID("ea975c11-8acc-4c30-b5cc-85f1b34dc13b"));
    auto trigger = dynamic_cast<CronTrigger *>(signalManager.trigger());
    int seconds = 10;
    Poco::DateTime currentTime;
    trigger->setBeginObservationalPeriod((currentTime.timestamp() - seconds * 1000000).epochTime());
    scheduler.addSignalManager(signalManager);
    scheduler.setUpdateTime(2);
    scheduler.start();
    while (TestTask::callNumber < 1)
    {
        std::this_thread::sleep_for(1ms);
    }
    scheduler.stop();

    TestTask::callNumber = 0;
}

QTEST_GUILESS_MAIN(TaskSchedulerTest)
#include "taskSchedulerTest.moc"