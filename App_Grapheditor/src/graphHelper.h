#pragma once

#include "fliplib/graphContainer.h"
#include <vector>

class QPointF;

namespace Poco
{
class UUID;
}

namespace qan
{
class Edge;
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

class FilterComment;
class FilterGroup;
class FilterMacro;
class FilterMacroConnector;
class FilterNode;
class FilterPort;

/**
 * Helper class to perform algorithms on the fliplib::GraphContainer.
 **/
class GraphHelper
{
public:
    GraphHelper(fliplib::GraphContainer *graph);

    fliplib::FilterGroup *find(FilterGroup *node);
    fliplib::FilterGroup *findGroup(int groupId);
    fliplib::InstanceFilter *find(FilterNode *node);
    fliplib::Port *find(FilterComment *node);
    fliplib::Port *find(FilterPort *node);
    fliplib::Pipe *find(qan::Edge *edge);
    fliplib::Macro *find(FilterMacro *macro);
    fliplib::Macro::Connector *find(FilterMacroConnector *macro);

    void updatePortIDs(FilterPort *port);

    void erase(FilterMacro *macro);
    void erase(FilterMacroConnector *macro);
    void erase(FilterGroup *group);
    void erase(qan::Edge *edge);

    void updatePosition(FilterNode *node, const QPointF &point);
    void updatePosition(FilterComment* comment, const QPointF &point);
    void updatePosition(FilterPort *port, const QPointF &point);
    void updatePosition(FilterMacro *port, const QPointF &point);
    void updatePosition(FilterMacroConnector *connector, const QPointF &point);

    QPointF positionToGroup(qan::Node *node, const QPointF &point) const;

private:

    template <typename T>
    void updatePositionImpl(T *node, const QPointF &point);
    fliplib::GraphContainer *m_actualGraph;

    std::vector<fliplib::FilterGroup>::iterator findInternal(FilterGroup *node);
    std::vector<fliplib::FilterGroup>::iterator findInternal(int groupId);
    std::vector<fliplib::Pipe>::iterator findInternal(qan::Edge *edge);

    void eraseGroup(int groupId);
};

}
}
}
}
