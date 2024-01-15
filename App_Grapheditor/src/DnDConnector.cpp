#include "DnDConnector.h"

using namespace precitec::gui::components::grapheditor;

DnDConnector::DnDConnector(QQuickItem* parent) : qan::NodeItem(parent)
{
    setAcceptDrops(false);
    setVisible(false);
}

DnDConnector::~DnDConnector() = default;

auto DnDConnector::connectorGraph() const noexcept -> qan::Graph*
{
    return m_connectorGraph.data();
}

auto DnDConnector::setConnectorGraph(qan::Graph* graph) noexcept -> void
{
    if (m_connectorGraph.data() != graph)
    {
        m_connectorGraph = graph;
        if (m_edgeItem)
        {
            m_edgeItem->setParentItem(graph->getContainerItem());
            m_edgeItem->setGraph(graph);
            m_edgeItem->setVisible(false);
        }
        if (m_connectorGraph == nullptr)
        {
            setVisible(false);
        }
        emit connectorGraphChanged();
    }
}

auto DnDConnector::connectorItem() noexcept -> QQuickItem*
{
    return m_connectorItem.data();
}

auto DnDConnector::setConnectorItem(QQuickItem* connectorItem) noexcept -> void
{
    if (m_connectorItem != connectorItem)
    {
        if (m_connectorItem)
        {
            disconnect(m_connectorItem.data(), nullptr, this, nullptr);
            m_connectorItem->deleteLater();
        }
        m_connectorItem = connectorItem;
        if (m_connectorItem)
        {
            m_connectorItem->setParentItem(this);
        }
        emit connectorItemChanged();
    }
}

auto DnDConnector::edgeComponent() noexcept -> QQmlComponent*
{
    return m_edgeComponent.data();
}

auto DnDConnector::setEdgeComponent(QQmlComponent* edgeComponent) noexcept -> void
{
    if (m_edgeComponent != edgeComponent)
    {
        m_edgeComponent = edgeComponent;
        if (m_edgeComponent && m_edgeComponent->isReady())
        {
            const auto context = qmlContext(this);          //Check this
            if (context != nullptr)
            {
                const auto edgeObject = m_edgeComponent->create(context); //Create a new edge
                m_edgeItem.reset(qobject_cast<qan::EdgeItem*>(edgeObject)); //Any existing edge item is destroyed

                if (m_edgeItem && !m_edgeComponent->isError())
                {
                    QQmlEngine::setObjectOwnership(m_edgeItem.data(), QQmlEngine::CppOwnership);
                    m_edgeItem->setVisible(false);
                    m_edgeItem->setAcceptDrops(false);

                    if (getGraph() != nullptr)
                    {
                        m_edgeItem->setGraph(getGraph());
                        m_edgeItem->setParentItem(getGraph()->getContainerItem());
                    }
                    if (getNode() && getNode()->getItem())
                    {
                        m_edgeItem->setSourceItem(getNode()->getItem());
                        m_edgeItem->setDestinationItem(this);       //Problem! This should be the mouse position
                    }
                    emit edgeItemChanged();
                }
                else
                {
                    qWarning() << "qan::Connector::setEdgeComponent(): Error while creating edge:";
                    qWarning() << "\t" << m_edgeComponent->errors();
                }
            }
        }
        emit edgeComponentChanged();
    }
}

auto DnDConnector::edgeItem() noexcept -> qan::EdgeItem*
{
    return m_edgeItem.data();
}

qan::PortItem* DnDConnector::sourceConnector() const noexcept
{
    return m_sourceConnector.data();
}

void DnDConnector::setSourceConnector(qan::PortItem* sourceConnector) noexcept
{
    if (m_sourceConnector.data() != sourceConnector)
    {
        m_sourceConnector = sourceConnector;
        emit sourceConnectorChanged();
    }
}

FilterPort* DnDConnector::sourcePort() const noexcept
{
    return m_sourcePort.data();
}

void DnDConnector::setSourcePort(FilterPort* sourcePort) noexcept
{
    if (m_sourcePort.data() != sourcePort)
    {
        m_sourcePort = sourcePort;
        emit sourcePortChanged();
    }
}

void DnDConnector::updatePosition(QPointF endPosition, const double &zoom, const QSizeF &offset)
{
    setX(endPosition.x()/zoom + offset.width());
    setY(endPosition.y()/zoom + offset.height());
}

void DnDConnector::connectorPressed()
{
    if (!m_connectorGraph)
    {
        qDebug() << "No graph! FilterConnector::connectorPressed()";
        return;
    }
    if (!m_edgeItem)
    {
        qDebug() << "No edge item! FilterConnector::connectorPressed()";
        return;
    }
    if (!m_sourceConnector && !m_sourcePort)
    {
        qDebug() << "No source connector and port! FilterConnector::connectorPressed()";
        return;
    }

    m_edgeItem->setGraph(m_connectorGraph);
    if (m_sourceConnector)
    {
        m_edgeItem->setSourceItem(m_sourceConnector);
    }
    else
    {
        m_edgeItem->setSourceItem(m_sourcePort.data()->getItem());
    }
    m_edgeItem->setDestinationItem(this);
    m_edgeItem->setVisible(true);
}

