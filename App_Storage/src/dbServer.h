#pragma once

#include "fliplib/graphContainer.h"
#include "message/db.interface.h"

#include <map>

#include <QByteArray>
#include <QUuid>

namespace precitec
{
namespace storage
{
class AbstractMeasureTask;
class GraphModel;
class ProductModel;
class SubGraphModel;
class ParameterSet;

class DbServer : public precitec::interface::TDb<precitec::interface::AbstractInterface>
{
public:
    explicit DbServer(const std::shared_ptr<ProductModel> &products, const std::shared_ptr<GraphModel> &graphs);
    ~DbServer() override;

    std::string getDBInfo() override;
    interface::ProductList getProductList(PocoUUID stationID) override;
    interface::MeasureTaskList getMeasureTasks(PocoUUID stationID, PocoUUID productID) override;
    interface::GraphList getGraph(PocoUUID measureTaskID, PocoUUID graphID) override;
    interface::ParameterList getFilterParameter(PocoUUID filterID, PocoUUID measureTaskID) override;
    interface::ParameterList getParameterSatz(PocoUUID parametersatzID, PocoUUID filterInstanceID) override;
    interface::ParameterList getProductParameter(PocoUUID productID) override;
    interface::ParameterList getHardwareParameterSatz(PocoUUID id) override;
    interface::ParameterList getHardwareParameter(PocoUUID parametersatzID, interface::Key key) override;
    interface::Product1dParameter getEinsDParameter(PocoUUID id) override;
    interface::FilterParametersList getParameterSatzForAllFilters(PocoUUID parametersatzID) override;
    interface::ProductCurves getProductCurves(PocoUUID id) override;
    interface::ReferenceCurveSet getReferenceCurveSet(PocoUUID productId, PocoUUID referenceId) override;

    void setSubGraphModel(const std::shared_ptr<SubGraphModel> &model)
    {
        m_subGraphs = model;
    }

private:
    Poco::UUID graphId(AbstractMeasureTask *task);
    std::shared_ptr<ProductModel> m_products;
    std::shared_ptr<GraphModel> m_graphs;
    std::shared_ptr<SubGraphModel> m_subGraphs;
};

}
}
