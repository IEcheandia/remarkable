#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class HardwareConfigurationRestoreChangeEntry : public components::userLog::Change
{
    Q_OBJECT
public:
    Q_INVOKABLE HardwareConfigurationRestoreChangeEntry(QObject *parent = nullptr);
    HardwareConfigurationRestoreChangeEntry(const QString &date, QObject *parent = nullptr);
    ~HardwareConfigurationRestoreChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;

private:
    QString m_date;
};

}
}
