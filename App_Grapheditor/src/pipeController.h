#pragma once

#include <QObject>
#include <memory>

#include <gtpo/node.h>

namespace qan
{
class Edge;
class Node;
}

namespace fliplib
{
class GraphContainer;
}

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{

class FilterConnector;
class FilterGraph;
class FilterNode;
class FilterPort;
class GraphModelVisualizer;

class PipeController : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool connecting READ isConnecting NOTIFY connectingChanged)
    /**
     * The FilterGraph which visualizes the current GraphContainer
     **/
    Q_PROPERTY(precitec::gui::components::grapheditor::FilterGraph *filterGraph READ filterGraph WRITE setFilterGraph NOTIFY filterGraphChanged)
    Q_PROPERTY(qreal zoom READ zoom WRITE setZoom NOTIFY zoomChanged)

    Q_PROPERTY(fliplib::GraphContainer *graph READ graph WRITE setGraph NOTIFY graphChanged)
public:
    PipeController(QObject *parent = nullptr);
    ~PipeController();

    FilterGraph *filterGraph() const
    {
        return m_filterGraph;
    }
    void setFilterGraph(FilterGraph *filterGraph);

    Q_INVOKABLE void insertNewPipeVisual(precitec::gui::components::grapheditor::FilterConnector* senderConnector, const QPoint &translation);
    Q_INVOKABLE bool isPipeConnectionPossible(precitec::gui::components::grapheditor::FilterConnector* senderConnector, const QPoint &translation) const;
    Q_INVOKABLE void insertNewPipeVisualPort(precitec::gui::components::grapheditor::FilterPort* senderPort, const QPoint &translation);
    Q_INVOKABLE bool isConnectionPossible(precitec::gui::components::grapheditor::FilterPort* senderPort, const QPoint &translation) const;

    Q_INVOKABLE void startFromFilter(precitec::gui::components::grapheditor::FilterConnector* senderConnector);
    Q_INVOKABLE void startFromPort(precitec::gui::components::grapheditor::FilterPort* senderPort);

    Q_INVOKABLE void deleteEdge(qan::Edge* edge);
    void deleteEdges(const qcm::Container<QVector, qan::Edge*>&edges);
    void erasePipeSourceDestination(const QUuid& sourceID, const QUuid& sourceConnectorID,  const QUuid& destinationID, const QUuid& destinationConnectorID);

    bool isConnecting()
    {
        return m_connecting;
    }

    qreal zoom() const
    {
        return m_zoom;
    }
    void setZoom(qreal zoom);

    void setGraph(fliplib::GraphContainer *graph);
    fliplib::GraphContainer *graph() const
    {
        return m_graph;
    }

Q_SIGNALS:
    void filterGraphChanged();
    void connectingChanged();
    void zoomChanged();
    void changed();
    void graphChanged();

private:
    void insertNewPipe(qan::Node* senderNode, FilterConnector* senderConnector, qan::Node* receiverNode, FilterConnector* receiverConnector);
    bool canBeConnected(qan::Node* senderNode, FilterConnector* senderConnector, qan::Node* receiverNode, FilterConnector* receiverConnector) const;
    void setConnecting(bool set);

    QPointF calculateDestinationPoint(FilterConnector* senderConnector, const QPoint &translation) const;
    QPointF calculateDestinationPoint(FilterPort* senderPort, const QPoint &translation) const;

    template <typename T>
    qan::Node* findTargetNode(qan::Node *node, const QRectF &boundaries) const;
    qan::Node* findTargetNode(const QRectF &boundaries) const;
    qan::Node* findTargetNode(const QPointF &destinationPoint) const;
    template <typename T>
    FilterConnector* findTargetConnector(T* node, const QRectF &boundaries) const;
    template <typename T>
    FilterConnector* findTargetConnector(T* node, const QPointF &destinationPoint) const;

    bool areasOverlap(const QPointF &topLeftPoint, const QPointF &bottomRightPoint, const QPointF &topLeftPointTwo, const QPointF &bottomRightPointTwo) const;

    void insertNewPipe(qan::Edge* edge, qan::Node* sourceNode, FilterConnector* sourceConnector, qan::Node* destinationNode, FilterConnector* destinationConnector);

    bool m_connecting{false};
    FilterGraph *m_filterGraph = nullptr;
    QMetaObject::Connection m_filterGraphDestroyedConnection;

    qreal m_zoom{1.0};
    fliplib::GraphContainer *m_graph = nullptr;
};

}
}
}
}
