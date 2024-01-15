#include "parameter.h"
#include "parameterSet.h"
#include "attribute.h"
#include "copyMode.h"
#include "jsonSupport.h"
#include "parameterFilterGroup.h"

#include "common/graph.h"
#include "message/device.h"

#include <QJsonObject>
#include <unordered_map>

namespace precitec
{
namespace storage
{

    using namespace std;
ParameterFilterGroup::ParameterFilterGroup(const QUuid &uuid, QObject *parent)
    : QObject(parent)
    , m_uuid(uuid)
{
}

ParameterFilterGroup::~ParameterFilterGroup() = default;


namespace
{

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



QJsonObject ParameterFilterGroup::toJson() const
{ 
 
    return QJsonObject{
        {
            json::typeIdToJson(m_typeId),
            json::filterIdToJson(m_filterId),
            json::toJson(m_parameters, this)
        }
    };
}

ParameterFilterGroup *ParameterFilterGroup::fromJson(const QJsonObject &object, QObject *parent)
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
    
    ParameterFilterGroup *ParameterGroup = new ParameterFilterGroup(uuid, parent);        
    ParameterGroup->setFilterId(json::parseFilterId(object));
    ParameterGroup->setTypeId(json::parseTypeId(object));
    ParameterGroup->m_parameters = json::parseParameters(object, ParameterGroup);
    return ParameterGroup;
}



QVariantList ParameterFilterGroup::parametersAsVariantList() const
{
    QVariantList list;
    std::transform(m_parameters.begin(), m_parameters.end(), std::back_inserter(list), [] (Parameter *p) { return QVariant::fromValue(p); });
    return list;
}

QJsonArray ParameterFilterGroup::changes() const
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



void ParameterFilterGroup::addChange(ChangeTracker &&change)
{
    m_changeTracker.emplace_back(std::move(change));
}




std::vector<Parameter*> ParameterFilterGroup::convertGroupsIntoParameterSet(std::vector<ParameterFilterGroup*> paramGroups, QObject* parent)
{
    std::vector<Parameter*> paramsVec; 
    
    for(auto& group : paramGroups)
    {
        QUuid type = group->typeId();
        QUuid filterId = group->filterId();
        for(auto& filterParam : group->m_parameters)
        {
            filterParam->setTypeId(type);
            filterParam->setFilterId(filterId);
            paramsVec.push_back(filterParam);
        }
    }     
 
    return paramsVec;    
}

std::vector<ParameterFilterGroup*> ParameterFilterGroup::groupParametersByFilterFromParameterSet(const precitec::storage::ParameterSet* paramSet, QObject* parent)
{  
    std::vector<ParameterFilterGroup*> groups = paramSet->parameterGroups();
    for(auto* param : paramSet->parameters())
    {  
        auto it = std::find_if(groups.begin(), groups.end(), [param](const ParameterFilterGroup* group)
        { return group->filterId() == param->filterId(); });
        
        if(it == groups.end())
        {     
            //If the group does not exist yet, we create it
            ParameterFilterGroup* newGroup = new ParameterFilterGroup{QUuid::createUuid(), parent};
            newGroup->setFilterId(param->filterId());
            newGroup->setTypeId(param->typeId());
            groups.push_back(newGroup);
            it = groups.end() -1;
        }
        if (std::find((*it)->m_parameters.begin(), (*it)->m_parameters.end(), param) == (*it)->m_parameters.end())
        {
        
            //We add the parameter to this group
            (*it)->m_parameters.push_back(param);
        }
    }
    
    return groups;   
        
}

std::vector<ParameterFilterGroup*> ParameterFilterGroup::groupParametersByFilter( std::vector<Parameter*> parameters, QObject* parent)
{  
    std::vector<ParameterFilterGroup*> groups;
    for(auto& param : parameters)
    {  
        auto it = std::find_if(groups.begin(), groups.end(), [param](const ParameterFilterGroup* group)
        { return group->filterId() == param->filterId(); });
        
        if(it == groups.end())
        {     
            //If the group does not exist yet, we create it
            ParameterFilterGroup* newGroup = new ParameterFilterGroup{QUuid::createUuid(), parent};
            newGroup->setFilterId(param->filterId());
            newGroup->setTypeId(param->typeId());
            groups.push_back(newGroup);
            it = groups.end() -1;
        }
        
        //We add the parameter to this group
        (*it)->m_parameters.push_back(param);
    }
    
    return groups;   
        
}

}
}
