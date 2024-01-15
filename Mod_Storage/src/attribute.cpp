#include "attribute.h"
#include "attributeField.h"
#include "jsonSupport.h"

#include <QFile>

#include <QJsonObject>

#include <Poco/Dynamic/Var.h>

#include "module/moduleLogger.h"
#include "message/device.interface.h"
#include "module/logType.h"


namespace precitec
{
namespace storage
{

Attribute::Attribute(const QUuid &uuid, QObject* parent)
    : QObject(parent)
    , m_uuid(uuid)
    
{
}

Attribute::~Attribute() = default;

QUuid Attribute::uuid() const
{
    return m_uuid;
}

bool Attribute::publicity() const
{
    return m_publicity;
}

void Attribute::setPublicity(bool publicity)
{
    if (publicity == m_publicity)
    {
        return;
    }
    m_publicity = publicity;
    emit publicityChanged();
}

int Attribute::editListOrder() const
{
    return m_editListOrder;
}

void Attribute::setEditListOrder(int editListOrder)
{
    if (editListOrder == m_editListOrder)
    {
        return;
    }
    m_editListOrder = editListOrder;
    emit editListOrderChanged();
}

QString Attribute::contentName() const
{
    return m_contentName;
}

void Attribute::setContentName(const QString& contentName)
{
    if (contentName == m_contentName)
    {
        return;
    }
    m_contentName = contentName;
    emit contentNameChanged();
}

QString Attribute::enumeration() const
{
    return m_enumeration;
}

void Attribute::setEnumeration(const QString& enumeration)
{
    if (enumeration == m_enumeration)
    {
        return;
    }
    m_enumeration = enumeration;
    emit enumerationChanged();
}

bool Attribute::visible() const
{
    return m_visible;
}

void Attribute::setVisible(bool visible)
{
    if (visible == m_visible)
    {
        return;
    }
    m_visible = visible;
    emit visibleChanged();
}

int Attribute::userLevel() const
{
    return m_userLevel;
}

void Attribute::setUserLevel(int userLevel)
{
    if (userLevel == m_userLevel)
    {
        return;
    }
    m_userLevel = userLevel;
    emit userLevelChanged();
}

QString Attribute::toolTip() const
{
    return m_toolTip;
}

void Attribute::setToolTip(const QString& toolTip)
{
    if (toolTip == m_toolTip)
    {
        return;
    }
    m_toolTip = toolTip;
    emit toolTipChanged();
}

QString Attribute::description() const
{
    return m_description;
}

void Attribute::setDescription(const QString& description)
{
    if (description == m_description)
    {
        return;
    }
    m_description = description;
    emit descriptionChanged();
}

QString Attribute::unit() const
{
    return m_unit;
}

void Attribute::setUnit(const QString& unit)
{
    if (unit == m_unit)
    {
        return;
    }
    m_unit = unit;
    emit unitChanged();
}

QVariant Attribute::defaultValue() const
{
    return m_defaultValue;
}

void Attribute::setDefaultValue(const QVariant& defaultValue)
{
    if (defaultValue == m_defaultValue)
    {
        return;
    }
    m_defaultValue = defaultValue;
    emit defaultValueChanged();
}

QVariant Attribute::maxValue() const
{
    return m_maxValue;
}

void Attribute::setMaxValue(const QVariant& maxValue)
{
    if (maxValue == m_maxValue)
    {
        return;
    }
    m_maxValue = maxValue;
    emit maxValueChanged();
}

QVariant Attribute::minValue() const
{
    return m_minValue;
}

void Attribute::setMinValue(const QVariant& minVaule)
{
    if (minVaule == m_minValue)
    {
        return;
    }
    m_minValue = minVaule;
    emit minValueChanged();
}

int Attribute::maxLength() const
{
    return m_maxLength;
}

void Attribute::setMaxLength(int maxLength)
{
    if (maxLength == m_maxLength)
    {
        return;
    }
    m_maxLength = maxLength;
    emit maxValueChanged();
}

bool Attribute::mandatory() const
{
    return m_mandatory;
}

void Attribute::setMandatory(bool mandatory)
{
    if (mandatory == m_mandatory)
    {
        return;
    }
    m_mandatory = mandatory;
    emit mandatoryChanged();
}

Parameter::DataType Attribute::type() const
{
    return m_type;
}

void Attribute::setType(Parameter::DataType type)
{
    if (type == m_type)
    {
        return;
    }
    m_type = type;
    emit typeChanged();
}

QString Attribute::name() const
{
    return m_name;
}

void Attribute::setName(const QString& name)
{
    if (m_name == name)
    {
        return;
    }
    m_name = name;
    emit nameChanged();
}

QUuid Attribute::variantId() const
{
    return m_variantId;
}

void Attribute::setVariantId(const QUuid& variantId)
{
    if (variantId == m_variantId)
    {
        return;
    }
    m_variantId = variantId;
    emit variantIdChanged();
}

void Attribute::setStep(int step)
{
    if (m_step == step)
    {
        return;
    }
    m_step = step;
    emit stepChanged();
}

int Attribute::step() const
{
    return m_step;
}

void Attribute::setFields(std::vector<AttributeField*> &&fields)
{
    m_fields = std::move(fields);
}

Attribute *Attribute::fromJson(const QJsonObject &object, QObject *parent)
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
    Attribute *a = new Attribute(uuid, parent);
    a->setVariantId(json::parseVariantId(object));
    a->setName(json::parseName(object));
    a->setType(json::parseDataType(object));
    a->setMinValue(json::parseMinValue(object));
    a->setMaxValue(json::parseMaxValue(object));
    a->setDefaultValue(json::parseDefaultValue(object));
    a->setUnit(json::parseUnit(object));
    a->setDescription(json::parseDescription(object));
    a->setToolTip(json::parseToolTip(object));
    a->setEnumeration(json::parseEnumeration(object));
    a->setContentName(json::parseContentName(object));
    a->setMandatory(json::parseMandatory(object));
    a->setVisible(json::parseVisible(object));
    a->setPublicity(json::parsePublicity(object));
    a->setMaxLength(json::parseMaxLength(object));
    a->setEditListOrder(json::parseEditListOrder(object));
    a->setStep(json::parseStep(object));
    a->setUserLevel(json::parseUserLevel(object));
    if (a->type() == Parameter::DataType::Float || a->type() == Parameter::DataType::Double)
    {
        a->setFloatingPointPrecision(json::parseFloatingPointPrecision(object));
    }
    a->setGroupId(json::parseGroupId(object));
    a->setGroupIndex(json::parseGroupIndex(object));

