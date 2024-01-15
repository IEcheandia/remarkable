#include <QTest>

#include "fliplib/graphContainer.h"
#include "../src/graphModel.h"
#include "../src/product.h"
#include "../src/graphFunctions.h"
#include "../src/seam.h"
#include "../src/subGraphModel.h"

using precitec::storage::Product;
using precitec::storage::LinkedGraphReference;
using precitec::storage::graphFunctions::getGraphFromModel;
using precitec::storage::graphFunctions::getCurrentGraphId;
using precitec::storage::Seam;

class GraphFunctionsTest : public QObject
{
    Q_OBJECT

    std::unique_ptr<Product> p;
    Seam* seam = nullptr;
    precitec::storage::GraphModel graphModel;
    precitec::storage::SubGraphModel subGraphModel;

private Q_SLOTS:
    void init();
    void testGetCurrentGraphId();
    void testGetGraphFromModel();
};

void GraphFunctionsTest::init()
{
    p = std::make_unique<Product>(QUuid::createUuid());
    p->createFirstSeamSeries();
    seam = p->createSeam();
    QVERIFY(seam);
}

void GraphFunctionsTest::testGetCurrentGraphId()
{
    auto graphUuid = getCurrentGraphId(nullptr, nullptr);
    QCOMPARE(graphUuid, QUuid{});

    auto* linkedGraphSeam = p->createSeam();
    linkedGraphSeam->setGraphReference(LinkedGraphReference{seam->uuid()});
    auto* brokenLinked = p->createSeam();
    brokenLinked->setGraphReference(LinkedGraphReference{QUuid::createUuid()});

    const auto inputUuid = QUuid::createUuid();
    seam->setGraph(inputUuid);
    graphUuid = getCurrentGraphId(seam, nullptr);
    QCOMPARE(graphUuid, inputUuid);
    auto linkedGraphUuid = getCurrentGraphId(linkedGraphSeam, nullptr);
    QCOMPARE(linkedGraphUuid, inputUuid);
    auto brokenLinkedGraphUuid = getCurrentGraphId(brokenLinked, nullptr);
    QCOMPARE(brokenLinkedGraphUuid, QUuid{});

    const std::vector<QUuid> inputGraphUuids{QUuid::createUuid(), QUuid::createUuid(), QUuid::createUuid()};
    seam->setSubGraphs(inputGraphUuids);
    graphUuid = getCurrentGraphId(seam, nullptr);
    QCOMPARE(graphUuid, QUuid{});
    linkedGraphUuid = getCurrentGraphId(linkedGraphSeam, nullptr);
    QCOMPARE(linkedGraphUuid, QUuid{});

    const auto expectedSubGraphId = subGraphModel.generateGraphId(seam->subGraphs());
    graphUuid = getCurrentGraphId(seam, &subGraphModel);
    QCOMPARE(graphUuid, expectedSubGraphId);
    linkedGraphUuid = getCurrentGraphId(linkedGraphSeam, &subGraphModel);
    QCOMPARE(linkedGraphUuid, expectedSubGraphId);
}

void GraphFunctionsTest::testGetGraphFromModel()
{
    auto graph = getGraphFromModel(nullptr, nullptr, nullptr);
    QCOMPARE(graph.id, Poco::UUID{});
    graph = getGraphFromModel(seam, nullptr, nullptr);
    QCOMPARE(graph.id, Poco::UUID{});

    const auto graphUuid = QUuid::createUuid();
    seam->setGraph(graphUuid);
    graph = getGraphFromModel(seam, &graphModel, nullptr);
    QCOMPARE(graph.id, Poco::UUID{});
}

QTEST_GUILESS_MAIN(GraphFunctionsTest)
#include "testGraphFunctions.moc"