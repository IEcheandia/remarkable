#include <QTest>
#include <QSignalSpy>

#include "../src/compatibility.h"
#include "../src/dbServer.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "seamInterval.h"
#include "productModel.h"
#include "graphModel.h"
#include "subGraphModel.h"

#include "common/product.h"

using precitec::storage::DbServer;
using precitec::storage::GraphModel;
using precitec::storage::ProductModel;
using precitec::storage::LinkedGraphReference;
using precitec::storage::compatibility::toPoco;
using precitec::storage::SubGraphModel;

class TestDbServer : public QObject
{
    Q_OBJECT
private:
    inline static const Poco::UUID defaultGraphId{"F9BB3465-CFDB-4DE2-8C8D-EB974C667ACA"};

    void SetUpMeasureTasksTest(DbServer& server)
    {
        auto model = std::make_shared<ProductModel>();
        auto graphModel = std::make_shared<GraphModel>();
        auto subGraphModel = std::make_shared<SubGraphModel>();
        server = DbServer(model, graphModel);
        server.setSubGraphModel(subGraphModel);
        QDir dir(QFINDTESTDATA("testdata/products/"));
        QVERIFY(dir.exists());
        model->loadProducts(dir);

        QDir graphDir(QFINDTESTDATA("testdata/graphs/"));
        QVERIFY(graphDir.exists());
        QSignalSpy loadingChangedSpy(graphModel.get(), &GraphModel::loadingChanged);
        QVERIFY(loadingChangedSpy.isValid());
        graphModel->loadGraphs(graphDir.absolutePath());
        QVERIFY(loadingChangedSpy.wait());
        QTRY_COMPARE(graphModel->isLoading(), false);
    }

private Q_SLOTS:
    void testGetDbInfo();
    void testGetProductList();
    void testGetGraph();
    void testGetSubGraphWithOneGraph();
    void testGetSubGraph();
    void testGetHardwareParameters();
    void testGetMeasureTasks();
    void testGetMeasureTasksFromLinkedGraph();
    void testGetFilterParameter();
};

void TestDbServer::testGetDbInfo()
{
    auto model = std::make_shared<ProductModel>();
    auto graphModel = std::make_shared<GraphModel>();
    DbServer server(model, graphModel);
    QCOMPARE(server.getDBInfo(), std::string("JSON based storage"));
}

void TestDbServer::testGetProductList()
{
    auto model = std::make_shared<ProductModel>();
    auto graphModel = std::make_shared<GraphModel>();
    DbServer server(model, graphModel);
    // no data loaded yet -> empty list
    QVERIFY(server.getProductList(Poco::UUID()).empty());
    // load products
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);

    auto productList = server.getProductList(Poco::UUID());
    QCOMPARE(productList.size(), 3u);
    const auto &product1 = productList.at(0);
    QCOMPARE(product1.productID(), Poco::UUID("1F086211-FBD4-4493-A580-6FF11E4925DD"));
    QCOMPARE(product1.hwParameterSatzID(), Poco::UUID::null());
    QCOMPARE(product1.name(), std::string("Product 1"));
    QCOMPARE(product1.endless(), false);
    QCOMPARE(product1.productType(), 1);

    const auto &product2 = productList.at(1);
    QCOMPARE(product2.productID(), Poco::UUID("2F086211-FBD4-4493-A580-6FF11E4925DE"));
    QCOMPARE(product2.hwParameterSatzID(), Poco::UUID("3F086211-FBD4-4493-A580-6FF11E4925DF"));
    QCOMPARE(product2.name(), std::string("Product 2"));
    QCOMPARE(product2.endless(), true);
    QCOMPARE(product2.productType(), 2);

    const auto& product3 = productList.at(2);
    QCOMPARE(product3.productID(), Poco::UUID("62f4ed0c-5d43-42ca-94cd-ec300f8377fa"));
}

