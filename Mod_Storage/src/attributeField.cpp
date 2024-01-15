#include "attributeField.h"
#include "jsonSupport.h"

namespace precitec
{
namespace storage
{

AttributeField::AttributeField(QObject* parent)
    : QObject(parent)
{
}

AttributeField::~AttributeField() = default;

AttributeField *AttributeField::fromJson(const QJsonObject& object, QObject* parent)
{
    AttributeField *ret = new AttributeField(parent);
    ret->m_attributeId = json::parseAttributeId(object);
    ret->m_name = json::parseName(object);
    ret->m_locate = json::parseLocate(object);
    ret->m_text = json::parseText(object);
    return ret;
}

QJsonObject AttributeField::toJson() const
{
    return QJsonObject{
        {           
            json::nameToJson(m_name),
            json::locateToJson(m_locate),
            json::textToJson(m_text)
        }
    };
}


    

}
}
