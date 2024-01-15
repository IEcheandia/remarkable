#include "upsConfigurationChangeEntry.h"

namespace precitec
{
namespace gui
{

UpsConfigurationChangeEntry::UpsConfigurationChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

UpsConfigurationChangeEntry::UpsConfigurationChangeEntry(bool wasEnabled, bool enabled, const QString &oldName, const QString &newName, QObject *parent)
    : components::userLog::Change(parent)
    , m_wasEnabled(wasEnabled)
    , m_enabled(enabled)
    , m_oldName(oldName)
    , m_newName(newName)
{
    setMessage(tr("Uninterruptible power supply configuration modified"));
}

UpsConfigurationChangeEntry::~UpsConfigurationChangeEntry() = default;

QJsonObject UpsConfigurationChangeEntry::data() const
{
    return {
        {
            qMakePair(QStringLiteral("wasEnabled"), m_wasEnabled),
            qMakePair(QStringLiteral("enabled"), m_enabled),
            qMakePair(QStringLiteral("oldName"), m_oldName),
            qMakePair(QStringLiteral("newName"), m_newName)
        }
    };
}

void UpsConfigurationChangeEntry::initFromJson(const QJsonObject &data)
{
    // TODO: implement
    Q_UNUSED(data)
}

}
}