void TestDbServer::testGetGraph()
{
    auto model = std::make_shared<ProductModel>();
    auto graphModel = std::make_shared<GraphModel>();
    DbServer server(model, graphModel);
    QCOMPARE(server.getGraph(Poco::UUID(), Poco::UUID("3F086211-FBD4-4493-A580-6FF11E4925DF")).empty(), false);
    QCOMPARE(server.getGraph(Poco::UUID(), Poco::UUID("3F086211-FBD4-4493-A580-6FF11E4925DF")).front().id(), defaultGraphId);
    // default graph
    QCOMPARE(server.getGraph(Poco::UUID(), defaultGraphId).empty(), false);

    // load a graph
    graphModel->loadGraphs(QFINDTESTDATA("testdata/graphs"));
    QTRY_COMPARE(graphModel->isLoading(), false);
    QCOMPARE(graphModel->rowCount(), 3);

    const auto graphList = server.getGraph(Poco::UUID(), Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QCOMPARE(graphList.size(), 1);
    const auto &graph = graphList.front();
    QCOMPARE(graph.id(), Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QCOMPARE(graph.pathComponents(), std::string());
    QCOMPARE(graph.components().size(), 1);
    QCOMPARE(graph.components().front().id(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(graph.components().front().filename(), std::string("Test_Component"));

    QCOMPARE(graph.filters().size(), 2);
    auto it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->name(), std::string("precitec::filter::TestFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->inPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 1);
    QCOMPARE(it->parameterList().front()->any(), Poco::DynamicAny(1));
    QCOMPARE(it->parameterList().front()->name(), std::string("An attribute"));

    // second filter
    it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("5599b861-7222-4af9-8bf5-b7a116a6ab1f"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("5599b861-7222-4af9-8bf5-b7a116a6ab1f"));
    QCOMPARE(it->name(), std::string("precitec::filter::AnotherFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 0);
    QCOMPARE(it->inPipeList().size(), 1);
    QCOMPARE(it->inPipeList().front().group(), 0);
    QCOMPARE(it->inPipeList().front().sender(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->inPipeList().front().name(), std::string("Image"));
    QCOMPARE(it->inPipeList().front().tag(), std::string("Foo"));
}

void TestDbServer::testGetSubGraphWithOneGraph()
{
    // create a product with a seam interval with sub graph
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    precitec::storage::Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    product.setFilePath(tempDir.filePath(QStringLiteral("testProduct.json")));
    auto seam = product.seamSeries().front()->createSeam();
    seam->setSubGraphs({QByteArrayLiteral("e58abf42-77a6-4456-9f78-56e002b38549")});
    product.save();

    auto model = std::make_shared<ProductModel>();
    model->loadProducts(tempDir.path());
    QCOMPARE(model->rowCount(), 1);

    auto graphModel = std::make_shared<SubGraphModel>();
    graphModel->loadSubGraphs(QFINDTESTDATA("testdata/graphs"));
    QTRY_COMPARE(graphModel->isLoading(), false);
    QCOMPARE(graphModel->rowCount(), 3);

    DbServer server(model, std::make_shared<GraphModel>());
    server.setSubGraphModel(graphModel);
    // we need to get the correct graph id
    auto tasks = server.getMeasureTasks(Poco::UUID(), toPoco(product.uuid()));
    const auto &graphId = tasks.at(2).graphID();
    QCOMPARE(tasks.size(), 3);
    QCOMPARE(tasks.at(2).taskID(), toPoco(seam->firstSeamInterval()->uuid()));
    QCOMPARE(graphId.isNull(), false);
    QVERIFY(graphId != Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));

    // now get the graph
    const auto graphList = server.getGraph(tasks.at(2).taskID(), graphId);
    QCOMPARE(graphList.size(), 1);
    const auto &graph = graphList.front();
    QCOMPARE(graph.id(), graphId);
    QCOMPARE(graph.pathComponents(), std::string());
    QCOMPARE(graph.components().size(), 1);
    QCOMPARE(graph.components().front().id(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(graph.components().front().filename(), std::string("Test_Component"));

    QCOMPARE(graph.filters().size(), 2);
    auto it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->name(), std::string("precitec::filter::TestFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->inPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 1);
    QCOMPARE(it->parameterList().front()->any(), Poco::DynamicAny(1));
    QCOMPARE(it->parameterList().front()->name(), std::string("An attribute"));

    // second filter
    it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("5599b861-7222-4af9-8bf5-b7a116a6ab1f"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("5599b861-7222-4af9-8bf5-b7a116a6ab1f"));
    QCOMPARE(it->name(), std::string("precitec::filter::AnotherFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 0);
    QCOMPARE(it->inPipeList().size(), 1);
    QCOMPARE(it->inPipeList().front().group(), 0);
    QCOMPARE(it->inPipeList().front().sender(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->inPipeList().front().name(), std::string("Image"));
    QCOMPARE(it->inPipeList().front().tag(), std::string("Foo"));
}

void TestDbServer::testGetSubGraph()
{
    // create a product with a seam interval with sub graph
    QTemporaryDir tempDir;
    QVERIFY(tempDir.isValid());
    precitec::storage::Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    product.setFilePath(tempDir.filePath(QStringLiteral("testProduct.json")));
    auto seam = product.seamSeries().front()->createSeam();
    // note: third graph doesn't exist
    seam->setSubGraphs({QByteArrayLiteral("e58abf42-77a6-4456-9f78-56e002b38549"), QByteArrayLiteral("0c3ebf5b-df35-4ac6-9b79-085cba2aba4f"), QByteArrayLiteral("92e23930-3c1c-44c2-b8ad-1053b7ae2ba6")});
    auto linked = product.seamSeries().front()->createSeam();
    linked->setGraphReference(LinkedGraphReference{seam->uuid()});
    product.save();

    auto model = std::make_shared<ProductModel>();
    model->loadProducts(tempDir.path());
    QCOMPARE(model->rowCount(), 1);

    auto graphModel = std::make_shared<SubGraphModel>();
    graphModel->loadSubGraphs(QFINDTESTDATA("testdata/subgraphs"));
    QTRY_COMPARE(graphModel->isLoading(), false);
    QCOMPARE(graphModel->rowCount(), 2);
    QCOMPARE(graphModel->category(graphModel->index(0, 0)), QStringLiteral("subgraphs"));
    QCOMPARE(graphModel->category(graphModel->index(1, 0)), QStringLiteral("subgraphs"));
    QCOMPARE(graphModel->category(graphModel->index(2, 0)), QString());

    DbServer server(model, std::make_shared<GraphModel>());
    server.setSubGraphModel(graphModel);
    // we need to get the correct graph id
    auto tasks = server.getMeasureTasks(Poco::UUID(), toPoco(product.uuid()));
    const auto &graphId = tasks.at(2).graphID();
    QCOMPARE(tasks.size(), 5);
    QCOMPARE(tasks.at(2).taskID(), toPoco(seam->firstSeamInterval()->uuid()));
    QCOMPARE(graphId.isNull(), false);
    QVERIFY(graphId != Poco::UUID("e58abf42-77a6-4456-9f78-56e002b38549"));
    QVERIFY(graphId != Poco::UUID("0c3ebf5b-df35-4ac6-9b79-085cba2aba4f"));

    // now get the graph
    const auto graphList = server.getGraph(tasks.at(2).taskID(), graphId);
    QCOMPARE(graphList.size(), 1);
    const auto &graph = graphList.front();
    QCOMPARE(graph.id(), graphId);
    QCOMPARE(graph.pathComponents(), std::string());
    QCOMPARE(graph.components().size(), 1);
    QCOMPARE(graph.components().front().id(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(graph.components().front().filename(), std::string("Test_Component"));

    // get the graph of the seam linking to the first one
    const auto linkedGraphList = server.getGraph(toPoco(linked->firstSeamInterval()->uuid()), graphId);
    QCOMPARE(linkedGraphList.size(), 1);
    const auto& linkedGraph = linkedGraphList.front();
    QCOMPARE(linkedGraph.id(), graphId);
    QCOMPARE(linkedGraph.pathComponents(), std::string());
    QCOMPARE(linkedGraph.components().size(), 1);
    QCOMPARE(linkedGraph.components().front().id(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(linkedGraph.components().front().filename(), std::string("Test_Component"));

    QCOMPARE(linkedGraph.filters().size(), 4);

    QCOMPARE(graph.filters().size(), 4);
    auto it = std::find_if(graph.filters().begin(), graph.filters().end(), [](const auto& filter)
                           { return filter.instanceID() == Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->name(), std::string("precitec::filter::TestFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->inPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 1);
    QCOMPARE(it->parameterList().front()->any(), Poco::DynamicAny(1));
    QCOMPARE(it->parameterList().front()->name(), std::string("An attribute"));

    // second filter
    it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("5599b861-7222-4af9-8bf5-b7a116a6ab1f"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("5599b861-7222-4af9-8bf5-b7a116a6ab1f"));
    QCOMPARE(it->name(), std::string("precitec::filter::AnotherFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 0);
    QCOMPARE(it->inPipeList().size(), 1);
    QCOMPARE(it->inPipeList().front().group(), 0);
    QCOMPARE(it->inPipeList().front().sender(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->inPipeList().front().name(), std::string("Image"));
    QCOMPARE(it->inPipeList().front().tag(), std::string("Foo"));

    // third filter
    it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("33675875-d7e5-47ba-85c6-fe34238cd56c"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("33675875-d7e5-47ba-85c6-fe34238cd56c"));
    QCOMPARE(it->name(), std::string("precitec::filter::TestFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->inPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 1);
    QCOMPARE(it->parameterList().front()->any(), Poco::DynamicAny(1));
    QCOMPARE(it->parameterList().front()->name(), std::string("An attribute"));

    // fourth filter
    it = std::find_if(graph.filters().begin(), graph.filters().end(), [] (const auto &filter) { return filter.instanceID() == Poco::UUID("f43aaa22-eab6-4bc5-88f7-07d48148acf8"); });
    QVERIFY(it != graph.filters().end());
    QCOMPARE(it->instanceID(), Poco::UUID("f43aaa22-eab6-4bc5-88f7-07d48148acf8"));
    QCOMPARE(it->name(), std::string("precitec::filter::AnotherFilter"));
    QCOMPARE(it->component(), Poco::UUID("f642831a-53b6-455c-8adf-88079d15eaba"));
    QCOMPARE(it->outPipeList().empty(), true);
    QCOMPARE(it->parameterList().size(), 0);
    QCOMPARE(it->inPipeList().size(), 1);
    QCOMPARE(it->inPipeList().front().group(), 0);
    QCOMPARE(it->inPipeList().front().sender(), Poco::UUID("cb9f223c-6762-4d8e-9faa-bca71f951444"));
    QCOMPARE(it->inPipeList().front().name(), std::string("Image"));
    QCOMPARE(it->inPipeList().front().tag(), std::string("Bar"));
}

void TestDbServer::testGetHardwareParameters()
{
    auto model = std::make_shared<ProductModel>();
    auto graphModel = std::make_shared<GraphModel>();
    DbServer server(model, graphModel);
    // not yet loaded, parameters should be empty
    QVERIFY(server.getHardwareParameterSatz(Poco::UUID("3F086211-FBD4-4493-A580-6FF11E4925DF")).empty());
    // now load
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);
    const auto parameters = server.getHardwareParameterSatz(Poco::UUID("3F086211-FBD4-4493-A580-6FF11E4925DF"));
    QCOMPARE(parameters.size(), 5u);
    const auto &parameter1 = parameters.at(0);
    const auto &parameter2 = parameters.at(1);
    const auto &parameter3 = parameters.at(2);
    const auto &parameter4 = parameters.at(3);
    const auto &parameter5 = parameters.at(4);

    QCOMPARE(parameter1->name(), std::string("BoolValue"));
    QCOMPARE(parameter2->name(), std::string("IntValue"));
    QCOMPARE(parameter3->name(), std::string("UintValue"));
    QCOMPARE(parameter4->name(), std::string("FloatValue"));
    QCOMPARE(parameter5->name(), std::string("DoubleValue"));

    QCOMPARE(parameter1->typID(), Poco::UUID("96599c45-4e20-4aaa-826d-25463670dd09"));
    QCOMPARE(parameter2->typID(), Poco::UUID("06599c45-4e20-4aaa-826d-25463670dd09"));
    QCOMPARE(parameter3->typID(), Poco::UUID("16599c45-4e20-4aaa-826d-25463670dd09"));
    QCOMPARE(parameter4->typID(), Poco::UUID("26599c45-4e20-4aaa-826d-25463670dd09"));
    QCOMPARE(parameter5->typID(), Poco::UUID("36599c45-4e20-4aaa-826d-25463670dd09"));

    QCOMPARE(parameter1->type(), precitec::TBool);
    QCOMPARE(parameter2->type(), precitec::TInt);
    QCOMPARE(parameter3->type(), precitec::TUInt);
    QCOMPARE(parameter4->type(), precitec::TFloat);
    QCOMPARE(parameter5->type(), precitec::TDouble);

    QCOMPARE(parameter1->value<bool>(), true);
    QCOMPARE(parameter2->value<int>(), 2);
    QCOMPARE(parameter3->value<uint>(), 3u);
    QCOMPARE(parameter4->value<float>(), 4.1f);
    QCOMPARE(parameter5->value<double>(), 5.12);
}

void TestDbServer::testGetMeasureTasks()
{
    DbServer server{nullptr, nullptr};
    SetUpMeasureTasksTest(server);

    QVERIFY(server.getMeasureTasks(Poco::UUID(), Poco::UUID("1F086211-FBD4-4493-A580-6FF11E4925DD")).empty());
    const auto &measureTasks = server.getMeasureTasks(Poco::UUID(), Poco::UUID("2F086211-FBD4-4493-A580-6FF11E4925DE"));
    QCOMPARE(measureTasks.size(), 9u);

    const auto &seamSeries = measureTasks.at(0);
    const auto &seam = measureTasks.at(1);
    const auto &seamInterval = measureTasks.at(2);

    QCOMPARE(seamSeries.level(), 0);
    QCOMPARE(seam.level(), 1);
    QCOMPARE(seamInterval.level(), 2);
    QCOMPARE(seamInterval.seaminterval(), 0);

    QCOMPARE(seamSeries.graphID(), defaultGraphId);
    QCOMPARE(seam.graphID(), Poco::UUID("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(seamInterval.graphID(), Poco::UUID("b142af2c-cca8-4b88-828c-5e14490e7337"));

    QCOMPARE(seamSeries.graphName(), std::string(" - "));
    QCOMPARE(seam.graphName(), std::string("TestGraph"));
    QCOMPARE(seamInterval.graphName(), std::string("TestGraph"));

    // sub graphs
    const auto &seamInterval2 = measureTasks.at(7);
    QCOMPARE(seamInterval2.level(), 2);
    QCOMPARE(seamInterval2.seaminterval(), 1);
    QCOMPARE(seamInterval2.graphName(), std::string(" - "));
    const auto generatedSubGraph = seamInterval2.graphID();
    QVERIFY(generatedSubGraph != defaultGraphId);
    QVERIFY(!generatedSubGraph.isNull());

    const auto &seamInterval3 = measureTasks.at(8);
    QCOMPARE(seamInterval3.level(), 2);
    QCOMPARE(seamInterval3.seaminterval(), 2);
    QCOMPARE(seamInterval3.graphName(), std::string(" - "));
    QCOMPARE(seamInterval3.graphID(), generatedSubGraph);

    // TODO: further extend test
}

void TestDbServer::testGetMeasureTasksFromLinkedGraph()
{
    DbServer server{nullptr, nullptr};
    SetUpMeasureTasksTest(server);
    const auto& measureTasks = server.getMeasureTasks(Poco::UUID(), Poco::UUID("62f4ed0c-5d43-42ca-94cd-ec300f8377fa"));
    QCOMPARE(measureTasks.size(), 8u); // 1 seam series at index 0 and 7 seams

    const auto& seamGraph = measureTasks.at(1);
    const auto& linkedGraphSeam = measureTasks.at(2);
    const auto& linkedLinkedGraphSeam = measureTasks.at(7);

    const auto expectedGraphId = Poco::UUID("af45c249-dcb0-4621-996c-5c9aeb403132");
    QCOMPARE(seamGraph.graphID(), expectedGraphId);
    QCOMPARE(linkedGraphSeam.graphID(), expectedGraphId);
    QCOMPARE(linkedLinkedGraphSeam.graphID(), expectedGraphId);
    // Graph and linked graphs should yield the same name.
    QCOMPARE(seamGraph.graphName(), "MyBeautifulGraph");
    QCOMPARE(linkedGraphSeam.graphName(), "MyBeautifulGraph");
    QCOMPARE(linkedLinkedGraphSeam.graphName(), "MyBeautifulGraph");

    const auto& seamEmptySubGraphs = measureTasks.at(3);
    const auto& linkedToEmptySubGraphs = measureTasks.at(4);
    QCOMPARE(seamEmptySubGraphs.graphID(), defaultGraphId);
    QCOMPARE(linkedToEmptySubGraphs.graphID(), defaultGraphId);

    const auto& seamSubGraph = measureTasks.at(5);
    const auto& linkedSubGraphSeam = measureTasks.at(6);
    QVERIFY(!seamSubGraph.graphID().isNull());
    QVERIFY(seamSubGraph.graphID() != defaultGraphId);
    QCOMPARE(linkedSubGraphSeam.graphID(), seamSubGraph.graphID());
}

void TestDbServer::testGetFilterParameter()
{
    auto model = std::make_shared<ProductModel>();
    auto graphModel = std::make_shared<GraphModel>();
    DbServer server(model, graphModel);
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    model->loadProducts(dir);

    // not existing measure task
    QCOMPARE(server.getFilterParameter(Poco::UUID("4F086211-FBD4-4493-A580-6FF11E4925DE"), Poco::UUID()).empty(), true);

    // get measure tasks
    const auto &measureTasks = server.getMeasureTasks(Poco::UUID(), Poco::UUID("2F086211-FBD4-4493-A580-6FF11E4925DE"));
    const auto &seamSeries = measureTasks.at(0);
    const auto &seam = measureTasks.at(1);
    const auto &seamInterval = measureTasks.at(2);

    QCOMPARE(server.getFilterParameter(Poco::UUID("4F086211-FBD4-4493-A580-6FF11E4925DE"), seamSeries.taskID()).empty(), true);
    const auto parameters = server.getFilterParameter(Poco::UUID("4F086211-FBD4-4493-A580-6FF11E4925DE"), seam.taskID());
    QCOMPARE(parameters.size(), 2u);
    const auto &parameter1 = parameters.at(0);
    const auto &parameter2 = parameters.at(1);
    QCOMPARE(parameter1->parameterID(), Poco::UUID("7F086211-FBD4-4493-A580-6FF11E4925DE"));
    QCOMPARE(parameter2->parameterID(), Poco::UUID("9F086211-FBD4-4493-A580-6FF11E4925DE"));
    const auto interval_parameters = server.getFilterParameter(Poco::UUID("4F086211-FBD4-4493-A580-6FF11E4925DE"), seamInterval.taskID());
    QCOMPARE(interval_parameters.size(), 2u);
    const auto &interval_parameter1 = interval_parameters.at(0);
    const auto &interval_parameter2 = interval_parameters.at(1);
    QCOMPARE(interval_parameter1->parameterID(), Poco::UUID("7F086211-FBD4-4493-A580-6FF11E4925DE"));
    QCOMPARE(interval_parameter2->parameterID(), Poco::UUID("9F086211-FBD4-4493-A580-6FF11E4925DE"));
}

QTEST_GUILESS_MAIN(TestDbServer)
#include "testDbServer.moc"
