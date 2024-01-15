#include "topBarButtonFilterModel.h"
#include "topBarButtonModel.h"

namespace precitec
{

namespace gui
{

TopBarButtonFilterModel::TopBarButtonFilterModel(QObject* parent) : QSortFilterProxyModel(parent)
{
    connect(this, &TopBarButtonFilterModel::showGraphEditorChanged, this, &TopBarButtonFilterModel::invalidate);
    connect(this, &TopBarButtonFilterModel::headMonitorAvailableChanged, this, &TopBarButtonFilterModel::invalidate);
}

TopBarButtonFilterModel::~TopBarButtonFilterModel() = default;

void TopBarButtonFilterModel::setShowGraphEditor(bool value)
{
    if (m_showGraphEditor == value)
    {
        return;
    }
    m_showGraphEditor = value;
    emit showGraphEditorChanged();
}

bool TopBarButtonFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    const auto sourceIndex = sourceModel()->index(source_row, 0, source_parent);
    if (!sourceIndex.isValid())
    {
        return false;
    }

    if (sourceIndex.data(Qt::UserRole + 4).value<TopBarButtonModel::TopBarButton>() == TopBarButtonModel::TopBarButton::Grapheditor)
    {
        return m_showGraphEditor;
    }

    if (sourceIndex.data(Qt::UserRole + 4).value<TopBarButtonModel::TopBarButton>() == TopBarButtonModel::TopBarButton::HeadMonitor)
    {
        return m_headMonitorAvailable;
    }

    return true;
}

void TopBarButtonFilterModel::setHeadMonitorAvailable(bool set)
{
    if (m_headMonitorAvailable == set)
    {
        return;
    }
    m_headMonitorAvailable = set;
    emit headMonitorAvailableChanged();
}

}
}
