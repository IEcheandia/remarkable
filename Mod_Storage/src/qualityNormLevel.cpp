#include "qualityNormLevel.h"
#include "qualityNormResult.h"
#include "gaugeRange.h"
#include "jsonSupport.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{

QualityNormLevel::QualityNormLevel(int level, QObject* parent)
    : QObject(parent)
    , m_level(level)
{
}

QualityNormLevel::~QualityNormLevel() = default;

QualityNormLevel* QualityNormLevel::fromJson(const QJsonObject& object, QualityNormResult* parent, int level)
{
    auto qualityNormLevel = new QualityNormLevel(level, parent);
    qualityNormLevel->m_gaugeRanges = json::parseGaugeRanges(object, qualityNormLevel);

    return qualityNormLevel;
}

GaugeRange* QualityNormLevel::range(qreal gauge)
{
    if (m_gaugeRanges.empty())
    {
        return nullptr;
    }

    auto foundRange = m_gaugeRanges.front();

    for (auto gaugeRange : m_gaugeRanges)
    {
        foundRange = gaugeRange;

        if (gauge <= gaugeRange->range())
        {
            break;
        }
    }

    return foundRange;
}

}
}

