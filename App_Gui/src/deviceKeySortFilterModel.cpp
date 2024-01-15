#include "deviceKeySortFilterModel.h"

namespace precitec
{
namespace gui
{

DeviceKeySortFilterModel::DeviceKeySortFilterModel(QObject *parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &DeviceKeySortFilterModel::searchTextChanged, this, &DeviceKeySortFilterModel::invalidate);
}

DeviceKeySortFilterModel::~DeviceKeySortFilterModel() = default;

void DeviceKeySortFilterModel::setSearchText(const QString &searchText)
{
    if (m_searchText == searchText)
    {
        return;
    }
    m_searchText = searchText;
    emit searchTextChanged();
}

bool DeviceKeySortFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_searchText.isEmpty())
    {
        return true;
    }
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    if (sourceIndex.data(Qt::DisplayRole).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    // comment
    if (sourceIndex.data(Qt::UserRole).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    return false;
}

}
}
