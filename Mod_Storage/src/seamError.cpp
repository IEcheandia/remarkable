#include "seamError.h"
#include "attribute.h"
#include "attributeModel.h"
#include "abstractMeasureTask.h"
#include "copyMode.h"
#include "referenceCurve.h"
#include "jsonSupport.h"
#include "changeTracker.h"
#include "common/graph.h"

#include <Poco/UUIDGenerator.h>
#include <QJsonObject>

using precitec::interface::FilterParameter;
using precitec::interface::TFilterParameter;

namespace precitec
{
namespace storage
{
namespace
{

double parseDouble(const QJsonObject &object, const QString &key, double defaultValue = 0.0)
{
    auto it = object.find(key);
    if (it == object.end())
    {
        return defaultValue;
    }
    return it.value().toDouble();
}

Poco::UUID toPoco(const QUuid &uuid)
{
    return Poco::UUID(uuid.toString(QUuid::WithoutBraces).toStdString());
}

}

SeamError::SeamError(QObject *parent) 
    : SeamError(QUuid::createUuid(), parent)
{
}

SeamError::SeamError(QUuid uuid, QObject *parent)
    : SimpleError(uuid, parent)
{
    connect(this, &SeamError::measureTaskChanged, this, std::bind(&SeamError::setEnvelope, this, QUuid()));
    connect(this, &SeamError::resultValueChanged, this, std::bind(&SeamError::setEnvelope, this, QUuid()));
    connect(this, &SeamError::minLimitChanged, this, &SeamError::updateLowerBounds);
    connect(this, &SeamError::maxLimitChanged, this, &SeamError::updateUpperBounds);
}

SeamError::~SeamError()
{
}

void SeamError::setThreshold(double threshold)
{
    if (qFuzzyCompare(m_threshold, threshold) || (qFuzzyIsNull(m_threshold) && qFuzzyIsNull(threshold)))
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

void SeamError::setSecondThreshold(double secondThreshold)
{
    if (qFuzzyCompare(m_secondThreshold, secondThreshold) || (qFuzzyIsNull(m_secondThreshold) && qFuzzyIsNull(secondThreshold)))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("SecondThreshold"), m_secondThreshold, secondThreshold}));
    }
    m_secondThreshold = secondThreshold;
    emit secondThresholdChanged();
}

bool SeamError::showSecondThreshold()
{
    return variantId() == QUuid{"C0C80DA1-4E9D-4EC0-859A-8D43A0674571"} || variantId() == QUuid{"55DCC3D9-FE50-4792-8E27-460AADDDD09F"};
}

void SeamError::setMin(double min)
{
    if (qFuzzyCompare(m_min, min) || (qFuzzyIsNull(m_min) && qFuzzyIsNull(min)))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Min"), m_min, min}));
    }
    m_min = min;
    emit minChanged();
}

void SeamError::setMax(double max)
{
    if (qFuzzyCompare(m_max, max) || (qFuzzyIsNull(m_max) && qFuzzyIsNull(max)))
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Max"), m_max, max}));
    }
    m_max = max;
    emit maxChanged();
}

void SeamError::setUseMiddleCurveAsReference(bool use)
{
    if (m_useMiddleCurveAsReference == use)
    {
        return;
    }
    m_useMiddleCurveAsReference = use;
    emit useMiddleCurveAsReferenceChanged();
}

void SeamError::setEnvelope(const QUuid &id)
{
    if (m_envelope == id || !measureTask())
    {
        return;
    }

    if (auto oldRef = measureTask()->findReferenceCurve(m_envelope))
    {
        oldRef->unsubscribe(uuid());
    }

    auto ref = measureTask()->findReferenceCurve(id);
    if (ref && ref->resultType() == resultValue())
    {
        m_envelope = id;
        ref->subscribe(uuid());
        setEnvelopeName(ref->name());
        m_nameChangedConnection = connect(ref, &ReferenceCurve::nameChanged, this, &SeamError::updateEnvelopeName);
    } else
    {
        m_envelope = QUuid{};
        setEnvelopeName(QStringLiteral(""));
        m_nameChangedConnection = {};
    }

    emit envelopeChanged();
}

void SeamError::setEnvelopeName(const QString& name)
{
    if (m_envelopeName.compare(name) == 0)
    {
        return;
    }
    m_envelopeName = name;
    emit envelopeNameChanged();
}

