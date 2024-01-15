#include "dbServer.h"
#include "graphFunctions.h"
#include "graphModel.h"
#include "graphReferenceFunctions.h"
#include "parameter.h"
#include "parameterSet.h"
#include "product.h"
#include "productModel.h"
#include "compatibility.h"
#include "seam.h"
#include "seamInterval.h"
#include "seamSeries.h"
#include "subGraphModel.h"
#include "seamError.h"
#include "intervalError.h"
#include "productError.h"
#include "seamSeriesError.h"
#include "referenceCurve.h"
#include "referenceCurveData.h"
#include "linkedSeam.h"

#include "common/product.h"

#include <QMutexLocker>
#include <QVector2D>

#include <functional>

using namespace precitec::interface;

namespace precitec
{
namespace storage
{
using namespace compatibility;
using graphFunctions::getGraphName;
using graphFunctions::visitRefOfLinkedTarget;

static const Poco::UUID g_defaultGraphID{"F9BB3465-CFDB-4DE2-8C8D-EB974C667ACA"};

DbServer::DbServer(const std::shared_ptr<ProductModel> &products, const std::shared_ptr<GraphModel> &graphs)
    : m_products(products)
    , m_graphs(graphs)
{
}

DbServer::~DbServer() = default;

std::string DbServer::getDBInfo()
{
    return std::string("JSON based storage");
}

ProductList DbServer::getProductList(PocoUUID stationID)
{
    QMutexLocker lock{m_products->storageMutex()};
    std::vector<Product*> jsonProducts;
    jsonProducts.reserve(m_products->rowCount());
    for (int i = 0; i < m_products->rowCount(); i++)
    {
        jsonProducts.push_back(m_products->index(i, 0).data(Qt::UserRole + 1).value<Product*>());
    }
    ProductList products;
    products.reserve(jsonProducts.size());
    for (const auto &product : jsonProducts)
    {
        products.emplace_back(toPoco(product->uuid()),
                              stationID,
                              product->hardwareParameters() ? toPoco(product->hardwareParameters()->uuid()) : Poco::UUID::null(),
                              product->type(),
                              product->isEndless(),
                              int(product->triggerSource()),
                              int(product->triggerMode()),
                              product->name().toStdString(),
                              product->startPositionYAxis(),
                              product->lwmTriggerSignalType(),
                              product->isDefaultProduct());
    }
    return products;
}

namespace
{
class graphReferenceOrDefault
{
public:
    graphReferenceOrDefault(SubGraphModel* subGraphModel, Product* product)
        : m_toSingle(subGraphModel, product)
        , m_product(product){};

