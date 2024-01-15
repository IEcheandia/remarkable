#include "filterAttributeGroupModel.h"
#include "GraphModelVisualizer.h"
#include "attribute.h"
#include "parameterSet.h"
#include "parameter.h"

#include <QUuid>

using precitec::storage::AbstractFilterAttributeModel;
using precitec::storage::Attribute;
using precitec::storage::ParameterSet;
using precitec::storage::Parameter;
using fliplib::GraphContainer;

namespace precitec
{
namespace gui
{
namespace components
{
namespace grapheditor
{


FilterAttributeGroupModel::FilterAttributeGroupModel(QObject* parent)
    : AbstractFilterAttributeModel(parent)
    , m_parameterSet(new ParameterSet{QUuid::createUuid(), this})
{
    connect(this, &FilterAttributeGroupModel::graphModelVisualizerChanged, this, &FilterAttributeGroupModel::graphChanged);
}

FilterAttributeGroupModel::~FilterAttributeGroupModel() = default;

void FilterAttributeGroupModel::setGraphModelVisualizer(GraphModelVisualizer* graph)
{
    if (m_graphModelVisualizer == graph)
    {
        return;
    }

    disconnect(m_graphDestroyedConnection);

    m_graphModelVisualizer = graph;

    if (m_graphModelVisualizer)
    {
        m_graphDestroyedConnection = connect(m_graphModelVisualizer, &QObject::destroyed, this, std::bind(&FilterAttributeGroupModel::setGraphModelVisualizer, this, nullptr));
    } else
    {
        m_graphDestroyedConnection = {};
    }

    emit graphModelVisualizerChanged();
}

GraphContainer& FilterAttributeGroupModel::graph()
{
    if (m_graphModelVisualizer && m_graphModelVisualizer->graph())
    {
        return *m_graphModelVisualizer->graph();
    } else
    {
        return m_emptyGraph;
    }
}

void FilterAttributeGroupModel::updateModel()
{
    beginResetModel();

    m_parameterSet->clear();

    constructAttributeGroups();

    endResetModel();
}

Parameter* FilterAttributeGroupModel::getFilterParameter(const QUuid& instanceId, Attribute* attribute, const QUuid& filterId, const QVariant& defaultValue) const
{
    if (auto parameter = m_parameterSet->findById(instanceId))
    {
        return parameter;
    }

    return m_parameterSet->createParameter(instanceId, attribute, filterId, defaultValue);
}

}
}
}
}
