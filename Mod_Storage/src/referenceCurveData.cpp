#include "referenceCurveData.h"

#include <QDataStream>

namespace precitec
{
namespace storage
{

ReferenceCurveData::ReferenceCurveData(QObject* parent)
    : QObject(parent)
    , m_uuid(QUuid::createUuid())
{
}

ReferenceCurveData::ReferenceCurveData(const QUuid &uuid, QObject* parent)
    : QObject(parent)
    , m_uuid(uuid)
{
}

ReferenceCurveData::~ReferenceCurveData() = default;

ReferenceCurveData* ReferenceCurveData::duplicate(const QUuid& id, QObject *parent) const
{
    auto curve = new ReferenceCurveData{id, parent};
    curve->setSamples(m_data);
    return curve;
}

QDataStream &operator<<(QDataStream& out, ReferenceCurveData* referenceCurve)
{
    out << referenceCurve->uuid().toString(QUuid::WithoutBraces);
    out << quint32(referenceCurve->samples().size());
    for (const auto& sample : referenceCurve->samples())
    {
        out << sample;
    }
    return out;
}

QDataStream &operator>>(QDataStream& in, ReferenceCurveData* referenceCurve)
{
    QString uuid;
    in >> uuid;
    referenceCurve->m_uuid = QUuid::fromString(uuid);
    qint32 sampleCount;
    in >> sampleCount;
    referenceCurve->m_data.clear();
    for (auto i = 0; i < sampleCount; i++)
    {
        QVector2D sample;
        in >> sample;
        referenceCurve->m_data.push_back(sample);
    }

    return in;
}

}
}
