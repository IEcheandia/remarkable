#pragma once
#include "changeTracker.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

#include <memory>
#include <vector>

#include <Poco/SharedPtr.h>

namespace precitec
{

namespace interface
{
class FilterParameter;
}

namespace storage
{
class Attribute;
class Parameter;
enum class CopyMode;
/**
 * @brief The ParameterFilterGroup class encapsulates parameter sets grouped by filter Id and its functionality
 **/
class ParameterFilterGroup : public QObject
{
    Q_OBJECT   
    
    /**
     * The unique id of this ParameterFilterGroup
     **/
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    
    /**
     * The unique identifier of the type this Parameter is for.
     * E.g. the hardware device in case of a hardware parameter.
     **/
    Q_PROPERTY(QUuid typeId READ typeId WRITE setTypeId NOTIFY typeIdChanged)        
    /**
     * In case this Parameter references a filter parameter instead of a hardware parameter,
     * this is the unique id of the instance filter the parameter belongs to.
     **/
    Q_PROPERTY(QUuid filterId READ filterId WRITE setFilterId NOTIFY filterIdChanged)
    
    
   //Q_PROPERTY(precitec::storage::Parameter *Parameters READ parameters WRITE setParameters NOTIFY parametersChanged)
     Q_PROPERTY(QVariantList parameters READ parametersAsVariantList CONSTANT)
     
     
public:
    /**
     * Constructs the Parameter with the given @p uuid and the given @p parent ParameterSet.
     **/
    explicit ParameterFilterGroup(const QUuid &uuid, QObject *parent = nullptr);
    ~ParameterFilterGroup() override;

    QUuid uuid() const
    {
        return m_uuid;
    }


    QUuid typeId() const
    {
        return m_typeId;
    }   
   

    QUuid filterId() const
    {
        return m_filterId;
    }
    
    /**
     * @returns The actual @link{Parameter}s belonging to this ParameterGroup.
     **/
    const std::vector<Parameter*> &parameters() const
    {
        return m_parameters;
    }
    
   QVariantList parametersAsVariantList() const;
    
    void setTypeId(const QUuid &uuid)
    {
        if (m_typeId == uuid)
        {
            return;
        }
    
        m_typeId = uuid;
        emit typeIdChanged();
    }

      
    void setFilterId(const QUuid &uuid)
    {
        if(m_filterId == uuid)
        {
            return;
        }
        
        m_filterId = uuid;
        emit filterIdChanged();
    }
    
    void pushNewParam(Parameter* param)
    {
        m_parameters.push_back(param);
        return;
    }    
    
    
     /**
     * @returns A QJsonObject representation for this object.
     * @see fromJson
     **/
    QJsonObject toJson() const;  
    

    /**
     * Creates a new Parameter from the provided json @p object.     
     **/
    static ParameterFilterGroup *fromJson(const QJsonObject &object, QObject *parent);
    
    QJsonArray changes() const;
     
    void setChangeTrackingEnabled(bool set)
    {
        m_changeTracking = set;
    }
    
    bool isChangeTracking() const
    {
        return m_changeTracking;
    }
    
  
    /**
     * Removes the @p parameter from this ParameterSet.
     * The passed in Parameter will be destroyed.
     **/
    void removeParameter(Parameter *parameter);
    
    static std::vector<Parameter*> convertGroupsIntoParameterSet(std::vector<ParameterFilterGroup*> paramGroups, QObject* parent);
    static std::vector<ParameterFilterGroup*> groupParametersByFilterFromParameterSet(const precitec::storage::ParameterSet* paramSet, QObject* parent);
    static std::vector<ParameterFilterGroup*> groupParametersByFilter(std::vector<Parameter*> parameters, QObject* parent);
    
Q_SIGNALS:
    void typeIdChanged();
    void filterIdChanged();

private:
    void addChange(ChangeTracker &&change);
    QUuid m_uuid;    
    QUuid m_typeId;
    QUuid m_filterId;
    std::vector<Parameter*> m_parameters;
    bool m_changeTracking = false;
    std::vector<ChangeTracker> m_changeTracker;
    
    
    
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ParameterFilterGroup*)
