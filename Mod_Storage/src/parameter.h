#pragma once
#include "changeTracker.h"
#include "parameterFilterGroup.h"

#include <QObject>
#include <QUuid>
#include <QVariant>

#include <memory>

namespace precitec
{

namespace interface
{
class FilterParameter;
}

namespace storage
{

class ParameterSet;

/**
 * @brief The Parameter describes one concrete Hardware or Filter parameter. It belongs to a ParameterSet
 **/
class Parameter : public QObject
{
    Q_OBJECT
    /**
     * The unique identifier of this Parameter
     **/
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    /**
     * The ParameterSet this Parameter belongs to.
     **/
    Q_PROPERTY(precitec::storage::ParameterSet *parameterSet READ parameterSet CONSTANT)
    /**
     * The unique identifier of the type this Parameter is for.
     * E.g. the hardware device in case of a hardware parameter.
     **/
    Q_PROPERTY(QUuid typeId READ typeId WRITE setTypeId NOTIFY typeIdChanged)
    /**
     * The descriptive name of this Parameter.
     **/
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    /**
     * The actual value of the property.
     **/
    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)
    /**
     * The data type of the value.
     **/
    Q_PROPERTY(DataType type READ type WRITE setType NOTIFY typeChanged)
    /**
     * In case this Parameter references a filter parameter instead of a hardware parameter,
     * this is the unique id of the instance filter the parameter belongs to.
     **/
    Q_PROPERTY(QUuid filterId READ filterId WRITE setFilterId NOTIFY filterIdChanged)
public:
    /**
     * Constructs the Parameter with the given @p uuid and the given @p parent ParameterSet.
     **/
    explicit Parameter(const QUuid &uuid, ParameterSet *parent);
    explicit Parameter(const QUuid &uuid, ParameterFilterGroup *parent);
    Parameter(const Parameter& other);
    ~Parameter() override;

    /**
     * The actual type of the value encapsulated in this Paramer.
     * We cannot use QVariant's automatic detection as Json does
     * not allow to distinguish between int, uint, float and double.
     **/
    enum class DataType
    {
        /**
         * Represents an int.
         **/
        Integer,
        /**
         * Represents an uint.
         **/
        UnsignedInteger,
        /**
         * Represents a float.
         **/
        Float,
        /**
         * Represents a double.
         **/
        Double,
        /**
         * Represents a bool.
         **/
        Boolean,
        /**
         * Represents an enumeration value.
         **/
        Enumeration,
        /**
         * Represents an error
         **/
        Error,
        /**
         * Represents an messwert
         **/
        Result,
        /**
         * Represents a string
         **/
        String,
        /**
         * Represents a text
         **/
        Text,
        /**
         * Represents a sensor
         **/
        Sensor,
        /**
         * Represents a seam figure file
         **/
        SeamFigure,
        /**
         * Represents a wobble figure file
         **/
        WobbleFigure,
        /**
         * Represents a generic file, stored like a String.
         **/
        File,
        /**
         * An invalid data type
         **/
        Unknown
    };
    Q_ENUM(DataType)

    QUuid uuid() const
    {
        return m_uuid;
    }

    QUuid typeId() const
    {
        return m_typeId;
    }

    ParameterSet *parameterSet() const
    {
        return m_parameterSet;
    }

    QString name() const
    {
        return m_name;
    }

    QVariant value() const
    {
        return m_value;
    }

    DataType type() const
    {
        return m_type;
    }

    QUuid filterId() const
    {
        return m_filterId;
    }

    void setTypeId(const QUuid &uuid);
    void setName(const QString &name);
    void setValue(const QVariant &value);
    void setType(DataType type);
    void setFilterId(const QUuid &uuid);

    /**
     * Duplicates this Parameter. Does not assign a new uuid.
     * @param parent The parent for the new duplicated Parameter
     **/
    Parameter *duplicate(ParameterSet *parent = nullptr) const;
   

    /**
     * @returns A QJsonObject representation for this object.
     * @see fromJson
     **/
    QJsonObject toJson() const;
    QJsonObject toJson(const QObject* ParameterFilterGroup) const;

    bool isChangeTracking() const;
    QJsonArray changes() const;

    std::shared_ptr<precitec::interface::FilterParameter> toFilterParameter() const;

    /**
     * Creates a new Parameter from the provided json @p object.
     * If the @p object is empty @c null is returned.
     *
     * The uuid is taken from the @p object if provided. If no uuid is
     * provided a new uuid gets generated.
     *
     * The @p parent is set as the parent ParameterSet for the created Parameter.
     *
     * @returns New created Parameter or @c null if @p object is empty.
     **/
    static Parameter *fromJson(const QJsonObject &object, ParameterSet *parent);
    static Parameter *fromJson(const QJsonObject &object, ParameterFilterGroup *parent);

Q_SIGNALS:
    void typeIdChanged();
    void nameChanged();
    void valueChanged();
    void typeChanged();
    void filterIdChanged();

private:
    void addChange(ChangeTracker &&change);
    QUuid m_uuid;
    ParameterSet *m_parameterSet = nullptr;
    ParameterFilterGroup *m_filterParameterGroup = nullptr;
    QUuid m_typeId;
    QString m_name;
    QVariant m_value;
    DataType m_type = DataType::Unknown;
    QUuid m_filterId;
    std::vector<ChangeTracker> m_changeTracker;
};

}
}
