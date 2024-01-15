#include "DirectoryModel.h"
#include "graphModel.h"
#include "subGraphModel.h"

using namespace precitec::gui::components::grapheditor;

DirectoryModel::DirectoryModel(QObject* parent) : QAbstractListModel(parent)
{ }

DirectoryModel::~DirectoryModel() = default;

DirectoryLoader* DirectoryModel::directoryLoader() const
{
    return m_directoryLoader;
}

void DirectoryModel::setDirectoryLoader(DirectoryLoader* newLoader)
{
    if (m_directoryLoader == newLoader)
    {
        return;
    }

    if (m_directoryLoader)
    {
        disconnect(m_directoryLoader, &DirectoryLoader::directoriesChanged, this, &DirectoryModel::init);
        disconnect(m_destroyedConnection);
    }

    m_directoryLoader = newLoader;

    if (m_directoryLoader)
    {
        m_destroyedConnection = connect(m_directoryLoader, &QObject::destroyed, this, std::bind(&DirectoryModel::setDirectoryLoader, this, nullptr));
        connect(m_directoryLoader, &DirectoryLoader::directoriesChanged, this, &DirectoryModel::init);
    }
    else
    {
        m_destroyedConnection = {};
        m_directories.clear();
    }

    emit directoryLoaderChanged();
}

int DirectoryModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_directories.size();
}

QHash<int, QByteArray> DirectoryModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("path")},
        {Qt::UserRole+1, QByteArrayLiteral("isGraph")},
        {Qt::UserRole+2, QByteArrayLiteral("macro")},
        {Qt::UserRole+3, QByteArrayLiteral("graphModel")},
    };
}

QVariant DirectoryModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &directoryEntry = m_directories.at(index.row());

    switch (role)
    {
        case Qt::DisplayRole:
            return std::get<0>(directoryEntry);
        case Qt::UserRole:
            return std::get<1>(directoryEntry);
        case Qt::UserRole+1:
            return std::get<2>(directoryEntry) == DirectoryType::Graph;
        case Qt::UserRole+2:
            return std::get<2>(directoryEntry) == DirectoryType::Macro;
        case Qt::UserRole+3:
            return QVariant::fromValue(std::get<4>(directoryEntry));
        case Qt::UserRole+4:
            return QVariant::fromValue(std::get<3>(directoryEntry));
    }

    return {};
}

bool DirectoryModel::fileNameExists(const QString &fileName, DirectoryType currentDirType) const
{
    const auto directories{m_directoryLoader->getDirectories()};

    for (const auto& [name, path, type, ownership] : directories)
    {
        if(type == currentDirType && ownership == DirectoryOwnership::User){
            QDir dir(path);
            QString url = fileName + ".xml";
            if (dir.exists(url)){
                return true;
            }
        }
    }

    return false;
}

DirectoryType DirectoryModel::getDirectoryType(const QModelIndex &index)
{
    if (!index.isValid())
    {
        return {};
    }

    const auto &directoryEntry = m_directories.at(index.row());

    return std::get<2>(directoryEntry);
}

QString DirectoryModel::nameForDirectory(const QString &currentDirPath) const
{
    if (currentDirPath.isEmpty())
    {
        return QLatin1String("");
    }
    const QFileInfo directory{currentDirPath};
    if (!directory.exists() || !directory.isDir())
    {
        return {};
    }
    auto it = std::find_if(m_directories.begin(), m_directories.end(),
                           [&directory] (const auto &tuple)
                           {
                               return directory == QFileInfo{std::get<1>(tuple)};
                           }
    );
    if (it == m_directories.end())
    {
        return {};
    }
    return std::get<0>(*it);
}

QString DirectoryModel::checkGraphFromSystemFolder ( const QString& currentDirPath) const
{
    const auto &currentDirName = nameForDirectory(currentDirPath);
    if (currentDirName.isEmpty())
    {
        return {};
    }

    if (QString::compare(currentDirName, "System graphs") == 0)
    {
        return data(dirPathByName(QStringLiteral("Custom graphs")), Qt::UserRole).toString();
    }

    if (QString::compare(currentDirName, "Subgraphs Input") == 0 || QString::compare(currentDirName, "Subgraphs ROI") == 0 || QString::compare(currentDirName, "Subgraphs Pre-Process") == 0 || QString::compare(currentDirName, "Subgraphs Process") == 0 || QString::compare(currentDirName, "Subgraphs Post-Process") == 0 || QString::compare(currentDirName, "Subgraphs Output") == 0)
    {
        return data(dirPathByName(QStringLiteral("Custom Subgraphs")), Qt::UserRole).toString();
    }

    if (QString::compare(currentDirName, QLatin1String("System Macros")) == 0)
    {
        return data(dirPathByName(QStringLiteral("Custom Macros")), Qt::UserRole).toString();
    }

    return QLatin1String("");
}

void DirectoryModel::init()
{
    if (!m_directoryLoader)
    {
        return;
    }

    emit beginResetModel();
    const auto directories{m_directoryLoader->getDirectories()};
    m_directories.clear();
    m_directories.reserve(directories.size());
    for (const auto& [name, path, type, ownership] : directories)
    {
        storage::AbstractGraphModel* abstractGraphModel{nullptr};
        if (type == DirectoryType::Graph || type == DirectoryType::Macro)
        {
            auto* model{new storage::GraphModel{this}};
            model->loadGraphs(path);
            abstractGraphModel = model;
        }
        else
        {
            auto* model{new storage::SubGraphModel{this}};
            if (ownership == DirectoryOwnership::System)
            {
                model->loadSubGraphs(path);
            }
            else
            {
                model->loadSubGraphs({}, path);
            }
            abstractGraphModel = model;
        }
        m_directories.emplace_back(std::make_tuple(name, path, type, ownership, abstractGraphModel));
    }
    emit endResetModel();
}

QModelIndex DirectoryModel::dirPathByName (const QString& dirName) const
{
    if (dirName.isEmpty())
    {
        return {};
    }

    auto it = std::find_if(m_directories.begin(), m_directories.end(), [dirName](const auto &currentDir){return QString::compare(std::get<0>(currentDir), dirName) == 0; });
    if (it == m_directories.end())
    {
        return {};
    }

    return index(std::distance(m_directories.begin(), it));
}

QModelIndex DirectoryModel::indexForUser(const QModelIndex& systemIndex) const
{
    if (!systemIndex.isValid())
    {
        return {};
    }
    const auto& entry{m_directories.at(systemIndex.row())};
    if (std::get<DirectoryOwnership>(entry) == DirectoryOwnership::User)
    {
        return systemIndex;
    }
    return indexForUser(std::get<DirectoryType>(entry));
}

QModelIndex DirectoryModel::indexForUser(DirectoryType type) const
{
    const auto it = std::find_if(m_directories.begin(), m_directories.end(), [type] (const auto& current) { return std::get<DirectoryOwnership>(current) == DirectoryOwnership::User && std::get<DirectoryType>(current) == type; });
    if (it == m_directories.end())
    {
        return {};
    }
    return index(std::distance(m_directories.begin(), it));
}

