#pragma once

#include <QObject>
#include <QUuid>
#include <QJsonDocument>

#include "parameter.h"
#include "attributeField.h"
#include "attributeFileInformation.h"
#include "message/device.interface.h"


namespace Poco
{
namespace Dynamic
{
class Var;
}
}


namespace precitec
{
namespace storage
{
class AttributeField;

/**
 * Represents an attribute description for filter parameters.
 */
class Attribute : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)
    Q_PROPERTY(bool publicity READ publicity WRITE setPublicity NOTIFY publicityChanged)
    Q_PROPERTY(int editListOrder READ editListOrder WRITE setEditListOrder NOTIFY editListOrderChanged)
    Q_PROPERTY(QString contentName READ contentName WRITE setContentName NOTIFY contentNameChanged)
    Q_PROPERTY(QString enumeration READ enumeration WRITE setEnumeration NOTIFY enumerationChanged)
    Q_PROPERTY(bool visible READ visible WRITE setVisible NOTIFY visibleChanged)
    Q_PROPERTY(int userLevel READ userLevel WRITE setUserLevel NOTIFY userLevelChanged)
    Q_PROPERTY(QString toolTip READ toolTip WRITE setToolTip NOTIFY toolTipChanged)
    Q_PROPERTY(QString description READ description WRITE setDescription NOTIFY descriptionChanged)
    Q_PROPERTY(QString unit READ unit WRITE setUnit NOTIFY unitChanged)
    Q_PROPERTY(QVariant defaultValue READ defaultValue WRITE setDefaultValue NOTIFY defaultValueChanged)
    Q_PROPERTY(QVariant maxValue READ maxValue WRITE setMaxValue NOTIFY maxValueChanged)
    Q_PROPERTY(QVariant minValue READ minValue WRITE setMinValue NOTIFY minValueChanged)
    Q_PROPERTY(int maxLength READ maxLength WRITE setMaxLength NOTIFY maxLengthChanged)
    Q_PROPERTY(bool mandatory READ mandatory WRITE setMandatory NOTIFY mandatoryChanged)
    Q_PROPERTY(precitec::storage::Parameter::DataType type READ type WRITE setType NOTIFY typeChanged)
    Q_PROPERTY(QString name READ name WRITE setName NOTIFY nameChanged)
    Q_PROPERTY(QUuid variantId READ variantId WRITE setVariantId NOTIFY variantIdChanged)
    Q_PROPERTY(int step READ step WRITE setStep NOTIFY stepChanged)
    /**
     * The precision for floating point values. If the type is not DataType::Float or DataType::Double the precision is @c -1.
     * By default for floating point values the precision is 3 when loading from json.
     **/
    Q_PROPERTY(int floatingPointPrecision READ floatingPointPrecision WRITE setFloatingPointPrecision NOTIFY floatingPointPrecisionChanged)
    Q_PROPERTY(QUuid groupId READ groupId WRITE setGroupId NOTIFY groupIdChanged)
    Q_PROPERTY(int groupIndex READ groupIndex WRITE setGroupIndex NOTIFY groupIndexChanged)
    /**
     * Whether this Attribute describes a value which is only meant as read only
     **/
    Q_PROPERTY(bool readOnly READ isReadOnly WRITE setReadOnly NOTIFY readOnlyChanged)

    /**
     * The information for @link type @c File. If not set a default constructed is returned.
     **/
    Q_PROPERTY(precitec::storage::AttributeFileInformation fileInformation READ fileInformation CONSTANT)
public:
    Attribute(const QUuid &uuid, QObject* parent = nullptr);
    ~Attribute() override;

    QUuid uuid() const;
    bool publicity() const;
    int editListOrder() const;
    QString contentName() const;
    QString enumeration() const;
    bool visible() const;
    int userLevel() const;
    QString toolTip() const;
    QString description() const;
    QString unit() const;
    QVariant defaultValue() const;
    QVariant maxValue() const;
    QVariant minValue() const;
    int maxLength() const;
    bool mandatory() const;
    Parameter::DataType type() const;
    QString name() const;
    QUuid variantId() const;
    int step() const;
    int floatingPointPrecision() const
    {
        return m_floatingPointPrecision;
    }
    const QUuid& groupId() const
    {
        return m_groupId;
    }
    int groupIndex() const
    {
        return m_groupIndex;
    }

