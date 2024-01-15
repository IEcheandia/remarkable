#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class HardwareConfigurationBackupChangeEntry : public components::userLog::Change
{
    Q_OBJECT
public:
    Q_INVOKABLE HardwareConfigurationBackupChangeEntry(QObject *parent = nullptr);
    ~HardwareConfigurationBackupChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;
};

}
}
