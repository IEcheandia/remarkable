#pragma once
#include "changeTracker.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

#include <vector>

#include <Poco/SharedPtr.h>

namespace precitec
{

namespace interface
{
class FilterParameter;
class KeyValue;
}

namespace storage
{
class Attribute;
class Parameter;
class ParameterFilterGroup;

enum class CopyMode;

/**
 * @brief Small wrapper class around a list of Parameters with a unique identifier.
 **/
class ParameterSet : public QObject
{
    Q_OBJECT
    /**
     * The unique id of this ParameterSet
     **/
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    /**
     * Read only variant for parameters from QML.
     **/
    Q_PROPERTY(QVariantList parameters READ parametersAsVariantList CONSTANT)
public:
    /**
     * Constructs the ParameterSet with the given @p uuid and @p parent.
     **/
    explicit ParameterSet(const QUuid &uuid, QObject *parent = nullptr);
    ~ParameterSet() override;

    QUuid uuid() const
    {
        return m_uuid;
    }

    /**
     * @returns The actual @link{Parameter}s belonging to this ParameterSet.
     **/
    const std::vector<Parameter*> &parameters() const
    {
        return m_parameters;
    }
    QVariantList parametersAsVariantList() const;

    /**
     * Duplicates this ParameterSet and assigns the given QUuid to the duplicated ParameterSet.
     * @param mode selects whether the duplicate gets a new, random Uuid or the same Uuid as this object.
     * @param parent The parent for the new duplicated ParameterSet
     **/
    ParameterSet *duplicate(CopyMode mode, QObject *parent = nullptr) const;

    /**
     * Creates a new Parameter with @p id for the @p filterId and adds the new created Parameter to this
     * ParameterSet.
     *
     * The description of the Parameter is taken from the @p attribute. If @p defaultValue is provided the value is
     * taken from @p defaultValue instead of the default value of @p attribute.
     **/
    Parameter *createParameter(const QUuid &id, precitec::storage::Attribute *attribute, const QUuid &filterId, const QVariant &defaultValue = {});

    /**
     * Creates a new Parameter from @p parameter and adds the new created Parameter to this ParameterSet.
     **/
    Parameter *createParameter(const interface::FilterParameter &parameter);

    /**
     * Creates a new Parameter from @p keyValue with typeId set to @p hardwareDevice and adds the new created Parameter to this ParameterSet.
     **/
    Parameter *createParameter(const Poco::SharedPtr<interface::KeyValue> &keyValue, const QUuid &hardwareDevice);

    /**
     * Finds the Parameter with @p name and @p typeId in this ParameterSet. Returns @c null if there is no such Parameter.
     **/
    Parameter *findByNameAndTypeId(const QString &name, const QUuid &typeId) const;

    /**
     * Finds the Parameter with @p id in this ParameterSet. Returns @c null if there is no such Parameter.
     **/
    Parameter *findById(const QUuid &id) const;

    /**
     * Removes the @p parameter from this ParameterSet.
     * The passed in Parameter will be destroyed.
     **/
    void removeParameter(Parameter *parameter);

    /**
     * Add the @p parameter to this ParameterSet.
     **/
    void addParameter(Parameter *parameter);

    /**
     * Removes all parameters from this ParameterSet.
     **/
    void clear();

    /**
     * @returns A QJsonObject representation for this object.
     * @see fromJson
     **/
    QJsonObject toJson() ;

    bool isChangeTracking() const
    {
        return m_changeTracking;
    }
    void setChangeTrackingEnabled(bool set)
    {
        m_changeTracking = set;
    }

    QJsonArray changes() const;

    /**
     * Loads the ParameterSet from the provided Json @p object and returns a new created Product.
     * In case the @p object does not contain a valid Product JSON structure, @c null is returned.
     *
     * The @p parent is set as parent for the newly created ParameterSet.
     *
     * In case no uuid is provided, a new uuid is created for the ParameterSet.
     *
     * @returns New ParameterSet on success, @c null on failure
     **/
    static ParameterSet *fromJson(const QJsonObject &object, QObject *parent = nullptr);
    static ParameterSet *fromJsonFilterParams(const QJsonObject &object, QObject *parent = nullptr);
    
      /**
     * @returns The actual @link{Parameter}s belonging to this ParameterSet.
     **/
    const std::vector<ParameterFilterGroup*> &parameterGroups() const
    {
        return m_filterParametersGroup;
    }

    /**
     * Generates ParameterFilterGroups
     **/
    void updateGrouping();

private:
    void addChange(ChangeTracker &&change);
    QUuid m_uuid;
    std::vector<Parameter*> m_parameters;
    std::vector<ChangeTracker> m_changeTracker;
    bool m_changeTracking = false;
    std::vector<ParameterFilterGroup*> m_filterParametersGroup;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::ParameterSet*)
