#pragma once

#include <QuickQanava.h>
#include "FilterPort.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class DnDConnector : public qan::NodeItem
{
    Q_OBJECT
    Q_PROPERTY(qan::Graph* connectorGraph READ connectorGraph WRITE setConnectorGraph NOTIFY connectorGraphChanged FINAL)
    Q_PROPERTY(QQuickItem* connectorItem READ connectorItem WRITE setConnectorItem NOTIFY connectorItemChanged FINAL)
    //DnD visual effect
    Q_PROPERTY(QQmlComponent* edgeComponent READ edgeComponent WRITE setEdgeComponent NOTIFY edgeComponentChanged FINAL)
    Q_PROPERTY(qan::EdgeItem* edgeItem READ edgeItem NOTIFY edgeItemChanged FINAL)
    Q_PROPERTY(qan::PortItem* sourceConnector READ sourceConnector WRITE setSourceConnector NOTIFY sourceConnectorChanged FINAL)
    Q_PROPERTY(FilterPort* sourcePort READ sourcePort WRITE setSourcePort NOTIFY sourcePortChanged FINAL)

public:
    explicit DnDConnector (QQuickItem* parent = nullptr);
    ~DnDConnector() override;

    auto connectorGraph() const noexcept -> qan::Graph*;
    auto setConnectorGraph(qan::Graph* graph) noexcept -> void;
    auto connectorItem() noexcept -> QQuickItem*;
    auto setConnectorItem(QQuickItem* connectorItem) noexcept -> void;
    auto edgeComponent() noexcept -> QQmlComponent*;
    auto setEdgeComponent(QQmlComponent* edgeComponent) noexcept -> void;
    auto edgeItem() noexcept -> qan::EdgeItem*;
    qan::PortItem* sourceConnector() const noexcept;
    void setSourceConnector(qan::PortItem* sourceConnector) noexcept;
    FilterPort* sourcePort() const noexcept;
    void setSourcePort(FilterPort* sourcePort) noexcept;

    Q_INVOKABLE void updatePosition(QPointF endPosition, const double &zoom, const QSizeF &offset = {0, 0});

    //DnD visual effect
    Q_INVOKABLE void connectorPressed();

Q_SIGNALS:
    void connectorGraphChanged();
    void connectorItemChanged();
    void edgeComponentChanged();
    void edgeItemChanged();
    void sourceConnectorChanged();
    void sourcePortChanged();

private:
    QPointer<qan::Graph> m_connectorGraph;
    QPointer<QQuickItem> m_connectorItem;
    QPointer<QQmlComponent> m_edgeComponent;
    QScopedPointer<qan::EdgeItem> m_edgeItem;
    QPointer<qan::PortItem> m_sourceConnector;
    QPointer<FilterPort> m_sourcePort;
};

}
}
}
}

QML_DECLARE_TYPE( precitec::gui::components::grapheditor::DnDConnector )

