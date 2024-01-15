#include "linkedSeam.h"
#include "copyMode.h"
#include "jsonSupport.h"

namespace precitec
{
namespace storage
{

LinkedSeam::LinkedSeam(QUuid uuid, Seam *link)
    : Seam(uuid, link->seamSeries())
    , m_link(link)
{
}

LinkedSeam::~LinkedSeam() = default;

LinkedSeam *LinkedSeam::create(QUuid newUuid, Seam *link, const QString &label)
{
    bool ok = false;
    int number = label.toInt(&ok);
    if (!ok)
    {
        return nullptr;
    }
    auto linkedSeam = new LinkedSeam(std::move(newUuid), link);
    linkedSeam->m_label = label;
    linkedSeam->setNumber(number);
    linkedSeam->setName(QString::number(linkedSeam->visualNumber()));
    return linkedSeam;
}

LinkedSeam* LinkedSeam::clone(CopyMode mode, Seam* link) const
{
    const auto newUuid = duplicateUuid(mode, uuid());
    auto* newLinked = new LinkedSeam(newUuid, link);
    newLinked->copy(mode, this);
    newLinked->m_label = m_label;
    newLinked->setPositionInAssemblyImage(positionInAssemblyImage());
    return newLinked;
}

QJsonObject LinkedSeam::toJson() const
{
    auto json = AbstractMeasureTask::toJson();
    const QJsonObject child{{
        json::positionInAssemblyImageToJson(positionInAssemblyImage()),
        json::linkedSeamLabelToJson(m_label)
    }};
    for (auto it = child.begin(); it != child.end(); it++)
    {
        json.insert(it.key(), it.value());
    }
    return json;
}

LinkedSeam *LinkedSeam::fromJson(const QJsonObject &object, Seam *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    LinkedSeam *seam = new LinkedSeam(uuid, parent);
    seam->AbstractMeasureTask::fromJson(object);
    seam->m_label = json::parseLinkedSeamLabel(object);
    seam->setPositionInAssemblyImage(json::parsePositionInAssemblyImage(object));
    return seam;
}

}
}
