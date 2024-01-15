#include "attributeGroup.h"
#include "attributeGroupItem.h"
#include "attribute.h"

namespace precitec
{
namespace storage
{

AttributeGroup::AttributeGroup(Attribute* description, fliplib::InstanceFilter::Attribute* instance, QObject* parent)
    : QObject(parent)
{
    if (!description)
    {
        m_contentName = QString::fromStdString(instance->name);
    } else
    {
        m_groupId = description->groupId();
        m_contentName = description->contentName();
        m_unit = description->unit();
        m_description = description->description();
        m_defaultValue = description->defaultValue();
        m_minValue = description->minValue();
        m_maxValue = description->maxValue();
        m_editListOrder = description->editListOrder();
    }

    m_visible = instance->visible;
    m_userLevel = instance->userLevel;
    m_publicity = instance->publicity;
    m_helpFile = QString::fromStdString(instance->helpFile);

    addItem(description, instance);
}

AttributeGroup::~AttributeGroup() = default;

void AttributeGroup::setUserLevel(int userLevel)
{
    if (m_userLevel == userLevel)
    {
        return;
    }
    m_userLevel = userLevel;

    for (auto& item : m_items)
    {
        item->setUserLevel(userLevel);
    }

    emit userLevelChanged();
}

void AttributeGroup::setSelected(bool selected)
{
    if (m_selected == selected)
    {
        return;
    }
    m_selected = selected;
    emit selectedChanged();
}

void AttributeGroup::setPublicity(bool publicity)
{
    if (m_publicity == publicity)
    {
        return;
    }
    m_publicity = publicity;

    for (auto& item : m_items)
    {
        item->setPublicity(publicity);
    }

    emit publicityChanged();
}

void AttributeGroup::addItem(precitec::storage::Attribute* description, fliplib::InstanceFilter::Attribute* instance)
{
    m_items.emplace_back(new AttributeGroupItem{description, instance, this});

    if (m_items.size() > 1)
    {
        std::sort(m_items.begin(), m_items.end(), [] (auto item1, auto item2) {
            return item1->groupIndex() < item2->groupIndex();
        });
    }

    emit itemsChanged();
}

}
}
