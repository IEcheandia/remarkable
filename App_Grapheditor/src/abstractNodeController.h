#pragma once
#include <QObject>

namespace fliplib
{
struct GraphContainer;
}

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

class FilterGraph;

/**
 * Class for group related functionality.
 **/
class AbstractNodeController : public QObject
{
    Q_OBJECT
    /**
     * The FilterGraph which visualizes the current GraphContainer
     **/
    Q_PROPERTY(precitec::gui::components::grapheditor::FilterGraph *filterGraph READ filterGraph WRITE setFilterGraph NOTIFY filterGraphChanged)
    /**
     * Grid size of the filter graph view. Used for snapping.
     **/
    Q_PROPERTY(int gridSize READ gridSize WRITE setGridSize NOTIFY gridSizeChanged)
    /**
     * Whether the grid size is used during filter movement.
     **/
    Q_PROPERTY(bool useGridSizeAutomatically READ useGridSizeAutomatically WRITE setUseGridSizeAutomatically NOTIFY useGridSizeAutomaticallyChanged)

    Q_PROPERTY(fliplib::GraphContainer *graph READ graph WRITE setActualGraph NOTIFY graphChanged)
public:
    ~AbstractNodeController();

    FilterGraph *filterGraph() const
    {
        return m_filterGraph;
    }
    void setFilterGraph(FilterGraph *filterGraph);

    void setActualGraph(fliplib::GraphContainer *graph);

    int gridSize() const
    {
        return m_gridSize;
    }
    void setGridSize(int newSize);

    bool useGridSizeAutomatically() const
    {
        return m_useGridSizeAutomatically;
    }
    void setUseGridSizeAutomatically(bool useGridSize);

    fliplib::GraphContainer *graph() const
    {
        return m_actualGraph;
    }

Q_SIGNALS:
    void filterGraphChanged();
    void changed();
    void gridSizeChanged();
    void useGridSizeAutomaticallyChanged();
    void graphChanged();

protected:
    AbstractNodeController(QObject *parent = nullptr);

private:
    FilterGraph *m_filterGraph = nullptr;
    QMetaObject::Connection m_filterGraphDestroyedConnection;
    fliplib::GraphContainer *m_actualGraph = nullptr;
    int m_gridSize = 1;
    bool m_useGridSizeAutomatically = false;
};

}
}
}
}
