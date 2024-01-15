#include "deleteBackupsConfigurationManager.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

DeleteBackupsConfigurationManager::DeleteBackupsConfigurationManager(QObject* parent)
    : QObject(parent)
{
    connect(this, &DeleteBackupsConfigurationManager::settingsChanged, this, &DeleteBackupsConfigurationManager::init);
}

DeleteBackupsConfigurationManager::~DeleteBackupsConfigurationManager() = default;

void DeleteBackupsConfigurationManager::setPath(const QString& path)
{
    if (m_path == path)
    {
        return;
    }
    m_path = path;
    emit pathChanged();
}

void DeleteBackupsConfigurationManager::setTimeToLive(int ttl)
{
    if (m_timeToLive == ttl)
    {
        return;
    }
    m_timeToLive = ttl;
    emit timeToLiveChanged();
}

void DeleteBackupsConfigurationManager::setSettings(const QVariantMap& settings)
{
    if (m_settings == settings)
    {
        return;
    }
    m_settings = settings;
    emit settingsChanged();
}

void DeleteBackupsConfigurationManager::init()
{
    if (auto it = m_settings.find(QStringLiteral("backupPath")); it != m_settings.end())
    {
        setPath(it.value().toString());
    }
    if (auto it = m_settings.find(QStringLiteral("TimeToLiveDays")); it != m_settings.end())
    {
        setTimeToLive(it.value().toString().toInt());
    }
}

QVariantMap DeleteBackupsConfigurationManager::save() const
{
    return QVariantMap{
        {QStringLiteral("backupPath"), path()},
        {QStringLiteral("TimeToLiveDays"), QString::number(timeToLive())},
    };
}

}
}
}
}
