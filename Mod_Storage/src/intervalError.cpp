#include "intervalError.h"
#include "attribute.h"
#include "attributeModel.h"
#include "copyMode.h"
#include "seam.h"
#include "seamInterval.h"
#include "jsonSupport.h"
#include "levelConfig.h"
#include "common/graph.h"

#include <QJsonObject>
#include <Poco/UUIDGenerator.h>
#include <QDebug>

using precitec::interface::FilterParameter;
using precitec::interface::TFilterParameter;

namespace precitec
{
namespace storage
{
namespace
{

Poco::UUID toPoco(const QUuid &uuid)
{
    return Poco::UUID(uuid.toString(QUuid::WithoutBraces).toStdString());
}

}

IntervalError::IntervalError(QObject *parent)
    : IntervalError(QUuid::createUuid(), parent)
{
}

IntervalError::IntervalError(QUuid uuid, QObject *parent)
    : SimpleError(std::move(uuid), parent)
{
    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        const auto levelConfig = new LevelConfig{this};
        m_levels.push_back(levelConfig);
        connect(levelConfig, &LevelConfig::minChanged, this, &IntervalError::minChanged);
        connect(levelConfig, &LevelConfig::maxChanged, this, &IntervalError::maxChanged);
        connect(levelConfig, &LevelConfig::thresholdChanged, this, &IntervalError::thresholdChanged);
        connect(levelConfig, &LevelConfig::secondThresholdChanged, this, &IntervalError::secondThresholdChanged);
    }
    connect(this, &IntervalError::minLimitChanged, this, &IntervalError::updateLowerBounds);
    connect(this, &IntervalError::maxLimitChanged, this, &IntervalError::updateUpperBounds);
}

IntervalError::~IntervalError() = default;

double IntervalError::threshold(uint index)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return 0;
    }
    return m_levels.at(index)->threshold();
}

void IntervalError::setThreshold(uint index, double threshold)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Level %1 Threshold").arg(index), m_levels.at(index)->threshold(), threshold}));
    }
    m_levels.at(index)->setThreshold(threshold);
}

double IntervalError::secondThreshold(uint index)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return 0;
    }
    return m_levels.at(index)->secondThreshold();
}

void IntervalError::setSecondThreshold(uint index, double secondThreshold)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Level %1 SecondThreshold").arg(index), m_levels.at(index)->secondThreshold(), secondThreshold}));
    }
    m_levels.at(index)->setSecondThreshold(secondThreshold);
}

double IntervalError::min(uint index)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return 0;
    }
    return m_levels.at(index)->min();
}

void IntervalError::setMin(uint index, double min)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Level %1 Min").arg(index), m_levels.at(index)->min(), min}));
    }
    m_levels.at(index)->setMin(min);
}

double IntervalError::max(uint index)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return 0;
    }
    return m_levels.at(index)->max();
}

void IntervalError::setMax(uint index, double max)
{
    if (index >= AbstractMeasureTask::maxLevel())
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{tr("Level %1 Max").arg(index), m_levels.at(index)->max(), max}));
    }
    m_levels.at(index)->setMax(max);
}

void IntervalError::initFromAttributes(const AttributeModel *attributeModel)
{
    if (! attributeModel)
    {
        return;
    }
    const auto attributes = attributeModel->findAttributesByVariantId(variantId());
    for (auto attribute : attributes)
    {
        if (attribute->name().compare(QLatin1String("Min")) == 0)
        {
            for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
            {
                setMin(i, attribute->defaultValue().toDouble());
            }
        } else if (attribute->name().compare(QLatin1String("Max")) == 0)
        {
            for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
            {
                setMax(i, attribute->defaultValue().toDouble());
            }
        } else if (attribute->name().compare(QLatin1String("Length")) == 0)
        {
            for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
            {
                setThreshold(i, attribute->defaultValue().toDouble());
            }
        } else if (attribute->name().compare(QLatin1String("SecondThreshold")) == 0)
        {
            for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
            {
                setSecondThreshold(i, attribute->defaultValue().toDouble());
            }
        }
    }
    SimpleError::initFromAttributes(attributeModel);
}

QJsonObject IntervalError::toJson() const
{
    auto json = SimpleError::toJson();
    const QJsonObject child{{
        json::levelConfigsToJson(m_levels),
        json::intervalIdsToJson(m_errorIds)
    }};
    for (auto it = child.begin(); it != child.end(); it++)
    {
        json.insert(it.key(), it.value());
    }
    return json;
}

IntervalError *IntervalError::fromJson(const QJsonObject &object, AbstractMeasureTask *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }

    IntervalError *error = new IntervalError(parent);
    error->SimpleError::fromJson(object, parent);
    error->m_errorIds = json::parseIntervalIds(object);
    error->m_levels = json::parseLevelConfigs(object, error);

    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        const auto levelConfig = error->m_levels.at(i);
        connect(levelConfig, &LevelConfig::minChanged, error, &IntervalError::minChanged);
        connect(levelConfig, &LevelConfig::maxChanged, error, &IntervalError::maxChanged);
        connect(levelConfig, &LevelConfig::thresholdChanged, error, &IntervalError::thresholdChanged);
        connect(levelConfig, &LevelConfig::secondThresholdChanged, error, &IntervalError::secondThresholdChanged);
    }

    // legacy
    auto length1it = object.find(QStringLiteral("levelOneLength"));
    if (length1it != object.end())
    {
        error->setThreshold(0, length1it.value().toDouble());
    }
    auto length2it = object.find(QStringLiteral("levelTwoLength"));
    if (length2it != object.end())
    {
        error->setThreshold(1, length2it.value().toDouble());
    }
    auto min1it = object.find(QStringLiteral("levelOneMin"));
    if (min1it != object.end())
    {
        error->setMin(0, min1it.value().toDouble());
    }
    auto min2it = object.find(QStringLiteral("levelTwoMin"));
    if (min2it != object.end())
    {
        error->setMin(1, min2it.value().toDouble());
    }
    auto max1it = object.find(QStringLiteral("levelOneMax"));
    if (max1it != object.end())
    {
        error->setMax(0, max1it.value().toDouble());
    }
    auto max2it = object.find(QStringLiteral("levelTwoMax"));
    if (max2it != object.end())
    {
        error->setMax(1, max2it.value().toDouble());
    }

    error->updateLowerBounds();
    error->updateUpperBounds();

    return error;
}

