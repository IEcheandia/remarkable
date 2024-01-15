#include "etherCATConfigurationChangeEntry.h"

namespace precitec
{
namespace gui
{

EtherCATConfigurationChangeEntry::EtherCATConfigurationChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

EtherCATConfigurationChangeEntry::EtherCATConfigurationChangeEntry(bool wasEnabled, bool enabled, const QByteArray &oldMacAddress, const QByteArray &newMacAddress, QObject *parent)
    : components::userLog::Change(parent)
    , m_wasEnabled(wasEnabled)
    , m_enabled(enabled)
    , m_oldMacAddress(oldMacAddress)
    , m_newMacAddress(newMacAddress)
{
    setMessage(tr("EtherCAT configuration modified"));
}

EtherCATConfigurationChangeEntry::~EtherCATConfigurationChangeEntry() = default;

QJsonObject EtherCATConfigurationChangeEntry::data() const
{
    return {
        {
            qMakePair(QStringLiteral("wasEnabled"), m_wasEnabled),
            qMakePair(QStringLiteral("enabled"), m_enabled),
            qMakePair(QStringLiteral("oldMacAddress"), QString::fromLocal8Bit(m_oldMacAddress)),
            qMakePair(QStringLiteral("newMacAddress"), QString::fromLocal8Bit(m_newMacAddress))
        }
    };
}

void EtherCATConfigurationChangeEntry::initFromJson(const QJsonObject &data)
{
    // TODO: implement
    Q_UNUSED(data)
}

}
}
