#include "availableTasksModel.h"

#include <QUrl>

namespace precitec
{
namespace gui
{
namespace components
{
namespace scheduler
{

static const std::vector<std::tuple<QString, std::string, QUrl>> s_tasks{
    {QStringLiteral("BackupToDirectoryTask"), std::string{QT_TR_NOOP("Backup to local directory")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/BackupLocalTaskConfiguration.qml")}},
    {QStringLiteral("BackupToRemoteTask"), std::string{QT_TR_NOOP("Backup to remote directory")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/BackupRemoteTaskConfiguration.qml")}},
    {QStringLiteral("DeleteOldBackupsTask"), std::string{QT_TR_NOOP("Delete old backups from local directory")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/DeleteBackupsTaskConfiguration.qml")}},
    {QStringLiteral("ExportProductTask"), std::string{QT_TR_NOOP("Export separated product to remote directory")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/ExportProductTaskConfiguration.qml")}},
    {QStringLiteral("TransferDirectoryTask"), std::string{QT_TR_NOOP("Transfer directory to remote location")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/TransferDirectoryTaskConfiguration.qml")}},
    {QStringLiteral("TransferFileTask"), std::string{QT_TR_NOOP("Transfer file to remote location")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/TransferFileTaskConfiguration.qml")}},
    {QStringLiteral("ResultExcelFileFromProductInstanceTask"), std::string{QT_TR_NOOP("Generate result XLSX file and transfer to remote location")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/ResultExcelConfiguration.qml")}},
    {QStringLiteral("TransferMetaDataTask"), std::string{QT_TR_NOOP("Transfer result metadata json to remote location")}, QUrl{QStringLiteral("qrc:///precitec/scheduler/TransferMetaDataTaskConfiguration.qml")}},
};

AvailableTasksModel::AvailableTasksModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AvailableTasksModel::~AvailableTasksModel() = default;

QHash<int, QByteArray> AvailableTasksModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("identifier")},
        {Qt::UserRole + 1, QByteArrayLiteral("configuration")},
    };
}


int AvailableTasksModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return s_tasks.size();
}

QVariant AvailableTasksModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    const auto &element = s_tasks.at(index.row());
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(std::get<1>(element));
    case Qt::UserRole:
        return std::get<0>(element);
    case Qt::UserRole + 1:
        return std::get<2>(element);
    }

    return {};
}

QModelIndex AvailableTasksModel::indexForIdentifier(const QString &identifier) const
{
    if (auto it = std::find_if(s_tasks.begin(), s_tasks.end(), [identifier] (const auto &tuple) { return std::get<0>(tuple) == identifier; }); it != s_tasks.end())
    {
        return index(std::distance(std::begin(s_tasks), it), 0);
    }
    return {};
}

}
}
}
}
