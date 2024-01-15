#include "graphModel.h"

#include "../App_Storage/src/compatibility.h"

#include <QDir>
#include <QFileSystemWatcher>
#include <QtConcurrent>

namespace precitec
{
namespace storage
{

GraphModel::GraphModel(QObject *parent)
    : AbstractGraphModel(parent)
    , m_watcher(new QFileSystemWatcher{this})
{
    connect(m_watcher, &QFileSystemWatcher::directoryChanged, this,
        [this]
        {
            loadGraphs(m_systemDir, m_userDir);
        }
    );
}

GraphModel::~GraphModel() = default;

QVariant GraphModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant{};
    }
    const auto &graph = this->graph(index);
    switch (role)
    {
    case Qt::DisplayRole:
        return QString::fromStdString(graph.name);
    case Qt::UserRole:
        return compatibility::toQt(graph.id);
    case Qt::UserRole+1:
    {
        QStringList ret;
        for (auto &group : graph.filterGroups)
        {
            ret << QString::fromStdString(group.name);
        }
        return ret;
    }
    case Qt::UserRole + 5:
        return graphImage(index);
    case Qt::UserRole + 6:
        return QString::fromStdString(graph.comment);
    case Qt::UserRole + 7:
        return pdfFiles().contains(QString::fromStdString(graph.name));
    case Qt::UserRole + 8:
        return QString::fromStdString(graph.group);
    }

    return QVariant{};
}

QHash<int, QByteArray> GraphModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("uuid")},
        {Qt::UserRole+1, QByteArrayLiteral("groups")},
        {Qt::UserRole+5, QByteArrayLiteral("image")},
        {Qt::UserRole+6, QByteArrayLiteral("comment")},
        {Qt::UserRole+7, QByteArrayLiteral("pdfAvailable")},
        {Qt::UserRole+8, QByteArrayLiteral("groupName")},
    };
}

void GraphModel::loadGraphs(const std::vector<std::pair<QDir, StorageType>> &graphStorageDirectories)
{
    if (isLoading())
    {
        return;
    }

    if (!m_watcher->directories().empty())
    {
        m_watcher->removePaths(m_watcher->directories());
    }
    QStringList directories;
    std::transform(graphStorageDirectories.begin(), graphStorageDirectories.end(), std::back_inserter(directories), [] (const auto &dir) { return std::get<0>(dir).absolutePath(); });
    m_watcher->addPaths(directories);

    std::vector<std::pair<QFileInfo, StorageType>> files;
    for (const auto &dir: graphStorageDirectories)
    {
        const auto graphFiles = std::get<0>(dir).entryInfoList(QStringList{QStringLiteral("*.xml")}, QDir::Files | QDir::Readable, QDir::Name);
        files.reserve(files.size() + graphFiles.size());
        std::transform(graphFiles.begin(), graphFiles.end(), std::back_inserter(files), [dir] (const auto &file) { return std::make_pair(file, std::get<1>(dir)); });
    }
    if (!checkSymmetricDifference(files))
    {
        return;
    }

    loadGraphsFromFiles(files);
}

void GraphModel::loadGraphs(const QString &systemGraphStorage, const QString &userGraphStorage)
{
    m_systemDir = systemGraphStorage;
    m_userDir = userGraphStorage;
    std::vector<std::pair<QDir, StorageType>> dirs{std::make_pair(QDir{systemGraphStorage}, StorageType::System)};
    if (!userGraphStorage.isNull())
    {
        dirs.emplace_back(std::make_pair(userGraphStorage, StorageType::User));
    }
    loadGraphs(dirs);
}

}
}
