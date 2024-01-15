#pragma once

#include <QAbstractListModel>
#include "GraphModelVisualizer.h"
#include <QUuid>

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterNodeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphModelVisualizer* graphModelVisualizer READ graphModelVisualizer WRITE setGraphModelVisualizer NOTIFY graphModelVisualizerChanged)
    //For showing all node, but the selected node

public:
    explicit FilterNodeModel(QObject *parent = nullptr);
    ~FilterNodeModel() override;

    GraphModelVisualizer* graphModelVisualizer() const;
    void setGraphModelVisualizer(GraphModelVisualizer* graphContainer);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

Q_SIGNALS:
    void graphModelVisualizerChanged();

private:
    void init();
    GraphModelVisualizer* m_graphVisualizer;
    QMetaObject::Connection m_destroyConnection;
    fliplib::GraphContainer* m_graph;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterNodeModel*)

