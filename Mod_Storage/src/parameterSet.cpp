#include "parameterSet.h"
#include "attribute.h"
#include "copyMode.h"
#include "jsonSupport.h"

#include "common/graph.h"
#include "message/device.h"

#include <QJsonObject>

namespace precitec
{
namespace storage
{

ParameterSet::ParameterSet(const QUuid &uuid, QObject *parent)
    : QObject(parent)
    , m_uuid(uuid)
{
}

ParameterSet::~ParameterSet() = default;



QJsonObject ParameterSet::toJson() 
{
         
    if(m_filterParametersGroup.size() != 0)
    {        
        // ensure filter parameter groups are up to date
        m_filterParametersGroup = ParameterFilterGroup::groupParametersByFilterFromParameterSet(this, this);
        
        return QJsonObject{
            {
                json::toJson(m_uuid),
                json::toJson(m_filterParametersGroup)
            }
        };
    }
    else
    {         
        return QJsonObject{
            {
                json::toJson(m_uuid),
                json::toJson(m_parameters)
            }
        };
    }   
   
}

ParameterSet *ParameterSet::fromJson(const QJsonObject &object, QObject *parent)
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
    ParameterSet *set = new ParameterSet(uuid, parent);
    set->m_parameters = json::parseParameters(object, set);
    return set;
}

ParameterSet *ParameterSet::fromJsonFilterParams(const QJsonObject &object, QObject *parent)
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
    ParameterSet *set = new ParameterSet(uuid, parent);    
   
   //Try to parse the parameters if no, try to find "Groups"
    auto parameters = json::parseParameters(object, set); 
   
    
    if(parameters.size() == 0)
    {
        set->m_filterParametersGroup = json::parseParametersGroup(object, set);
        if( set->m_filterParametersGroup.size() == 0)
        {    
            return set;
        }
        else
        {
            //Transform FilterParametersGroups into Parameters vector          
            set->m_parameters = ParameterFilterGroup::convertGroupsIntoParameterSet(set->m_filterParametersGroup, parent);
        }
    }
    else
    {      
        set->m_parameters = parameters;     
        set->updateGrouping();
    }   

    
    return set;
}

void ParameterSet::updateGrouping()
{
    for (auto* group : m_filterParametersGroup)
    {
        group->deleteLater();
    }
    m_filterParametersGroup.clear();
    m_filterParametersGroup = ParameterFilterGroup::groupParametersByFilter(m_parameters, this);
}

ParameterSet *ParameterSet::duplicate(CopyMode mode, QObject *parent) const
{
    auto newUuid = duplicateUuid(mode, uuid());
    auto ps = new ParameterSet(std::move(newUuid), parent);
    ps->m_parameters.reserve(m_parameters.size());    
    for (auto parameter : m_parameters)
    {
        ps->m_parameters.emplace_back(parameter->duplicate(ps));
    }   
    
    return ps;
}

Parameter *ParameterSet::createParameter(const QUuid &id, Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue)
{
    auto param = new Parameter{id, this};
    param->setName(attribute->name());
    param->setValue(defaultValue.isValid() ? defaultValue : attribute->defaultValue());
    param->setTypeId(attribute->variantId());
    param->setFilterId(filterId);
    param->setType(attribute->type());
    if (isChangeTracking())
    {
        addChange(std::move(ParameterCreatedChange{param}));
    }

    m_parameters.push_back(param);
    return param;
}

