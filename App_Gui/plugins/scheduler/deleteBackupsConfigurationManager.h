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
 * Configuration manager to delete backups older than n days from local directory.
 **/
class DeleteBackupsConfigurationManager : public QObject
{
    Q_OBJECT
    /**
     * Path to the directory where the backup will be stored
     **/
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    /**
     * Number of days to keep backups
     **/
    Q_PROPERTY(int timeToLive READ timeToLive WRITE setTimeToLive NOTIFY timeToLiveChanged)
    /**
     * The settings containing the configuration as a key-value map
     */
    Q_PROPERTY(QVariantMap settings READ settings WRITE setSettings NOTIFY settingsChanged)
public:
    DeleteBackupsConfigurationManager(QObject *parent = nullptr);
    ~DeleteBackupsConfigurationManager() override;

    QString path() const
    {
        return m_path;
    }
    void setPath(const QString &path);

    int timeToLive() const
    {
        return m_timeToLive;
    }
    void setTimeToLive(int ttl);

    QVariantMap settings() const
    {
        return m_settings;
    }
    void setSettings(const QVariantMap &settings);

    Q_INVOKABLE QVariantMap save() const;

Q_SIGNALS:
    void pathChanged();
    void timeToLiveChanged();
    void settingsChanged();

private:
    void init();
    QString m_path;
    int m_timeToLive{0};

    QVariantMap m_settings;

};


}
}
}
}
