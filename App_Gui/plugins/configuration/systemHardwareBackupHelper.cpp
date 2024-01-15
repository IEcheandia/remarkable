#include "systemHardwareBackupHelper.h"
#include "etherCATConfigurationController.h"
#include "upsModel.h"

#include <precitec/connectionsModel.h>
#include <precitec/ipv4SettingModel.h>

#include <QDir>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QSaveFile>

namespace precitec
{
namespace gui
{

static const QString s_backupFile{QStringLiteral("systemHardwareConfiguration.json")};

using components::network::ConnectionsModel;
using components::network::Ipv4SettingModel;

SystemHardwareBackupHelper::SystemHardwareBackupHelper(QObject *parent)
    : QObject(parent)
    , m_etherCat(new EtherCATConfigurationController{this})
    , m_ups(new UpsModel{this})
    , m_networkConnections(new ConnectionsModel{this})
{
}

SystemHardwareBackupHelper::~SystemHardwareBackupHelper() = default;

QString SystemHardwareBackupHelper::backupFile() const
{
    return s_backupFile;
}

void SystemHardwareBackupHelper::backup()
{
    QSaveFile file{QDir{m_directory}.filePath(s_backupFile)};
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }

    Ipv4SettingModel ipv4;
    QJsonArray networkConfigurations;
    for (int i = 0; i < m_networkConnections->rowCount({}); i++)
    {
        const auto index = m_networkConnections->index(i, 0);
        if (!index.data(Qt::UserRole).toBool())
        {
            continue;
        }
        const auto uuid = index.data(Qt::UserRole + 2).toString();
        ipv4.setConnection(uuid);
        QJsonArray ipAddresses;
        for (int j = 0; j < ipv4.rowCount({}); j++)
        {
            const auto ipIndex = ipv4.index(j, 0);
            ipAddresses.push_back(QJsonObject{{
                {QStringLiteral("address"), ipIndex.data().toString()},
                {QStringLiteral("netmask"), ipIndex.data(Qt::UserRole + 1).toString()},
                {QStringLiteral("gateway"), ipIndex.data(Qt::UserRole + 2).toString()}
            }});
        }
        auto methodToString = [] (Ipv4SettingModel::Method method) -> QString
        {
            switch (method)
            {
            case Ipv4SettingModel::Method::Automatic:
                return QStringLiteral("auto");
            case Ipv4SettingModel::Method::LinkLocal:
                return QStringLiteral("link-local");
            case Ipv4SettingModel::Method::Manual:
                return QStringLiteral("manual");
            case Ipv4SettingModel::Method::Shared:
                return QStringLiteral("shared");
            case Ipv4SettingModel::Method::Disabled:
                return QStringLiteral("disabled");
            default:
                __builtin_unreachable();
            }
        };
        networkConfigurations.push_back(QJsonObject{{
            {QStringLiteral("uuid"), uuid},
            {QStringLiteral("mac"), index.data(Qt::UserRole + 4).toString()},
            {QStringLiteral("ipv4"), ipAddresses},
            {QStringLiteral("method"), methodToString(ipv4.method())}
        }});
    }

    const auto upsIndex = m_ups->selectedIndex();

