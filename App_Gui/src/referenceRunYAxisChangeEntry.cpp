#include "referenceRunYAxisChangeEntry.h"

namespace precitec
{
namespace gui
{

ReferenceRunYAxisChangeEntry::ReferenceRunYAxisChangeEntry(QObject *parent)
    : components::userLog::Change(parent)
{
    setMessage(tr("Reference run of Y axis"));
}

ReferenceRunYAxisChangeEntry::~ReferenceRunYAxisChangeEntry() = default;

QJsonObject ReferenceRunYAxisChangeEntry::data() const
{
    return {};
}

void ReferenceRunYAxisChangeEntry::initFromJson(const QJsonObject &data)
{
    Q_UNUSED(data)
}

}
}

