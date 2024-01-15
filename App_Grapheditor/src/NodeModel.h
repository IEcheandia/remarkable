#pragma once

#include <QAbstractListModel>
#include "GraphModelVisualizer.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class NodeModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphModelVisualizer* graphModelVisualizer READ graphModelVisualizer WRITE setGraphModelVisualizer NOTIFY graphModelVisualizerChanged)

public:
    explicit NodeModel(QObject *parent = nullptr);
    ~NodeModel() override;

    GraphModelVisualizer* graphModelVisualizer() const;
    void setGraphModelVisualizer(GraphModelVisualizer* filterGraph);

    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    Q_INVOKABLE qan::Node* getClickedNode(const QModelIndex &index);

Q_SIGNALS:
    void graphModelVisualizerChanged();

private:
    GraphModelVisualizer* m_graphVisualizer;
    QMetaObject::Connection m_destroyConnection;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::NodeModel*)
