#pragma once

#include <QObject>

namespace precitec
{
namespace storage
{

class ErrorTemplate : public QObject
{
    Q_OBJECT

    Q_PROPERTY(int enumType READ enumType CONSTANT)

    Q_PROPERTY(QString name READ name CONSTANT)

public:
    explicit ErrorTemplate(const int enumType, const QString &name, QObject *parent = nullptr);
    ~ErrorTemplate() override;

    int enumType() const
    {
        return m_enumType;
    }

    QString name() const
    {
        return m_name;
    }

private:
    int m_enumType;
    QString m_name;
};

}
}


