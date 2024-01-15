#include "FilterNodeSortModel.h"
#include <QDebug>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

FilterNodeSortModel::FilterNodeSortModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &FilterNodeSortModel::searchIDChanged, this, &FilterNodeSortModel::invalidate);
    connect(this, &FilterNodeSortModel::searchTextChanged, this, &FilterNodeSortModel::invalidate);
}

FilterNodeSortModel::~FilterNodeSortModel() = default;

QUuid FilterNodeSortModel::searchID() const
{
    return m_searchID;
}

void FilterNodeSortModel::setSearchID(const QUuid& searchID)
{
    if (m_searchID == searchID)
    {
        return;
    }
    m_searchID = searchID;
    emit searchIDChanged();
}

QString FilterNodeSortModel::searchText() const
{
    return m_searchText;
}

void FilterNodeSortModel::setSearchText(const QString& text)
{
    if (m_searchText != text)
    {
        m_searchText = text;
        emit searchTextChanged();
    }
    /*if (m_searchText == text)
    {
        return;
    }
    m_searchText = text;
    emit searchTextChanged();*/
}

bool FilterNodeSortModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (m_searchID.isNull())            //Falls nichts vorgegeben dann gebe alle aus.
    {
        return true;
    }
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }
    //check if input has same filterNode id
    if (sourceIndex.data(Qt::DisplayRole) == m_searchID)    //Filtere den Source-Filter aus
    {
        return false;
    }
    if (!sourceIndex.data(Qt::UserRole).toString().contains(m_searchText, Qt::CaseInsensitive)) //Filtere die Filter heraus, dessen Namen nicht den Suchtext beinhalten.
    {
        return false;
    }

    return true;
}

}
}
}
}

