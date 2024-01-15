#include "attributeGroupItem.h"
#include "attribute.h"
#include "../App_Storage/src/compatibility.h"

#include <QUuid>
#include <QVariant>

namespace precitec
{
namespace storage
{

AttributeGroupItem::AttributeGroupItem(Attribute* description, fliplib::InstanceFilter::Attribute* instance, QObject* parent)
    : QObject(parent)
    , m_description(description)
    , m_instance(instance)
{
}

AttributeGroupItem::~AttributeGroupItem() = default;

QString AttributeGroupItem::name() const
{
    return QString::fromStdString(m_instance->name);
}

QUuid AttributeGroupItem::uuid() const
{
    if (m_description)
    {
        return m_description->uuid();
    }
    if (m_instance)
    {
        return precitec::storage::compatibility::toQt(m_instance->attributeId);
    }
    return {};
}

QUuid AttributeGroupItem::instanceId() const
{
    return precitec::storage::compatibility::toQt(m_instance->instanceAttributeId);
}

int AttributeGroupItem::userLevel() const
{
    return m_instance->userLevel;
}

void AttributeGroupItem::setUserLevel(int userLevel)
{
    if (m_instance->userLevel == userLevel)
    {
        return;
    }
    m_instance->userLevel = userLevel;
    emit userLevelChanged();
}

bool AttributeGroupItem::publicity() const
{
    return m_instance->publicity;
}

void AttributeGroupItem::setPublicity(bool publicity)
{
    if (m_instance->publicity == publicity)
    {
        return;
    }
    m_instance->publicity = publicity;
    emit publicityChanged();
}

QVariant AttributeGroupItem::value() const
{
    const auto value = m_instance->value;
    if (m_description)
    {
        return m_description->convert(value);
    }
    else if (value.isString())
    {
        return QString::fromStdString(value.convert<std::string>());
    }
    return {};
}

void AttributeGroupItem::setValue(const QVariant& value)
{
    if (this->value() == value)
    {
        return;
    }
    switch (value.type())
    {
        case QVariant::Bool:
        {
            if (value.value<bool>())
            {
                m_instance->value = 1;
            } else
            {
                m_instance->value = 0;
            }
            break;
        }
        case QVariant::Int:
        {
            m_instance->value = value.toInt();
            break;
        }
        case QVariant::Double:
        {
            m_instance->value = value.toDouble();
            break;
        }
        default:
            m_instance->value = value.toString().toStdString();
    }
    emit valueChanged();
}

QString AttributeGroupItem::helpFile() const
{
    return QString::fromStdString(m_instance->helpFile);
}

QString AttributeGroupItem::unit() const
{
    if (m_description)
    {
        return m_description->unit();
    }
    return {};
}

QString AttributeGroupItem::description() const
{
    if (m_description)
    {
        return m_description->description();
    }
    return {};
}

QVariant AttributeGroupItem::defaultValue() const
{
    if (m_description)
    {
        return m_description->defaultValue();
    }
    return {};
}

QVariant AttributeGroupItem::maxValue() const
{
    if (m_description)
    {
        return m_description->maxValue();
    }
    return {};
}

QVariant AttributeGroupItem::minValue() const
{
    if (m_description)
    {
        return m_description->minValue();
    }
    return {};
}

int AttributeGroupItem::groupIndex() const
{
    if (m_description)
    {
        return m_description->groupIndex();
    }
    return 0;
}

}
}
