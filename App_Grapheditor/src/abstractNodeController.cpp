#include "abstractNodeController.h"
#include "FilterGraph.h"
#include "graphHelper.h"

#include "fliplib/graphContainer.h"

#include <vector>

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

AbstractNodeController::AbstractNodeController(QObject* parent)
    : QObject(parent)
{
}

AbstractNodeController::~AbstractNodeController() = default;

void AbstractNodeController::setFilterGraph(FilterGraph* filterGraph)
{
    if (m_filterGraph == filterGraph)
    {
        return;
    }
    disconnect(m_filterGraphDestroyedConnection);
    m_filterGraph = filterGraph;
    if (m_filterGraph)
    {
        m_filterGraphDestroyedConnection = connect(m_filterGraph, &QObject::destroyed, this, std::bind(&AbstractNodeController::setFilterGraph, this, nullptr));
    }
    else
    {
        m_filterGraphDestroyedConnection = {};
    }
    emit filterGraphChanged();
}

void AbstractNodeController::setActualGraph(fliplib::GraphContainer *graph)
{
    m_actualGraph = graph;
    emit graphChanged();
}

void AbstractNodeController::setGridSize(int newSize)
{
    if (m_gridSize == newSize)
    {
        return;
    }
    m_gridSize = newSize;
    emit gridSizeChanged();
}

void AbstractNodeController::setUseGridSizeAutomatically(bool useGridSize)
{
    if (m_useGridSizeAutomatically == useGridSize)
    {
        return;
    }
    m_useGridSizeAutomatically = useGridSize;
    emit useGridSizeAutomaticallyChanged();
}

}
}
}
}
