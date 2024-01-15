#include "seamSeriesError.h"
#include "copyMode.h"
#include "parameter.h"
#include "seamSeries.h"
#include "common/graph.h"

#include <Poco/UUIDGenerator.h>
#include <QUuid>

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

SeamSeriesError::SeamSeriesError(QObject *parent)
    : OverlyingError(parent)
{
}

SeamSeriesError::SeamSeriesError(QUuid uuid, QObject *parent)
    : OverlyingError(std::move(uuid), parent)
{
}

SeamSeriesError::~SeamSeriesError() = default;

void SeamSeriesError::setMeasureTask(AbstractMeasureTask *task)
{
    if (m_measureTask == task)
    {
        return;
    }
    m_measureTask = task;
    emit measureTaskChanged();
}

int SeamSeriesError::getIntValue(const QString &name) const
{
    if (name.compare(QLatin1String("SeamInterval")) == 0
        || name.compare(QLatin1String("Seam")) == 0)
    {
        return 0;
    } else if (name.compare(QLatin1String("SeamSeries")) == 0)
    {
        if (auto series = qobject_cast<SeamSeries*>(m_measureTask))
        {
            return series->number();
        }
        return 0;
    }
    return OverlyingError::getIntValue(name);
}

std::string SeamSeriesError::getStringValue(const QString &name) const
{
    if (name.compare(QLatin1String("Scope")) == 0)
    {
        return std::string("SeamSeries");
    }
    return QUuid{}.toString(QUuid::WithoutBraces).toStdString();
}

std::vector<std::shared_ptr<FilterParameter>> SeamSeriesError::toParameterList() const
{
    std::vector<std::shared_ptr<FilterParameter>> list;
    list.reserve(13);

    static const std::map<QString, Parameter::DataType> names{
        {QStringLiteral("Result"), Parameter::DataType::Result},
        {QStringLiteral("Error"), Parameter::DataType::Error},
        {QStringLiteral("Min"), Parameter::DataType::Double},
        {QStringLiteral("Max"), Parameter::DataType::Double},
        {QStringLiteral("Threshold"), Parameter::DataType::Double},
        {QStringLiteral("Scope"), Parameter::DataType::String},
        {QStringLiteral("SeamInterval"), Parameter::DataType::Integer},
        {QStringLiteral("SeamSeries"), Parameter::DataType::Integer},
        {QStringLiteral("Seam"), Parameter::DataType::Integer},
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
                                false,
                                filterId,
                                typeId));
                break;
            default:
                break;
        }
    }

    return list;
}

SeamSeriesError *SeamSeriesError::fromJson(const QJsonObject &object, AbstractMeasureTask *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    SeamSeriesError *error = new SeamSeriesError(parent);
    error->setMeasureTask(parent);
    error->OverlyingError::fromJson(object);

    return error;
}

bool SeamSeriesError::isChangeTracking()
{
    if (m_measureTask)
    {
        return m_measureTask->isChangeTracking();
    }
    return false;
}

SeamSeriesError *SeamSeriesError::duplicate(CopyMode mode, AbstractMeasureTask *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto se = new SeamSeriesError{std::move(newUuid), parent};
    se->setMeasureTask(parent);
    se->setVariantId(variantId());
    se->setName(name());
    se->setResultValue(resultValue());
    se->setErrorType(errorType());
    se->setThreshold(threshold());

    return se;
}

}
}

