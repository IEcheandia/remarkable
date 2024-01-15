#include "basicFigureSelectionModel.h"

namespace precitec
{
namespace gui
{

BasicFigureSelectionModel::BasicFigureSelectionModel(QObject* parent)
    : QIdentityProxyModel(parent)
{
}

BasicFigureSelectionModel::~BasicFigureSelectionModel() = default;

QHash<int, QByteArray> BasicFigureSelectionModel::roleNames() const
{
    // see FileModel
    // new roles added starting at UserRole + 100
    return {
        {Qt::DisplayRole, QByteArrayLiteral("name")},
        {Qt::UserRole, QByteArrayLiteral("path")},
        {Qt::UserRole + 1, QByteArrayLiteral("type")},
        {Qt::UserRole + 2, QByteArrayLiteral("typeName")},
        {Qt::UserRole + 3, QByteArrayLiteral("id")},
        {Qt::UserRole + 4, QByteArrayLiteral("fileName")},
        {Qt::UserRole + 5, QByteArrayLiteral("visibleName")},
        {Qt::UserRole + 100, QByteArrayLiteral("checked")},
    };
}

QVariant BasicFigureSelectionModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return {};
    }
    if (role != Qt::UserRole + 100)
    {
        return QIdentityProxyModel::data(index, role);
    }

    return m_selectedId == getId(index);
}

int BasicFigureSelectionModel::getId(const QModelIndex& index) const
{
    return QIdentityProxyModel::data(index, Qt::UserRole + 3).toInt();
}

void BasicFigureSelectionModel::markAsChecked(int row)
{
    const auto selectedIndex = index(row, 0);
    if (!selectedIndex.isValid())
    {
        return;
    }
    setSelectedId(getId(selectedIndex));
}

void BasicFigureSelectionModel::setSelectedId(int newId)
{
    if (newId == m_selectedId)
    {
        return;
    }
    m_selectedId = newId;
    emit dataChanged(index(0, 0), index(rowCount() - 1, 0), {Qt::UserRole + 100});
    emit selectedIdChanged();
}

}
}
