#include <QTest>
#include <QObject>

#include "Scheduler/eventTrigger.h"
#include "Scheduler/triggerFactory.h"

#include <Poco/DateTime.h>
using precitec::scheduler::EventTrigger;
using precitec::scheduler::EventTriggerFactory;
using precitec::scheduler::TriggerFactory;
class EventTriggerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testGetSignalInfo();
    void testGetSignalInfoWithTwoSignals();
    void testGetSignalInfoWithSettingMistake();
};

void EventTriggerTest::testCtor()
{
    EventTrigger trigger;
}

void EventTriggerTest::testGetSignalInfo()
{
    EventTrigger trigger;

    precitec::interface::SchedulerEvents event = precitec::interface::SchedulerEvents::ProductAdded;
    trigger.setSettings({{"event", std::to_string(static_cast<int>(event))}});

    trigger.schedulerEventFunction(event, {});

    QCOMPARE(trigger.getSignalInfo(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())).signalNumberDuringObservationalPeriod, 1);
    QCOMPARE(trigger.getSignalInfo(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())).signalNumberDuringObservationalPeriod, 0);
}

void EventTriggerTest::testGetSignalInfoWithTwoSignals()
{
    EventTrigger trigger;

    precitec::interface::SchedulerEvents event = precitec::interface::SchedulerEvents::ProductAdded;
    trigger.setSettings({{"event", std::to_string(static_cast<int>(event))}});
    trigger.schedulerEventFunction(event, {});
    trigger.schedulerEventFunction(event, {});

    QCOMPARE(trigger.getSignalInfo(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())).signalNumberDuringObservationalPeriod, 2);
}

void EventTriggerTest::testGetSignalInfoWithSettingMistake()
{
    EventTrigger trigger;

    precitec::interface::SchedulerEvents event = precitec::interface::SchedulerEvents::ProductAdded;
    trigger.setSettings({{"event", "settings with mistake"}});
    trigger.schedulerEventFunction(event, {});
    trigger.schedulerEventFunction(event, {});

    QCOMPARE(trigger.getSignalInfo(std::chrono::system_clock::to_time_t(std::chrono::system_clock::now())).signalNumberDuringObservationalPeriod, 0);
}

QTEST_GUILESS_MAIN(EventTriggerTest)
#include "eventTriggerTest.moc"