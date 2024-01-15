#pragma once

#include <QObject>

class QDBusObjectPath;

namespace precitec
{
namespace gui
{

/**
 * Controller for OpenVPN configuration.
 **/
class OpenVpnController : public QObject
{
    Q_OBJECT
    /**
     * Active state of the systemd unit of the OpenVPN server.
     **/
    Q_PROPERTY(QString activeState READ activeState NOTIFY activeStateChanged)
    /**
     * Sub state of the systemd unit of the OpenVPN server.
     **/
    Q_PROPERTY(QString subState READ subState NOTIFY subStateChanged)
    /**
     * Directory containing certificates
     **/
    Q_PROPERTY(QString openVpnConfigDir READ openVpnConfigDir WRITE setOpenVpnConfigDir NOTIFY openVpnConfigDirChanged)
    /**
     * Directory containing example configuration
     **/
    Q_PROPERTY(QString exampleConfigDir READ exampleConfigDir WRITE setExampleConfigDir NOTIFY exampleConfigDirChanged)
public:
    OpenVpnController(QObject *parent = nullptr);
    ~OpenVpnController() override;

    QString activeState() const
    {
        return m_activeState;
    }
    QString subState() const
    {
        return m_subState;
    }

    QString openVpnConfigDir() const
    {
        return m_openVpnConfigDir;
    }

    QString exampleConfigDir() const
    {
        return m_exampleConfigDir;
    }

    void setOpenVpnConfigDir(const QString &path);
    void setExampleConfigDir(const QString &path);

    /**
     * Imports required certificates from the provided @p path
     **/
    Q_INVOKABLE void import(const QString &path);

    /**
     * Downloads the example client configuration to @p targetPath
     **/
    Q_INVOKABLE void downloadClientConfig(const QString &targetPath);

    /**
     * Content of the example client configuration.
     **/
    Q_INVOKABLE QString clientConfig() const;

Q_SIGNALS:
    void activeStateChanged();
    void subStateChanged();
    void openVpnConfigDirChanged();
    void exampleConfigDirChanged();
    void restartRequired();

private:
    void findService();
    void queryState(const QDBusObjectPath &unit);
    void setActiveState(const QString &state);
    void setSubState(const QString &state);
    QString exampleClientConfigPath() const;

    QString m_activeState = tr("init");
    QString m_subState = tr("init");
    QString m_openVpnConfigDir;
    QString m_exampleConfigDir;
};

}
}
