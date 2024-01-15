#pragma once

#include <QObject>
#include <QUuid>

namespace precitec
{
namespace storage
{

/**
 * A (enumeration) field of an Attribute.
 **/
class AttributeField : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QUuid attributeId READ attributeId CONSTANT)
    Q_PROPERTY(QString name READ name CONSTANT)
    Q_PROPERTY(QString text READ text CONSTANT)
    Q_PROPERTY(int locate READ locate CONSTANT)
public:
    explicit AttributeField(QObject *parent = nullptr);
    ~AttributeField() override;

    QUuid attributeId() const
    {
        return m_attributeId;
    }

    QString name() const
    {
        return m_name;
    }

    QString text() const
    {
        return m_text;
    }

    int locate() const
    {
        return m_locate;
    }

    static AttributeField *fromJson(const QJsonObject &object, QObject *parent);
    
    QJsonObject toJson() const;

private:
    QUuid m_attributeId;
    QString m_name;
    QString m_text;
    int m_locate = 0;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::AttributeField*)
