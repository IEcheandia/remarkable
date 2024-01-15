#pragma once

#include <QDataStream>
#include <QVector2D>
#include <QPointF>
#include <QUuid>
#include <vector>

namespace precitec
{
namespace storage
{

struct LegacyReferenceCurve {
    QUuid m_uuid;
    QUuid m_upper;
    QUuid m_middle;
    QUuid m_lower;

    std::vector<QVector2D> m_upperData;
    std::vector<QVector2D> m_middleData;
    std::vector<QVector2D> m_lowerData;

    friend QDataStream &operator>>(QDataStream& in, precitec::storage::LegacyReferenceCurve& curve);
};

inline QDataStream &operator>>(QDataStream &in, precitec::storage::LegacyReferenceCurve& curve)
{
    QString uuid;
    in >> uuid;
    curve.m_uuid = QUuid::fromString(uuid);

    QString lowerUuid;
    in >> lowerUuid;
    curve.m_lower = QUuid::fromString(lowerUuid);
    qint32 lowerSampleCount;
    in >> lowerSampleCount;
    for (auto i = 0; i < lowerSampleCount; i++)
    {
        QPointF sample;
        in >> sample;
        curve.m_lowerData.emplace_back(sample);
    }

    QString middleUuid;
    in >> middleUuid;
    curve.m_middle = QUuid::fromString(middleUuid);
    qint32 middleSampleCount;
    in >> middleSampleCount;
    for (auto i = 0; i < middleSampleCount; i++)
    {
        QPointF sample;
        in >> sample;
        curve.m_middleData.emplace_back(sample);
    }

    QString upperUuid;
    in >> upperUuid;
    curve.m_upper = QUuid::fromString(upperUuid);
    qint32 upperSampleCount;
    in >> upperSampleCount;
    for (auto i = 0; i < upperSampleCount; i++)
    {
        QPointF sample;
        in >> sample;
        curve.m_upperData.emplace_back(sample);
    }

    return in;
}

}
}
