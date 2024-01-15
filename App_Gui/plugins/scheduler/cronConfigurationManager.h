#pragma once

#include <QObject>
#include <QVariant>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

/**
 * Configuration manager to load and store cron settings.
 *
 * Takes the @link settings and processes the cron string to split it into individual parts.
 * Supported are:
 * @li minute (* or specific)
 * @li hour (* or specific)
 * @li day of month (* or 1 to 31)
 * @li month (*, 0 for none or 1 to 12)
 * @li day of week (0: Sunday, 1: Monday, 7: Sunday)
 **/
class CronConfigurationManager : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool everyMinute READ everyMinute WRITE setEveryMinute NOTIFY everyMinuteChanged)
    Q_PROPERTY(bool everyHour READ everyHour WRITE setEveryHour NOTIFY everyHourChanged)
    Q_PROPERTY(bool everyDayOfMonth READ everyDayOfMonth WRITE setEveryDayOfMonth NOTIFY everyDayOfMonthChanged)
    Q_PROPERTY(bool everyMonth READ everyMonth WRITE setEveryMonth NOTIFY everyMonthChanged)
    Q_PROPERTY(bool everyDayOfWeek READ everyDayOfWeek WRITE setEveryDayOfWeek NOTIFY everyDayOfWeekChanged)
    Q_PROPERTY(int specificMinute READ specificMinute WRITE setSpecificMinute NOTIFY specificMinuteChanged)
    Q_PROPERTY(int specificHour READ specificHour WRITE setSpecificHour NOTIFY specificHourChanged)
    Q_PROPERTY(int specificDayOfMonth READ specificDayOfMonth WRITE setSpecificDayOfMonth NOTIFY specificDayOfMonthChanged)
    Q_PROPERTY(int specificMonth READ specificMonth WRITE setSpecificMonth NOTIFY specificMonthChanged)
    Q_PROPERTY(int specificDayOfWeek READ specificDayOfWeek WRITE setSpecificDayOfWeek NOTIFY specificDayOfWeekChanged)

    Q_PROPERTY(QVariantMap settings READ settings WRITE setSettings NOTIFY settingsChanged)
public:
    CronConfigurationManager(QObject *parent = nullptr);
    ~CronConfigurationManager() override;

    bool everyMinute() const
    {
        return m_everyMinute;
    }
    void setEveryMinute(bool set);

    bool everyHour() const
    {
        return m_everyHour;
    }
    void setEveryHour(bool set);

    bool everyDayOfMonth() const
    {
        return m_everyDayOfMonth;
    }
    void setEveryDayOfMonth(bool set);

    bool everyMonth() const
    {
        return m_everyMonth;
    }
    void setEveryMonth(bool set);

    bool everyDayOfWeek() const
    {
        return m_everyDayOfWeek;
    }
    void setEveryDayOfWeek(bool set);

    int specificMinute() const
    {
        return m_specificMinute;
    }
    void setSpecificMinute(int value);

    int specificHour() const
    {
        return m_specificHour;
    }
    void setSpecificHour(int value);

    int specificDayOfMonth() const
    {
        return m_specificDayOfMonth;
    }
    void setSpecificDayOfMonth(int value);

    int specificMonth() const
    {
        return m_specificMonth;
    }
    void setSpecificMonth(int value);

    int specificDayOfWeek() const
    {
        return m_specificDayOfWeek;
    }
    void setSpecificDayOfWeek(int value);

    QVariantMap settings() const
    {
        return m_settings;
    }
    void setSettings(const QVariantMap &settings);

    Q_INVOKABLE QVariantMap save() const;

Q_SIGNALS:
    void everyMinuteChanged();
    void everyHourChanged();
    void everyDayOfMonthChanged();
    void everyMonthChanged();
    void everyDayOfWeekChanged();
    void specificMinuteChanged();
    void specificHourChanged();
    void specificDayOfMonthChanged();
    void specificMonthChanged();
    void specificDayOfWeekChanged();
    void settingsChanged();

private:
    void init();
    bool m_everyMinute = true;
    bool m_everyHour = true;
    bool m_everyDayOfMonth = true;
    bool m_everyMonth = true;
    bool m_everyDayOfWeek = true;

    int m_specificMinute = 0;
    int m_specificHour = 0;
    int m_specificDayOfMonth = 1;
    int m_specificMonth = 0;
    int m_specificDayOfWeek = 0;

    QVariantMap m_settings;

};

}
}
}
}
