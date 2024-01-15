#pragma once

#include <QObject>
#include <QPointF>
#include <QUuid>
#include "fliplib/graphContainer.h"

namespace qan
{
class Node;
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
class GraphEditor;

class CopyPasteHandler : public QObject
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::FilterGraph* filterGraph READ filterGraph WRITE setFilterGraph NOTIFY filterGraphChanged)
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphEditor* graphEditor READ graphEditor WRITE setGraphEditor NOTIFY graphEditorChanged)
    Q_PROPERTY(QPointF copyPosition READ copyPosition WRITE setCopyPosition NOTIFY copyPositionChanged)

public:
    explicit CopyPasteHandler(QObject *parent = nullptr);
    ~CopyPasteHandler() override;

    FilterGraph* filterGraph() const;
    void setFilterGraph(FilterGraph* newGraph);
    GraphEditor* graphEditor() const;
    void setGraphEditor(GraphEditor* newEditor);
    QPointF copyPosition() const;
    void setCopyPosition(const QPointF &newPosition);

    Q_INVOKABLE void copyObjects();
    Q_INVOKABLE void pasteObjects();

Q_SIGNALS:
    void filterGraphChanged();
    void graphEditorChanged();
    void copyPositionChanged();

private:
    void resetGraphContainer();
    bool checkNodes(qan::Node* edgeSource, qan::Node* node);

    FilterGraph* m_filterGraph = nullptr;
    GraphEditor* m_graphEditor = nullptr;
    QPointF m_copyPosition = {0.0,0.0};

    fliplib::GraphContainer m_copyGraphInstances;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::CopyPasteHandler*)
