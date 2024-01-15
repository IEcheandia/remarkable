#include "hardwareRoiFlushedChangeEntry.h"


namespace precitec
{
namespace gui
{

HardwareRoiFlushedChangeEntry::HardwareRoiFlushedChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
    setMessage(tr("Hardware ROI and exposure time flushed to persistent storage"));
}

HardwareRoiFlushedChangeEntry::~HardwareRoiFlushedChangeEntry() = default;

QJsonObject HardwareRoiFlushedChangeEntry::data() const
{
    return {};
}

void HardwareRoiFlushedChangeEntry::initFromJson(const QJsonObject &data)
{
    Q_UNUSED(data)
}

}
}