void SeamError::updateEnvelopeName()
{
    if (!measureTask())
    {
        setEnvelopeName(QStringLiteral(""));
    }
    else if (auto ref = measureTask()->findReferenceCurve(m_envelope))
    {
        setEnvelopeName(ref->name());
    }
}

void SeamError::setMeasureTask(AbstractMeasureTask *task)
{
    if (measureTask())
    {
        if (auto ref = measureTask()->findReferenceCurve(m_envelope))
        {
            ref->unsubscribe(uuid());
        }
    }
    SimpleError::setMeasureTask(task);
}

void SeamError::unsubscribe()
{
    if (measureTask())
    {
        if (auto ref = measureTask()->findReferenceCurve(m_envelope))
        {
            ref->unsubscribe(uuid());
        }
    }
}

void SeamError::initFromAttributes(const AttributeModel *attributeModel)
{
    if (!attributeModel)
    {
        return;
    }
    const auto attributes = attributeModel->findAttributesByVariantId(variantId());
    for (auto attribute : attributes)
    {
        if (attribute->name().compare(QLatin1String("Min")) == 0)
        {
            setMin(attribute->defaultValue().toDouble());
        } else if (attribute->name().compare(QLatin1String("Max")) == 0)
        {
            setMax(attribute->defaultValue().toDouble());
        } else if (attribute->name().compare(QLatin1String("Length")) == 0)
        {
            setThreshold(attribute->defaultValue().toDouble());
        }
    }
    SimpleError::initFromAttributes(attributeModel);
}

double SeamError::getDoubleValue(const QString &name) const
{
    if (name.compare(QLatin1String("Min")) == 0)
    {
        return m_min + (boundaryType() == BoundaryType::Static ? shift() : 0.0);
    }
    else if (name.compare(QLatin1String("Max")) == 0)
    {
        return m_max + (boundaryType() == BoundaryType::Static ? shift() : 0.0);
    }
    else if (name.compare(QLatin1String("Threshold")) == 0)
    {
        return m_threshold;
    }
    else if (name.compare(QLatin1String("SecondThreshold")) == 0)
    {
        return m_secondThreshold;
    }
    else if (name.compare(QLatin1String("LwmSignalThreshold")) == 0)
    {
        return measureTask() && measureTask()->product() ? measureTask()->product()->lwmTriggerSignalThreshold() : 0.0;
    }
    return 0.0;
}

std::string SeamError::getStringValue(const QString& name) const
{
    if (name.compare(QLatin1String("Reference")) == 0)
    {
        auto envelope = measureTask()->findReferenceCurve(m_envelope);
        if (!envelope)
        {
            return QUuid{}.toString(QUuid::WithoutBraces).toStdString();
        }
        return m_envelope.toString(QUuid::WithoutBraces).toStdString();
    }
    return SimpleError::getStringValue(name);
}

std::vector<std::shared_ptr<FilterParameter>> SeamError::toParameterList() const
{
    std::vector<std::shared_ptr<FilterParameter>> list;
    list.reserve(13);

    static const std::map<QString, Parameter::DataType> names {
        {QStringLiteral("Result"), Parameter::DataType::Result},
        {QStringLiteral("Error"), Parameter::DataType::Error},
        {QStringLiteral("Min"), Parameter::DataType::Double},
        {QStringLiteral("Max"), Parameter::DataType::Double},
        {QStringLiteral("Threshold"), Parameter::DataType::Double},
        {QStringLiteral("Scope"), Parameter::DataType::String},
        {QStringLiteral("SeamSeries"), Parameter::DataType::Integer},
        {QStringLiteral("Seam"), Parameter::DataType::Integer},
        {QStringLiteral("SeamInterval"), Parameter::DataType::Integer},
        {QStringLiteral("Reference"), Parameter::DataType::String},
        {QStringLiteral("MiddleReference"), Parameter::DataType::Boolean},
        {QStringLiteral("SecondThreshold"), Parameter::DataType::Double},
        {QStringLiteral("LwmSignalThreshold"), Parameter::DataType::Double}
    };

    const auto typeId = toPoco(variantId());
    const auto filterId = toPoco(uuid());
    for (auto pair : names)
    {
        const auto parameterId = Poco::UUIDGenerator().createRandom();
        const auto parameterName = pair.first.toStdString();
        switch (pair.second)
        {
            case Parameter::DataType::Integer:
            case Parameter::DataType::Enumeration:
            case Parameter::DataType::Error:
            case Parameter::DataType::Result:
                list.emplace_back(std::make_shared<TFilterParameter<int>>(
                                parameterId,
                                parameterName,
                                getIntValue(pair.first),
                                filterId,
                                typeId));
                break;
            case Parameter::DataType::Double:
                list.emplace_back(std::make_shared<TFilterParameter<double>>(
                                parameterId,
                                parameterName,
                                getDoubleValue(pair.first),
                                filterId,
                                typeId));
                break;
            case Parameter::DataType::String:
                list.emplace_back(std::make_shared<TFilterParameter<std::string>>(
                                parameterId,
                                parameterName,
                                getStringValue(pair.first),
                                filterId,
                                typeId));
                break;
            case Parameter::DataType::Boolean:
                list.emplace_back(std::make_shared<TFilterParameter<bool>>(
                                parameterId,
                                parameterName,
                                useMiddleCurveAsReference(),
                                filterId,
                                typeId));
                break;
            default:
                break;
        }
    }

    return list;
}

