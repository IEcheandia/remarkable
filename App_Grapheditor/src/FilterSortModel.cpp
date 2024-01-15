#include "FilterSortModel.h"

using namespace precitec::gui::components::grapheditor;

FilterSortModel::FilterSortModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &FilterSortModel::searchTextChanged, this, &FilterSortModel::invalidate);
    connect(this, &FilterSortModel::excludeBridgesChanged, this, &FilterSortModel::invalidate);
}

FilterSortModel::~FilterSortModel() = default;

QString FilterSortModel::getSearchText() const
{
    return m_searchText;
}

void FilterSortModel::setSearchText(const QString& searchText)
{
    if (m_searchText == searchText)
    {
        return;
    }
    m_searchText = searchText;
    emit searchTextChanged();
}

bool FilterSortModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (m_excludeBridges)
    {
        if (sourceIndex.data(Qt::DisplayRole).toString().compare(QStringLiteral("Filter_Bridges")) == 0)
        {
            return false;
        }
    }
    if (m_searchText.isEmpty())
    {
        return true;
    }
    if (!sourceIndex.isValid())
    {
        return false;
    }

    //check if input is in filterGroupName
    if (sourceIndex.data(Qt::DisplayRole).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    //check if input is in filterName
    if (sourceIndex.data(Qt::UserRole+1).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    return false;
}

void FilterSortModel::setExcludeBridges(bool exclude)
{
    if (m_excludeBridges == exclude)
    {
        return;
    }
    m_excludeBridges = exclude;
    emit excludeBridgesChanged();
}
