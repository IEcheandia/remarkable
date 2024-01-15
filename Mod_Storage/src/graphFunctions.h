#pragma once

#include <string>

class QUuid;

namespace fliplib
{
struct GraphContainer;
}

namespace precitec::storage
{
class AbstractMeasureTask;
class GraphModel;
class Product;
class SubGraphModel;

struct SingleGraphReference;
struct SubGraphReference;
struct LinkedGraphReference;

namespace graphFunctions
{
class toSingleGraphReference
{
public:
    explicit toSingleGraphReference(SubGraphModel* subGraphModel, Product* product);

    [[nodiscard]] QUuid operator()(const SingleGraphReference& ref);
    [[nodiscard]] QUuid operator()(const SubGraphReference& ref);
    [[nodiscard]] QUuid operator()(const LinkedGraphReference& ref);

private:
    SubGraphModel* m_subGraphModel = nullptr;
    Product* m_product = nullptr;
};

/**
 * @brief Obtains a single unique identifier for a graph from the task.
 *
 * @param subGraphMode may be used to convert Ids of subGraphs to a single Id.
 */
[[nodiscard]] QUuid getCurrentGraphId(AbstractMeasureTask* task,
                                      SubGraphModel* subGraphModel);

/**
 * @brief Obtains the name of the single graph from the model or returns a default when subgraphs are used.
 */
[[nodiscard]] std::string getGraphName(AbstractMeasureTask* task, GraphModel* model);

/**
 * @brief checks that that model is not nullptr which the task needs to get a graph.
 */
[[nodiscard]] bool hasMatchingModel(AbstractMeasureTask* task,
                                    GraphModel* graphModel,
                                    SubGraphModel* subGraphModel);

/**
 * @brief Takes the graph reference of the given task and obtains the appropriate GraphContainer from the given models.
 */
[[nodiscard]] fliplib::GraphContainer getGraphFromModel(AbstractMeasureTask* task,
                                                        GraphModel* graphModel,
                                                        SubGraphModel* subGraphModel);
}
}