#pragma once

//#include <QAbstractListModel>
#include "GraphModelVisualizer.h"

namespace precitec
{

namespace gui
{

namespace components
{

namespace grapheditor
{

class FilterGroupModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphModelVisualizer* graphModelVisualizer READ graphModelVisualizer WRITE setGraphModelVisualizer NOTIFY graphModelVisualizerChanged)

public:
    explicit FilterGroupModel(QObject *parent = nullptr);
    ~FilterGroupModel() override;

    GraphModelVisualizer* graphModelVisualizer() const;
    void setGraphModelVisualizer(GraphModelVisualizer* graphContainer);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

Q_SIGNALS:
    void graphModelVisualizerChanged();
    void graphChanged();

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
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterGroupModel*)
