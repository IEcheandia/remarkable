#include "backupLocalDirectoryConfigurationManager.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

BackupLocalDirectoryConfigurationManager::BackupLocalDirectoryConfigurationManager(QObject* parent)
    : QObject(parent)
{
    connect(this, &BackupLocalDirectoryConfigurationManager::settingsChanged, this, &BackupLocalDirectoryConfigurationManager::init);
}

BackupLocalDirectoryConfigurationManager::~BackupLocalDirectoryConfigurationManager() = default;

void BackupLocalDirectoryConfigurationManager::setPath(const QString& path)
{
    if (m_path == path)
    {
        return;
    }
    m_path = path;
    emit pathChanged();
}

void BackupLocalDirectoryConfigurationManager::setSettings(const QVariantMap& settings)
{
    if (m_settings == settings)
    {
        return;
    }
    m_settings = settings;
    emit settingsChanged();
}

void BackupLocalDirectoryConfigurationManager::setConfiguration(bool set)
{
    if (m_configuration == set)
    {
        return;
    }
    m_configuration = set;
    emit configurationChanged();
}

void BackupLocalDirectoryConfigurationManager::setLogFiles(bool set)
{
    if (m_logFiles == set)
    {
        return;
    }
    m_logFiles = set;
    emit logFilesChanged();
}

void BackupLocalDirectoryConfigurationManager::setScreenshots(bool set)
{
    if (m_screenshots == set)
    {
        return;
    }
    m_screenshots = set;
    emit screenshotsChanged();
}

void BackupLocalDirectoryConfigurationManager::setSoftware(bool set)
{
    if (m_software == set)
    {
        return;
    }
    m_software = set;
    emit softwareChanged();
}

void BackupLocalDirectoryConfigurationManager::init()
{
    if (auto it = m_settings.find(QStringLiteral("backupPath")); it != m_settings.end())
    {
        setPath(it.value().toString());
    }
    if (auto it = m_settings.find(QStringLiteral("config")); it != m_settings.end())
    {
        setConfiguration(it.value().toBool());
    }
    else
    {
        setConfiguration(false);
    }
    if (auto it = m_settings.find(QStringLiteral("logs")); it != m_settings.end())
    {
        setLogFiles(it.value().toBool());
    }
    else
    {
        setLogFiles(false);
    }
    if (auto it = m_settings.find(QStringLiteral("screenshots")); it != m_settings.end())
    {
        setScreenshots(it.value().toBool());
    }
    else
    {
        setScreenshots(false);
    }
    if (auto it = m_settings.find(QStringLiteral("software")); it != m_settings.end())
    {
        setSoftware(it.value().toBool());
    }
    else
    {
        setSoftware(false);
    }
}

QVariantMap BackupLocalDirectoryConfigurationManager::save() const
{
    return QVariantMap{
        {QStringLiteral("backupPath"), path()},
        {QStringLiteral("config"), configuration() ? QStringLiteral("true") : QStringLiteral("false")},
        {QStringLiteral("logs"), logFiles() ? QStringLiteral("true") : QStringLiteral("false")},
        {QStringLiteral("screenshots"), screenshots() ? QStringLiteral("true") : QStringLiteral("false")},
        {QStringLiteral("software"), software() ? QStringLiteral("true") : QStringLiteral("false")},
    };
}

QVariantMap BackupLocalDirectoryConfigurationManager::save(const QVariantMap &base) const
{
    auto map{save()};
    map.insert(base);
    return map;
}

}
}
}
}