    if (a->type() == Parameter::DataType::File)
    {
        a->m_fileInforamtion = AttributeFileInformation::fromJson(object.value(QLatin1String("file")).toObject());
    }
    
    if((a->type() == Parameter::DataType::Enumeration))
    {
        a->setFields(json::parseAttributeFields(object, a));
    }
        
    
    return a;
}

void Attribute::loadFieldsFromJson(QJsonDocument document)
{    
   
    QJsonParseError parseError;    
    
    if (document.isNull())
    {
        wmLog(eError, "Attribute Json file parse error: " + parseError.errorString().toStdString() +"\n");       
    }
  
    
    auto fields = json::parseAttributeFields(document.object(), nullptr);  
    std::copy_if(fields.begin(), fields.end(), std::back_inserter(m_fields), [this] (auto field) 
    {         
        return this->uuid() == field->attributeId();
        
    });       
    
    std::sort(m_fields.begin(), m_fields.end(), [] (auto a, auto b) { return a->locate() < b->locate(); });  
    
    
    
}

QJsonObject Attribute::toJson() const
{    
    return QJsonObject{
        {            
            json::toJson(m_variantId),
            json::toJson(m_uuid),
            json::nameToJson(m_name),
            json::dataTypeToJson(m_type),
            json::mandatoryToJson(m_mandatory),
            json::maxLengthToJson(m_maxLength),
            json::minValueToJson(m_minValue),
            json::maxValueToJson(m_maxValue),
            json::defaultValueToJson(m_defaultValue),
            json::unitToJson(m_unit),
            json::descriptionToJson(m_description),
            json::toolTipToJson(m_toolTip),
            json::userLevelToJson(m_userLevel),
            json::visibleToJson(m_visible),
            json::stepToJson(m_step),
            json::enumerationToJson(m_enumeration),
            json::contentNameToJson(m_contentName),
            json::editListOrderToJson(m_editListOrder),
            json::publicityToJson(m_publicity),
            json::toJson(m_fields) 
            
        }
    };
}

QStringList Attribute::fields() const
{
    QStringList ret;
    for (auto field : m_fields)
    {
        ret << field->name();
    }
    return ret;
}

void Attribute::setFloatingPointPrecision(int precision)
{
    if (m_floatingPointPrecision == precision)
    {
        return;
    }
    m_floatingPointPrecision = precision;
    emit floatingPointPrecisionChanged();
}

void Attribute::setGroupId(const QUuid& groupId)
{
    if (groupId == m_groupId)
    {
        return;
    }
    m_groupId = groupId;
    emit groupIdChanged();
}

void Attribute::setGroupIndex(int index)
{
    if (index == m_groupIndex)
    {
        return;
    }
    m_groupIndex = index;
    emit groupIndexChanged();
}

QVariant Attribute::convert(const Poco::Dynamic::Var &var)
{
    switch (m_type)
    {
    case Parameter::DataType::Integer:
    case Parameter::DataType::Enumeration:
    case Parameter::DataType::Error:
    case Parameter::DataType::Result:
    case Parameter::DataType::Sensor:
    case Parameter::DataType::SeamFigure:
    case Parameter::DataType::WobbleFigure:
        try
        {
            return {var.convert<int>()};
        } catch (...)
        {
        }
        break;
    case Parameter::DataType::UnsignedInteger:
        try
        {
            return {var.convert<uint>()};
        } catch (...)
        {
        }
        break;
    case Parameter::DataType::Float:
    case Parameter::DataType::Double:
        try
        {
            return {var.convert<double>()};
        } catch (...)
        {
        }
        break;
    case Parameter::DataType::String:
    case Parameter::DataType::Text:
    case Parameter::DataType::File:
        try
        {
            return QString::fromStdString(var.convert<std::string>());
        } catch (...)
        {
        }
        break;
    case Parameter::DataType::Boolean:
        try
        {
            return {var.convert<bool>()};
        } catch (...)
        {
        }
        break;
    default:
        break;
    }
    return {};
}

void Attribute::setReadOnly(bool set)
{
    if (m_readOnly == set)
    {
        return;
    }
    m_readOnly = set;
    emit readOnlyChanged();
}


QVariant Attribute::convertEnumValueToInt(interface::SmpKeyValue keyValue, QVariant row)
{   
    int result;
    //wmLog(eInfo, "ConvertEnumValueToInt: Keyvalue->value: %d and row: %d\n", keyValue->value<int>(), row);
    
    if(this->type() != Parameter::DataType::Enumeration)
    {    
        return row;
    }
                                                                                
    
    result = this->minValue().toInt() + row.toInt();
    QVariant QResult (result);
    return (QResult);

}

int Attribute::convertFromValueToIndex(int value)
{
    wmLog(eDebug, "ConverFromValueToIndex: Attribute: %s and Value: %d and return value is: %d \n",this->contentName().toStdString(), value, value - this->minValue().toInt()); 
    return (value - this->minValue().toInt());
}
}

};