    template<class RefT>
    Poco::UUID operator()(const RefT& ref)
    {
        if constexpr (std::is_same_v<RefT, LinkedGraphReference>)
        {
            return visitRefOfLinkedTarget(ref, m_product, *this);
        }

        if constexpr (std::is_same_v<RefT, SubGraphReference>)
        {
            // Special handling neccessary to keep the previous behaviour.
            if (ref.value.empty())
            {
                return g_defaultGraphID;
            }
        }
        const auto& graphId = m_toSingle(ref);
        if (graphId.isNull())
        {
            return g_defaultGraphID;
        }
        return toPoco(graphId);
    }

private:
    graphFunctions::toSingleGraphReference m_toSingle;
    Product* m_product = nullptr;
};
}

Poco::UUID DbServer::graphId(AbstractMeasureTask* task)
{
    return std::visit(graphReferenceOrDefault{m_subGraphs.get(), task->product()},
                      task->graphReference());
}

MeasureTaskList DbServer::getMeasureTasks(PocoUUID stationID, PocoUUID productID)
{
    Q_UNUSED(stationID)

    QMutexLocker lock{m_products->storageMutex()};
    MeasureTaskList taskList;
    const auto &products = m_products->products();
    const QUuid qtProductId = toQt(productID);
    const auto it = std::find_if(products.begin(), products.end(), [qtProductId] (auto product) { return product->uuid() == qtProductId; });
    if (it == products.end())
    {
        return taskList;
    }
    const auto &seamSeries = (*it)->seamSeries();
    for (const auto &s : seamSeries)
    {
        taskList.emplace_back(toPoco(s->uuid()),
                              graphId(s),
                              s->number(),
                              0,                               // seam number
                              0,                               // seam interval number
                              0,                               // level
                              0,                               // trigger delta
                              0,                               // length
                              0,                               // velocity
                              0,                               // start
                              0,                               // number triggers
                              getGraphName(s, m_graphs.get()), // graph name
                              s->name().toStdString(),         // name
                              toPoco(s->graphParamSet()),      // filter parameters
                              s->hardwareParameters() ? toPoco(s->hardwareParameters()->uuid()) : Poco::UUID::null(),
                              0, // moveDirection
                              0, // thicknessLeft,
                              0, // thicknessRight,
                              0, // targetDifference
                              0, // roiX
                              0, // roiY
                              0, // roiW
                              0, // roiH
                              0  // interval level
        );
        for (auto seam : s->seams())
        {
            if (seam->metaObject()->inherits(&LinkedSeam::staticMetaObject))
            {
                continue;
            }
            taskList.emplace_back(toPoco(seam->uuid()), // measure task id
                                  graphId(seam),
                                  s->number(),                        // seam series number
                                  seam->number(),                     // seam number
                                  0,                                  // seam interval number
                                  1,                                  // level
                                  seam->triggerDelta(),               // trigger delta
                                  0,                                  // length
                                  seam->velocity(),                   // velocity
                                  0,                                  // start
                                  0,                                  // number triggers
                                  getGraphName(seam, m_graphs.get()), // graph name
                                  seam->name().toStdString(),         // name
                                  toPoco(seam->graphParamSet()),      // filter parameters
                                  seam->hardwareParameters() ? toPoco(seam->hardwareParameters()->uuid()) : Poco::UUID::null(),
                                  seam->moveDirection(),
                                  seam->thicknessLeft(),
                                  seam->thicknessRight(),
                                  seam->targetDifference(),
                                  seam->roi().x(),
                                  seam->roi().y(),
                                  seam->roi().width(),
                                  seam->roi().height(),
                                  0 // interval level
            );
            for (auto interval : seam->seamIntervals())
            {
                taskList.emplace_back(toPoco(interval->uuid()), // measure task id
                                      graphId(seam),
                                      s->number(),                        // seam series number
                                      seam->number(),                     // seam number
                                      interval->number(),                 // seam interval number
                                      2,                                  // level
                                      seam->triggerDelta(),               // trigger delta
                                      interval->length(),                 // length
                                      seam->velocity(),                   // velocity
                                      0,                                  // start
                                      0,                                  // number triggers
                                      getGraphName(seam, m_graphs.get()), // graph name
                                      interval->name().toStdString(),     // name
                                      toPoco(seam->graphParamSet()),      // filter parameters
                                      Poco::UUID::null(),
                                      seam->moveDirection(),
                                      seam->thicknessLeft(),
                                      seam->thicknessRight(),
                                      seam->targetDifference(),
                                      0, // roiX
                                      0, // roiY
                                      0, // roiW
                                      0, // roiH
                                      interval->level());
            }
        }
    }

    return taskList;
}

namespace
{
precitec::interface::GraphList toGraphList(const fliplib::GraphContainer &graph)
{
    precitec::interface::Graph retGraph{graph.id};
    // path components seem not to be used, so not passing around

    std::vector<precitec::interface::Component> components;
    for (const auto &description : graph.filterDescriptions)
    {
        if (std::any_of(components.begin(), components.end(), [&description] (const auto &component) { return component.id() == description.componentId; }))
        {
            continue;
        }
        components.emplace_back(description.componentId, description.component);
    }
    retGraph.setComponents(std::move(components));

    std::vector<precitec::interface::Filter> instanceFilters;
    for (const auto &filter : graph.instanceFilters)
    {
        auto it = std::find_if(graph.filterDescriptions.cbegin(), graph.filterDescriptions.cend(), [&filter] (const auto &description) { return filter.filterId == description.id; });
        if (it == graph.filterDescriptions.cend())
        {
            continue;
        }
        instanceFilters.emplace_back(filter.id, (*it).name, (*it).componentId);
        // out pipes are not used
        std::vector<precitec::interface::InPipe> inPipes;
        for (const auto &pipe : graph.pipes)
        {
            if (pipe.receiver != filter.id)
            {
                continue;
            }
            inPipes.emplace_back(pipe.sender, pipe.senderConnectorName, pipe.receiverConnectorGroup, pipe.receiverConnectorName);
        }
        instanceFilters.back().setInPipeList(std::move(inPipes));
        std::vector<std::shared_ptr<precitec::interface::FilterParameter>> parameters;
        parameters.reserve(filter.attributes.size());

        for (const auto &parameter : filter.attributes)
        {
            parameters.emplace_back(std::make_shared<precitec::interface::TFilterParameter<std::string>>(Poco::UUID(), parameter.name, parameter.value, Poco::UUID(), Poco::UUID()));
        }

        instanceFilters.back().setParameterList(std::move(parameters));
    }
    retGraph.setFilters(std::move(instanceFilters));

    return GraphList{retGraph};
}

GraphList graphListFromSubGraphs(const std::vector<QUuid>& subGraphs,
                                 SubGraphModel* subGraphModel,
                                 const Poco::UUID& requestedGraphID,
                                 const Poco::UUID& graphID)
{
    if (subGraphs.empty())
    {
        wmLog(eWarning, "Requested graph %s does not exist, providing default graph instead.\n", graphID.toString().c_str());
        return {{g_defaultGraphID}};
    }
    // verify that the sub graph matches our requested graphId
    if (requestedGraphID != graphID)
    {
        return {{g_defaultGraphID}};
    }

    if (!subGraphModel)
    {
        return {{g_defaultGraphID}};
    }

    return toGraphList(subGraphModel->getOrCreateCombinedGraph(subGraphs, graphID));
}

struct toGraphListRecursive
{
    GraphList operator()(const SingleGraphReference&)
    {
        return GraphList{{g_defaultGraphID}};
    }
    GraphList operator()(const SubGraphReference& ref)
    {
        return graphListFromSubGraphs(ref.value, subGraphModel, requestedGraphID, graphID);
    }
    GraphList operator()(const LinkedGraphReference& ref)
    {
        return visitRefOfLinkedTarget(ref, m_product, *this);
    }

