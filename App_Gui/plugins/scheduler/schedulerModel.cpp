#include "schedulerModel.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QFile>
#include <QSaveFile>

#include "module/moduleLogger.h"

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

SchedulerModel::SchedulerModel(QObject* parent)
    : QAbstractListModel(parent)
{
    connect(this, &SchedulerModel::configFileChanged, this, &SchedulerModel::loadFromFile);
    connect(this, &SchedulerModel::dataChanged, this, &SchedulerModel::save);
    connect(this, &SchedulerModel::rowsInserted, this, &SchedulerModel::save);
    connect(this, &SchedulerModel::rowsRemoved, this, &SchedulerModel::save);
}

SchedulerModel::~SchedulerModel() = default;

QHash<int, QByteArray> SchedulerModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("parameters")},
        {Qt::UserRole, QByteArrayLiteral("triggerName")},
        {Qt::UserRole + 1, QByteArrayLiteral("taskName")},
        {Qt::UserRole + 2, QByteArrayLiteral("triggerSettings")},
        {Qt::UserRole + 3, QByteArrayLiteral("taskSettings")},
        {Qt::UserRole + 4, QByteArrayLiteral("triggerEvent")},
        {Qt::UserRole + 5, QByteArrayLiteral("hasTriggerEvent")},
    };
}


int SchedulerModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_data.size();
}

QVariant SchedulerModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &element = m_data.at(index.row());

    switch (role)
    {
    case Qt::DisplayRole:
        return displayText(element);
    case Qt::UserRole:
        return element.triggerName;
    case Qt::UserRole + 1:
        return element.taskName;
    case Qt::UserRole + 2:
        return element.triggerSettings;
    case Qt::UserRole + 3:
        return element.taskSettings;
    case Qt::UserRole + 4:
        if (element.triggerEvent.has_value())
        {
            return element.triggerEvent.value();
        }
    case Qt::UserRole + 5:
        return element.triggerEvent.has_value();
    }

    return {};
}
void SchedulerModel::setConfigFile(const QString& file)
{
    if (m_configFile == file)
    {
        return;
    }
    m_configFile = file;
    emit configFileChanged();
}


void SchedulerModel::loadFromFile()
{
    QFile file{m_configFile};
    if (file.open(QIODevice::ReadOnly))
    {
        loadFromJson(QJsonDocument::fromJson(file.readAll()));
    }
    else
    {
        beginResetModel();
        m_data.clear();
        endResetModel();
    }
}

void SchedulerModel::loadFromJson(const QJsonDocument& json)
{
    beginResetModel();
    m_data.clear();
    const auto &array = json.array();
    m_data.reserve(array.size());
    for (const auto &value : array)
    {
        if (!value.isObject())
        {
            continue;
        }
        const auto &object = value.toObject();
        const auto uuidIt = object.find(QStringLiteral("Uuid"));
        const auto taskIt = object.find(QStringLiteral("Task"));
        const auto triggerIt = object.find(QStringLiteral("Trigger"));

        if (uuidIt == object.end() || taskIt == object.end() || triggerIt == object.end())
        {
            continue;
        }

        Data data;
        data.uuid = QUuid{uuidIt.value().toString()};
        const auto &taskObject = taskIt.value().toObject();
        const auto &triggerObject = triggerIt.value().toObject();

        const auto taskNameIt = taskObject.find(QStringLiteral("Name"));
        const auto triggerNameIt = triggerObject.find(QStringLiteral("Name"));

        if (taskNameIt == taskObject.end() || triggerNameIt == triggerObject.end())
        {
            continue;
        }
        data.triggerName = triggerNameIt.value().toString();
        data.taskName = taskNameIt.value().toString();
        if (const auto settingsIt = taskObject.find(QStringLiteral("Settings")); settingsIt != taskObject.end())
        {
            data.taskSettings = settingsIt.value().toObject().toVariantMap();
        }
        if (const auto settingsIt = triggerObject.find(QStringLiteral("Settings")); settingsIt != triggerObject.end())
        {
            const auto &object = settingsIt.value().toObject();
            data.triggerSettings = object.toVariantMap();
            if (const auto eventIt = object.find(QStringLiteral("event")); eventIt != object.end())
            {
                data.triggerEvent = eventIt.value().toString().toInt();
            }
        }
        m_data.emplace_back(std::move(data));
    }
    endResetModel();
}