namespace
{
QUuid toQt(const Poco::UUID &uuid)
{
    return QUuid::fromString(QString::fromStdString(uuid.toString()));
}

template <typename T>
void initValueAndType(Parameter *param, T *parameter)
{
    switch (parameter->type())
    {
    case TInt:
        param->setType(Parameter::DataType::Integer);
        param->setValue(parameter->template value<int>());
        break;
    case TUInt:
        param->setType(Parameter::DataType::UnsignedInteger);
        param->setValue(parameter->template value<uint>());
        break;
    case TBool:
        param->setType(Parameter::DataType::Boolean);
        param->setValue(parameter->template value<bool>());
        break;
    case TFloat:
        param->setType(Parameter::DataType::Float);
        param->setValue(parameter->template value<float>());
        break;
    case TDouble:
        param->setType(Parameter::DataType::Double);
        param->setValue(parameter->template value<double>());
        break;
    case TString:
        param->setType(Parameter::DataType::String);
        param->setValue(QString::fromStdString(parameter->template value<std::string>()));
        break;
    case TChar:
    case TByte:
    case TNumTypes:
    case TOpMode:
    case TUnknown:
        break;
    }
}

}

Parameter *ParameterSet::createParameter(const interface::FilterParameter &parameter)
{
    auto param = new Parameter{toQt(parameter.parameterID()), this};
    param->setName(QString::fromStdString(parameter.name()));
    param->setFilterId(toQt(parameter.instanceID()));
    param->setTypeId(toQt(parameter.typID()));
    initValueAndType(param, &parameter);

    m_parameters.push_back(param);
    return param;
}

Parameter *ParameterSet::createParameter(const Poco::SharedPtr<interface::KeyValue> &keyValue, const QUuid &hardwareDevice)
{
    auto param = new Parameter{QUuid::createUuid(), this};
    param->setName(QString::fromStdString(keyValue->key()));
    param->setTypeId(hardwareDevice);
    initValueAndType(param, keyValue.get());

    m_parameters.push_back(param);
    return param;
}

void ParameterSet::removeParameter(Parameter *parameter)
{
    auto it = std::find(m_parameters.begin(), m_parameters.end(), parameter);
    if (it != m_parameters.end())
    {
        if (isChangeTracking())
        {
            addChange(std::move(ParameterRemovedChange{(*it)}));
        }
        (*it)->deleteLater();
        m_parameters.erase(it);
    }
}

void ParameterSet::addParameter(Parameter *parameter)
{
    m_parameters.emplace_back(parameter);
}

void ParameterSet::clear()
{
    for (auto parameter: m_parameters)
    {
        if (isChangeTracking())
        {
            addChange(std::move(ParameterRemovedChange{parameter}));
        }
        parameter->deleteLater();
    }
    m_parameters.clear();
}

void ParameterSet::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}

QJsonArray ParameterSet::changes() const
{
    QJsonArray changes;
    std::transform(m_changeTracker.begin(), m_changeTracker.end(), std::back_inserter(changes), [] (const ChangeTracker &change) { return change.json(); });
    QJsonArray parameterChanges;
    for (auto parameter : m_parameters)
    {
        auto changesOnParameter = parameter->changes();
        if (changesOnParameter.empty())
        {
            continue;
        }
        parameterChanges.push_back(QJsonObject{
            qMakePair(QStringLiteral("changes"), changesOnParameter),
            qMakePair(QStringLiteral("parameter"), parameter->toJson())
        });
    }
    if (!parameterChanges.empty())
    {
        changes.push_back(QJsonObject{qMakePair(QStringLiteral("parameters"), parameterChanges)});
    }
    return changes;
}

QVariantList ParameterSet::parametersAsVariantList() const
{
    QVariantList list;
    std::transform(m_parameters.begin(), m_parameters.end(), std::back_inserter(list), [] (Parameter *p) { return QVariant::fromValue(p); });
    return list;
}

Parameter *ParameterSet::findByNameAndTypeId(const QString &name, const QUuid &typeId) const
{
    auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [name, typeId] (auto parameter) { return parameter->typeId() == typeId && parameter->name() == name; });
    if (it == m_parameters.end())
    {
        return nullptr;
    }
    return *it;
}

Parameter *ParameterSet::findById(const QUuid &id) const
{
    auto it = std::find_if(m_parameters.begin(), m_parameters.end(), [id] (auto parameter) { return parameter->uuid() == id; });
    if (it == m_parameters.end())
    {
        return nullptr;
    }
    return *it;
}

}
}