    Product* m_product = nullptr;
    SubGraphModel* subGraphModel = nullptr;
    const Poco::UUID& requestedGraphID;
    const Poco::UUID& graphID;
};
}

GraphList DbServer::getGraph(PocoUUID measureTaskID, PocoUUID graphID)
{
    if (graphID == g_defaultGraphID)
    {
        return {{g_defaultGraphID}};
    }
    const auto graphIndex = m_graphs->indexFor(toQt(graphID));
    if (!graphIndex.isValid())
    {
        // check whether we have a sub graph
        QMutexLocker lock{m_products->storageMutex()};
        auto measureTask = m_products->findMeasureTask(toQt(measureTaskID));
        if (!measureTask)
        {
            return {{g_defaultGraphID}};
        }

        return std::visit(toGraphListRecursive{measureTask->product(), m_subGraphs.get(), graphId(measureTask), graphID},
                          measureTask->graphReference());
    }
    return toGraphList(m_graphs->graph(graphIndex));
}

ParameterList DbServer::getParameterSatz(PocoUUID parametersatzID, PocoUUID filterInstanceID)
{
    // not used
    Q_UNUSED(parametersatzID)
    Q_UNUSED(filterInstanceID)
    return ParameterList{};
}


namespace
{
ParameterList toParameterList(const std::vector<Parameter*> &parameters)
{
    ParameterList ret;
    ret.reserve(parameters.size());
    for (const auto parameter : parameters)
    {
        auto fp = parameter->toFilterParameter();
        if (fp)
        {
            ret.emplace_back(std::move(fp));
        }
    }
    return ret;
}
}
ParameterList DbServer::getProductParameter(PocoUUID productID)
{
    // required for sum errors
    QMutexLocker lock{m_products->storageMutex()};
    const auto &products = m_products->products();
    const QUuid qtProductId = toQt(productID);
    const auto it = std::find_if(products.begin(), products.end(), [qtProductId] (auto product) { return product->uuid() == qtProductId; });
    if (it == products.end())
    {
        return ParameterList{};
    }

    const auto &productErrors = (*it)->overlyingErrors();
    const auto &seamSeriesErrors = (*it)->allSeamSeriesErrors();
    const auto &seamErrors = (*it)->allSeamErrors();
    ParameterList ret;
    ret.reserve((productErrors.size() + seamSeriesErrors.size() + seamErrors.size() + (*it)->intervalErrorCount()) * 13);
    for (auto error : productErrors)
    {
        const auto parameters = error->toParameterList();
        std::copy(parameters.begin(), parameters.end(), std::back_inserter(ret));
    }
    for (auto error : seamSeriesErrors)
    {
        const auto parameters = error->toParameterList();
        std::copy(parameters.begin(), parameters.end(), std::back_inserter(ret));
    }
    for (auto error : seamErrors)
    {
        const auto parameters = error->toParameterList();
        std::copy(parameters.begin(), parameters.end(), std::back_inserter(ret));
    }

    const auto &intervalErrors = (*it)->allIntervalErrors();
    for (auto error : intervalErrors)
    {
        const auto parameters = error->toParameterList();
        std::copy(parameters.begin(), parameters.end(), std::back_inserter(ret));
    }

    return ret;
}

ParameterList DbServer::getFilterParameter(PocoUUID filterID, PocoUUID measureTaskID)
{
    QMutexLocker lock{m_products->storageMutex()};
    const auto &products = m_products->products();
    AbstractMeasureTask *measureTask = nullptr;
    const auto qtMeasureTaskID = toQt(measureTaskID);
    Product *product = nullptr;
    for (auto p : products)
    {
        if (auto task = p->findMeasureTask(qtMeasureTaskID))
        {
            measureTask = task;
            product = p;
            break;
        }
    }
    if (!measureTask)
    {
        return {};
    }
    auto ps = product->filterParameterSet(measureTask->graphParamSet());
    if (!ps)
    {
        return {};
    }
    std::vector<Parameter*> parameters;
    const auto qtFilterId = toQt(filterID);
    std::copy_if(ps->parameters().begin(), ps->parameters().end(), std::back_inserter(parameters),
        [qtFilterId] (Parameter *parameter)
        {
            return parameter->filterId() == qtFilterId;
        }
    );
    return toParameterList(parameters);
}

ParameterList DbServer::getHardwareParameterSatz(PocoUUID id)
{
    QMutexLocker lock{m_products->storageMutex()};
    const QUuid qtId = toQt(id);
    const auto &products = m_products->products();
    ParameterSet *parameterSet = nullptr;
    for (auto product : products)
    {
        if (auto hw = product->findHardwareParameterSet(qtId))
        {
            parameterSet = hw;
            break;
        }
    }
    if (!parameterSet)
    {
        return ParameterList{};
    }
    return toParameterList(parameterSet->parameters());
}

ParameterList DbServer::getHardwareParameter(PocoUUID parametersatzID, interface::Key key)
{
    // only needed if we use DbNotificationServer::setupHardwareParameter
    Q_UNUSED(parametersatzID)
    Q_UNUSED(key)
    return ParameterList{};
}

Product1dParameter DbServer::getEinsDParameter(PocoUUID id)
{
    QMutexLocker lock{m_products->storageMutex()};

    const auto &products = m_products->products();
    const QUuid productId = toQt(id);

    const auto it = std::find_if(products.begin(), products.end(), [&productId] (auto &p) { return p->uuid() == productId;});

    auto product1dParameter = Product1dParameter{id};

    if (it != products.end())
    {
        const auto productCurves = (*it)->allReferenceCurves();

        for (auto curve : productCurves)
        {
            int seamNumber;
            int seamSeriesNumber;

            if (auto seam = qobject_cast<Seam*>(curve->measureTask()))
            {
                seamNumber = seam->number();
                seamSeriesNumber = seam->seamSeries()->number();
            } else
            {
                continue;
            }

            auto upperParameter = Seam1dParameter{1, seamNumber, seamSeriesNumber, toPoco(curve->upper())};
            if (auto upperData = (*it)->referenceCurveData(curve->upper()))
            {
                const auto& samples = upperData->samples();
                std::transform(samples.begin(), samples.end(), std::back_inserter(upperParameter.m_oSeamCurve), [] (const auto& sample) { return PosVal{int(1000.0f * sample.x()), sample.y()}; });
            }
            product1dParameter.m_oSeamCurves.push_back(upperParameter);

            auto middlParameter = Seam1dParameter{1, seamNumber, seamSeriesNumber, toPoco(curve->middle())};
            if (auto middleData = (*it)->referenceCurveData(curve->middle()))
            {
                const auto& samples = middleData->samples();
                std::transform(samples.begin(), samples.end(), std::back_inserter(middlParameter.m_oSeamCurve), [] (const auto& sample) { return PosVal{int(1000.0f * sample.x()), sample.y()}; });
            }
            product1dParameter.m_oSeamCurves.push_back(middlParameter);

            auto lowerParameter = Seam1dParameter{1, seamNumber, seamSeriesNumber, toPoco(curve->lower())};
            if (auto lowerData = (*it)->referenceCurveData(curve->lower()))
            {
                const auto& samples = lowerData->samples();
                std::transform(samples.begin(), samples.end(), std::back_inserter(lowerParameter.m_oSeamCurve), [] (const auto& sample) { return PosVal{int(1000.0f * sample.x()), sample.y()}; });
            }
            product1dParameter.m_oSeamCurves.push_back(lowerParameter);
        }
    }

    return product1dParameter;
}

ProductCurves DbServer::getProductCurves(PocoUUID id)
{
    QMutexLocker lock{m_products->storageMutex()};

    const auto &products = m_products->products();
    const QUuid productId = toQt(id);

    const auto it = std::find_if(products.begin(), products.end(), [&productId] (auto &p) { return p->uuid() == productId;});

    auto productCurves = ProductCurves{};

    if (it != products.end())
    {
        const auto& referenceCurves = (*it)->allReferenceCurves();

        for (auto curve : referenceCurves)
        {
            if (!curve->used())
            {
                continue;
            }

            if (!qobject_cast<Seam*>(curve->measureTask()))
            {
                continue;
            }

            productCurves.m_curveSets.emplace_back(toPoco(curve->uuid()));
        }
    }

    return productCurves;
}

ReferenceCurveSet DbServer::getReferenceCurveSet(PocoUUID productId, PocoUUID referenceId)
{
    QMutexLocker lock{m_products->storageMutex()};

    const auto& products = m_products->products();
    const auto& pId = toQt(productId);

    const auto product_it = std::find_if(products.begin(), products.end(), [&pId] (auto p) { return p->uuid() == pId;});

    if (product_it != products.end())
    {
        const auto& referenceCurves = (*product_it)->allReferenceCurves();

        const auto& rId = toQt(referenceId);
        const auto reference_it = std::find_if(referenceCurves.begin(), referenceCurves.end(), [&rId] (auto r) { return r->uuid() == rId;});

        if (reference_it != referenceCurves.end())
        {
            auto curve = (*reference_it);

            quint32 seamNumber;
            quint32 seamSeriesNumber;

            if (auto seam = qobject_cast<Seam*>(curve->measureTask()))
            {
                seamNumber = seam->number();
                seamSeriesNumber = seam->seamSeries()->number();
            } else
            {
                return {};
            }

            ReferenceCurveSet referenceCurveSet{curve->resultType(), seamNumber, seamSeriesNumber, toPoco(curve->uuid())};

            referenceCurveSet.m_upper.m_uuid = toPoco(curve->upper());
            if (auto upperData = (*product_it)->referenceCurveData(curve->upper()))
            {
                const auto& samples = upperData->samples();
                std::transform(samples.begin(), samples.end(), std::back_inserter(referenceCurveSet.m_upper.m_curve), [] (const auto& sample) { return geo2d::DPoint{sample.x(), sample.y()}; });
            }

            referenceCurveSet.m_middle.m_uuid = toPoco(curve->middle());
            if (auto middleData = (*product_it)->referenceCurveData(curve->middle()))
            {
                const auto& samples = middleData->samples();
                std::transform(samples.begin(), samples.end(), std::back_inserter(referenceCurveSet.m_middle.m_curve), [] (const auto& sample) { return geo2d::DPoint{sample.x(), sample.y()}; });
            }

            referenceCurveSet.m_lower.m_uuid = toPoco(curve->lower());
            if (auto lowerData = (*product_it)->referenceCurveData(curve->lower()))
            {
                const auto& samples = lowerData->samples();
                std::transform(samples.begin(), samples.end(), std::back_inserter(referenceCurveSet.m_lower.m_curve), [] (const auto& sample) { return geo2d::DPoint{sample.x(), sample.y()}; });
            }

            return referenceCurveSet;
        }
    }

    return {};
}

FilterParametersList DbServer::getParameterSatzForAllFilters(PocoUUID parametersatzID)
{
    QMutexLocker lock{m_products->storageMutex()};
    auto ps = m_products->findFilterParameterSet(toQt(parametersatzID));
    if (!ps)
    {
        return FilterParametersList{};
    }

    std::map<QUuid, std::vector<Parameter*>> parameters;
    for (auto parameter : ps->parameters())
    {
        auto it = parameters.find(parameter->filterId());
        if (it != parameters.end())
        {
            it->second.push_back(parameter);
        } else
        {
            parameters.emplace(parameter->filterId(), std::vector<Parameter*>{parameter});
        }
    }
    FilterParametersList ret;
    for (auto pair: parameters)
    {
        ret.emplace_back(toPoco(pair.first), toParameterList(pair.second));
    }

    return ret;
}

}
}
