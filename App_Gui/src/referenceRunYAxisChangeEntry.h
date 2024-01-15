#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class ReferenceRunYAxisChangeEntry : public components::userLog::Change
{
    Q_OBJECT
public:
    Q_INVOKABLE ReferenceRunYAxisChangeEntry(QObject *parent = nullptr);
    ~ReferenceRunYAxisChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;
};

}
}
