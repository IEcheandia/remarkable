#include "overlyingError.h"
#include "attribute.h"
#include "attributeModel.h"
#include "jsonSupport.h"
#include "changeTracker.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{
namespace
{

int parseInt(const QJsonObject &object, const QString &key, int defaultValue = 0)
{
    auto it = object.find(key);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toInt();
}

double parseDouble(const QJsonObject &object, const QString &key, double defaultValue = 0.0)
{
    auto it = object.find(key);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toDouble();
}

}

OverlyingError::OverlyingError(QObject *parent)
    : OverlyingError{QUuid::createUuid(), parent}
{
}

OverlyingError::OverlyingError(QUuid uuid, QObject *parent)
    : QObject(parent)
    , m_uuid(std::move(uuid))
{
}

OverlyingError::~OverlyingError() = default;

void OverlyingError::setVariantId(const QUuid& id)
{
    if (m_variantId == id)
    {
        return;
    }
    m_variantId = id;
}

void OverlyingError::setResultValue(int resultValue)
{
    if (m_resultValue== resultValue)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Value"), m_resultValue, resultValue}));
    }
    m_resultValue = resultValue;
    emit resultValueChanged();
}

void OverlyingError::setErrorType(int errorType)
{
    if (m_errorType == errorType)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Error"), m_errorType, errorType}));
    }
    m_errorType = errorType;
    emit errorTypeChanged();
}

void OverlyingError::setName(const QString &name)
{
    if (m_name == name)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Name"), m_name, name}));
    }
    m_name = name;
    emit nameChanged();
}

void OverlyingError::setThreshold(int threshold)
{
    if (m_threshold == threshold)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Threshold"), m_threshold, threshold}));
    }
    m_threshold = threshold;
    emit thresholdChanged();
}

void OverlyingError::initFromAttributes(const AttributeModel *attributeModel)
{
    if (!attributeModel)
    {
        return;
    }
    const auto attributes = attributeModel->findAttributesByVariantId(variantId());
    for (auto attribute : attributes)
    {
        if (attribute->name().compare(QLatin1String("Length")) == 0)
        {
            setThreshold(attribute->defaultValue().toInt());
        }
    }
}

int OverlyingError::getIntValue(const QString &name) const
{
    if (name.compare(QLatin1String("Result")) == 0)
    {
        return m_resultValue;
    } else if (name.compare(QLatin1String("Error")) == 0)
    {
        return m_errorType;
    }
    return -1;
}

double OverlyingError::getDoubleValue(const QString& name) const
{
    if (name.compare(QLatin1String("Threshold")) == 0)
    {
        return double(m_threshold);
    }
    return 0.0;
}

QJsonObject OverlyingError::toJson() const
{
    return {{
        json::toJson(m_uuid),
        json::nameToJson(m_name),
        json::variantIdToJson(m_variantId),
        qMakePair(QStringLiteral("value"), m_resultValue),
        qMakePair(QStringLiteral("error"), m_errorType),
        qMakePair(QStringLiteral("threshold"), m_threshold)
    }};
}

void OverlyingError::fromJson(const QJsonObject &object)
{
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    m_uuid = uuid;
    setVariantId(json::parseVariantId(object));
    setName(json::parseName(object));
    setResultValue(parseInt(object, QStringLiteral("value")));
    setErrorType(parseInt(object, QStringLiteral("error")));
    setThreshold(parseDouble(object, QStringLiteral("threshold")));
}

void OverlyingError::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray OverlyingError::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker &change) { return change.json(); });
    return changes;
}

}
}

