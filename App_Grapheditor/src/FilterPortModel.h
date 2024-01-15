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

class FilterPortModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphModelVisualizer* graphModelVisualizer READ graphModelVisualizer WRITE setGraphModelVisualizer NOTIFY graphModelVisualizerChanged)

public:
    explicit FilterPortModel(QObject *parent = nullptr);
    ~FilterPortModel() override;

    GraphModelVisualizer* graphModelVisualizer() const;
    void setGraphModelVisualizer(GraphModelVisualizer* graphContainer);

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;

    Q_INVOKABLE QObject* getPort(const QModelIndex &index);

Q_SIGNALS:
    void graphModelVisualizerChanged();

private:
    void init();
    GraphModelVisualizer* m_graphVisualizer = nullptr;
    QMetaObject::Connection m_destroyConnection;
    fliplib::GraphContainer* m_graph = nullptr;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterPortModel*)
