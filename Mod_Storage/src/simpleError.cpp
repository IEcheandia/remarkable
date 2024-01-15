#include "simpleError.h"
#include "attribute.h"
#include "attributeModel.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"
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

SimpleError::SimpleError(QObject *parent)
    : SimpleError(QUuid::createUuid(), parent)
{
}

SimpleError::SimpleError(QUuid uuid, QObject *parent)
    : QObject(parent)
    , m_uuid(std::move(uuid))
{
}

SimpleError::~SimpleError() = default;

void SimpleError::setVariantId(const QUuid& id)
{
    if (m_variantId == id)
    {
        return;
    }
    m_variantId = id;

    updateBoundaryType();
}

void SimpleError::setResultValue(int resultValue)
{
    if (m_resultValue == resultValue)
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

void SimpleError::setErrorType(int errorType)
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

void SimpleError::setShift(double shift)
{
    if (qFuzzyCompare(m_shift, shift) || (qFuzzyIsNull(m_shift) && qFuzzyIsNull(shift)))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Shift"), m_shift, shift}));
    }
    m_shift = shift;
    emit shiftChanged();
}

void SimpleError::setMinLimit(double min)
{
    if (qFuzzyCompare(m_minLimit, min) || (qFuzzyIsNull(m_minLimit) && qFuzzyIsNull(min)))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("MinLimit"), m_minLimit, min}));
    }
    m_minLimit = min;
    emit minLimitChanged();
}

void SimpleError::setMaxLimit(double max)
{
    if (qFuzzyCompare(m_maxLimit, max)|| (qFuzzyIsNull(m_maxLimit) && qFuzzyIsNull(max)))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("MaxLimit"), m_maxLimit, max}));
    }
    m_maxLimit = max;
    emit maxLimitChanged();
}

void SimpleError::setName(const QString &name)
{
    if (m_name.compare(name) == 0)
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

void SimpleError::setMeasureTask(AbstractMeasureTask *task)
{
    if (m_measureTask == task)
    {
        return;
    }
    m_measureTask = task;
    emit measureTaskChanged();
}

void SimpleError::initFromAttributes(const AttributeModel *attributeModel)
{
    if (!attributeModel)
    {
        return;
    }
    const auto attributes = attributeModel->findAttributesByVariantId(m_variantId);
    for (auto attribute : attributes)
    {
        if (attribute->name().compare(QLatin1String("Shift")) == 0)
        {
            setShift(attribute->defaultValue().toDouble());
        } else if (attribute->name().compare(QLatin1String("MinLimit")) == 0)
        {
            setMinLimit(attribute->defaultValue().toDouble());
        } else if (attribute->name().compare(QLatin1String("MaxLimit")) == 0)
        {
            setMaxLimit(attribute->defaultValue().toDouble());
        }
    }
}

int SimpleError::getIntValue(const QString &name) const
{
    if (name.compare(QLatin1String("SeamSeries")) == 0)
    {
        if (auto si = qobject_cast<SeamInterval*>(m_measureTask))
        {
            return si->seam()->seamSeries()->number();
        }
        if (auto seam = qobject_cast<Seam*>(m_measureTask))
        {
            return seam->seamSeries()->number();
        }
        if (auto series = qobject_cast<SeamSeries*>(m_measureTask))
        {
            return series->number();
        }
        return 0;
    } else if (name.compare(QLatin1String("Seam")) == 0)
    {
        if (auto si = qobject_cast<SeamInterval*>(m_measureTask))
        {
            return si->seam()->number();
        }
        if (auto seam = qobject_cast<Seam*>(m_measureTask))
        {
            return seam->number();
        }
        return 0;
    } else if (name.compare(QLatin1String("SeamInterval")) == 0)
    {
        if (auto si = qobject_cast<SeamInterval*>(m_measureTask))
        {
            return si->number();
        }
        return 0;
    } else if (name.compare(QLatin1String("Result")) == 0)
    {
        return m_resultValue;
    } else if (name.compare(QLatin1String("Error")) == 0)
    {
        return m_errorType;
    }
    return -1;
}

std::string SimpleError::getStringValue(const QString& name) const
{
    if (name.compare(QLatin1String("Scope")) == 0)
    {
        if (qobject_cast<SeamInterval*>(m_measureTask))
        {
            return std::string("SeamInterval");
        }
        if (qobject_cast<Seam*>(m_measureTask))
        {
            return std::string("Seam");
        }
        if (qobject_cast<SeamSeries*>(m_measureTask))
        {
            return std::string("SeamSeries");
        }
    }
    return std::string("Product");
}

QJsonObject SimpleError::toJson() const
{
    return {{
        json::toJson(m_uuid),
        json::nameToJson(m_name),
        json::variantIdToJson(m_variantId),
        qMakePair(QStringLiteral("value"), m_resultValue),
        qMakePair(QStringLiteral("error"), m_errorType),
        qMakePair(QStringLiteral("shift"), m_shift),
        qMakePair(QStringLiteral("minLimit"), m_minLimit),
        qMakePair(QStringLiteral("maxLimit"), m_maxLimit)
    }};
}

void SimpleError::fromJson(const QJsonObject &object, AbstractMeasureTask *parent)
{
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    m_uuid = uuid;
    setVariantId(json::parseVariantId(object));
    setName(json::parseName(object));
    setMeasureTask(parent);
    setResultValue(parseInt(object, QStringLiteral("value")));
    setErrorType(parseInt(object, QStringLiteral("error")));
    setMinLimit(parseDouble(object, QStringLiteral("minLimit")));
    setMaxLimit(parseDouble(object, QStringLiteral("maxLimit")));
    setShift(parseDouble(object, QStringLiteral("shift")));
}

bool SimpleError::isChangeTracking() const
{
    if (m_measureTask)
    {
        return m_measureTask->isChangeTracking();
    }
    return false;
}

void SimpleError::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray SimpleError::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker &change) { return change.json(); });
    return changes;
}

void SimpleError::updateBoundaryType()
{
    if (   m_variantId == QUuid{"F8F4E0A8-D259-40F9-B134-68AA24E0A06C"} //SignalSumErrorAccumulatedOutlierReferenceBoundary
        || m_variantId == QUuid{"5EB04560-2641-4E64-A016-14207E59A370"} //SignalSumErrorSingleOutlierReferenceBoundary
        || m_variantId == QUuid{"527B7421-5DDD-436C-BE33-C1A359A736F6"} //SignalSumErrorAccumulatedAreaReferenceBoundary
        || m_variantId == QUuid{"D36ECEBA-286B-4D06-B596-0491B6544F40"} //SignalSumErrorSingleAreaReferenceBoundary
        || m_variantId == QUuid{"4A6AE9B0-3A1A-427F-8D58-2D0205452377"} //SignalSumErrorInlierReferenceBoundary
        || m_variantId == QUuid{"7CF9F16D-36DE-4840-A2EA-C41979F91A9B"} //SignalSumErrorPeakReferenceBoundary
        || m_variantId == QUuid{"C0C80DA1-4E9D-4EC0-859A-8D43A0674571"} //SignalSumErrorDualOutlierReferenceBoundary
       )
    {
        m_boundaryType = BoundaryType::Reference;
    } else
    {
        m_boundaryType = BoundaryType::Static;
    }
    emit boundaryTypeChanged();
}

}
}
