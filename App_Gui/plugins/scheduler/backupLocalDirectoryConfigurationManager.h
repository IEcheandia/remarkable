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
 * Configuration manager to load and store backup to local directory settings.
 **/
class BackupLocalDirectoryConfigurationManager : public QObject
{
    Q_OBJECT
    /**
     * Path to the directory where the backup will be stored
     **/
    Q_PROPERTY(QString path READ path WRITE setPath NOTIFY pathChanged)
    /**
     * Whether configuration directory is included in backup
     **/
    Q_PROPERTY(bool configuration READ configuration WRITE setConfiguration NOTIFY configurationChanged)
    /**
     * Whether logfiles directory is included in backup
     **/
    Q_PROPERTY(bool logFiles READ logFiles WRITE setLogFiles NOTIFY logFilesChanged)
    /**
     * Whether screenshots directory is included in backup
     **/
    Q_PROPERTY(bool screenshots READ screenshots WRITE setScreenshots NOTIFY screenshotsChanged)
    /**
     * Whether currently used software is included in backup
     **/
    Q_PROPERTY(bool software READ software WRITE setSoftware NOTIFY softwareChanged)

    /**
     * The settings containing the configuration as a key-value map
     */
    Q_PROPERTY(QVariantMap settings READ settings WRITE setSettings NOTIFY settingsChanged)
public:
    BackupLocalDirectoryConfigurationManager(QObject *parent = nullptr);
    ~BackupLocalDirectoryConfigurationManager() override;

    QString path() const
    {
        return m_path;
    }
    void setPath(const QString &path);

    bool configuration() const
    {
        return m_configuration;
    }
    void setConfiguration(bool set);

    bool logFiles() const
    {
        return m_logFiles;
    }
    void setLogFiles(bool set);

    bool screenshots() const
    {
        return m_screenshots;
    }
    void setScreenshots(bool set);

    bool software() const
    {
        return m_software;
    }
    void setSoftware(bool set);

    QVariantMap settings() const
    {
        return m_settings;
    }
    void setSettings(const QVariantMap &settings);

    Q_INVOKABLE QVariantMap save() const;
    Q_INVOKABLE QVariantMap save(const QVariantMap &base) const;

Q_SIGNALS:
    void pathChanged();
    void configurationChanged();
    void logFilesChanged();
    void screenshotsChanged();
    void softwareChanged();
    void settingsChanged();

private:
    void init();
    QString m_path;
    bool m_configuration = false;
    bool m_logFiles = false;
    bool m_screenshots = false;
    bool m_software = false;

    QVariantMap m_settings;

};


}
}
}
}