    bool isReadOnly() const
    {
        return m_readOnly;
    }

    

    AttributeFileInformation fileInformation() const
    {
        return m_fileInforamtion.value_or(AttributeFileInformation{});
    }

    /**
     * The text elements of the (enumeration) fields.
     **/
    Q_INVOKABLE QStringList fields() const;

    void setPublicity(bool publicity);
    void setEditListOrder(int editListOrder);
    void setContentName(const QString& contentName);
    void setEnumeration(const QString& enumeration);
    void setVisible(bool visible);
    void setUserLevel(int userLevel);
    void setToolTip(const QString& toolTip);
    void setDescription(const QString& description);
    void setUnit(const QString& unit);
    void setDefaultValue(const QVariant& defaultValue);
    void setMaxValue(const QVariant& maxValue);
    void setMinValue(const QVariant& minVaule);
    void setMaxLength(int maxLength);
    void setMandatory(bool mandatory);
    void setType(Parameter::DataType type);
    void setName(const QString& name);
    void setVariantId(const QUuid& variantId);
    void setStep(int step);
    void setFloatingPointPrecision(int precision);
    void setGroupId(const QUuid& groupId);
    void setGroupIndex(int index);
    void setReadOnly(bool set);
    void setFields(std::vector<AttributeField*> &&fields);

    /**
     * Converts the value @p var to a QVariant based on the type information of the Attribute.
     * This is required for properly interpreting the value of an InstanceAttribute.
     **/
    QVariant convert(const Poco::Dynamic::Var &var);

    static Attribute *fromJson(const QJsonObject &object, QObject *parent);    
    QJsonObject toJson() const;
    
    void loadFieldsFromJson(QJsonDocument document);    
    
    
    /** Method to map a combobox index to real value of a paramater. 
     * Used with parameter attributes type "enum" in Parameter Editor.
     **/    
    QVariant convertEnumValueToInt(interface::SmpKeyValue keyValue, QVariant row);
    
     Q_INVOKABLE int convertFromValueToIndex(int value);


Q_SIGNALS:
    void publicityChanged();
    void editListOrderChanged();
    void contentNameChanged();
    void enumerationChanged();
    void visibleChanged();
    void userLevelChanged();
    void toolTipChanged();
    void descriptionChanged();
    void unitChanged();
    void defaultValueChanged();
    void maxValueChanged();
    void minValueChanged();
    void maxLengthChanged();
    void mandatoryChanged();
    void typeChanged();
    void nameChanged();
    void variantIdChanged();
    void stepChanged();
    void floatingPointPrecisionChanged();
    void groupIdChanged();
    void groupIndexChanged();
    void readOnlyChanged();

private:
    QUuid m_uuid;
    bool m_publicity = false;
    int m_editListOrder = 0;
    QString m_contentName;
    QString m_enumeration;
    bool m_visible = false;
    int m_userLevel = 0;
    QString m_toolTip;
    QString m_description;
    QString m_unit;
    QVariant m_defaultValue;
    QVariant m_maxValue;
    QVariant m_minValue;
    int m_maxLength = 0;
    bool m_mandatory = false;
    Parameter::DataType m_type = Parameter::DataType::Unknown;
    QString m_name;
    QUuid m_variantId;
    QUuid m_groupId;
    int m_groupIndex = 0;
    int m_step = 0;
    std::vector<AttributeField*> m_fields;
    int m_floatingPointPrecision = -1;
    bool m_readOnly{false};
    std::optional<AttributeFileInformation> m_fileInforamtion;
    
};

}
}

Q_DECLARE_METATYPE(precitec::storage::Attribute*)
