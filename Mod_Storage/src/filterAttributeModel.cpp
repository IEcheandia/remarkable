#include "filterAttributeModel.h"
#include "attributeGroup.h"
#include "attributeGroupItem.h"
#include "graphModel.h"
#include "subGraphModel.h"

namespace precitec
{
namespace storage
{

FilterAttributeModel::FilterAttributeModel(QObject *parent)
    : AbstractFilterAttributeModel(parent)
{
    connect(this, &FilterAttributeModel::graphIdChanged, this, &FilterAttributeModel::updateGraph);
    connect(this, &FilterAttributeModel::graphModelChanged, this, &FilterAttributeModel::updateGraph);
    connect(this, &FilterAttributeModel::subGraphModelChanged, this, &FilterAttributeModel::updateGraph);
}

FilterAttributeModel::~FilterAttributeModel() = default;

void FilterAttributeModel::setGraphId(const QUuid& id)
{
    if (m_graphId == id)
    {
        return;
    }
    m_graphId = id;
    emit graphIdChanged();
}

void FilterAttributeModel::setGraphModel(GraphModel* model)
{
    if (m_graphModel == model)
    {
        return;
    }

    disconnect(m_graphModelDestroyedConnection);
    disconnect(m_graphModelResetConnection);

    m_graphModel = model;

    if (m_graphModel)
    {
        m_graphModelDestroyedConnection = connect(m_graphModel, &GraphModel::destroyed, this, std::bind(&FilterAttributeModel::setGraphModel, this, nullptr));
        m_graphModelResetConnection = connect(m_graphModel, &GraphModel::modelReset, this, &FilterAttributeModel::updateGraph);
    } else
    {
        m_graphModelDestroyedConnection = {};
        m_graphModelResetConnection = {};
    }

    emit graphModelChanged();
}

void FilterAttributeModel::setSubGraphModel(SubGraphModel* model)
{
    if (m_subGraphModel == model)
    {
        return;
    }

    disconnect(m_subGraphModelDestroyedConnection);

    m_subGraphModel = model;

    if (m_subGraphModel)
    {
        m_subGraphModelDestroyedConnection = connect(m_subGraphModel, &SubGraphModel::destroyed, this, std::bind(&FilterAttributeModel::setSubGraphModel, this, nullptr));
    } else
    {
        m_subGraphModelDestroyedConnection = {};
    }

    emit subGraphModelChanged();
}

void FilterAttributeModel::updateGraph()
{
    bool found = false;
    if (m_graphModel)
    {
        const auto index = m_graphModel->indexFor(m_graphId);
        if (index.isValid())
        {
            m_graph = m_graphModel->graph(index);
            found = true;
        }
    }
    if (!found && m_subGraphModel)
    {
        m_graph = m_subGraphModel->combinedGraph(m_graphId);
        found = true;
    }
    if (!found)
    {
        m_graph = {};
    }

    emit graphChanged();
}

void FilterAttributeModel::updateModel()
{
    beginResetModel();

    constructAttributeGroups();

    endResetModel();
}

}
}