QJsonObject SeamError::toJson() const
{
    auto json = SimpleError::toJson();
    json.insert(QStringLiteral("threshold"), m_threshold);
    json.insert(QStringLiteral("min"), m_min);
    json.insert(QStringLiteral("max"), m_max);
    if (boundaryType() == BoundaryType::Reference)
    {
        json.insert(QStringLiteral("envelope"), m_envelope.toString(QUuid::WithoutBraces));
        json.insert(QStringLiteral("useMiddleCurveAsReference"), m_useMiddleCurveAsReference);
    }
    json.insert(QStringLiteral("secondThreshold"), m_secondThreshold);
    return json;
}

SeamError *SeamError::fromJson(const QJsonObject &object, AbstractMeasureTask *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    SeamError *error = new SeamError(parent);
    error->SimpleError::fromJson(object, parent);
    error->setThreshold(parseDouble(object, QStringLiteral("threshold")));
    error->setMin(parseDouble(object, QStringLiteral("min")));
    error->setMax(parseDouble(object, QStringLiteral("max")));
    if (error->boundaryType() == BoundaryType::Reference)
    {
        error->setEnvelope(json::parseEnvelope(object));
        error->setUseMiddleCurveAsReference(json::parseMiddleCurveAsReference(object));
    }
    //legacy
    auto length = object.find(QStringLiteral("length"));
    if (length != object.end())
    {
        error->setThreshold(length.value().toDouble());
    }
    error->setSecondThreshold(parseDouble(object, QStringLiteral("secondThreshold")));

    error->updateLowerBounds();
    error->updateUpperBounds();

    return error;
}

SeamError *SeamError::duplicate(CopyMode mode, AbstractMeasureTask *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto se = new SeamError{std::move(newUuid), parent};

    se->setMeasureTask(parent);
    se->setVariantId(variantId());
    se->setName(name());
    se->setResultValue(resultValue());
    se->setErrorType(errorType());
    se->setThreshold(threshold());
    se->setMin(min());
    se->setMax(max());
    se->setShift(shift());
    se->setMinLimit(minLimit());
    se->setMaxLimit(maxLimit());
    se->setUseMiddleCurveAsReference(useMiddleCurveAsReference());
    se->setSecondThreshold(secondThreshold());
    // envelope is set by the abstract measure task, as the duplicated reference has a new uuid

    return se;
}

void SeamError::updateLowerBounds()
{
    if (m_min + shift() < minLimit())
    {
        setShift(0.0);
    }
    if (m_min < minLimit())
    {
        setMin(minLimit());
    }
    if (m_min > m_max)
    {
        setMax(minLimit());
    }
    if (m_max > maxLimit())
    {
        setMaxLimit(minLimit());
    }
}

void SeamError::updateUpperBounds()
{
    if (m_max + shift() > maxLimit())
    {
        setShift(0.0);
    }
    if (m_max > maxLimit())
    {
        setMax(maxLimit());
    }
    if (m_max < m_min)
    {
        setMin(maxLimit());
    }
    if (m_min < minLimit())
    {
        setMinLimit(maxLimit());
    }
}

}
}
