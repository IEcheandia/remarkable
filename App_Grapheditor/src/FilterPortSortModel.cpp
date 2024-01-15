#include "FilterPortSortModel.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

FilterPortSortModel::FilterPortSortModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &FilterPortSortModel::searchTypeChanged, this, &FilterPortSortModel::invalidate);
    connect(this, &FilterPortSortModel::searchUUIDChanged, this, &FilterPortSortModel::invalidate);
}

FilterPortSortModel::~FilterPortSortModel() = default;

int FilterPortSortModel::searchType() const
{
    return m_searchType;
}

void FilterPortSortModel::setSearchType(int searchID)
{
    if (m_searchType == searchID)
    {
        return;
    }
    m_searchType = searchID;
    emit searchTypeChanged();
}

QUuid FilterPortSortModel::searchUUID() const
{
    return m_searchUUID;
}

void FilterPortSortModel::setSearchUUID(const QUuid& uuid)
{
    if (m_searchUUID == uuid)
    {
        return;
    }
    m_searchUUID = uuid;
    emit searchUUIDChanged();
}

bool FilterPortSortModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }

    const auto sourceType = sourceIndex.data(Qt::UserRole + 2).toInt();
    const auto sourceId = sourceIndex.data(Qt::DisplayRole).toUuid();

    if (m_searchUUID.isNull() && (sourceId.isNull() || sourceType == m_searchType))
    {
        return false;
    }

    if (sourceType != 2 && sourceType != 3)
    {
        return false;
    }

    if (sourceType == m_searchType || sourceId == m_searchUUID)
    {
        return false;
    }

    return true;
}

}
}
}
}


