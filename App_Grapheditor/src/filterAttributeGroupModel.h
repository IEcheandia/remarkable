#pragma once

#include "abstractFilterAttributeModel.h"

namespace precitec
{
namespace storage
{

class Attribute;
class Parameter;
class ParameterSet;

}
namespace gui
{
namespace components
{
namespace grapheditor
{

class GraphModelVisualizer;

/**
 * @brief Model to display Attribute groups
 *
 * This is a small model which functions as the data holder for the
 * @link{AttributeGroups}s, contained in a filter instance.
 * Attributes, which belong to no group, are represented as a group
 * with a single element.
 **/

class FilterAttributeGroupModel : public precitec::storage::AbstractFilterAttributeModel
{
    Q_OBJECT

    /**
     * GraphModelVisualizer
     * Contains a fliplib::GraphContainer with the actual graph
     **/
    Q_PROPERTY(precitec::gui::components::grapheditor::GraphModelVisualizer* graphModelVisualizer READ graphModelVisualizer WRITE setGraphModelVisualizer NOTIFY graphModelVisualizerChanged)

public:
    explicit FilterAttributeGroupModel(QObject* parent = nullptr);
    ~FilterAttributeGroupModel() override;

    precitec::gui::components::grapheditor::GraphModelVisualizer* graphModelVisualizer() const
    {
        return m_graphModelVisualizer;
    }
    void setGraphModelVisualizer(precitec::gui::components::grapheditor::GraphModelVisualizer* graph);

    Q_INVOKABLE precitec::storage::Parameter* getFilterParameter(const QUuid& instanceId, precitec::storage::Attribute* attribute, const QUuid& filterId, const QVariant& defaultValue = {}) const;

Q_SIGNALS:
    void graphModelVisualizerChanged();

private:
    fliplib::GraphContainer& graph() override;
    void updateModel() override;

    precitec::gui::components::grapheditor::GraphModelVisualizer* m_graphModelVisualizer = nullptr;
    QMetaObject::Connection m_graphDestroyedConnection;

    // fallback return value, in case the GraphModelVisualizer has not yet been set
    fliplib::GraphContainer m_emptyGraph = {};

    precitec::storage::ParameterSet* m_parameterSet;
};

}
}
}
}
Q_DECLARE_METATYPE(precitec::gui::components::grapheditor::FilterAttributeGroupModel*)
