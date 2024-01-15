#include "qualityNorm.h"
#include "qualityNormResult.h"
#include "jsonSupport.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{

QualityNorm::QualityNorm(QObject *parent)
    : QObject(parent)
    , m_uuid(QUuid::createUuid())
{
}

QualityNorm::QualityNorm(const QUuid& id, const QString& name, QObject* parent)
    : QObject(parent)
    , m_uuid(id)
    , m_name(name)
{
}

QualityNorm::~QualityNorm() = default;

void QualityNorm::setName(const QString& name)
{
    if (m_name.compare(name) == 0)
    {
        return;
    }
    m_name = name;
    emit nameChanged();
}

QualityNorm* QualityNorm::fromJson(const QJsonObject& object, QObject* parent)
{
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }

    auto norm = new QualityNorm(parent);
    norm->m_uuid = uuid;
    norm->setName(json::parseName(object));
    norm->m_qualityNormResults = json::parseQualityNormResults(object, norm);

    return norm;
}

void QualityNorm::addCommonResultsFromJson(const QJsonObject& object)
{
    const auto commonResults = json::parseCommonResults(object, this);
    m_qualityNormResults.insert(m_qualityNormResults.end(), commonResults.begin(), commonResults.end());
}

QualityNormResult* QualityNorm::qualityNormResult(int resultType)
{
    auto it = std::find_if(m_qualityNormResults.begin(), m_qualityNormResults.end(), [resultType] (auto qnr) { return qnr->resultType() == resultType; });
    if (it == m_qualityNormResults.end())
    {
        return nullptr;
    }
    return *it;
}

}
}