QString SchedulerModel::displayText(const Data &data) const
{
    QString ret;
    if (data.triggerName == QStringLiteral("CronTrigger"))
    {
        if (auto it = data.triggerSettings.find(QStringLiteral("cron")); it != data.triggerSettings.end())
        {
            ret.append(it->toString());
        }
    }

    auto addSeparator = [&ret]
    {
        if (!ret.isEmpty())
        {
            ret.append(QStringLiteral("; "));
        }
    };

    if (data.taskName == QStringLiteral("BackupToDirectoryTask"))
    {
        if (auto it = data.taskSettings.find(QStringLiteral("backupPath")); it != data.taskSettings.end())
        {
            addSeparator();
            ret.append(it->toString());
        }
    }
    if (data.taskName == QStringLiteral("DeleteOldBackupsTask"))
    {
        if (auto it = data.taskSettings.find(QStringLiteral("backupPath")); it != data.taskSettings.end())
        {
            addSeparator();
            ret.append(it->toString());
        }
        if (auto it = data.taskSettings.find(QStringLiteral("TimeToLiveDays")); it != data.taskSettings.end())
        {
            addSeparator();
            ret.append(tr("Delete backups older than %1 days").arg(it->toString()));
        }
    }
    if (data.taskName == QStringLiteral("TransferDirectoryTask") ||
        data.taskName == QStringLiteral("BackupToRemoteTask") ||
        data.taskName == QStringLiteral("TransferFileTask") ||
        data.taskName == QStringLiteral("ResultExcelFileFromProductInstanceTask") ||
        data.taskName == QStringLiteral("TransferMetaDataTask") ||
        data.taskName == QStringLiteral("ExportProductTask"))
    {
        addSeparator();
        const auto protocol{data.taskSettings.value(QStringLiteral("Protocol"), QStringLiteral("sftp")).toString()};
        const auto userName{data.taskSettings.value(QStringLiteral("TargetUserName")).toString()};
        const auto ipAddress{data.taskSettings.value(QStringLiteral("TargetIpAddress")).toString()};
        const auto port{data.taskSettings.value(QStringLiteral("Port")).toString()};
        const auto path{data.taskSettings.value(QStringLiteral("TargetDirectoryPath")).toString()};

        ret.append(protocol);
        ret.append(QLatin1String{"://"});
        if (!userName.isEmpty())
        {
            ret.append(userName);
            ret.append(QLatin1String("@"));
        }
        ret.append(ipAddress);
        if (!port.isEmpty())
        {
            ret.append(QLatin1String(":"));
            ret.append(port);
        }
        ret.append(path);
        if (auto it = data.taskSettings.find(QStringLiteral("TargetFileName")); it != data.taskSettings.end())
        {
            ret.append(it->toString());
        }
    }
    return ret;
}

void SchedulerModel::edit(int row, const QVariantMap &triggerSettings, const QVariantMap &taskSettings)
{
    const auto index = this->index(row, 0);
    if (!index.isValid())
    {
        return;
    }
    m_data.at(row).triggerSettings = triggerSettings;
    m_data.at(row).taskSettings = taskSettings;
    emit dataChanged(index, index);
}

void SchedulerModel::remove(int row)
{
    const auto index = this->index(row, 0);
    if (!index.isValid())
    {
        return;
    }
    beginRemoveRows({}, row, row);
    auto it = m_data.begin();
    std::advance(it, row);
    m_data.erase(it);
    endRemoveRows();
}

void SchedulerModel::add(const QString &trigger, const QVariantMap &triggerSettings, const QVariant &triggerEvent, const QString &task, const QVariantMap &taskSettings)
{
    beginInsertRows({}, m_data.size(), m_data.size());
    Data data;
    data.uuid = QUuid::createUuid();
    data.taskName = task;
    data.taskSettings = taskSettings;
    data.triggerName = trigger;
    data.triggerSettings = triggerSettings;
    if (triggerEvent.isValid())
    {
        data.triggerEvent = triggerEvent.toInt();
    }
    m_data.emplace_back(std::move(data));
    endInsertRows();
}

void SchedulerModel::save()
{
    QJsonArray array{};
    for (const auto &element : m_data)
    {
        auto triggerObject = QJsonObject::fromVariantMap(element.triggerSettings);
        if (element.triggerEvent.has_value())
        {
            triggerObject.insert(QStringLiteral("event"), QString::number(element.triggerEvent.value()));
        }
        QJsonObject object{{
            {QStringLiteral("Uuid"), element.uuid.toString(QUuid::WithoutBraces)},
            {QStringLiteral("Task"), QJsonObject{{
                {QStringLiteral("Name"), element.taskName},
                {QStringLiteral("Settings"), QJsonObject::fromVariantMap(element.taskSettings)},
            }}},
            {QStringLiteral("Trigger"), QJsonObject{{
                {QStringLiteral("Name"), element.triggerName},
                {QStringLiteral("Settings"), triggerObject},
            }}},
        }};
        array.push_back(object);
    }
    QFile file{m_configFile};
    if (!file.open(QIODevice::WriteOnly))
    {
        return;
    }
    file.write(QJsonDocument{array}.toJson());
    file.close();
}


}
}
}
}
