#include "NodeSortModel.h"
#include <QDebug>

using namespace precitec::gui::components::grapheditor;

NodeSortModel::NodeSortModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &NodeSortModel::searchTextChanged, this, &NodeSortModel::invalidate);
    connect(this, &NodeSortModel::searchFilterChanged, this, &NodeSortModel::invalidate);
    connect(this, &NodeSortModel::searchPortChanged, this, &NodeSortModel::invalidate);
    connect(this, &NodeSortModel::searchCommentChanged, this, &NodeSortModel::invalidate);
    connect(this, &NodeSortModel::searchGroupChanged, this, &NodeSortModel::invalidate);
}

NodeSortModel::~NodeSortModel() = default;

QString NodeSortModel::getSearchText() const
{
    return m_searchText;
}

void NodeSortModel::setSearchText(const QString& searchText)
{
    if (m_searchText == searchText)
    {
        return;
    }
    m_searchText = searchText;
    emit searchTextChanged();
}

bool NodeSortModel::searchFilter() const
{
    return m_searchFilter;
}

void NodeSortModel::setSearchFilter(bool value)
{
    if (m_searchFilter != value)
    {
        m_searchFilter = value;
        emit searchFilterChanged();
    }
}

bool NodeSortModel::searchPort() const
{
    return m_searchPort;
}

void NodeSortModel::setSearchPort(bool value)
{
    if (m_searchPort != value)
    {
        m_searchPort = value;
        emit searchPortChanged();
    }
}

bool NodeSortModel::searchComment() const
{
    return m_searchComment;
}

void NodeSortModel::setSearchComment(bool value)
{
    if (m_searchComment != value)
    {
        m_searchComment = value;
        emit searchCommentChanged();
    }
}

bool NodeSortModel::searchGroup() const
{
    return m_searchGroup;
}

void NodeSortModel::setSearchGroup(bool value)
{
    if (m_searchGroup != value)
    {
        m_searchGroup = value;
        emit searchGroupChanged();
    }
}


bool NodeSortModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }

//Check search property stuff
    if (!m_searchFilter)
    {
        if (sourceIndex.data(Qt::UserRole + 1).toString() == "Filternode")
        {
            return false;
        }
    }
    if (!m_searchPort)
    {
        if (sourceIndex.data(Qt::UserRole + 1).toString() == "Filterport")
        {
            return false;
        }
    }
    if (!m_searchComment)
    {
        if (sourceIndex.data(Qt::UserRole + 1).toString() == "Filtercomment")
        {
            return false;
        }
    }
    if (!m_searchGroup)
    {
        if (sourceIndex.data(Qt::UserRole + 1).toString() == "Filtergroup")
        {
            return false;
        }
    }

    if (m_searchText.isEmpty())
    {
        return true;
    }

//Check textfield stuff
    //check if input is in label of nodes
    if (sourceIndex.data(Qt::DisplayRole).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    //check if input is in nodeType of nodes
    if (sourceIndex.data(Qt::UserRole).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    //check if input is in type of nodes
    if (sourceIndex.data(Qt::UserRole + 1).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    //check if input is in id of nodes
    if (sourceIndex.data(Qt::UserRole + 2).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }
    //check if input is in group label of nodes
    if (sourceIndex.data(Qt::UserRole + 3).toString().contains(m_searchText, Qt::CaseInsensitive))
    {
        return true;
    }

    return false;
}
