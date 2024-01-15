#include "abstractVideoImportModel.h"

#include <QDir>

namespace precitec
{
namespace gui
{

AbstractVideoImportModel::AbstractVideoImportModel(QObject *parent)
    : QAbstractListModel(parent)
{
}

AbstractVideoImportModel::~AbstractVideoImportModel() = default;

QVariant AbstractVideoImportModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    switch (role)
    {
    case Qt::DisplayRole:
        return m_subDirectories.at(index.row()).fileName();
    case Qt::UserRole:
        return QVariant::fromValue(m_subDirectories.at(index.row()));
    case Qt::UserRole + 1:
        return m_subDirectories.at(index.row()).absoluteFilePath();
    }
    return {};
}

QHash<int, QByteArray> AbstractVideoImportModel::roleNames() const
{
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("fileInfo")},
        {Qt::UserRole + 1, QByteArrayLiteral("path")}
    };
}

int AbstractVideoImportModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return m_subDirectories.size();
}

void AbstractVideoImportModel::init(const QFileInfo &path)
{
    beginResetModel();
    m_subDirectories.clear();

    if (path.exists() && path.isDir())
    {
        m_subDirectories = QDir{path.absoluteFilePath()}.entryInfoList(QDir::AllDirs | QDir::NoDotAndDotDot);
    }
    endResetModel();
}

}
}
