#include "openVpnController.h"

#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusObjectPath>
#include <QDBusPendingCall>
#include <QDBusPendingReply>
#include <QDir>

#include <precitec/notificationSystem.h>

namespace precitec
{
namespace gui
{

using components::notifications::NotificationSystem;

static const QString s_systemdService{QStringLiteral("org.freedesktop.systemd1")};
static const QString s_propertiesInterface{QStringLiteral("org.freedesktop.DBus.Properties")};
static const QString s_unitInterface{QStringLiteral("org.freedesktop.systemd1.Unit")};
static const QString s_exampleConfigName{QStringLiteral("weldmaster-openvpn-client.conf")};

OpenVpnController::OpenVpnController(QObject* parent)
    : QObject(parent)
{
    findService();
}

OpenVpnController::~OpenVpnController() = default;

void OpenVpnController::findService()
{
    auto msg = QDBusMessage::createMethodCall(s_systemdService,
                                              QStringLiteral("/org/freedesktop/systemd1"),
                                              QStringLiteral("org.freedesktop.systemd1.Manager"),
                                              QStringLiteral("GetUnit"));
    msg.setArguments({QStringLiteral("openvpn-server@weldmaster.service")});
    auto reply = QDBusConnection::systemBus().asyncCall(msg);
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher{reply, this};
    connect(watcher, &QDBusPendingCallWatcher::finished, this,
        [this, watcher]
        {
            QDBusPendingReply<QDBusObjectPath> reply = *watcher;
            if (reply.isError())
            {
                NotificationSystem::instance()->warning(tr("%1 : %2").arg(reply.error().name()).arg(reply.error().message()));
                setActiveState(tr("unit not found"));
                setSubState(tr("unit not found"));
            } else
            {
                queryState(reply.argumentAt<0>());
            }
            watcher->deleteLater();
        }
    );
}

void OpenVpnController::queryState(const QDBusObjectPath &unit)
{
    auto msg1 = QDBusMessage::createMethodCall(s_systemdService,
                                              unit.path(),
                                              s_propertiesInterface,
                                              QStringLiteral("Get"));
    msg1.setArguments({s_unitInterface, QStringLiteral("ActiveState")});
    QDBusPendingCallWatcher *watcher1 = new QDBusPendingCallWatcher{QDBusConnection::systemBus().asyncCall(msg1), this};
    connect(watcher1, &QDBusPendingCallWatcher::finished, this,
        [this, watcher1]
        {
            QDBusPendingReply<QVariant> reply = *watcher1;
            if (reply.isError())
            {
                NotificationSystem::instance()->warning(tr("%1 : %2").arg(reply.error().name()).arg(reply.error().message()));
            }
            setActiveState(reply.isError() ? tr("unit not found") : reply.argumentAt<0>().toString());
            watcher1->deleteLater();
        }
    );

    auto msg2 = QDBusMessage::createMethodCall(s_systemdService,
                                              unit.path(),
                                              s_propertiesInterface,
                                              QStringLiteral("Get"));
    msg2.setArguments({s_unitInterface, QStringLiteral("SubState")});
    QDBusPendingCallWatcher *watcher2 = new QDBusPendingCallWatcher{QDBusConnection::systemBus().asyncCall(msg2), this};
    connect(watcher2, &QDBusPendingCallWatcher::finished, this,
        [this, watcher2]
        {
            QDBusPendingReply<QVariant> reply = *watcher2;
            if (reply.isError())
            {
                NotificationSystem::instance()->warning(tr("%1 : %2").arg(reply.error().name()).arg(reply.error().message()));
            }
            setSubState(reply.isError() ? tr("unit not found") : reply.argumentAt<0>().toString());
            watcher2->deleteLater();
        }
    );
}

void OpenVpnController::setActiveState(const QString &state)
{
    if (m_activeState == state)
    {
        return;
    }
    m_activeState = state;
    emit activeStateChanged();
}

void OpenVpnController::setSubState(const QString &state)
{
    if (m_subState == state)
    {
        return;
    }
    m_subState = state;
    emit subStateChanged();
}

void OpenVpnController::import(const QString& path)
{
    static const std::vector<QString> s_fileNames{{
        QStringLiteral("ca.crt"),
        QStringLiteral("weldmaster.crt"),
        QStringLiteral("weldmaster.key"),
        QStringLiteral("dh.pem")
    }};
    bool failed = false;
    QDir sourceDir{path};
    QDir targetDir{openVpnConfigDir()};
    for (const auto &fileName : s_fileNames)
    {
        const QString targetFileName = targetDir.absoluteFilePath(fileName);
        if (QFile::exists(targetFileName))
        {
            QFile::remove(targetFileName);
        }
        if (!QFile::copy(sourceDir.absoluteFilePath(fileName), targetDir.absoluteFilePath(fileName)))
        {
            NotificationSystem::instance()->warning(tr("Failed to import %1").arg(fileName));
            failed = true;
        }
    }
    if (!failed)
    {
        NotificationSystem::instance()->information(tr("Imported all required openVPN configuration files. System restart required"));
        emit restartRequired();
    }
}

void OpenVpnController::downloadClientConfig(const QString& targetPath)
{
    if (QFile::copy(exampleClientConfigPath(), QDir{targetPath}.absoluteFilePath(s_exampleConfigName)))
    {
        NotificationSystem::instance()->information(tr("Downloaded example client configuartion as %1 to attached device").arg(s_exampleConfigName));
    } else
    {
        NotificationSystem::instance()->warning(tr("Failed to download example client configuration"));
    }
}

QString OpenVpnController::clientConfig() const
{
    QFile file{exampleClientConfigPath()};
    if (file.open(QIODevice::ReadOnly))
    {
        return QString::fromUtf8(file.readAll());
    }
    return {};
}

QString OpenVpnController::exampleClientConfigPath() const
{
    return QDir{m_exampleConfigDir}.absoluteFilePath(s_exampleConfigName);
}

void OpenVpnController::setExampleConfigDir(const QString& path)
{
    if (m_exampleConfigDir == path)
    {
        return;
    }
    m_exampleConfigDir = path;
    emit exampleConfigDirChanged();
}

void OpenVpnController::setOpenVpnConfigDir(const QString& path)
{
    if (m_openVpnConfigDir == path)
    {
        return;
    }
    m_openVpnConfigDir = path;
    emit openVpnConfigDirChanged();
}

}
}
