#include <QTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <iterator>

#include "testGraphReference.h"

#include "../src/copyMode.h"
#include "../src/parameter.h"
#include "../src/parameterSet.h"
#include "../src/product.h"
#include "../src/seam.h"
#include "../src/seamInterval.h"
#include "../src/seamSeries.h"

using precitec::storage::CopyMode;
using precitec::storage::GraphReference;
using precitec::storage::LinkedGraphReference;
using precitec::storage::Parameter;
using precitec::storage::ParameterSet;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;
using precitec::storage::SingleGraphReference;
using precitec::storage::SubGraphReference;

class TestProductLoading : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTest();
    void testLoadProductFailure_data();
    void testLoadProductFailure();
    void testLoadProduct_data();
    void testLoadProduct();
    void testLoadTrigger_data();
    void testLoadTrigger();
    void testLoadYAxis_data();
    void testLoadYAxis();
    void testLoadGraphReference_data();
    void testLoadGraphReference();
    void testLoadSeamSeriesGeneratedUuid();
    void testLoadSeamGeneratedUuid();
    void testLoadSeamIntervalGeneratedUuid_data();
    void testLoadSeamIntervalGeneratedUuid();
    void testLoadHardwareParameters_data();
    void testLoadHardwareParameters();
    void testLoadFilterParameterSets();
    void testLoadFilterParameterGroups();
    void testAssemblyImage();
};

void TestProductLoading::initTest()
{
    qRegisterMetaType<precitec::storage::ParameterSet*>();
    qRegisterMetaType<precitec::storage::Product*>();
    qRegisterMetaType<precitec::storage::SeamSeries*>();
    qRegisterMetaType<precitec::storage::Seam*>();
}

void TestProductLoading::testLoadProductFailure_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("array instead of object") << QFINDTESTDATA("testdata/products/arrayInsteadOfObject.json");
    QTest::newRow("not json") << QFINDTESTDATA("testdata/products/notjson.json");
    QTest::newRow("empty") << QFINDTESTDATA("testdata/products/empty.json");
    QTest::newRow("does not exist") << QStringLiteral("testdata/products/doesnotexist.json");
    QTest::newRow("nouuid") << QFINDTESTDATA("testdata/products/nouuid.json");
    QTest::newRow("invalid uuid")<< QFINDTESTDATA("testdata/products/invaliduuid.json");
}

void TestProductLoading::testLoadProductFailure()
{
    QFETCH(QString, fileName);
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(!product);
}

void TestProductLoading::testLoadProduct_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("type");
    QTest::addColumn<bool>("endless");
    QTest::addColumn<bool>("defaultProduct");

    QTest::newRow("no name") << QFINDTESTDATA("testdata/products/noname.json") << QUuid("1F086211-FBD4-4493-A580-6FF11E4925DD") << QString() << 0 << false << false;
    QTest::newRow("valid") << QFINDTESTDATA("testdata/products/product.json") << QUuid("1F086211-FBD4-4493-A580-6FF11E4925DD") << QStringLiteral("FooBar") << 1 << true << false;
    QTest::newRow("invalid type") << QFINDTESTDATA("testdata/products/invalidtype.json") << QUuid("1F086211-FBD4-4493-A580-6FF11E4925DD") << QStringLiteral("FooBar") << 0 << false << false;
    QTest::newRow("not endless") << QFINDTESTDATA("testdata/products/notendless.json") << QUuid("1F086211-FBD4-4493-A580-6FF11E4925DD") << QStringLiteral("FooBar") << 1 << false << false;
    QTest::newRow("endless not bool") << QFINDTESTDATA("testdata/products/invalidendless.json") << QUuid("1F086211-FBD4-4493-A580-6FF11E4925DD") << QStringLiteral("FooBar") << 1 << false << true;
}

void TestProductLoading::testLoadProduct()
{
    QFETCH(QString, fileName);
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    QTEST(product->uuid(), "uuid");
    QTEST(product->property("uuid").toUuid(), "uuid");
    QTEST(product->name(), "name");
    QTEST(product->property("name").toString(), "name");
    QTEST(product->type(), "type");
    QTEST(product->property("type").toInt(), "type");
    QTEST(product->isEndless(), "endless");
    QTEST(product->property("endless").toBool(), "endless");
    QTEST(product->isDefaultProduct(), "defaultProduct");
    QTEST(product->property("defaultProduct").toBool(), "defaultProduct");
    QCOMPARE(product->triggerSource(), Product::TriggerSource::Software);
    QCOMPARE(product->triggerMode(), Product::TriggerMode::Burst);
    QCOMPARE(product->startPositionYAxis(), 0);
    QCOMPARE(product->seamSeries().empty(), true);
    QVERIFY(!product->hardwareParameters());
    QVERIFY(!product->property("hardwareParameters").value<ParameterSet*>());
    QCOMPARE(product->filePath(), fileName);
    QCOMPARE(product->lengthUnit(), Product::LengthUnit::Millimeter);
    QCOMPARE(product->assemblyImage(), QString());
    QCOMPARE(product->property("assemblyImage").toString(), QString());
}

