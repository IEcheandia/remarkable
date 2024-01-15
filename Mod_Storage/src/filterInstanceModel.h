#pragma once

#include <QAbstractListModel>
#include <QUuid>

#include "fliplib/graphContainer.h"

namespace precitec
{
namespace storage
{

class GraphModel;
class SubGraphModel;

/**
 * A model providing all filters of a given graph.
 **/
class FilterInstanceModel : public QAbstractListModel
{
    Q_OBJECT
    /**
     * The id of the graph for which one the filters should be provided
     **/
    Q_PROPERTY(QUuid graphId READ graphId WRITE setGraphId NOTIFY graphIdChanged)
    /**
     * The GraphModel providing the storage of graphs.
     **/
    Q_PROPERTY(precitec::storage::GraphModel *graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)
    /**
     * The SubGraphModel providing the storage of sub graphs.
     **/
    Q_PROPERTY(precitec::storage::SubGraphModel *subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)
public:
    explicit FilterInstanceModel(QObject *parent = nullptr);
    ~FilterInstanceModel() override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    QHash<int, QByteArray> roleNames() const override;

    QUuid graphId() const
    {
        return m_graphId;
    }
    void setGraphId(const QUuid &id);

    GraphModel *graphModel() const
    {
        return m_graphModel;
    }

    void setGraphModel(GraphModel *model);

    SubGraphModel *subGraphModel() const
    {
        return m_subGraphModel;
    }

    void setSubGraphModel(SubGraphModel *model);

    /**
     * Helper method to return a null QUuid for qml assigning of graphId.
     **/
    Q_INVOKABLE QUuid nullUuid() const
    {
        return QUuid{};
    }

Q_SIGNALS:
    void graphIdChanged();
    void graphModelChanged();
    void subGraphModelChanged();

protected:
    const fliplib::GraphContainer &graph() const
    {
        return m_graph;
    }

    bool hasVisibleAttributes(const fliplib::InstanceFilter &filter) const;
    int maxVisibleUserLevel(const fliplib::InstanceFilter &filter) const;

private:
    void init();
    bool hasVisibleAttributes(const QModelIndex &index) const;
    int maxVisibleUserLevel(const QModelIndex &index) const;
    QUuid m_graphId;
    GraphModel *m_graphModel = nullptr;
    SubGraphModel *m_subGraphModel = nullptr;
    QMetaObject::Connection m_destroyConnection;
    QMetaObject::Connection m_subGraphDestroyConnection;
    QMetaObject::Connection m_graphInit;
    fliplib::GraphContainer m_graph{};
};

}
}
Q_DECLARE_METATYPE(precitec::storage::FilterInstanceModel*)
