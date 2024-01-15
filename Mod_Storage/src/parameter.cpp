#include "parameter.h"
#include "parameterSet.h"
#include "jsonSupport.h"
#include "parameterFilterGroup.h"

#include <QJsonObject>

#include "common/graph.h"

namespace precitec
{
namespace storage
{

Parameter::Parameter(const QUuid &uuid, ParameterSet *parent)
    : QObject(parent)
    , m_uuid(uuid)
    , m_parameterSet(parent)
    , m_filterParameterGroup(nullptr)
{
}

Parameter::Parameter(const QUuid &uuid, ParameterFilterGroup *parent)
    : QObject(parent)
    , m_uuid(uuid)
    , m_parameterSet(nullptr)
    , m_filterParameterGroup(parent)
    
{
}

Parameter::Parameter(const Parameter& other) //copy constructor
: m_uuid(other.uuid())
, m_parameterSet(other.parameterSet())
, m_filterParameterGroup(other.m_filterParameterGroup)
, m_typeId(other.typeId())
, m_name(other.name())
, m_value(other.value())
, m_type(other.type())
, m_filterId(other.filterId())
, m_changeTracker(other.m_changeTracker)
{
}

Parameter::~Parameter() = default;

void Parameter::setName(const QString &name)
{
    if (m_name == name)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("name"), m_name, name}));
    }
    m_name = name;
    emit nameChanged();
}

void Parameter::setTypeId(const QUuid &uuid)
{
    if (m_typeId == uuid)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("typeId"), m_typeId, uuid}));
    }
    m_typeId = uuid;
    emit typeIdChanged();
}

void Parameter::setValue(const QVariant &value)
{
    if (m_value == value)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("value"), m_value, value}));
    }
    m_value = value;
    emit valueChanged();
}

void Parameter::setType(DataType type)
{
    if (m_type == type)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("type"), QVariant::fromValue(m_type), QVariant::fromValue(type)}));
    }
    m_type = type;
    emit typeChanged();
}

void Parameter::setFilterId(const QUuid &uuid)
{
    if (m_filterId == uuid)
    {
        return;
    }
    if (isChangeTracking())
    {
        addChange(std::move(PropertyChange{QStringLiteral("filterId"), m_filterId, uuid}));
    }
    m_filterId = uuid;
    emit filterIdChanged();
}

Parameter *Parameter::duplicate(ParameterSet *parent) const
{
    auto parameter = new Parameter(uuid(), parent);
    parameter->setName(name());
    parameter->setTypeId(typeId());
    parameter->setValue(value());
    parameter->setType(type());
    parameter->setFilterId(filterId());
    return parameter;
}



QJsonObject Parameter::toJson() const
{  
    return QJsonObject{
        {
            json::toJson(m_uuid),
            json::nameToJson(m_name),
            json::typeIdToJson(m_typeId),
            json::filterIdToJson(m_filterId),
            json::toJson(m_value),
            json::toJson(m_type)
        }
    };
}

QJsonObject Parameter::toJson(const QObject* ParameterFilterGroup) const
{ 
    return QJsonObject{
        {
            json::toJson(m_uuid),
            json::nameToJson(m_name),             
            json::toJson(m_value),
            json::toJson(m_type)
        }
    };
}

Parameter *Parameter::fromJson(const QJsonObject &object, ParameterSet *parent)
{
    
    if (object.isEmpty())
    {
        return nullptr;
    }
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
    Parameter *parameter = new Parameter(uuid, parent);
    
    parameter->setName(json::parseName(object));
    parameter->setTypeId(json::parseTypeId(object));
    parameter->setValue(json::parseValue(object));
    parameter->setType(json::parseDataType(object));
    parameter->setFilterId(json::parseFilterId(object));
   
    return parameter;
}


Parameter *Parameter::fromJson(const QJsonObject &object, ParameterFilterGroup *parent)
{
    if (object.isEmpty())
    {
        return nullptr;
    }
    auto uuid = json::parseUuid(object);
    if (uuid.isNull())
    {
        uuid = QUuid::createUuid();
    }
  
    Parameter *parameter = new Parameter(uuid, parent);
    parameter->setName(json::parseName(object));    
    parameter->setValue(json::parseValue(object));
    parameter->setType(json::parseDataType(object));  

    return parameter;
}

bool Parameter::isChangeTracking() const
{
    if(m_parameterSet == nullptr)
    {
        return m_filterParameterGroup? m_filterParameterGroup->isChangeTracking() : false;
    }
    else
    {
        return m_parameterSet ? m_parameterSet->isChangeTracking() : false;
    }
}
   
   
void Parameter::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray Parameter::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker &change) { return change.json(); });
    return changes;
}

namespace
{
Poco::UUID toPoco(const QUuid &uuid)
{
    return Poco::UUID(uuid.toString(QUuid::WithoutBraces).toStdString());
}
}

std::shared_ptr<precitec::interface::FilterParameter> Parameter::toFilterParameter() const
{
    using interface::TFilterParameter;
    const auto parameterId = toPoco(m_uuid);
    const auto parameterName = m_name.toStdString();
    const auto typeId = toPoco(m_typeId);
    const auto filterId = toPoco(m_filterId);
    switch (type())
    {
    case Parameter::DataType::Integer:
    case Parameter::DataType::Enumeration:
    case Parameter::DataType::Error:
    case Parameter::DataType::Result:
    case Parameter::DataType::Sensor:
    case Parameter::DataType::SeamFigure:
    case Parameter::DataType::WobbleFigure:
        return std::make_shared<TFilterParameter<int>>(
                                parameterId,
                                parameterName,
                                value().toInt(),
                                filterId,
                                typeId);
    case Parameter::DataType::UnsignedInteger:
        return std::make_shared<TFilterParameter<uint>>(
                                parameterId,
                                parameterName,
                                value().toUInt(),
                                filterId,
                                typeId);
        break;
    case Parameter::DataType::Float:
        return std::make_shared<TFilterParameter<float>>(
                                parameterId,
                                parameterName,
                                value().toFloat(),
                                filterId,
                                typeId);
        break;
    case Parameter::DataType::Double:
        return std::make_shared<TFilterParameter<double>>(
                                parameterId,
                                parameterName,
                                value().toDouble(),
                                filterId,
                                typeId);
        break;
    case Parameter::DataType::Boolean:
        return std::make_shared<TFilterParameter<bool>>(
                                parameterId,
                                parameterName,
                                value().toBool(),
                                filterId,
                                typeId);
        break;
    case Parameter::DataType::String:
    case Parameter::DataType::Text:
        return std::make_shared<TFilterParameter<std::string>>(
                                parameterId,
                                parameterName,
                                value().toString().toStdString(),
                                filterId,
                                typeId);
        break;
    case Parameter::DataType::Unknown:
        break;
    default:
        Q_UNREACHABLE();
    }
    return {};
}

}
}