void TestProductLoading::testLoadTrigger_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<Product::TriggerSource>("source");
    QTest::addColumn<Product::TriggerMode>("mode");

    QTest::newRow("trigger not object") << QFINDTESTDATA("testdata/products/triggernotobject.json") << Product::TriggerSource::Software << Product::TriggerMode::Burst;
    QTest::newRow("source not set") << QFINDTESTDATA("testdata/products/sourcenotset.json") << Product::TriggerSource::Software << Product::TriggerMode::Single;
    QTest::newRow("Source invalid") << QFINDTESTDATA("testdata/products/sourceinvalid.json") << Product::TriggerSource::Software << Product::TriggerMode::Continue;
    QTest::newRow("Source software") << QFINDTESTDATA("testdata/products/sourcesoftware.json") << Product::TriggerSource::Software << Product::TriggerMode::Continue;
    QTest::newRow("Source external") << QFINDTESTDATA("testdata/products/sourceexternal.json") << Product::TriggerSource::External << Product::TriggerMode::Continue;
    QTest::newRow("Source grabber") << QFINDTESTDATA("testdata/products/sourcegrabber.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::Continue;
    QTest::newRow("Mode not set") << QFINDTESTDATA("testdata/products/modenotset.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::Burst;
    QTest::newRow("Mode invalid") << QFINDTESTDATA("testdata/products/modeinvalid.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::Burst;
    QTest::newRow("Mode single") << QFINDTESTDATA("testdata/products/modesingle.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::Single;
    QTest::newRow("Mode burst") << QFINDTESTDATA("testdata/products/modeburst.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::Burst;
    QTest::newRow("Mode continue") << QFINDTESTDATA("testdata/products/modecontinue.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::Continue;
    QTest::newRow("Mode none") << QFINDTESTDATA("testdata/products/modenone.json") << Product::TriggerSource::GrabberControlled << Product::TriggerMode::None;
}

void TestProductLoading::testLoadTrigger()
{
    QFETCH(QString, fileName);
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    QTEST(product->triggerSource(), "source");
    QTEST(product->property("triggerSource").value<Product::TriggerSource>(), "source");
    QTEST(product->triggerMode(), "mode");
    QTEST(product->property("triggerMode").value<Product::TriggerMode>(), "mode");
}

void TestProductLoading::testLoadYAxis_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<int>("pos");

    QTest::newRow("hardware not object") << QFINDTESTDATA("testdata/products/hardwarenotobject.json") << 0;
    QTest::newRow("start pos missing") << QFINDTESTDATA("testdata/products/startposmissing.json") << 0;
    QTest::newRow("start pos string") << QFINDTESTDATA("testdata/products/startposstring.json") << 0;
    QTest::newRow("start pos 0") << QFINDTESTDATA("testdata/products/startpos0.json") << 0;
    QTest::newRow("start pos 1") << QFINDTESTDATA("testdata/products/startpos1.json") << 1;
}

void TestProductLoading::testLoadYAxis()
{
    QFETCH(QString, fileName);
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    QTEST(product->startPositionYAxis(), "pos");
    QTEST(product->property("startPositionYAxis").toInt(), "pos");
}

void TestProductLoading::testLoadGraphReference_data()
{
    QTest::addColumn<QString>("fileName");
    QTest::addColumn<GraphReference>("expected");

    const GraphReference expectedGraph = SingleGraphReference{
        "c1a3dcf1-994a-40af-88a7-4a644c855dfa"};
    const GraphReference expectedLinkedGraph = LinkedGraphReference{
        "c1a3dcf1-994a-40af-88a7-4a644c855dfa"};
    const GraphReference expectedSubGraphs = SubGraphReference{
        std::vector<QUuid>{"5390b93d-9de8-44d4-ae13-2f72c883d91f",
                           "f64f12a6-1654-4cbb-8a22-796c4bba30f5",
                           "bb4b9c5d-e367-44a2-ad25-cc00291d2613"}};
    const QString basePath{"testdata/productLoadingGraphReferences/"};

    QTest::newRow("load json without graph ref")
        << QFINDTESTDATA(basePath + "seamnographref.json") << GraphReference{};
    QTest::newRow("load graph from json")
        << QFINDTESTDATA(basePath + "seamgraph.json") << expectedGraph;
    QTest::newRow("load subGraphs from json")
        << QFINDTESTDATA(basePath + "seamsubgraphs.json") << expectedSubGraphs;
    QTest::newRow("load linkedGraph from json")
        << QFINDTESTDATA(basePath + "seamgraphlinked.json") << expectedLinkedGraph;
    QTest::newRow("with graph and subGraphs, prefers subGraphs")
        << QFINDTESTDATA(basePath + "seamgraphandsubgraphs.json") << expectedSubGraphs;
    QTest::newRow("with graph and linkedGraph, prefers graphs")
        << QFINDTESTDATA(basePath + "seamgraphandlinkedgraph.json") << expectedGraph;
}

void TestProductLoading::testLoadGraphReference()
{
    QFETCH(QString, fileName);
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);

    QCOMPARE(product->seams().size(), 1);
    const Seam* const seam = product->seams().at(0);
    QTEST(seam->graphReference(), "expected");
}

void TestProductLoading::testLoadSeamSeriesGeneratedUuid()
{
    // the product we load defines three seam series
    // two have a default id, one has a specified id
    const QString fileName = QFINDTESTDATA("testdata/products/seamseriesuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    const auto &seamSeries = product->seamSeries();
    QCOMPARE(seamSeries.size(), 3u);
    QCOMPARE(seamSeries.at(0)->product(), product.get());
    QCOMPARE(seamSeries.at(1)->product(), product.get());
    QCOMPARE(seamSeries.at(2)->product(), product.get());
    QCOMPARE(seamSeries.at(0)->property("product").value<Product*>(), product.get());
    QCOMPARE(seamSeries.at(1)->property("product").value<Product*>(), product.get());
    QCOMPARE(seamSeries.at(2)->property("product").value<Product*>(), product.get());
    const auto &allSeamSeries = product->allSeamSeries();
    QCOMPARE(allSeamSeries.size(), 3u);
    QCOMPARE(allSeamSeries.at(0).value<SeamSeries*>()->uuid(), seamSeries.at(0)->uuid());
    QCOMPARE(allSeamSeries.at(1).value<SeamSeries*>()->uuid(), seamSeries.at(1)->uuid());
    QCOMPARE(allSeamSeries.at(2).value<SeamSeries*>()->uuid(), seamSeries.at(2)->uuid());

    QVERIFY(seamSeries.at(0)->uuid() != seamSeries.at(1)->uuid());
    QVERIFY(seamSeries.at(0)->uuid() != seamSeries.at(2)->uuid());
    QVERIFY(seamSeries.at(1)->uuid() != seamSeries.at(2)->uuid());
    QVERIFY(!seamSeries.at(0)->uuid().isNull());
    QVERIFY(!seamSeries.at(1)->uuid().isNull());
    QVERIFY(!seamSeries.at(2)->uuid().isNull());
    QCOMPARE(seamSeries.at(2)->uuid(), QUuid(QByteArrayLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(seamSeries.at(0)->number(), 0u);
    QCOMPARE(seamSeries.at(1)->number(), 1u);
    QCOMPARE(seamSeries.at(2)->number(), 2u);
    QVERIFY(seamSeries.at(0)->seams().empty());
    QVERIFY(seamSeries.at(1)->seams().empty());
    QVERIFY(seamSeries.at(2)->seams().empty());
    QVERIFY(seamSeries.at(0)->graph().isNull());
    QVERIFY(seamSeries.at(1)->graph().isNull());
    QVERIFY(seamSeries.at(2)->graph().isNull());

    QVERIFY(!seamSeries.at(0)->hardwareParameters());
    QVERIFY(!seamSeries.at(0)->property("hardwareParameters").value<ParameterSet*>());
    QVERIFY(!seamSeries.at(1)->hardwareParameters());
    QVERIFY(!seamSeries.at(1)->property("hardwareParameters").value<ParameterSet*>());
    QVERIFY(seamSeries.at(2)->hardwareParameters());
    QVERIFY(seamSeries.at(2)->property("hardwareParameters").value<ParameterSet*>());
    QVERIFY(seamSeries.at(2)->hardwareParameters()->parameters().empty());
    QCOMPARE(seamSeries.at(2)->hardwareParameters()->uuid(), QUuid(QByteArrayLiteral("1F086211-FBD4-4493-A580-6FF11E4925DD")));
    QCOMPARE(product->findHardwareParameterSet(seamSeries.at(2)->hardwareParameters()->uuid()), seamSeries.at(2)->hardwareParameters());
}

void TestProductLoading::testLoadSeamGeneratedUuid()
{
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    const auto &seamSeries = product->seamSeries();
    QCOMPARE(seamSeries.size(), 3u);
    std::vector<Seam*> seams = seamSeries.at(0)->seams();
    QCOMPARE(seams.size(), 1u);
    QCOMPARE(seams.at(0)->number(), 0u);
    QCOMPARE(seams.at(0)->visualNumber(), 1u);
    QCOMPARE(seams.at(0)->seamSeries(), seamSeries.at(0));
    QCOMPARE(seams.at(0)->property("seamSeries").value<SeamSeries*>(), seamSeries.at(0));
    QVERIFY(!seams.at(0)->uuid().isNull());
    QCOMPARE(seams.at(0)->triggerDelta(), 0);
    QCOMPARE(seams.at(0)->velocity(), 0);
    QVERIFY(!seams.at(0)->firstSeamInterval());

    QCOMPARE(product->findSeam(seams.at(0)->uuid()), seams.at(0));
    QCOMPARE(product->findSeam(0u, 0u), seams.at(0));
    QVERIFY(!product->findSeam(QUuid::createUuid()));

    seams = seamSeries.at(1)->seams();
    QCOMPARE(seams.size(), 2u);
    QCOMPARE(seams.at(0)->number(), 0u);
    QCOMPARE(seams.at(0)->visualNumber(), 1u);
    QCOMPARE(seams.at(0)->seamSeries(), seamSeries.at(1));
    QCOMPARE(seams.at(0)->property("seamSeries").value<SeamSeries*>(), seamSeries.at(1));
    QVERIFY(!seams.at(0)->uuid().isNull());
    QCOMPARE(seams.at(1)->number(), 1u);
    QCOMPARE(seams.at(1)->visualNumber(), 2u);
    QCOMPARE(seams.at(1)->seamSeries(), seamSeries.at(1));
    QCOMPARE(seams.at(1)->property("seamSeries").value<SeamSeries*>(), seamSeries.at(1));
    QVERIFY(!seams.at(1)->uuid().isNull());
    QVERIFY(seams.at(1)->uuid() != seams.at(0)->uuid());
    QVERIFY(seams.at(1)->uuid() != seamSeries.at(1)->uuid());
    QCOMPARE(seams.at(1)->velocity(), 3520);
    QCOMPARE(product->findSeam(seams.at(0)->uuid()), seams.at(0));
    QCOMPARE(product->findSeam(1u, 0u), seams.at(0));
    QCOMPARE(product->findSeam(seams.at(1)->uuid()), seams.at(1));
    QCOMPARE(product->findSeam(1u, 1u), seams.at(1));

    seams = seamSeries.at(2)->seams();
    QCOMPARE(seams.size(), 3u);
    QCOMPARE(seams.at(0)->number(), 0u);
    QCOMPARE(seams.at(0)->visualNumber(), 1u);
    QCOMPARE(seams.at(0)->seamSeries(), seamSeries.at(2));
    QCOMPARE(seams.at(0)->property("seamSeries").value<SeamSeries*>(), seamSeries.at(2));
    QVERIFY(!seams.at(0)->uuid().isNull());
    QCOMPARE(seams.at(1)->number(), 1u);
    QCOMPARE(seams.at(1)->visualNumber(), 2u);
    QCOMPARE(seams.at(1)->seamSeries(), seamSeries.at(2));
    QCOMPARE(seams.at(1)->property("seamSeries").value<SeamSeries*>(), seamSeries.at(2));
    QVERIFY(!seams.at(1)->uuid().isNull());
    QCOMPARE(seams.at(2)->number(), 2u);
    QCOMPARE(seams.at(2)->visualNumber(), 3u);
    QCOMPARE(seams.at(2)->seamSeries(), seamSeries.at(2));
    QCOMPARE(seams.at(2)->property("seamSeries").value<SeamSeries*>(), seamSeries.at(2));
    QVERIFY(!seams.at(2)->uuid().isNull());
    QCOMPARE(seams.at(2)->uuid(), QUuid(QByteArrayLiteral("3F086211-FBD4-4493-A580-6FF11E4925DF")));
    QVERIFY(seams.at(1)->uuid() != seams.at(0)->uuid());
    QVERIFY(seams.at(1)->uuid() != seamSeries.at(2)->uuid());
    QVERIFY(seams.at(2)->uuid() != seams.at(0)->uuid());
    QVERIFY(seams.at(2)->uuid() != seams.at(1)->uuid());
    QVERIFY(seams.at(2)->uuid() != seamSeries.at(2)->uuid());
    QCOMPARE(seams.at(2)->triggerDelta(), 1000);
    QVERIFY(seams.at(0)->graph().isNull());
    QVERIFY(seams.at(1)->graph().isNull());
    QVERIFY(seams.at(2)->graph().isNull());
    QCOMPARE(product->findSeam(seams.at(0)->uuid()), seams.at(0));
    QCOMPARE(product->findSeam(2u, 0u), seams.at(0));
    QCOMPARE(product->findSeam(seams.at(1)->uuid()), seams.at(1));
    QCOMPARE(product->findSeam(2u, 1u), seams.at(1));
    QCOMPARE(product->findSeam(seams.at(2)->uuid()), seams.at(2));
    QCOMPARE(product->findSeam(2u, 2u), seams.at(2));

    QVERIFY(!product->findSeam(2u, 3u));
    QVERIFY(!product->findSeam(3u, 0u));

    auto allSeams = product->allSeams();
    QCOMPARE(allSeams.count(), 6);
    QCOMPARE(allSeams.at(0).value<Seam*>(), seamSeries.at(0)->seams().at(0));
    QCOMPARE(allSeams.at(1).value<Seam*>(), seamSeries.at(1)->seams().at(0));
    QCOMPARE(allSeams.at(2).value<Seam*>(), seamSeries.at(1)->seams().at(1));
    QCOMPARE(allSeams.at(3).value<Seam*>(), seamSeries.at(2)->seams().at(0));
    QCOMPARE(allSeams.at(4).value<Seam*>(), seamSeries.at(2)->seams().at(1));
    QCOMPARE(allSeams.at(5).value<Seam*>(), seamSeries.at(2)->seams().at(2));

    allSeams = seamSeries.at(2)->allSeams();
    QCOMPARE(allSeams.size(), 3u);
    QCOMPARE(allSeams.at(0).value<Seam*>()->uuid(), seams.at(0)->uuid());
    QCOMPARE(allSeams.at(1).value<Seam*>()->uuid(), seams.at(1)->uuid());
    QCOMPARE(allSeams.at(2).value<Seam*>()->uuid(), seams.at(2)->uuid());

    // Test navigation with previous and next
    QVERIFY(!product->previousSeam(seamSeries.at(0)->seams().at(0)));
    QCOMPARE(product->nextSeam(seamSeries.at(0)->seams().at(0)), seamSeries.at(1)->seams().at(0));
    QCOMPARE(product->previousSeam(seamSeries.at(1)->seams().at(0)), seamSeries.at(0)->seams().at(0));
    QCOMPARE(product->nextSeam(seamSeries.at(1)->seams().at(0)), seamSeries.at(1)->seams().at(1));
    QCOMPARE(product->previousSeam(seamSeries.at(1)->seams().at(1)), seamSeries.at(1)->seams().at(0));
    QCOMPARE(product->nextSeam(seamSeries.at(1)->seams().at(1)), seamSeries.at(2)->seams().at(0));
    QCOMPARE(product->previousSeam(seamSeries.at(2)->seams().at(0)), seamSeries.at(1)->seams().at(1));
    QCOMPARE(product->nextSeam(seamSeries.at(2)->seams().at(0)), seamSeries.at(2)->seams().at(1));
    QCOMPARE(product->previousSeam(seamSeries.at(2)->seams().at(1)), seamSeries.at(2)->seams().at(0));
    QCOMPARE(product->nextSeam(seamSeries.at(2)->seams().at(1)), seamSeries.at(2)->seams().at(2));
    QCOMPARE(product->previousSeam(seamSeries.at(2)->seams().at(2)), seamSeries.at(2)->seams().at(1));
    QVERIFY(!product->nextSeam(seamSeries.at(2)->seams().at(2)));
}

void TestProductLoading::testLoadSeamIntervalGeneratedUuid_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestProductLoading::testLoadSeamIntervalGeneratedUuid()
{
    const QString fileName = QFINDTESTDATA("testdata/products/seamintervaluuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> origProduct{Product::fromJson(fileName)};
    QVERIFY(origProduct);

    QTemporaryFile jsonFile;
    QVERIFY(jsonFile.open());
    origProduct->toJson(&jsonFile);
    QVERIFY(jsonFile.flush());

    std::unique_ptr<Product> product{Product::fromJson(jsonFile.fileName())};
    QVERIFY(product);
    const auto &seamSeries = product->seamSeries();
    QCOMPARE(seamSeries.size(), 1u);
    QCOMPARE(seamSeries.at(0)->name(), QStringLiteral("seamSeriesName"));
    auto seam = seamSeries.at(0)->seams().front();
    QVERIFY(seam);
    QCOMPARE(seam->name(), QStringLiteral("seamName"));
    QCOMPARE(seam->graph(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    const auto &seamIntervals = seam->seamIntervals();
    QCOMPARE(seamIntervals.size(), 3u);
    auto interval1 = seamIntervals.at(0);
    auto interval2 = seamIntervals.at(1);
    auto interval3 = seamIntervals.at(2);
    QCOMPARE(seam->firstSeamInterval(), interval1);
    QCOMPARE(interval1->number(), 0u);
    QCOMPARE(interval2->number(), 1u);
    QCOMPARE(interval3->number(), 2u);
    QCOMPARE(interval1->visualNumber(), 1u);
    QCOMPARE(interval2->visualNumber(), 2u);
    QCOMPARE(interval3->visualNumber(), 3u);
    QCOMPARE(interval1->name(), QStringLiteral("seamIntervalName"));
    QCOMPARE(interval2->name(), QString());
    QCOMPARE(interval3->name(), QString());
    QVERIFY(interval1->uuid() != interval2->uuid());
    QVERIFY(interval1->uuid() != interval3->uuid());
    QVERIFY(interval2->uuid() != interval3->uuid());
    QCOMPARE(interval3->uuid(), QUuid(QByteArrayLiteral("AF086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(interval1->seam(), seam);
    QCOMPARE(interval2->seam(), seam);
    QCOMPARE(interval3->seam(), seam);
    QCOMPARE(interval1->property("seam").value<Seam*>(), seam);
    QCOMPARE(interval2->property("seam").value<Seam*>(), seam);
    QCOMPARE(interval3->property("seam").value<Seam*>(), seam);
    QCOMPARE(interval1->graph(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(interval2->graph(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(interval3->property("graph").toUuid(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(interval1->length(), 0);
    QCOMPARE(interval1->property("length").toInt(), 0);
    QCOMPARE(interval2->length(), 20);
    QCOMPARE(interval2->property("length").toInt(), 20);
    QCOMPARE(interval3->length(), 300);
    QCOMPARE(interval3->property("length").toInt(), 300);
    QCOMPARE(seam->length(), 320);

    // duplicate seam
    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto duplicated = seam->duplicate(copyMode, seam->seamSeries());
    bool const hasDifferentUuid = duplicated->uuid() != seam->uuid();
    QCOMPARE(hasDifferentUuid, changeUuid);
    QCOMPARE(duplicated->seamSeries(), seam->seamSeries());
    QCOMPARE(duplicated->name(), seam->name());
    QCOMPARE(duplicated->number(), seam->number());
    QCOMPARE(duplicated->graph(), seam->graph());
    const auto &duplicatedSeamIntervals = duplicated->seamIntervals();
    QCOMPARE(duplicatedSeamIntervals.size(), 3u);

    auto duplicatedInterval1 = duplicatedSeamIntervals.at(0);
    auto duplicatedInterval2 = duplicatedSeamIntervals.at(1);
    auto duplicatedInterval3 = duplicatedSeamIntervals.at(2);
    QCOMPARE(duplicated->firstSeamInterval(), duplicatedInterval1);
    QCOMPARE(duplicatedInterval1->number(), 0u);
    QCOMPARE(duplicatedInterval2->number(), 1u);
    QCOMPARE(duplicatedInterval3->number(), 2u);
    QCOMPARE(duplicatedInterval1->visualNumber(), 1u);
    QCOMPARE(duplicatedInterval2->visualNumber(), 2u);
    QCOMPARE(duplicatedInterval3->visualNumber(), 3u);
    QCOMPARE(duplicatedInterval1->name(), QStringLiteral("seamIntervalName"));
    QCOMPARE(duplicatedInterval2->name(), QString());
    QCOMPARE(duplicatedInterval3->name(), QString());
    QVERIFY(duplicatedInterval1->uuid() != duplicatedInterval2->uuid());
    QVERIFY(duplicatedInterval1->uuid() != duplicatedInterval3->uuid());
    QVERIFY(duplicatedInterval2->uuid() != duplicatedInterval3->uuid());
    QCOMPARE(duplicatedInterval3->uuid() != QUuid(QByteArrayLiteral("AF086211-FBD4-4493-A580-6FF11E4925DE")), changeUuid);
    QCOMPARE(duplicatedInterval1->seam(), duplicated);
    QCOMPARE(duplicatedInterval2->seam(), duplicated);
    QCOMPARE(duplicatedInterval3->seam(), duplicated);
    QCOMPARE(duplicatedInterval1->property("seam").value<Seam*>(), duplicated);
    QCOMPARE(duplicatedInterval2->property("seam").value<Seam*>(), duplicated);
    QCOMPARE(duplicatedInterval3->property("seam").value<Seam*>(), duplicated);
    QCOMPARE(duplicatedInterval1->graph(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(duplicatedInterval2->graph(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(duplicatedInterval3->graph(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(duplicatedInterval1->property("graph").toUuid(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(duplicatedInterval2->property("graph").toUuid(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(duplicatedInterval3->property("graph").toUuid(), QUuid(QByteArrayLiteral("BF086211-FBD4-4493-A580-6FF11E4925DF")));
    QCOMPARE(duplicatedInterval1->length(), 0);
    QCOMPARE(duplicatedInterval1->property("length").toInt(), 0);
    QCOMPARE(duplicatedInterval2->length(), 20);
    QCOMPARE(duplicatedInterval2->property("length").toInt(), 20);
    QCOMPARE(duplicatedInterval3->length(), 300);
    QCOMPARE(duplicatedInterval3->property("length").toInt(), 300);
    QCOMPARE(duplicated->length(), 320);
}

void TestProductLoading::testLoadHardwareParameters_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestProductLoading::testLoadHardwareParameters()
{    
    const QString fileName = QFINDTESTDATA("testdata/products/hardwareparams.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> origProduct{Product::fromJson(fileName)};
    QVERIFY(origProduct);    
    
    QTemporaryFile jsonFile;
    QVERIFY(jsonFile.open());
    origProduct->toJson(&jsonFile);
    QVERIFY(jsonFile.flush());

    std::unique_ptr<Product> product{Product::fromJson(jsonFile.fileName())};
    QVERIFY(product);
    QVERIFY(product->hardwareParameters());
    QVERIFY(!product->hardwareParameters()->uuid().isNull());

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto duplicatedHwParams = product->hardwareParameters()->duplicate(copyMode, product.get());
    QVERIFY(duplicatedHwParams);
    bool const hasDifferentUuid = duplicatedHwParams->uuid() != product->hardwareParameters()->uuid();
    QCOMPARE(hasDifferentUuid, changeUuid);
    const auto &parameters = duplicatedHwParams->parameters();
    QCOMPARE(product->findHardwareParameterSet(product->hardwareParameters()->uuid()), product->hardwareParameters());
    // an id which does not exist
    QVERIFY(!product->findHardwareParameterSet(QUuid(QByteArrayLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE"))));
    QCOMPARE(parameters.size(), 6u);
    QVERIFY(!parameters.at(0)->uuid().isNull());
    QVERIFY(!parameters.at(1)->uuid().isNull());
    QVERIFY(parameters.at(0)->uuid() != parameters.at(1)->uuid());
    QCOMPARE(parameters.at(0)->uuid(), QUuid(QByteArrayLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(parameters.at(0)->name(), QStringLiteral("Test"));
    QCOMPARE(parameters.at(1)->name(), QStringLiteral("Further Test"));
    QCOMPARE(parameters.at(0)->typeId(), QUuid(QByteArrayLiteral("96599c45-4e20-4aaa-826d-25463670dd09")));
    QVERIFY(parameters.at(1)->typeId().isNull());
    QVERIFY(parameters.at(0)->value().canConvert<bool>());
    QCOMPARE(parameters.at(0)->value().toBool(), true);
    QCOMPARE(parameters.at(0)->property("value").toBool(), true);
    QCOMPARE(parameters.at(0)->value().isValid(), true);
    QCOMPARE(parameters.at(1)->value().isValid(), false);

    QCOMPARE(duplicatedHwParams->findByNameAndTypeId(QStringLiteral("Test"), QUuid(QByteArrayLiteral("96599c45-4e20-4aaa-826d-25463670dd09"))), parameters.at(0));
    QVERIFY(duplicatedHwParams->findByNameAndTypeId(QStringLiteral("Test2"), QUuid(QByteArrayLiteral("96599c45-4e20-4aaa-826d-25463670dd09"))) == nullptr);
    QVERIFY(duplicatedHwParams->findByNameAndTypeId(QStringLiteral("Test"), QUuid(QByteArrayLiteral("96599c45-4e20-4aaa-826d-25463670dd08"))) == nullptr);

    QCOMPARE(parameters.at(0)->type(), Parameter::DataType::Boolean);
    QCOMPARE(parameters.at(0)->property("type").value<Parameter::DataType>(), Parameter::DataType::Boolean);
    QCOMPARE(parameters.at(1)->type(), Parameter::DataType::Unknown);
    QCOMPARE(parameters.at(1)->property("type").value<Parameter::DataType>(), Parameter::DataType::Unknown);
    QCOMPARE(parameters.at(2)->type(), Parameter::DataType::Integer);
    QCOMPARE(parameters.at(2)->property("type").value<Parameter::DataType>(), Parameter::DataType::Integer);
    QCOMPARE(parameters.at(2)->value().toInt(), 20);
    QCOMPARE(parameters.at(2)->property("value").toInt(), 20);
    QCOMPARE(parameters.at(3)->type(), Parameter::DataType::UnsignedInteger);
    QCOMPARE(parameters.at(3)->property("type").value<Parameter::DataType>(), Parameter::DataType::UnsignedInteger);
    QCOMPARE(parameters.at(3)->value().toUInt(), 50u);
    QCOMPARE(parameters.at(3)->property("value").toUInt(), 50u);
    QCOMPARE(parameters.at(4)->type(), Parameter::DataType::Float);
    QCOMPARE(parameters.at(4)->property("type").value<Parameter::DataType>(), Parameter::DataType::Float);
    QCOMPARE(parameters.at(4)->value().toFloat(), 50.7f);
    QCOMPARE(parameters.at(4)->property("value").toFloat(), 50.7f);
    QCOMPARE(parameters.at(5)->type(), Parameter::DataType::Double);
    QCOMPARE(parameters.at(5)->property("type").value<Parameter::DataType>(), Parameter::DataType::Double);
    QCOMPARE(parameters.at(5)->value().toDouble(), -20050.767);
    QCOMPARE(parameters.at(5)->property("value").toDouble(), -20050.767);

    QCOMPARE(parameters.at(0)->filterId(), QUuid());
    QCOMPARE(parameters.at(1)->filterId(), QUuid());
    QCOMPARE(parameters.at(2)->filterId(), QUuid());
    QCOMPARE(parameters.at(3)->filterId(), QUuid());
    QCOMPARE(parameters.at(4)->filterId(), QUuid());
    QCOMPARE(parameters.at(5)->filterId(), QUuid());

    QVERIFY(product->findHardwareParameterSet(QUuid(QByteArrayLiteral("3F086211-FBD4-4493-A580-6FF11E4925DD"))));
    QVERIFY(!product->findHardwareParameterSet(QUuid(QByteArrayLiteral("5F086211-FBD4-4493-A580-6FF11E4925DD"))));
    QVERIFY(!product->findHardwareParameterSet(QUuid(QByteArrayLiteral("6F086211-FBD4-4493-A580-6FF11E4925DD"))));
    // Test enabling changetracking
    QCOMPARE(product->isChangeTracking(), false);
    QCOMPARE(product->hardwareParameters()->isChangeTracking(), false);
    for (auto parameter : product->hardwareParameters()->parameters())
    {
        QCOMPARE(parameter->isChangeTracking(), false);
    }
    for (auto seamSeries : product->seamSeries())
    {
        QCOMPARE(seamSeries->isChangeTracking(), false);
        QCOMPARE(seamSeries->hardwareParameters()->isChangeTracking(), false);
        for (auto parameter : seamSeries->hardwareParameters()->parameters())
        {
            QCOMPARE(parameter->isChangeTracking(), false);
        }
        for (auto seam : seamSeries->seams())
        {
            QCOMPARE(seam->isChangeTracking(), false);
            QCOMPARE(seam->hardwareParameters()->isChangeTracking(), false);
            for (auto parameter : seam->hardwareParameters()->parameters())
            {
                QCOMPARE(parameter->isChangeTracking(), false);
            }
            for (auto interval : seam->seamIntervals())
            {
                QCOMPARE(interval->isChangeTracking(), false);
                QCOMPARE(interval->hardwareParameters()->isChangeTracking(), false);
                for (auto parameter : interval->hardwareParameters()->parameters())
                {
                    QCOMPARE(parameter->isChangeTracking(), false);
                }
            }
        }
    }
  

    // now enable change tracking
    product->setChangeTrackingEnabled(true);
    // and all should be true
    QCOMPARE(product->isChangeTracking(), true);
    QCOMPARE(product->hardwareParameters()->isChangeTracking(), true);
    for (auto parameter : product->hardwareParameters()->parameters())
    {
        QCOMPARE(parameter->isChangeTracking(), true);
    }
    for (auto seamSeries : product->seamSeries())
    {
        QCOMPARE(seamSeries->isChangeTracking(), true);
        QCOMPARE(seamSeries->hardwareParameters()->isChangeTracking(), true);
        for (auto parameter : seamSeries->hardwareParameters()->parameters())
        {
            QCOMPARE(parameter->isChangeTracking(), true);
        }
        for (auto seam : seamSeries->seams())
        {
            QCOMPARE(seam->isChangeTracking(), true);
            QCOMPARE(seam->hardwareParameters()->isChangeTracking(), true);
            for (auto parameter : seam->hardwareParameters()->parameters())
            {
                QCOMPARE(parameter->isChangeTracking(), true);
            }
            for (auto interval : seam->seamIntervals())
            {
                QCOMPARE(interval->isChangeTracking(), true);
                QCOMPARE(interval->hardwareParameters()->isChangeTracking(), true);
                for (auto parameter : interval->hardwareParameters()->parameters())
                {
                    QCOMPARE(parameter->isChangeTracking(), true);
                }
            }
        }
    }
  

    // get the changes
    auto productChanges = product->changes();
    QCOMPARE(productChanges.size(), 1);
    auto productWrapperObject = productChanges.first().toObject();
    QCOMPARE(productWrapperObject.contains(QLatin1String("product")), true);
    auto productObject = productWrapperObject.value(QLatin1String("product")).toObject();
    QCOMPARE(productObject.contains(QLatin1String("name")), true);
    QCOMPARE(productObject.contains(QLatin1String("type")), true);
    QCOMPARE(productObject.contains(QLatin1String("uuid")), true);
    QCOMPARE(productObject.value(QLatin1String("name")).toString(), product->name());
    QCOMPARE(productObject.value(QLatin1String("type")).toInt(), product->type());
    QCOMPARE(QUuid(productObject.value(QLatin1String("uuid")).toString()), product->uuid());

    // let's do some changes
    product->setName(QStringLiteral("test"));
    product->setType(product->type() + 1);
    product->setEndless(!product->isEndless());
    productChanges = product->changes();
    // each change and the product
    QCOMPARE(productChanges.size(), 4);
    product->seamSeries().front()->seams().front()->setName(QStringLiteral("Test seam"));
    productChanges = product->changes();
    QCOMPARE(productChanges.size(), 5);
    product->hardwareParameters()->parameters().front()->setName(QStringLiteral("Hardware parameter"));
    productChanges = product->changes();
    QCOMPARE(productChanges.size(), 6);
  

    int changeCounter = 0;
    int productCounter = 0;
    int seamSeriesCounter = 0;
    int hardwareParametersCounter = 0;
    int filterParametersCounter = 0;

    for (auto element : productChanges)
    {
        auto object = element.toObject();
        if (object.contains(QLatin1String("change")))
        {
            changeCounter++;
        }
        if (object.contains(QLatin1String("product")))
        {
            productCounter++;
        }
        if (object.contains(QLatin1String("seamSeries")))
        {
            seamSeriesCounter++;
        }
        if (object.contains(QLatin1String("hardwareParameters")))
        {
            hardwareParametersCounter++;
        }
        if (object.contains(QLatin1String("parameterSets")))
        {
            filterParametersCounter++;
        }
    }
    QCOMPARE(changeCounter, 3);
    QCOMPARE(productCounter, 1);
    QCOMPARE(seamSeriesCounter, 1);
    QCOMPARE(hardwareParametersCounter, 1);
  //  QCOMPARE(filterParametersCounter, 1);
   
}

void TestProductLoading::testLoadFilterParameterSets()
{    
    const QString fileName = QFINDTESTDATA("testdata/products/hardwareparams.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> origProduct{Product::fromJson(fileName)};
    QVERIFY(origProduct);   
   
    QTemporaryFile jsonFile;
    QVERIFY(jsonFile.open());
    origProduct->toJson(&jsonFile);
    QVERIFY(jsonFile.flush());
    
    origProduct->setChangeTrackingEnabled(true);
    for (auto ps : origProduct->filterParameterSets())
    {
        QCOMPARE(ps->isChangeTracking(), true);
        for (auto parameter : ps->parameters())
        {
            QCOMPARE(parameter->isChangeTracking(), true);
        }
    }
   
    std::unique_ptr<Product> product{Product::fromJson(jsonFile.fileName())};
    QVERIFY(product);
    auto parameterSets = product->filterParameterSets();
    QCOMPARE(parameterSets.size(), 1u);
    QCOMPARE(parameterSets.front()->uuid(), QUuid(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QCOMPARE(parameterSets.back()->parameters().size(), 1u);
    QVERIFY(product->containsFilterParameterSet(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QCOMPARE(product->filterParameterSet(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")), parameterSets.front());

    auto parameter = parameterSets.back()->parameters().front();
    QCOMPARE(parameter->uuid(), QUuid(QByteArrayLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(parameter->name(), QStringLiteral("Test"));
    QCOMPARE(parameter->typeId(), QUuid(QByteArrayLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE")));
    QVERIFY(parameter->value().canConvert<bool>());
    QCOMPARE(parameter->value().toBool(), true);
    QCOMPARE(parameter->property("value").toBool(), true);
    QCOMPARE(parameter->value().isValid(), true);
    QCOMPARE(parameter->filterId(), QUuid(QByteArrayLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE")));

    auto parameterSet = product->filterParameterSet(QUuid(QByteArrayLiteral("6F086211-FBD4-4493-A580-6FF11E4925DD")));
    QVERIFY(!parameterSet);
    parameterSet = product->filterParameterSet(QUuid(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QCOMPARE(parameterSet, parameterSets.front());
    QVERIFY(!product->filterParameterSet(QUuid(QByteArrayLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE"))));

    QSignalSpy destroyedSpy(parameterSet, &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());
    product->discardFilterParameterSet(parameterSet);
    QVERIFY(destroyedSpy.wait());
    QVERIFY(product->containsFilterParameterSet(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    parameterSet = product->filterParameterSet(QUuid(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QVERIFY(!parameterSet);
    product->ensureAllFilterParameterSetsLoaded();
    QVERIFY(product->containsFilterParameterSet(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    parameterSet = product->filterParameterSet(QUuid(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QVERIFY(parameterSet);

    QSignalSpy destroyedSpy2(parameterSet, &QObject::destroyed);
    product->removeFilterParameterSet(parameterSet->uuid());
    QVERIFY(!product->filterParameterSet(parameterSet->uuid()));
    QVERIFY(destroyedSpy2.wait());
}

void TestProductLoading::testLoadFilterParameterGroups()
{
    const QString fileName = QFINDTESTDATA("testdata/products/filterparams.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> origProduct {Product::fromJson(fileName)};  
    QVERIFY(origProduct);   
   
    QTemporaryFile jsonFile;
    QVERIFY(jsonFile.open());
    origProduct->toJson(&jsonFile);
    QVERIFY(jsonFile.flush());     
    
    const QString fileName2 = QFINDTESTDATA("testdata/products/filterparamsTest.json");
    QVERIFY(!fileName2.isEmpty());
    std::unique_ptr<Product> origProduct2 {Product::fromJson(fileName2)};  
    QVERIFY(origProduct2);      
    
    // Ruta específica para el nuevo archivo JSON
    QString path = QDir::currentPath() + "/testFilterOutput2.json";
    QFile jsonFile2(path);
    QVERIFY(jsonFile2.open(QIODevice::WriteOnly | QIODevice::Text));
    origProduct2->toJson(&jsonFile2);
    QVERIFY(jsonFile2.flush());   
    
    //Test changesTracking feature
    origProduct2->setChangeTrackingEnabled(true);
    for (auto ps : origProduct2->filterParameterSets())
    {
        QCOMPARE(ps->isChangeTracking(), true);
        for (auto parameter : ps->parameters())
        {
            QCOMPARE(parameter->isChangeTracking(), true);
        }
    }
     
    std::unique_ptr<Product> product{Product::fromJson(jsonFile2.fileName())};
    QVERIFY(product);    
  
    auto parameterSets = product->filterParameterSets();    
    QCOMPARE(parameterSets.size(), 1u);
    QCOMPARE(parameterSets.front()->uuid(), QUuid(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QCOMPARE(parameterSets.front()->parameters().size(), 4u);
    
    auto paramsGroups = parameterSets.front()->parameterGroups();
    QCOMPARE(paramsGroups.front()->filterId(), QUuid(QByteArrayLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(paramsGroups.front()->typeId(), QUuid(QByteArrayLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(paramsGroups.back()->typeId(), QUuid(QByteArrayLiteral("fdd30629-cba3-4d19-8f44-ee03b7a441a9")));
    
    
    auto parameter = parameterSets.back()->parameters().front();
    QCOMPARE(parameter->uuid(), QUuid(QByteArrayLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE")));
    QCOMPARE(parameter->name(), QStringLiteral("Param1"));
   
    QVERIFY(parameter->value().canConvert<bool>());
    QCOMPARE(parameter->value().toBool(), true);
    QCOMPARE(parameter->property("value").toBool(), true);
    QCOMPARE(parameter->value().isValid(), true);
    
    auto parameter2 = paramsGroups.back()->parameters().back();
    QCOMPARE(parameter2->value().toDouble(), 0.5);    
    
    auto parameterSet = product->filterParameterSet(QUuid(QByteArrayLiteral("b450c464-4c40-4608-bc50-4b2fd895af75")));
    QSignalSpy destroyedSpy(parameterSet, &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());
    product->removeFilterParameterSet(parameterSet->uuid());
    QVERIFY(!product->filterParameterSet(parameterSet->uuid()));
    QVERIFY(destroyedSpy.wait());
    
}


void TestProductLoading::testAssemblyImage()
{
    Product p{QUuid::createUuid()};
    QCOMPARE(p.assemblyImage(), QString());
    QSignalSpy assemblyImageSpy{&p, &Product::assemblyImageChanged};
    QVERIFY(assemblyImageSpy.isValid());
    p.setAssemblyImage(QStringLiteral("foo.png"));
    QCOMPARE(p.assemblyImage(), QStringLiteral("foo.png"));
    QCOMPARE(assemblyImageSpy.count(), 1);
    // setting same should not emit
    p.setAssemblyImage(QStringLiteral("foo.png"));
    QCOMPARE(assemblyImageSpy.count(), 1);

    QTemporaryFile file;
    QVERIFY(file.open());
    p.setFilePath(file.fileName());
    QVERIFY(p.save());

    // let's load again
    std::unique_ptr<Product> p2{Product::fromJson(file.fileName())};
    QCOMPARE(p2->assemblyImage(), QStringLiteral("foo.png"));
}

QTEST_GUILESS_MAIN(TestProductLoading)
#include "testProductLoading.moc"
