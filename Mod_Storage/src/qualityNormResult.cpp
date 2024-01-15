#include "qualityNormResult.h"
#include "qualityNorm.h"
#include "qualityNormLevel.h"
#include "jsonSupport.h"
#include "abstractMeasureTask.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{

QualityNormResult::QualityNormResult(QObject* parent)
    : QObject(parent)
{
    for (auto i = 1u; i <= AbstractMeasureTask::maxLevel(); i++)
    {
        m_levels.emplace_back(new QualityNormLevel(i, this));
    }
}

QualityNormResult::~QualityNormResult() = default;

void QualityNormResult::setResultType(int resultType)
{
    if (m_resultType == resultType)
    {
        return;
    }
    m_resultType = resultType;
    emit resultTypeChanged();
}

QualityNormResult* QualityNormResult::fromJson(const QJsonObject& object, QualityNorm* parent)
{
    auto result = new QualityNormResult(parent);
    result->setResultType(json::parseEnumType(object));

    for (auto i = 1u; i <= AbstractMeasureTask::maxLevel(); i++)
    {
        result->m_levels.at(i - 1) = json::parseQualityNormLevel(object, result, i);
    }

    return result;
}

QualityNormLevel* QualityNormResult::levelAt(int level)
{
    if (level >= int (m_levels.size()))
    {
        return nullptr;
    }
    return m_levels.at(level);
}

}
}