    QJsonDocument json{QJsonObject{{
        {QStringLiteral("ups"), QJsonObject{{
            {QStringLiteral("mode"), upsIndex.data(Qt::UserRole).toInt()},
            {QStringLiteral("driver"), upsIndex.data(Qt::UserRole+1).toInt()}
        }}},
        {QStringLiteral("etherCAT"), QJsonObject{{
            {QStringLiteral("mac"), QString::fromUtf8(m_etherCat->macAddress())},
            {QStringLiteral("enabled"), m_etherCat->isEnabled()}
        }}},
        {QStringLiteral("connections"), networkConfigurations}
    }}};
    file.write(json.toJson());
    file.commit();
}

void SystemHardwareBackupHelper::setDirectory(const QString &directory)
{
    if (m_directory == directory)
    {
        return;
    }
    m_directory = directory;
    emit directoryChanged();
}

bool SystemHardwareBackupHelper::load()
{
    QFile file{QDir{m_directory}.filePath(s_backupFile)};
    if (!file.exists())
    {
        return false;
    }
    if (!file.open(QIODevice::ReadOnly))
    {
        return false;
    }
    const auto json = QJsonDocument::fromJson(file.readAll()).object();
    if (json.empty())
    {
        return false;
    }
    loadUps(json);
    loadEtherCAT(json);
    loadConnections(json);

    return true;
}

void SystemHardwareBackupHelper::loadUps(const QJsonObject &object)
{
    auto it = object.find(QLatin1String("ups"));
    if (it == object.end())
    {
        return;
    }
    const auto ups = it->toObject();
    auto mode = ups.find(QLatin1String("mode"));
    auto driver = ups.find(QLatin1String("driver"));
    if (mode == ups.end() || driver == ups.end())
    {
        return;
    }
    m_ups->selectByModeAndDriver(mode->toInt(), driver->toInt());
}

void SystemHardwareBackupHelper::loadEtherCAT(const QJsonObject& object)
{
    auto it = object.find(QLatin1String("etherCAT"));
    if (it == object.end())
    {
        return;
    }
    const auto etherCAT = it->toObject();
    auto mac = etherCAT.find(QLatin1String("mac"));
    if (mac == etherCAT.end())
    {
        return;
    }
    m_etherCATMacAddress = mac->toString();
    const auto enabled = etherCAT.find(QLatin1String("enabled"));
    if (enabled != etherCAT.end())
    {
        m_etherCATEnabled = enabled->toBool();
        emit etherCATEnabledChanged();
    }
    emit etherCATMacAddressChanged();
}

void SystemHardwareBackupHelper::loadConnections(const QJsonObject& object)
{
    auto it = object.find(QLatin1String("connections"));
    if (it == object.end())
    {
        return;
    }
    const auto connections = it->toArray();
    for (const auto &connection : connections)
    {
        const auto con = connection.toObject();
        Connection c;
        c.uuid = con.value(QLatin1String("uuid")).toString();
        c.macAddress = con.value(QLatin1String("mac")).toString();

        auto ipModel = new Ipv4SettingModel{this};
        ipModel->setConnection(c.uuid);
        while (ipModel->rowCount({}) > 0)
        {
            ipModel->remove(0);
        }

        QString methodString = con.value(QLatin1String("method")).toString();
        if (methodString.isEmpty())
        {
            methodString = QStringLiteral("manual");
        }
        Ipv4SettingModel::Method method = Ipv4SettingModel::Method::Manual;
        if (methodString.compare(QLatin1String("auto")) == 0)
        {
            method = Ipv4SettingModel::Method::Automatic;
        } else if (methodString.compare(QLatin1String("link-local")) == 0)
        {
            method = Ipv4SettingModel::Method::LinkLocal;
        } else if (methodString.compare(QLatin1String("manual")) == 0)
        {
            method = Ipv4SettingModel::Method::Manual;
        } else if (methodString.compare(QLatin1String("shared")) == 0)
        {
            method = Ipv4SettingModel::Method::Shared;
        } else
        {
            method = Ipv4SettingModel::Method::Disabled;
        }
        ipModel->setMethod(method);

        const auto ips = con.value(QLatin1String("ipv4")).toArray();
        for (auto ipValue : ips)
        {
            const auto ip = ipValue.toObject();
            QString gateway = ip.value(QLatin1String("gateway")).toString();
            if (gateway.isNull())
            {
                gateway = QStringLiteral("0.0.0.0");
            }
            ipModel->add(ip.value(QLatin1String("address")).toString(), ip.value(QLatin1String("netmask")).toString(), gateway);
        }
        c.ips = ipModel;

        m_connections.push_back(c);
    }
    emit connectionsChanged();
}

QStringList SystemHardwareBackupHelper::connectionUUIDS() const
{
    QStringList ret;
    for (const auto &connection : m_connections)
    {
        if (m_networkConnections->indexForUuid(connection.uuid).isValid())
        {
            ret << connection.uuid;
        }
    }

    return ret;
}

QString SystemHardwareBackupHelper::nameForConnection(const QString &uuid) const
{
    return m_networkConnections->indexForUuid(uuid).data().toString();
}

QString SystemHardwareBackupHelper::macAddressForConnection(const QString &uuid) const
{
    for (const auto &connection : m_connections)
    {
        if (connection.uuid == uuid)
        {
            return connection.macAddress;
        }
    }
    return {};
}

gui::components::network::Ipv4SettingModel *SystemHardwareBackupHelper::ipForConnection(const QString &uuid) const
{
    for (const auto &connection : m_connections)
    {
        if (connection.uuid == uuid)
        {
            return connection.ips;
        }
    }
    return nullptr;
}

}
}
