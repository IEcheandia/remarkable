#pragma once

#include <precitec/change.h>
#include "message/device.h"

#include <QUuid>
#include <QVariant>

namespace precitec
{
namespace gui
{

class DeviceKeyValueChangeEntry : public components::userLog::Change
{
    Q_OBJECT
    Q_PROPERTY(QUuid device READ device CONSTANT)
    Q_PROPERTY(QString key READ key CONSTANT)
    Q_PROPERTY(QVariant value READ value CONSTANT)
public:
    Q_INVOKABLE DeviceKeyValueChangeEntry(QObject *parent = nullptr);
    DeviceKeyValueChangeEntry(const QUuid &device, const precitec::interface::SmpKeyValue &kv, QObject *parent = nullptr);
    ~DeviceKeyValueChangeEntry() override;

    QUrl detailVisualization() const override;

    QUuid device() const
    {
        return m_device;
    }
    QString key() const
    {
        return m_key;
    }
    QVariant value() const
    {
        return m_value;
    }

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

private:
    QUuid m_device;
    QString m_key;
    QVariant m_value;
};

}
}
