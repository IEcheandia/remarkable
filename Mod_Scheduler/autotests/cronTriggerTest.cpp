#include <QTest>
#include <QObject>

#include "Scheduler/cronTrigger.h"
#include "Scheduler/triggerFactory.h"

#include <Poco/DateTime.h>
using precitec::scheduler::CronTrigger;
using precitec::scheduler::CronTriggerFactory;
using precitec::scheduler::TriggerFactory;
class CronTriggerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testGetSignalInfo();
    void testGetSignalInfoWithSettingMistake();
};

void CronTriggerTest::testCtor()
{
    CronTrigger trigger;
}

void CronTriggerTest::testGetSignalInfo()
{
    CronTrigger trigger;

    // Please see https://github.com/mariusbancila/croncpp
    trigger.setSettings({{"cron", "* * * * * *"}});
    Poco::DateTime currentTime(2022, 1, 1, 8, 30, 0);
    int seconds = 100;
    trigger.setBeginObservationalPeriod((currentTime.timestamp() - seconds * 1000000).epochTime());

    QCOMPARE(trigger.getSignalInfo(currentTime.timestamp().epochTime()).signalNumberDuringObservationalPeriod, seconds - 1);
}

void CronTriggerTest::testGetSignalInfoWithSettingMistake()
{
    CronTrigger trigger;

    trigger.setSettings({{"cron", "settings with mistake"}});
    Poco::DateTime currentTime(2022, 1, 1, 8, 30, 0);
    int seconds = 101;
    trigger.setBeginObservationalPeriod((currentTime.timestamp() - seconds * 1000000).epochTime());

    QCOMPARE(trigger.getSignalInfo(currentTime.timestamp().epochTime()).signalNumberDuringObservationalPeriod, 0);
}

QTEST_GUILESS_MAIN(CronTriggerTest)
#include "cronTriggerTest.moc"