IntervalError *IntervalError::duplicate(CopyMode mode, Seam *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto se = new IntervalError{std::move(newUuid), parent};
    se->setMeasureTask(parent);
    se->setVariantId(variantId());
    se->setName(name());
    se->setResultValue(resultValue());
    se->setErrorType(errorType());
    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        se->m_levels.at(i) = m_levels.at(i)->duplicate(se);
    }
    se->setShift(shift());
    se->setMinLimit(minLimit());
    se->setMaxLimit(maxLimit());

    if (mode == CopyMode::WithDifferentIds)
    {
        // duplicate only from parent seam, to be able to add intervals
        for (auto interval : parent->seamIntervals())
        {
            se->addInterval(interval);
        }
    }
    else
    {
        // Make an identical copy, including the error Id map.
        se->m_errorIds = m_errorIds;
    }

    return se;
}

void IntervalError::addInterval(SeamInterval* interval)
{
    m_errorIds.emplace(interval->uuid(), QUuid::createUuid());
}

void IntervalError::removeInterval(SeamInterval* interval)
{
    auto it = m_errorIds.find(interval->uuid());

    if (it != m_errorIds.end())
    {
        m_errorIds.erase(it);
    }
}

double IntervalError::getDoubleValue(const QString &name, int level) const
{
    if (name.compare(QLatin1String("Min")) == 0)
    {
        return m_levels.at(level)->min() + shift();
    } else if (name.compare(QLatin1String("Max")) == 0)
    {
        return m_levels.at(level)->max() + shift();
    } else if (name.compare(QLatin1String("Threshold")) == 0)
    {
        return m_levels.at(level)->threshold();
    } else if (name.compare(QLatin1String("SecondThreshold")) == 0)
    {
        return m_levels.at(level)->secondThreshold();
    } else if (name.compare(QLatin1String("LwmSignalThreshold")) == 0)
    {
        return measureTask() && measureTask()->product() ? measureTask()->product()->lwmTriggerSignalThreshold() : 0.0;
    }
    return 0.0;
}

std::string IntervalError::getStringValue(const QString &name) const
{
    if (name.compare(QLatin1String("Scope")) == 0)
    {
        return std::string("SeamInterval");
    }
    return QUuid{}.toString(QUuid::WithoutBraces).toStdString();
}

std::vector<std::shared_ptr<FilterParameter>> IntervalError::toParameterList() const
{
    const auto seam = qobject_cast<Seam*>(measureTask());
    const auto intervals = seam->seamIntervals();

    std::vector<std::shared_ptr<FilterParameter>> list;
    list.reserve(13 * intervals.size());

    static const std::map<QString, Parameter::DataType> names{
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

    for (auto interval : intervals)
    {
        auto it = m_errorIds.find(interval->uuid());
        const auto filterId = toPoco((it != m_errorIds.end()) ? (*it).second : QUuid::createUuid());

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
                                    pair.first.compare(QLatin1String("SeamInterval")) == 0 ? interval->number() : getIntValue(pair.first),
                                    filterId,
                                    typeId));
                    break;
                case Parameter::DataType::Double:
                    list.emplace_back(std::make_shared<TFilterParameter<double>>(
                                    parameterId,
                                    parameterName,
                                    getDoubleValue(pair.first, interval->level()),
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
                                    false,
                                    filterId,
                                    typeId));
                    break;
                default:
                    break;
            }
        }
    }

    return list;
}

void IntervalError::updateLowerBounds()
{
    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        if (min(i) + shift() < minLimit())
        {
            setShift(0.0);
        }
        if (min(i) < minLimit())
        {
            setMin(i, minLimit());
        }
        if (min(i) > max(i))
        {
            setMax(i, minLimit());
        }
        if (max(i) > maxLimit())
        {
            setMaxLimit(minLimit());
        }
    }
}

void IntervalError::updateUpperBounds()
{
    for (auto i = 0u; i < AbstractMeasureTask::maxLevel(); i++)
    {
        if (max(i) + shift() > maxLimit())
        {
            setShift(0.0);
        }
        if (max(i) > maxLimit())
        {
            setMax(i, maxLimit());
        }
        if (max(i) < min(i))
        {
            setMin(i, maxLimit());
        }
        if (min(i) < minLimit())
        {
            setMinLimit(maxLimit());
        }
    }
}

double IntervalError::lowestMin() const
{
    const auto minimum = std::min_element(m_levels.begin(), m_levels.end(), [] (auto left, auto right) { return left->min() < right->min(); });
    return (*minimum)->min();
}

double IntervalError::highestMax() const
{
    const auto maximum = std::max_element(m_levels.begin(), m_levels.end(), [] (auto left, auto right) { return left->max() < right->max(); });
    return (*maximum)->max();
}

bool IntervalError::showSecondThreshold()
{
    return variantId() == QUuid{"C0C80DA1-4E9D-4EC0-859A-8D43A0674571"} || variantId() == QUuid{"55DCC3D9-FE50-4792-8E27-460AADDDD09F"};
}

}
}
