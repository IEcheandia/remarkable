#include "hardwareConfigurationRestoreChangeEntry.h"

namespace precitec
{
namespace gui
{

HardwareConfigurationRestoreChangeEntry::HardwareConfigurationRestoreChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
}

HardwareConfigurationRestoreChangeEntry::HardwareConfigurationRestoreChangeEntry(const QString &date, QObject*parent)
    : components::userLog::Change(parent)
    , m_date(date)
{
    setMessage(tr("Hardware configuration restored"));
}

HardwareConfigurationRestoreChangeEntry::~HardwareConfigurationRestoreChangeEntry() = default;

QJsonObject HardwareConfigurationRestoreChangeEntry::data() const
{
    return QJsonObject{{
        qMakePair(QStringLiteral("date"), m_date)
    }};
}

void HardwareConfigurationRestoreChangeEntry::initFromJson(const QJsonObject &data)
{
    Q_UNUSED(data)
}

}
}

