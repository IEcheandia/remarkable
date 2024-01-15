#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class HardwareRoiFlushedChangeEntry : public components::userLog::Change
{
    Q_OBJECT
public:
    Q_INVOKABLE HardwareRoiFlushedChangeEntry(QObject *parent = nullptr);
    ~HardwareRoiFlushedChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;
};

}
}
