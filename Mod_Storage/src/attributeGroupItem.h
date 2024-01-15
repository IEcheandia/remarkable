#pragma once

#include <QObject>
#include "fliplib/graphContainer.h"

class QUuid;
class QVariant;

namespace precitec
{
namespace storage
{

class Attribute;

/**
 * @brief A pair, consisting of an attribute template, read from the AttributeModel
 *  and a filter instance attribute, belonging to the current selected node
 **/

class AttributeGroupItem : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name CONSTANT)

    Q_PROPERTY(QUuid uuid READ uuid CONSTANT)

    Q_PROPERTY(QUuid instanceId READ instanceId CONSTANT)

    Q_PROPERTY(int userLevel READ userLevel WRITE setUserLevel NOTIFY userLevelChanged)

    Q_PROPERTY(bool publicity READ publicity WRITE setPublicity NOTIFY publicityChanged)

    Q_PROPERTY(QVariant value READ value WRITE setValue NOTIFY valueChanged)

    Q_PROPERTY(QString helpFile READ helpFile CONSTANT)

    Q_PROPERTY(QString unit READ unit CONSTANT)

    Q_PROPERTY(QString description READ description CONSTANT)

    Q_PROPERTY(QVariant defaultValue READ defaultValue CONSTANT)

    Q_PROPERTY(QVariant maxValue READ maxValue CONSTANT)

    Q_PROPERTY(QVariant minValue READ minValue CONSTANT)

    Q_PROPERTY(int groupIndex READ groupIndex CONSTANT)

    Q_PROPERTY(precitec::storage::Attribute* attribute READ attribute CONSTANT)

public:
    AttributeGroupItem(precitec::storage::Attribute* description, fliplib::InstanceFilter::Attribute* instance, QObject* parent = nullptr);
    ~AttributeGroupItem() override;

    QString name() const;

    QUuid uuid() const;

    QUuid instanceId() const;

    int userLevel() const;
    void setUserLevel(int userLevel);

    bool publicity() const;
    void setPublicity(bool publicity);

    QVariant value() const;
    void setValue(const QVariant& value);

    QString helpFile() const;

    QString unit() const;

    QString description() const;

    QVariant defaultValue() const;

    QVariant maxValue() const;

    QVariant minValue() const;

    int groupIndex() const;

    Attribute* attribute() const
    {
        return m_description;
    }

Q_SIGNALS:
    void userLevelChanged();
    void publicityChanged();
    void valueChanged();

private:
    Attribute* m_description;
    fliplib::InstanceFilter::Attribute* m_instance;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::AttributeGroupItem*)
Q_DECLARE_METATYPE(std::vector<precitec::storage::AttributeGroupItem*>)

