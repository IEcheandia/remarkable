#pragma once

#include <QObject>
#include <QHostAddress>

#include <vector>

class QJsonObject;

namespace precitec
{
namespace gui
{

class EtherCATConfigurationController;
class UpsModel;

namespace components
{
namespace network
{
class Ipv4SettingModel;
class ConnectionsModel;
}
}

/**
 * Small helper to perform the hardware backup and restore.
 * Hardware backup includes
 * @li etherCAT configuration
 * @li UPS configuration
 * @li network configuration
 **/
class SystemHardwareBackupHelper : public QObject
{
    Q_OBJECT
    /**
     * Configuration directory where to create the hardware configuration file
     **/
    Q_PROPERTY(QString directory READ directory WRITE setDirectory NOTIFY directoryChanged)
    /**
     * Internally used UpsModel.
     **/
    Q_PROPERTY(precitec::gui::UpsModel *upsModel READ upsModel CONSTANT)
    /**
     * Mac address of etherCAT device. Set on load.
     **/
    Q_PROPERTY(QString etherCATMacAddress READ etherCATMacAddress NOTIFY etherCATMacAddressChanged)

    /**
     * Whether etherCAT is enabled. Set on load.
     * Default value in case the backup did not yet include the enabled state is @c true.
     **/
    Q_PROPERTY(bool etherCATEnabled READ isEtherCATEnabled NOTIFY etherCATEnabledChanged)

    /**
     * The uuids of the Connections in the backup file filtered by the connections existing on this system.
     **/
    Q_PROPERTY(QStringList connectionUUIDS READ connectionUUIDS NOTIFY connectionsChanged)
public:
    explicit SystemHardwareBackupHelper(QObject *parent = nullptr);
    ~SystemHardwareBackupHelper() override;

    /**
     * Creates a hardware configuration file.
     **/
    Q_INVOKABLE void backup();

    /**
     * Loads the hardware configuration backup file.
     **/
    Q_INVOKABLE bool load();

    QString directory() const
    {
        return m_directory;
    }
    void setDirectory(const QString &directory);

    UpsModel *upsModel() const
    {
        return m_ups;
    }

    QString etherCATMacAddress() const
    {
        return m_etherCATMacAddress;
    }

    QStringList connectionUUIDS() const;

    /**
     * The name of the network connection with @p uuid
     **/
    Q_INVOKABLE QString nameForConnection(const QString &uuid) const;

    /**
     * The macAddress of the network connection with @p uuid
     **/
    Q_INVOKABLE QString macAddressForConnection(const QString &uuid) const;

    /**
     * The ip settings model for the connection with @p uuid.
     **/
    Q_INVOKABLE precitec::gui::components::network::Ipv4SettingModel *ipForConnection(const QString &uuid) const;

    /**
     * The file name the SystemHardwareBackupHelper uses for the backup without path.
     **/
    QString backupFile() const;

    bool isEtherCATEnabled() const
    {
        return m_etherCATEnabled;
    }

Q_SIGNALS:
    void directoryChanged();
    void etherCATMacAddressChanged();
    void connectionsChanged();
    void etherCATEnabledChanged();

private:
    void loadUps(const QJsonObject &object);
    void loadEtherCAT(const QJsonObject &object);
    void loadConnections(const QJsonObject &object);
    QString m_directory;
    EtherCATConfigurationController *m_etherCat;
    UpsModel *m_ups;
    components::network::ConnectionsModel *m_networkConnections;
    QString m_etherCATMacAddress;
    bool m_etherCATEnabled = true;

    struct Connection
    {
        QString uuid;
        QString macAddress;
        components::network::Ipv4SettingModel *ips;
    };
    std::vector<Connection> m_connections;
};

}
}
