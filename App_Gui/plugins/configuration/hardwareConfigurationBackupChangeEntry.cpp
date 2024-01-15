#include "hardwareConfigurationBackupChangeEntry.h"

namespace precitec
{
namespace gui
{

HardwareConfigurationBackupChangeEntry::HardwareConfigurationBackupChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
    setMessage(tr("Hardware configuration backup created"));
}

HardwareConfigurationBackupChangeEntry::~HardwareConfigurationBackupChangeEntry() = default;

QJsonObject HardwareConfigurationBackupChangeEntry::data() const
{
    return {};
}

void HardwareConfigurationBackupChangeEntry::initFromJson(const QJsonObject &data)
{
    Q_UNUSED(data)
}

}
}
