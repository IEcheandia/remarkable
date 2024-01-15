#include "FilterGraphFilterModel.h"
#include <QDebug>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

FilterGraphFilterModel::FilterGraphFilterModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &FilterGraphFilterModel::searchTextChanged, this, &FilterGraphFilterModel::invalidate);
    setSortRole(Qt::DisplayRole);
    sort(0);
}

FilterGraphFilterModel::~FilterGraphFilterModel() = default;

QString FilterGraphFilterModel::searchText() const
{
    return m_searchText;
}

void FilterGraphFilterModel::setSearchText(const QString& text)
{
    if (m_searchText != text)
    {
        m_searchText = text;
        emit searchTextChanged();
    }
}

bool FilterGraphFilterModel::lessThan(const QModelIndex& source_left, const QModelIndex& source_right) const
{
    const auto& leftData = sourceModel()->data(source_left, sortRole()).toString();
    const auto& rightData = sourceModel()->data(source_right, sortRole()).toString();

    return QString::compare(leftData, rightData, Qt::CaseInsensitive) < 0;
}

bool FilterGraphFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_searchText.isNull())            //Falls nichts vorgegeben dann gebe alle aus.
    {
        return true;
    }
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    if (sourceIndex.data(Qt::DisplayRole).toString().contains(m_searchText, Qt::CaseInsensitive)) //Filtere die Filter heraus, dessen Namen nicht den Suchtext beinhalten.
    {
        return true;
    }
    // filter on comment of macros
    if (sourceIndex.data(Qt::UserRole + 6).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    return false;
}

}
}
}
}

