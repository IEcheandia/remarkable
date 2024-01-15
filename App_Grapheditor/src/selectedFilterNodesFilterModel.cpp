#include "selectedFilterNodesFilterModel.h"
#include "FilterGraph.h"
#include "filterMacro.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

SelectedFilterNodesFilterModel::SelectedFilterNodesFilterModel(QObject* parent)
    : QSortFilterProxyModel(parent)
{
    connect(this, &SelectedFilterNodesFilterModel::filterGraphChanged, this,
        [this]
        {
            setSourceModel(m_filterGraph->selectedNodesModel());
        }
    );
}

SelectedFilterNodesFilterModel::~SelectedFilterNodesFilterModel() = default;

bool SelectedFilterNodesFilterModel::filterAcceptsRow(int source_row, const QModelIndex& source_parent) const
{
    if (source_parent.isValid())
    {
        return false;
    }
    if (sourceModel()->index(source_row, 0, source_parent).data(qcm::ContainerModel::ItemDataRole).value<FilterNode*>())
    {
        return true;
    }
    if (sourceModel()->index(source_row, 0, source_parent).data(qcm::ContainerModel::ItemDataRole).value<FilterMacro*>())
    {
        return true;
    }
    return false;
}

void SelectedFilterNodesFilterModel::setFilterGraph(FilterGraph* graph)
{
    if (m_filterGraph == graph)
    {
        return;
    }
    m_filterGraph = graph;
    disconnect(m_filterGraphDestroyConnection);
    if (m_filterGraph)
    {
        m_filterGraphDestroyConnection = connect(m_filterGraph, &FilterGraph::destroyed, this, std::bind(&SelectedFilterNodesFilterModel::setFilterGraph, this, nullptr));
    }
    else
    {
        m_filterGraphDestroyConnection = {};
    }

    emit filterGraphChanged();
}

}
}
}
}
