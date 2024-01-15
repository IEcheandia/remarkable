#pragma once

#include <precitec/change.h>

namespace precitec
{
namespace gui
{

class ShutdownChangeEntry : public components::userLog::Change
{
    Q_OBJECT
public:
    enum class Operation {
        Shutdown,
        Restart,
        StopProcesses,
        RestartProcesses
    };
    Q_INVOKABLE ShutdownChangeEntry(QObject *parent = nullptr);
    ShutdownChangeEntry(Operation operation, QObject *parent = nullptr);
    ~ShutdownChangeEntry() override;

protected:
    QJsonObject data() const override;
    void initFromJson(const QJsonObject &data) override;
};

}
}
