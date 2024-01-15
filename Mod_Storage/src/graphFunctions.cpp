#include "graphFunctions.h"

#include "abstractMeasureTask.h"
#include "graphModel.h"
#include "graphReferenceFunctions.h"
#include "product.h"
#include "seam.h"
#include "subGraphModel.h"

#include "fliplib/graphContainer.h"

namespace precitec::storage::graphFunctions
{

namespace
{
class toName
{
public:
    std::string operator()(const SingleGraphReference& ref)
    {
        return model->name(ref.value).toStdString();
    }

    std::string operator()(const SubGraphReference& ref)
    {
        static const std::string defaultName(" - ");
        return defaultName;
    }

    std::string operator()(const LinkedGraphReference& ref)
    {
        return visitRefOfLinkedTarget(ref, m_product, *this);
    }

    GraphModel* model = nullptr;
    Product* m_product = nullptr;
};
}

toSingleGraphReference::toSingleGraphReference(SubGraphModel* subGraphModel, Product* product)
    : m_subGraphModel(subGraphModel)
    , m_product(product)
{
}

QUuid toSingleGraphReference::operator()(SingleGraphReference const& ref)
{
    return ref.value;
}

QUuid toSingleGraphReference::operator()(SubGraphReference const& ref)
{
    if (!m_subGraphModel)
    {
        return {};
    }
    return m_subGraphModel->generateGraphId(ref.value);
}

QUuid toSingleGraphReference::operator()(LinkedGraphReference const& ref)
{
    return visitRefOfLinkedTarget(ref, m_product, *this);
}

class toGraphContainer
{
public:
    explicit toGraphContainer(GraphModel* graphModel, SubGraphModel* subGraphModel, Product* product);

    [[nodiscard]] fliplib::GraphContainer operator()(const SingleGraphReference& ref);
    [[nodiscard]] fliplib::GraphContainer operator()(const SubGraphReference& ref);
    [[nodiscard]] fliplib::GraphContainer operator()(const LinkedGraphReference& ref);

private:
    GraphModel* m_graphModel = nullptr;
    SubGraphModel* m_subGraphModel = nullptr;

    toSingleGraphReference toSingleRef;
};

toGraphContainer::toGraphContainer(GraphModel* graphModel, SubGraphModel* subGraphModel, Product* product)
    : m_graphModel(graphModel)
    , m_subGraphModel(subGraphModel)
    , toSingleRef(subGraphModel, product)
{
}

fliplib::GraphContainer toGraphContainer::operator()(const SingleGraphReference& ref)
{
    if (!m_graphModel)
    {
        return {};
    }
    const auto index = m_graphModel->indexFor(toSingleRef(ref));
    if (!index.isValid())
    {
        return {};
    }
    return m_graphModel->graph(index);
}

fliplib::GraphContainer toGraphContainer::operator()(const SubGraphReference& ref)
{
    if (!m_subGraphModel)
    {
        return {};
    }
    return m_subGraphModel->combinedGraph(toSingleRef(ref));
}

fliplib::GraphContainer toGraphContainer::operator()(const LinkedGraphReference& ref)
{
    return {};
}

QUuid getCurrentGraphId(AbstractMeasureTask* task,
                        SubGraphModel* subGraphModel)
{
    if (!task)
    {
        return {};
    }
    return std::visit(toSingleGraphReference{subGraphModel, task->product()}, task->graphReference());
}

std::string getGraphName(AbstractMeasureTask* task, GraphModel* model)
{

    return std::visit(toName{model, task->product()}, task->graphReference());
}

bool hasMatchingModel(AbstractMeasureTask* task,
                      GraphModel* graphModel,
                      SubGraphModel* subGraphModel)
{
    return std::visit(overloaded{[&](const SingleGraphReference& ref)
                                 {
                                     return graphModel != nullptr;
                                 },
                                 [&](const SubGraphReference& ref)
                                 {
                                     return subGraphModel != nullptr;
                                 },
                                 [&](const LinkedGraphReference& ref)
                                 {
                                     return false;
                                 }},
                      task->graphReference());
}

fliplib::GraphContainer getGraphFromModel(AbstractMeasureTask* task, GraphModel* graphModel, SubGraphModel* subGraphModel)
{
    if (!task)
    {
        return {};
    }
    return std::visit(toGraphContainer{graphModel, subGraphModel, task->product()}, task->graphReference());
}
}
