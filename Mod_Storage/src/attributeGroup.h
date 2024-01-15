#pragma once

#include <QObject>
#include <QUuid>
#include <QVariant>

#include "fliplib/graphContainer.h"

namespace precitec
{
namespace storage
{

class Attribute;
class AttributeGroupItem;

/**
 * @brief Container to collect all attributes with a common groupId
 *
 * Takes a pair of a Attribute description and a filter instance Attribute and
 * constructs a AttributeGroupItem.
 * Sorts group items, based on their groupIndex
 **/

class AttributeGroup : public QObject
{
    Q_OBJECT

    /**
     * Attribute Group Items contained in this Attribute Group
     **/
    Q_PROPERTY(std::vector<precitec::storage::AttributeGroupItem*> items READ items NOTIFY itemsChanged);

public:
    AttributeGroup(precitec::storage::Attribute* description, fliplib::InstanceFilter::Attribute* instance, QObject* parent = nullptr);
    ~AttributeGroup() override;

    const QString& contentName() const
    {
        return m_contentName;
    }

    const QString& unit() const
    {
        return m_unit;
    }

    const QString& description() const
    {
        return m_description;
    }

    const QString& helpFile() const
    {
        return m_helpFile;
    }

    const QVariant& defaultValue() const
    {
        return m_defaultValue;
    }

    const QVariant& maxValue() const
    {
        return m_maxValue;
    }

    const QVariant& minValue() const
    {
        return m_minValue;
    }

    const QUuid& groupId() const
    {
        return m_groupId;
    }

    int userLevel() const
    {
        return m_userLevel;
    }
    void setUserLevel(int userLevel);

    int editListOrder() const
    {
        return m_editListOrder;
    }

    bool selected() const
    {
        return m_selected;
    }
    void setSelected(bool selected);

    bool visible() const
    {
        return m_visible;
    }

    bool publicity() const
    {
        return m_publicity;
    }
    void setPublicity(bool publicity);

    const std::vector<AttributeGroupItem*>& items() const
    {
        return m_items;
    }

    void addItem(precitec::storage::Attribute* description, fliplib::InstanceFilter::Attribute* instance);

Q_SIGNALS:
    void userLevelChanged();
    void itemsChanged();
    void selectedChanged();
    void publicityChanged();

private:
    QString m_contentName;
    QString m_unit;
    QString m_description;
    QString m_helpFile;
    QVariant m_defaultValue;
    QVariant m_minValue;
    QVariant m_maxValue;

    int m_userLevel = 0;
    int m_editListOrder = 0;
    bool m_selected = false;
    bool m_visible = false;
    bool m_publicity = false;

    QUuid m_groupId;

    std::vector<AttributeGroupItem*> m_items;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::AttributeGroup*)


