#pragma once

#include "abstractFilterAttributeModel.h"

namespace precitec
{
namespace storage
{

class GraphModel;
class SubGraphModel;

/**
 * A model providing the attributes of an InstanceFilter.
 **/
class FilterAttributeModel : public AbstractFilterAttributeModel
{
    Q_OBJECT
    /**
     * The id of the graph for which one the filters should be provided
     **/
    Q_PROPERTY(QUuid graphId READ graphId WRITE setGraphId NOTIFY graphIdChanged)
    /**
     * The GraphModel providing the storage of graphs.
     **/
    Q_PROPERTY(precitec::storage::GraphModel* graphModel READ graphModel WRITE setGraphModel NOTIFY graphModelChanged)
    /**
     * The SubGraphModel providing the storage of sub graphs.
     **/
    Q_PROPERTY(precitec::storage::SubGraphModel* subGraphModel READ subGraphModel WRITE setSubGraphModel NOTIFY subGraphModelChanged)

public:
    explicit FilterAttributeModel(QObject* parent = nullptr);
    ~FilterAttributeModel() override;

    QUuid graphId() const
    {
        return m_graphId;
    }
    void setGraphId(const QUuid &id);

    GraphModel* graphModel() const
    {
        return m_graphModel;
    }
    void setGraphModel(GraphModel* model);

    SubGraphModel* subGraphModel() const
    {
        return m_subGraphModel;
    }
    void setSubGraphModel(SubGraphModel* model);

Q_SIGNALS:
    void graphIdChanged();
    void graphModelChanged();
    void subGraphModelChanged();

private:
    void updateGraph();
    void updateModel() override;
    fliplib::GraphContainer& graph() override
    {
        return m_graph;
    }

    QUuid m_graphId;

    fliplib::GraphContainer m_graph = {};

    GraphModel* m_graphModel = nullptr;
    QMetaObject::Connection m_graphModelDestroyedConnection;
    QMetaObject::Connection m_graphModelResetConnection;

    SubGraphModel* m_subGraphModel = nullptr;
    QMetaObject::Connection m_subGraphModelDestroyedConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::storage::FilterAttributeModel*)
