#pragma once

#include <QObject>

namespace precitec
{
namespace gui
{

class ScreenConfigurationController : public QObject
{
    Q_OBJECT
public:
    explicit ScreenConfigurationController(QObject *parent = nullptr);
    ~ScreenConfigurationController() override;

    Q_INVOKABLE void startExternalTool();

    Q_INVOKABLE void startKeyboardConfiguration();

    Q_INVOKABLE void startNetworkConfiguration();

    Q_INVOKABLE void startDebugConsole();

    Q_INVOKABLE bool isKeyboardConfigurationAvailable();

    Q_INVOKABLE bool isNetworkManagementConfigurationAvailable();
};

}
}
