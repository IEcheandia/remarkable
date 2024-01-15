#include "cronConfigurationManager.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

CronConfigurationManager::CronConfigurationManager(QObject *parent)
    : QObject(parent)
{
    connect(this, &CronConfigurationManager::settingsChanged, this, &CronConfigurationManager::init);
}

CronConfigurationManager::~CronConfigurationManager() = default;

void CronConfigurationManager::setEveryMinute(bool set)
{
    if (m_everyMinute == set)
    {
        return;
    }
    m_everyMinute = set;
    emit everyMinuteChanged();
}

void CronConfigurationManager::setEveryHour(bool set)
{
    if (m_everyHour == set)
    {
        return;
    }
    m_everyHour = set;
    emit everyHourChanged();
}

void CronConfigurationManager::setEveryDayOfMonth(bool set)
{
    if (m_everyDayOfMonth == set)
    {
        return;
    }
    m_everyDayOfMonth = set;
    emit everyDayOfMonthChanged();
}

void CronConfigurationManager::setEveryMonth(bool set)
{
    if (m_everyMonth == set)
    {
        return;
    }
    m_everyMonth = set;
    emit everyMonthChanged();
}

void CronConfigurationManager::setEveryDayOfWeek(bool set)
{
    if (m_everyDayOfWeek == set)
    {
        return;
    }
    m_everyDayOfWeek = set;
    emit everyDayOfWeekChanged();
}

void CronConfigurationManager::setSpecificMinute(int value)
{
    if (m_specificMinute == value)
    {
        return;
    }
    m_specificMinute = value;
    emit specificMinuteChanged();
}

void CronConfigurationManager::setSpecificHour(int value)
{
    if (m_specificHour == value)
    {
        return;
    }
    m_specificHour = value;
    emit specificHourChanged();
}

void CronConfigurationManager::setSpecificDayOfMonth(int value)
{
    if (m_specificDayOfMonth == value)
    {
        return;
    }
    m_specificDayOfMonth = value;
    emit specificDayOfMonthChanged();
}

void CronConfigurationManager::setSpecificMonth(int value)
{
    if (m_specificMonth == value)
    {
        return;
    }
    m_specificMonth = value;
    emit specificMonthChanged();
}

void CronConfigurationManager::setSpecificDayOfWeek(int value)
{
    if (m_specificDayOfWeek == value)
    {
        return;
    }
    m_specificDayOfWeek = value;
    emit specificDayOfWeekChanged();
}

void CronConfigurationManager::setSettings(const QVariantMap &settings)
{
    if (m_settings == settings)
    {
        return;
    }
    m_settings = settings;
    emit settingsChanged();
}

void CronConfigurationManager::init()
{
    auto it = m_settings.find(QStringLiteral("cron"));
    if (it == m_settings.end())
    {
        return;
    }
    auto parts = it->toString().split(QStringLiteral(" "));
    if (parts.size() == 6)
    {
        parts.pop_front();
    }
    if (parts.size() != 5)
    {
        return;
    }
    for (int i = 0; i < parts.size(); i++)
    {
        const bool every = parts.at(i) == QStringLiteral("*");
        int value = 0;
        if (!every)
        {
            bool ok = false;
            value = parts.at(i).toInt(&ok);
            if (!ok)
            {
                value = 0;
            }
        }
        switch (i)
        {
        case 0:
            setSpecificMinute(value);
            setEveryMinute(every);
            break;
        case 1:
            setSpecificHour(value);
            setEveryHour(every);
            break;
        case 2:
            setSpecificDayOfMonth(value);
            setEveryDayOfMonth(every);
            break;
        case 3:
            setSpecificMonth(value);
            setEveryMonth(every);
            break;
        case 4:
            setSpecificDayOfWeek(value);
            setEveryDayOfWeek(every);
            break;
        default:
            __builtin_unreachable();
        }
    }
}

QVariantMap CronConfigurationManager::save() const
{
    QString cron = QStringLiteral("0 ");
    cron.append(everyMinute() ? QStringLiteral("*") : QString::number(specificMinute()));
    cron.append(QStringLiteral(" "));
    cron.append(everyHour() ? QStringLiteral("*") : QString::number(specificHour()));
    cron.append(QStringLiteral(" "));
    cron.append(everyDayOfMonth() ? QStringLiteral("*") : QString::number(specificDayOfMonth()));
    cron.append(QStringLiteral(" "));
    cron.append(everyMonth() ? QStringLiteral("*") : QString::number(specificMonth()));
    cron.append(QStringLiteral(" "));
    cron.append(everyDayOfWeek() ? QStringLiteral("*") : QString::number(specificDayOfWeek()));

    return QVariantMap{{QStringLiteral("cron"), cron}};
}

}
}
}
}
