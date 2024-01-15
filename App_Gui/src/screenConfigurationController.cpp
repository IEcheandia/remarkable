#include "screenConfigurationController.h"

#include <QProcess>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusPendingCall>

namespace precitec
{
namespace gui
{
static const QString s_kcmScreen{QStringLiteral("kscreen")};
static const QString s_kcmKeyboard{QStringLiteral("kcm_keyboard")};
static const QString s_kcmNetwork{QStringLiteral("kcm_networkmanagement")};

namespace
{
void startKcm(const QString& component)
{
    QProcess::startDetached(QStringLiteral("/usr/bin/kcmshell5"), {component});
}

bool isAvailable(const QString& component)
{
    QProcess process;
    process.setArguments({QStringLiteral("--list")});
    process.setProgram(QStringLiteral("/usr/bin/kcmshell5"));
    process.start();
    process.waitForFinished();
    return process.readAllStandardOutput().contains(component.toUtf8());
}

}

ScreenConfigurationController::ScreenConfigurationController(QObject *parent)
    : QObject(parent)
{
}

ScreenConfigurationController::~ScreenConfigurationController() = default;

void ScreenConfigurationController::startExternalTool()
{
    startKcm(s_kcmScreen);
}

void ScreenConfigurationController::startKeyboardConfiguration()
{
    startKcm(s_kcmKeyboard);
}

void ScreenConfigurationController::startNetworkConfiguration()
{
    startKcm(s_kcmNetwork);
}

void ScreenConfigurationController::startDebugConsole()
{
    QDBusMessage msg = QDBusMessage::createMethodCall(QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("/KWin"),
                                                      QStringLiteral("org.kde.KWin"),
                                                      QStringLiteral("showDebugConsole"));
    QDBusConnection::sessionBus().asyncCall(msg);
}

bool ScreenConfigurationController::isKeyboardConfigurationAvailable()
{
    return isAvailable(s_kcmKeyboard);
}

bool ScreenConfigurationController::isNetworkManagementConfigurationAvailable()
{
    return isAvailable(s_kcmNetwork);
}

}
}
