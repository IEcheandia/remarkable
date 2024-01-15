#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QTemporaryFile>

#include "../src/attribute.h"
#include "../src/copyMode.h"
#include "../src/linkedSeam.h"
#include "../src/parameterSet.h"
#include "../src/product.h"
#include "../src/productError.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "../src/seamInterval.h"

#include "common/graph.h"

using precitec::storage::Attribute;
using precitec::storage::CopyMode;
using precitec::storage::LinkedSeam;
using precitec::storage::ParameterSet;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SubGraphReference;
using precitec::storage::SingleGraphReference;
using precitec::storage::LinkedGraphReference;
using precitec::storage::valueOrDefault;
using precitec::storage::SeamSeries;

namespace
{
void verifyHasGraph(const Seam* seam, const QUuid& expected, const std::string& scenario)
{
    QVERIFY2(seam->usesSubGraph() == false, scenario.c_str());
    QVERIFY2(seam->graph() == expected, scenario.c_str());
    QVERIFY2(seam->subGraphs().empty(), scenario.c_str());

    QVERIFY2(hasGraph(seam->graphReference()), scenario.c_str());
    QVERIFY2(!hasSubGraphs(seam->graphReference()), scenario.c_str());
    QCOMPARE(std::get<SingleGraphReference>(seam->graphReference()).value, expected);
}

void verifyHasSubGraph(const Seam* seam, const std::vector<QUuid>& expected, const std::string& scenario)
{
    QVERIFY2(seam->usesSubGraph() == true, scenario.c_str());
    QVERIFY2(seam->graph().isNull(), scenario.c_str());
    const auto& actual = seam->subGraphs();
    QVERIFY2(std::equal(actual.begin(), actual.end(), expected.begin(), expected.end()), scenario.c_str());

    QVERIFY2(!hasGraph(seam->graphReference()), scenario.c_str());
    QVERIFY2(hasSubGraphs(seam->graphReference()), scenario.c_str());
    const auto actualFromRef = std::get<SubGraphReference>(seam->graphReference()).value;
    QVERIFY2(std::equal(actualFromRef.begin(), actualFromRef.end(), expected.begin(), expected.end()), scenario.c_str());
}

bool graphIsLinkedTo(const Seam* toTest, const Seam* target)
{
    return valueOrDefault<LinkedGraphReference>(toTest->graphReference()) == target->uuid();
}
}

class ProductTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCreateSeam_data();
    void testCreateSeam();
    void testChangePath();
    void testSaveNoPath();
    void testSave();
    void testCreateHardwareParametersOnMeasureTask_data();
    void testCreateHardwareParametersOnMeasureTask();
    void testCreateRemoveParameter_data();
    void testCreateRemoveParameter();
    void testAddFilterParameterSet();
    void testSeamAssemblyImagePosition();
    void testSeamSubGraph_data();
    void testSeamSubGraph();
    void testLinkedGraph_data();
    void testLinkedGraph();
    void testDuplicate_data();
    void testDuplicate();
    void testDuplicateSeam_data();
    void testDuplicateSeam();
    void testCreateSeamSeries_data();
    void testCreateSeamSeries();
};

void ProductTest::testCreateSeam_data()
{
    QTest::addColumn<bool>("changeTracking");

    QTest::newRow("enabled") << true;
    QTest::newRow("disabled") << false;
}

void ProductTest::testCreateSeam()
{
    Product p{QUuid::createUuid()};
    QCOMPARE(p.isChangeTracking(), false);
    QFETCH(bool, changeTracking);
    p.setChangeTrackingEnabled(changeTracking);
    QCOMPARE(p.isChangeTracking(), changeTracking);

    QSignalSpy seamsChangedSpy(&p, &Product::seamsChanged);
    QVERIFY(seamsChangedSpy.isValid());

    QVERIFY(p.seamSeries().empty());
    QVERIFY(!p.createSeam());
    p.createFirstSeamSeries();
    QVERIFY(!p.seamSeries().empty());
    // calling again should not create another seam series
    p.createFirstSeamSeries();
    QCOMPARE(p.seamSeries().size(), 1);

    QCOMPARE(seamsChangedSpy.count(), 0);
    auto seam = p.createSeam();
    QCOMPARE(seamsChangedSpy.count(), 1);
    QVERIFY(seam);
    QCOMPARE(seam->isChangeTracking(), changeTracking);
    QCOMPARE(seam->seamSeries()->isChangeTracking(), changeTracking);
    QCOMPARE(seam->seamSeries(), p.seamSeries().front());
    QVERIFY(!seam->seamIntervals().empty());
    QCOMPARE(seam->number(), 0u);
    QCOMPARE(seam->visualNumber(), 1u);
    QCOMPARE(seam->positionInAssemblyImage(), QPointF(-1, -1));
    const auto intervalCount = seam->seamIntervals().size();
    seam->createFirstSeamInterval();
    QCOMPARE(seam->seamIntervals().size(), intervalCount);
    auto s = p.createSeam();
    QCOMPARE(s->number(), 1u);
    QCOMPARE(s->visualNumber(), 2u);
    s = p.createSeam();
    QCOMPARE(s->number(), 2u);
    QCOMPARE(s->visualNumber(), 3u);
    s = p.createSeam();
    QCOMPARE(s->number(), 3u);
    QCOMPARE(s->visualNumber(), 4u);

    QCOMPARE(seamsChangedSpy.count(), 4);

    p.addFilterParameterSet(new ParameterSet{QUuid::createUuid(), &p});
    seam->setGraphParamSet(p.filterParameterSets().front()->uuid());

    // duplicate first seam
    seam->setName(QStringLiteral("Copy test"));
    const auto copyMode = CopyMode::WithDifferentIds;
    auto copied = p.seamSeries().front()->createSeamCopy(copyMode, seam);
    QCOMPARE(copied->number(), 4u);
    QCOMPARE(copied->visualNumber(), 5u);
    QCOMPARE(copied->name(), QStringLiteral("Copy test"));
    QVERIFY(copied->graphParamSet() != seam->graphParamSet());
    QVERIFY(!copied->graphParamSet().isNull());
    QVERIFY(p.filterParameterSet(copied->graphParamSet()));
    QVERIFY(copied->firstSeamInterval());

    // test findAbstractMeasureTask
    QCOMPARE(p.findMeasureTask(seam->uuid()), seam);
    QCOMPARE(p.findMeasureTask(seam->firstSeamInterval()->uuid()), seam->firstSeamInterval());
    QCOMPARE(p.findMeasureTask(seam->seamSeries()->uuid()), seam->seamSeries());
    QVERIFY(!p.findMeasureTask({}));
    QVERIFY(!p.findMeasureTask(p.uuid()));

    QCOMPARE(seamsChangedSpy.count(), 5);
    QSignalSpy destroyedSpy(seam, &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());
    seam->seamSeries()->destroySeam(seam);
    QCOMPARE(seamsChangedSpy.count(), 6);
    QVERIFY(destroyedSpy.wait());
}

void ProductTest::testChangePath()
{
    Product p{QUuid::createUuid()};
    QCOMPARE(p.filePath(), QString());
    QSignalSpy pathChangedSpy(&p, &Product::filePathChanged);
    QVERIFY(pathChangedSpy.isValid());

    p.setFilePath(QString());
    QVERIFY(pathChangedSpy.isEmpty());
    p.setFilePath(QStringLiteral("foo"));
    QCOMPARE(p.filePath(), QStringLiteral("foo"));
    QCOMPARE(pathChangedSpy.count(), 1);

    // setting same doesn't change
    p.setFilePath(QStringLiteral("foo"));
    QCOMPARE(pathChangedSpy.count(), 1);
}

void ProductTest::testSaveNoPath()
{
    Product p{QUuid::createUuid()};
    QCOMPARE(p.save(), false);
}

void ProductTest::testSave()
{
    QTemporaryFile file;
    QVERIFY(file.open());
    Product p{QUuid::createUuid()};
    p.setFilePath(file.fileName());
    QVERIFY(p.save());

    std::unique_ptr<Product> p2{Product::fromJson(file.fileName())};
    QCOMPARE(p2->uuid(), p.uuid());
}

void ProductTest::testCreateHardwareParametersOnMeasureTask_data()
{
    QTest::addColumn<bool>("changeTracking");

    QTest::newRow("enabled") << true;
    QTest::newRow("disabled") << false;
}

void ProductTest::testCreateHardwareParametersOnMeasureTask()
{
    Product p{QUuid::createUuid()};
    QCOMPARE(p.isChangeTracking(), false);
    QFETCH(bool, changeTracking);
    p.setChangeTrackingEnabled(changeTracking);
    QCOMPARE(p.isChangeTracking(), changeTracking);

    p.createFirstSeamSeries();
    auto series = p.seamSeries().front();
    QVERIFY(series);
    QVERIFY(!series->hardwareParameters());
    QSignalSpy hardwareParametersChangedSpy(series, &SeamSeries::hardwareParametersChanged);
    QVERIFY(hardwareParametersChangedSpy.isValid());
    series->createHardwareParameters();
    QCOMPARE(hardwareParametersChangedSpy.count(), 1);
    QVERIFY(series->hardwareParameters());
    QCOMPARE(series->hardwareParameters()->isChangeTracking(), changeTracking);
    // creating again should not do anything
    series->createHardwareParameters();
    QCOMPARE(hardwareParametersChangedSpy.count(), 1);
}

void ProductTest::testCreateRemoveParameter_data()
{
    QTest::addColumn<bool>("changeTracking");

    QTest::newRow("enabled") << true;
    QTest::newRow("disabled") << false;
}

void ProductTest::testCreateRemoveParameter()
{
    Product p{QUuid::createUuid()};
    QCOMPARE(p.isChangeTracking(), false);
    QFETCH(bool, changeTracking);
    p.setChangeTrackingEnabled(changeTracking);
    QCOMPARE(p.isChangeTracking(), changeTracking);

    auto *ps = new ParameterSet{QUuid::createUuid(), &p};
    QCOMPARE(ps->isChangeTracking(), false);
    p.addFilterParameterSet(ps);
    QCOMPARE(ps->isChangeTracking(), changeTracking);
    QCOMPARE(ps->parameters().size(), 0);

    auto *ps2 = new ParameterSet{QUuid::createUuid(), &p};
    p.addFilterParameterSet(ps2);

    Attribute a{QUuid::createUuid()};
    a.setDefaultValue(5);
    auto parameter = ps->createParameter(QUuid::createUuid(), &a, QUuid::createUuid());
    QVERIFY(parameter);
    QCOMPARE(parameter->isChangeTracking(), changeTracking);
    QCOMPARE(parameter->value(), QVariant(5));
    QCOMPARE(ps->parameters().size(), 1);
    QCOMPARE(ps->parameters().front(), parameter);
    QCOMPARE(ps->parameters().front()->value(), QVariant(5));

    auto filterParameter = parameter->toFilterParameter();
    // type is still unknown
    QVERIFY(!filterParameter);
    // set some useful things
    parameter->setType(precitec::storage::Parameter::DataType::Integer);
    parameter->setTypeId(QUuid::createUuid());
    parameter->setName(QStringLiteral("foo"));
    filterParameter = parameter->toFilterParameter();
    QVERIFY(filterParameter);
    QCOMPARE(QUuid(QByteArray::fromStdString(filterParameter->parameterID().toString())), parameter->uuid());
    QCOMPARE(QUuid(QByteArray::fromStdString(filterParameter->typID().toString())), parameter->typeId());
    QCOMPARE(filterParameter->value<int>(), 5);
    QCOMPARE(filterParameter->name(), std::string("foo"));
    QCOMPARE(filterParameter->type(), precitec::TInt);

    // create in second parameterset
    auto *parameter2 = ps2->createParameter(*filterParameter.get());
    QVERIFY(parameter2);
    QCOMPARE(parameter2->uuid(), parameter->uuid());
    QCOMPARE(parameter2->name(), parameter->name());
    QCOMPARE(parameter2->type(), parameter->type());
    QCOMPARE(parameter2->typeId(), parameter->typeId());
    QCOMPARE(parameter2->filterId(), parameter->filterId());
    QCOMPARE(parameter2->value(), parameter->value());

    parameter->setType(precitec::storage::Parameter::DataType::SeamFigure);
    filterParameter = parameter->toFilterParameter();
    QVERIFY(filterParameter);
    QCOMPARE(filterParameter->type(), precitec::TInt);
    QCOMPARE(filterParameter->value<int>(), 5);

    parameter->setType(precitec::storage::Parameter::DataType::WobbleFigure);
    filterParameter = parameter->toFilterParameter();
    QVERIFY(filterParameter);
    QCOMPARE(filterParameter->type(), precitec::TInt);
    QCOMPARE(filterParameter->value<int>(), 5);

    parameter->setType(precitec::storage::Parameter::DataType::UnsignedInteger);
    filterParameter = parameter->toFilterParameter();
    QVERIFY(filterParameter);
    QCOMPARE(filterParameter->type(), precitec::TUInt);
    QCOMPARE(filterParameter->value<uint>(), 5u);

    parameter2 = ps2->createParameter(*filterParameter.get());
    QVERIFY(parameter2);
    QCOMPARE(parameter2->uuid(), parameter->uuid());
    QCOMPARE(parameter2->name(), parameter->name());
    QCOMPARE(parameter2->type(), parameter->type());
    QCOMPARE(parameter2->typeId(), parameter->typeId());
    QCOMPARE(parameter2->filterId(), parameter->filterId());
    QCOMPARE(parameter2->value(), parameter->value());

    parameter->setType(precitec::storage::Parameter::DataType::Float);
    filterParameter = parameter->toFilterParameter();
    QCOMPARE(filterParameter->type(), precitec::TFloat);
    QCOMPARE(filterParameter->value<float>(), 5.0f);

    parameter2 = ps2->createParameter(*filterParameter.get());
    QVERIFY(parameter2);
    QCOMPARE(parameter2->uuid(), parameter->uuid());
    QCOMPARE(parameter2->name(), parameter->name());
    QCOMPARE(parameter2->type(), parameter->type());
    QCOMPARE(parameter2->typeId(), parameter->typeId());
    QCOMPARE(parameter2->filterId(), parameter->filterId());
    QCOMPARE(parameter2->value(), parameter->value());

    parameter->setType(precitec::storage::Parameter::DataType::Double);
    filterParameter = parameter->toFilterParameter();
    QCOMPARE(filterParameter->type(), precitec::TDouble);
    QCOMPARE(filterParameter->value<double>(), 5.0);

    parameter2 = ps2->createParameter(*filterParameter.get());
    QVERIFY(parameter2);
    QCOMPARE(parameter2->uuid(), parameter->uuid());
    QCOMPARE(parameter2->name(), parameter->name());
    QCOMPARE(parameter2->type(), parameter->type());
    QCOMPARE(parameter2->typeId(), parameter->typeId());
    QCOMPARE(parameter2->filterId(), parameter->filterId());
    QCOMPARE(parameter2->value(), parameter->value());

    parameter->setType(precitec::storage::Parameter::DataType::Boolean);
    parameter->setValue(true);
    filterParameter = parameter->toFilterParameter();
    QCOMPARE(filterParameter->type(), precitec::TBool);
    QCOMPARE(filterParameter->value<bool>(), true);

    parameter2 = ps2->createParameter(*filterParameter.get());
    QVERIFY(parameter2);
    QCOMPARE(parameter2->uuid(), parameter->uuid());
    QCOMPARE(parameter2->name(), parameter->name());
    QCOMPARE(parameter2->type(), parameter->type());
    QCOMPARE(parameter2->typeId(), parameter->typeId());
    QCOMPARE(parameter2->filterId(), parameter->filterId());
    QCOMPARE(parameter2->value(), parameter->value());

    QSignalSpy destroyedSpy{parameter, &QObject::destroyed};
    QVERIFY(destroyedSpy.isValid());
    ps->removeParameter(parameter);
    QCOMPARE(ps->parameters().size(), 0);
    QVERIFY(destroyedSpy.wait());

    // test default value functionality
    parameter = ps->createParameter(QUuid::createUuid(), &a, QUuid::createUuid(), 3);
    QVERIFY(parameter);
    QCOMPARE(parameter->value(), QVariant(3));
}

void ProductTest::testAddFilterParameterSet()
{
    Product p{QUuid::createUuid()};
    QVERIFY(p.filterParameterSets().empty());
    auto *ps = new ParameterSet{QUuid::createUuid(), &p};
    p.addFilterParameterSet(ps);
    QCOMPARE(p.filterParameterSet(ps->uuid()), ps);

    // now add another parameter set with same uuid, that should destroy our existing
    QSignalSpy destroyedSpy(ps, &QObject::destroyed);
    QVERIFY(destroyedSpy.isValid());

    auto *ps2 = new ParameterSet{ps->uuid(), &p};
    p.addFilterParameterSet(ps2);
    QCOMPARE(p.filterParameterSet(ps->uuid()), ps2);
    QVERIFY(destroyedSpy.wait());

    QSignalSpy destroyedSpy2(ps2, &QObject::destroyed);
    QVERIFY(destroyedSpy2.isValid());
    p.addFilterParameterSet(ps2);
    qApp->processEvents();
    QVERIFY(destroyedSpy2.isEmpty());
}

void ProductTest::testSeamAssemblyImagePosition()
{
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QSignalSpy positionChangedSpy{seam, &Seam::positionInAssemblyImageChanged};
    QVERIFY(positionChangedSpy.isValid());

    seam->setPositionInAssemblyImage({100, 200});
    QCOMPARE(seam->positionInAssemblyImage(), QPointF(100, 200));
    QCOMPARE(positionChangedSpy.count(), 1);

    // setting same should not change
    seam->setPositionInAssemblyImage({100, 200});
    QCOMPARE(positionChangedSpy.count(), 1);

    // now persist
    const auto object = seam->toJson();
    auto seam2{Seam::fromJson(object, p.seamSeries().front())};
    QCOMPARE(seam2->positionInAssemblyImage(), seam->positionInAssemblyImage());
}

void ProductTest::testSeamSubGraph_data()
{
    QTest::addColumn<bool>("changeTracking");

    QTest::newRow("enabled") << true;
    QTest::newRow("disabled") << false;
}

void ProductTest::testSeamSubGraph()
{
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    QVERIFY(seam);
    verifyHasSubGraph(seam, {}, "Initially has empty subGraph");

    QCOMPARE(p.isChangeTracking(), false);
    QFETCH(bool, changeTracking);
    p.setChangeTrackingEnabled(changeTracking);
    QCOMPARE(p.isChangeTracking(), changeTracking);

    QSignalSpy graphChangedSpy{seam, &Seam::graphChanged};
    QVERIFY(graphChangedSpy.isValid());
    QSignalSpy subGraphChangedSpy{seam, &Seam::subGraphChanged};
    QVERIFY(subGraphChangedSpy.isValid());
    int expectedGraphChangedCount = 0;
    int expectedSubGraphChangedCount = 0;

    const auto firstUuid = QUuid::createUuid();
    seam->setGraph(firstUuid);
    verifyHasGraph(seam, firstUuid, "First graph set.");
    QCOMPARE(graphChangedSpy.size(), ++expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), expectedSubGraphChangedCount);

    std::vector<QUuid> graphIds{QUuid::createUuid(), QUuid::createUuid(), QUuid::createUuid()};
    seam->setSubGraphs(graphIds);
    verifyHasSubGraph(seam, graphIds, "Replace graph by subGraphs.");
    QCOMPARE(graphChangedSpy.size(), ++expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), expectedSubGraphChangedCount);

    auto p2 = Product::fromJson(p.toJson(), &p);
    auto seam2 = p2->seams().front();
    verifyHasSubGraph(seam2, graphIds, "JSON-based copy has identical subGraphs.");

    // duplicate this seam to verify that subgraphs are duplicated correctly
    const auto copyMode = CopyMode::WithDifferentIds;
    auto duplicatedSeam2 = seam2->duplicate(copyMode, nullptr);
    verifyHasSubGraph(duplicatedSeam2, graphIds, "Duplicate has identical subGraphs.");

    // set a differently ordered list of subGraphs
    std::vector<QUuid> const permutation{graphIds.at(2), graphIds.at(1), graphIds.at(0)};
    seam->setSubGraphs(permutation);
    QCOMPARE(graphChangedSpy.size(), expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), ++expectedSubGraphChangedCount);
    verifyHasSubGraph(seam, permutation, "Replace subGraphs by different permutation.");

    seam->setSubGraphs({});
    QCOMPARE(graphChangedSpy.size(), expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), ++expectedSubGraphChangedCount);
    verifyHasSubGraph(seam, {}, "Replace subGraphs by empty subGraph.");

    seam->setSubGraphs(graphIds);
    QCOMPARE(graphChangedSpy.size(), expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), ++expectedSubGraphChangedCount);
    verifyHasSubGraph(seam, graphIds, "Replace subGraphs by original subGraphs.");

    seam->setGraph(firstUuid);
    QCOMPARE(graphChangedSpy.size(), ++expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), expectedSubGraphChangedCount);
    verifyHasGraph(seam, firstUuid, "Replace the subGraphs by a single graph.");

    seam->setSubGraphs({});
    QCOMPARE(graphChangedSpy.size(), ++expectedGraphChangedCount);
    QCOMPARE(subGraphChangedSpy.size(), expectedSubGraphChangedCount);
    verifyHasGraph(seam, firstUuid, "When a graph is set, it cannot be overwritten by an empty subGraph.");

    // verify change tracker
    if (changeTracking)
    {
        auto changes = seam->changes();
        QCOMPARE(changes.size(), 6);
        QCOMPARE(changes.at(0).toObject().value(QLatin1String("description")), QStringLiteral("PropertyChange"));
        QCOMPARE(changes.at(1).toObject().value(QLatin1String("description")), QStringLiteral("PropertyChange"));
        QCOMPARE(changes.at(2).toObject().value(QLatin1String("description")), QStringLiteral("PropertyChange"));

        QCOMPARE(changes.at(0).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("propertyName")), QStringLiteral("graph"));
        QCOMPARE(changes.at(1).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("propertyName")), QStringLiteral("graph"));
        QCOMPARE(changes.at(2).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("propertyName")), QStringLiteral("graph"));

        QCOMPARE(changes.at(0).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("oldValue")).toVariant().toUuid(), QUuid());
        QCOMPARE(changes.at(0).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("newValue")).toVariant().toUuid(), firstUuid);

        QCOMPARE(changes.at(1).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("oldValue")).toVariant().toUuid(), firstUuid);
        QCOMPARE(changes.at(1).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("newValue")).toString(), graphIds.at(0).toString() + graphIds.at(1).toString() + graphIds.at(2).toString());

        QCOMPARE(changes.at(2).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("oldValue")).toString(), graphIds.at(0).toString() + graphIds.at(1).toString() + graphIds.at(2).toString());
        QCOMPARE(changes.at(5).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("newValue")).toVariant().toUuid(), firstUuid);
    }
    else
    {
        QCOMPARE(seam->changes().size(), 0);
    }
}

void ProductTest::testLinkedGraph_data()
{
    QTest::addColumn<bool>("changeTracking");
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("tracking changes and changing UUIDs") << true << true;
    QTest::newRow("tracking changes and not changing UUIDs") << true << false;
    QTest::newRow("not tracking changes and changing UUIDs") << false << true;
    QTest::newRow("not tracking changes and not changing UUIDs") << false << false;
}

void ProductTest::testLinkedGraph()
{
    QFETCH(bool, changeTracking);
    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto* original = p.createSeam();
    QVERIFY(original);

    const auto inputGraphUuid = QUuid::createUuid();
    original->setGraph(inputGraphUuid);

    auto* linked = p.createSeam();

    // initialize both seams with different parameter sets
    const auto paramSet = QUuid::createUuid();
    original->setGraphParamSet(paramSet);
    const auto differentParamSet = QUuid::createUuid();
    linked->setGraphParamSet(differentParamSet);
    QCOMPARE(original->graphParamSet(), paramSet);
    QCOMPARE(linked->graphParamSet(), differentParamSet);

    // track changes starting from this point on
    p.setChangeTrackingEnabled(changeTracking);
    QCOMPARE(p.isChangeTracking(), changeTracking);

    // link the two seams, now the linked seam should return the parameters of the original
    linked->setGraphReference(LinkedGraphReference{original->uuid()});
    QVERIFY(hasLinkedGraph(linked->graphReference()));
    QCOMPARE(linked->graphParamSet(), paramSet);

    // Set the same reference again, should not alter the result and should not show up in the changes.
    linked->setGraphReference(LinkedGraphReference{original->uuid()});
    QVERIFY(hasLinkedGraph(linked->graphReference()));

    // Try to set a different parameter set on the linked seam. Should have no effect.
    linked->setGraphParamSet(QUuid::createUuid());
    QCOMPARE(original->graphParamSet(), paramSet);
    QCOMPARE(linked->graphParamSet(), paramSet);

    // Change the parameter set of the original seam. This should also be visible through the linked seam.
    original->setGraphParamSet(differentParamSet);
    QCOMPARE(original->graphParamSet(), differentParamSet);
    QCOMPARE(linked->graphParamSet(), differentParamSet);

    // Creating a copy of the product via JSON conversion: the copy should still be linked to the same seam
    const auto* const p2 = Product::fromJson(p.toJson(), &p);
    const auto* const jsonCopiedOriginal = p2->seams().at(0);
    const auto* const jsonCopiedLinked = p2->seams().at(1);
    QVERIFY(hasGraph(jsonCopiedOriginal->graphReference()));
    QVERIFY(hasLinkedGraph(jsonCopiedLinked->graphReference()));
    QCOMPARE(jsonCopiedOriginal->uuid(), original->uuid());
    QVERIFY(graphIsLinkedTo(jsonCopiedLinked, jsonCopiedOriginal));

    // Creating a copy of the product via duplicate: the linked duplicate should always refer to the
    // duplicate of the link it was originally refering to. When changing UUIDs, this means, references
    // are adapted and no longer refer to the original seam.
    const auto* const p3 = p.duplicate(copyMode);
    const auto* const duplicatedOriginal = p3->seams().at(0);
    const auto* const duplicatedLinked = p3->seams().at(1);
    QVERIFY(graphIsLinkedTo(duplicatedLinked, duplicatedOriginal));
    QCOMPARE(!graphIsLinkedTo(duplicatedLinked, original), changeUuid);

    // Duplicating a single seam. It should always point to the original seam, even if its own uuid changed.
    const auto* singleSeamDuplicated = p.seamSeries().at(0)->createSeamCopy(copyMode, linked);
    QVERIFY(graphIsLinkedTo(singleSeamDuplicated, original));

    // Unlink the linked seam. The seam no longer refers to a valid parameter set.
    const auto finalGraphUuid = QUuid::createUuid();
    linked->setGraphReference(SingleGraphReference{finalGraphUuid});
    QVERIFY(!hasLinkedGraph(linked->graphReference()));
    QVERIFY(linked->graphParamSet().isNull());

    // Check results of the changeTracking.
    if (changeTracking)
    {
        auto changes = linked->changes();
        QCOMPARE(changes.size(), 2);

        QCOMPARE(changes.at(0).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("oldValue")).toVariant().toUuid(), QUuid());
        QCOMPARE(changes.at(0).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("newValue")).toString(), QString{"Linked to Seam "} + original->uuid().toString());
        QCOMPARE(changes.at(1).toObject().value(QLatin1String("change")).toObject().value(QStringLiteral("newValue")).toString(), finalGraphUuid.toString());
    }
    else
    {
        QCOMPARE(linked->changes().size(), 0);
    }
}

void ProductTest::testDuplicate_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}


void ProductTest::testDuplicate()
{
    const QString fileName = QFINDTESTDATA("../../App_Storage/autotests/testdata/products/product2.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> origProduct{Product::fromJson(fileName)};
    QVERIFY(origProduct);

    // create a seam link in origProduct
    QCOMPARE(origProduct->seamSeries().front()->seams().back()->metaObject()->inherits(&LinkedSeam::staticMetaObject), false);
    origProduct->seamSeries().front()->createSeamLink(origProduct->seamSeries().front()->seams().front(), QStringLiteral("5"));
    QCOMPARE(origProduct->seamSeries().front()->seams().back()->metaObject()->inherits(&LinkedSeam::staticMetaObject), true);

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto product = origProduct->duplicate(copyMode, origProduct.get());
    QVERIFY(product);
    QCOMPARE(product->name(), QStringLiteral("Product 2"));
    QCOMPARE(product->type(), 2);
    QCOMPARE(product->isEndless(), true);
    bool const hasDifferentProductUuid = product->uuid() != QUuid("2F086211-FBD4-4493-A580-6FF11E4925DE");
    QCOMPARE(hasDifferentProductUuid, changeUuid);
    QCOMPARE(product->seamSeries().size(), 1);
    auto fps = product->filterParameterSets();
    QCOMPARE(fps.size(), 2u);
    bool const hasDifferentFPS0Uuid = fps.at(0)->uuid() != QUuid("6F086211-FBD4-4493-A580-6FF11E4925DD");
    QCOMPARE(hasDifferentFPS0Uuid, changeUuid);
    QCOMPARE(fps.at(0)->parameters().size(), 4u);

    bool const hasDifferentFPS1Uuid = fps.at(1)->uuid() != QUuid("DA165F6A-B562-47D0-A4EC-A0E2FD777DCA");
    QCOMPARE(hasDifferentFPS1Uuid, changeUuid);
    QCOMPARE(fps.at(1)->parameters().size(), 4u);

    QVERIFY(product->hardwareParameters());
    bool const hasDifferentHardwareParamsUuid 
        = product->hardwareParameters()->uuid() != origProduct->hardwareParameters()->uuid();
    QCOMPARE(hasDifferentHardwareParamsUuid, changeUuid);

    QCOMPARE(product->overlyingErrors().size(), 1);
    bool const hasDifferentOverlyingErrorUuid 
         = product->overlyingErrors().at(0)->uuid() != QUuid("2560e134-3ee4-40bd-96ff-a925fafb4d77");
    QCOMPARE(hasDifferentOverlyingErrorUuid, changeUuid);
    QCOMPARE(product->overlyingErrors().at(0)->name(), "My Error Name");
    
    QCOMPARE(product->seamSeries().size(), 1);
    bool const hasDifferentSeamSeriesUuid 
        = product->seamSeries().at(0)->uuid() != origProduct->seamSeries().at(0)->uuid();
    QCOMPARE(hasDifferentSeamSeriesUuid, changeUuid);

    // When keeping the uuid, we also want to copy the linked seam, so we get one more here.
    std::size_t const numLinkedSeams = changeUuid ? 0u : 1u;
    auto seams = product->seamSeries().front()->seams();
    QCOMPARE(seams.size(), 2u + numLinkedSeams);

    auto seam1 = seams.at(0);
    QCOMPARE(seam1->number(), 0);
    QCOMPARE(seam1->visualNumber(), 1);
    QCOMPARE(seam1->triggerDelta(), 1000);
    QCOMPARE(seam1->velocity(), 100000);
    QCOMPARE(seam1->graphParamSet(), fps.at(0)->uuid());
    QCOMPARE(seam1->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(seam1->subGraphs().empty(), true);
    QCOMPARE(seam1->linkedSeams().size(), numLinkedSeams);
    QCOMPARE(seam1->metaObject()->inherits(&LinkedSeam::staticMetaObject), false);
    if(!changeUuid) {
        QCOMPARE(seam1->linkedSeams().at(0)->number(), 5);
    }
    
    auto seam2 = seams.at(1);
    QCOMPARE(seam2->number(), 1);
    QCOMPARE(seam2->visualNumber(), 2);
    QCOMPARE(seam2->triggerDelta(), 1000);
    QCOMPARE(seam2->velocity(), 100000);
    QCOMPARE(seam2->graphParamSet(), fps.at(1)->uuid());
    QVERIFY(seam2->graph().isNull());
    QCOMPARE(seam2->subGraphs().empty(), false);
    QCOMPARE(seam2->subGraphs().size(), 4u);
    QCOMPARE(seam2->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(seam2->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(seam2->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(seam2->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));
    QCOMPARE(seam2->linkedSeams().empty(), true);
    QCOMPARE(seam2->metaObject()->inherits(&LinkedSeam::staticMetaObject), false);

    auto intervals = seam1->seamIntervals();
    QCOMPARE(intervals.size(), 3u);
    auto interval1 = intervals.at(0);
    auto interval2 = intervals.at(1);
    auto interval3 = intervals.at(2);

    QCOMPARE(interval1->number(), 0);
    QCOMPARE(interval2->number(), 1);
    QCOMPARE(interval3->number(), 2);

    QCOMPARE(interval1->visualNumber(), 1);
    QCOMPARE(interval2->visualNumber(), 2);
    QCOMPARE(interval3->visualNumber(), 3);

    QCOMPARE(interval1->length(), 100000000);
    QCOMPARE(interval2->length(), 100000000);
    QCOMPARE(interval3->length(), 100000000);

    QCOMPARE(interval1->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(interval2->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(interval3->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));

    QCOMPARE(interval1->subGraphs().empty(), true);
    QCOMPARE(interval2->subGraphs().empty(), true);
    QCOMPARE(interval3->subGraphs().empty(), true);

    QCOMPARE(interval1->graphParamSet(), fps.at(0)->uuid());
    QCOMPARE(interval2->graphParamSet(), fps.at(0)->uuid());
    QCOMPARE(interval3->graphParamSet(), fps.at(0)->uuid());

    auto intervals2 = seam2->seamIntervals();
    QCOMPARE(intervals2.size(), 3u);
    auto interval4 = intervals2.at(0);
    auto interval5 = intervals2.at(1);
    auto interval6 = intervals2.at(2);

    QCOMPARE(interval4->number(), 0);
    QCOMPARE(interval5->number(), 1);
    QCOMPARE(interval6->number(), 2);

    QCOMPARE(interval4->visualNumber(), 1);
    QCOMPARE(interval5->visualNumber(), 2);
    QCOMPARE(interval6->visualNumber(), 3);

    QCOMPARE(interval4->length(), 100000000);
    QCOMPARE(interval5->length(), 100000000);
    QCOMPARE(interval6->length(), 100000000);

    QVERIFY(interval4->graph().isNull());
    QVERIFY(interval5->graph().isNull());
    QVERIFY(interval6->graph().isNull());

    QCOMPARE(interval4->subGraphs().empty(), false);
    QCOMPARE(interval4->subGraphs().size(), 4u);
    QCOMPARE(interval4->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(interval4->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(interval4->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(interval4->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    QCOMPARE(interval5->subGraphs().empty(), false);
    QCOMPARE(interval5->subGraphs().size(), 4u);
    QCOMPARE(interval5->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(interval5->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(interval5->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(interval5->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    QCOMPARE(interval6->subGraphs().empty(), false);
    QCOMPARE(interval6->subGraphs().size(), 4u);
    QCOMPARE(interval6->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(interval6->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(interval6->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(interval6->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    QCOMPARE(interval4->graphParamSet(), fps.at(1)->uuid());
    QCOMPARE(interval5->graphParamSet(), fps.at(1)->uuid());
    QCOMPARE(interval6->graphParamSet(), fps.at(1)->uuid());

    QSignalSpy seamsChangedSpy{product, &Product::seamsChanged};
    QVERIFY(seamsChangedSpy.isValid());
    product->seamSeries().front()->destroySeam(seams.front());
    QCOMPARE(seamsChangedSpy.count(), 1);
}

void ProductTest::testDuplicateSeam_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void ProductTest::testDuplicateSeam()
{
    const QString fileName = QFINDTESTDATA("../../App_Storage/autotests/testdata/products/product2.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> origProduct{Product::fromJson(fileName)};
    QVERIFY(origProduct);

    // create a seam link in origProduct
    auto const & origSeams = origProduct->seamSeries().front()->seams();
    QCOMPARE(origSeams.back()->metaObject()->inherits(&LinkedSeam::staticMetaObject), false);
    origProduct->seamSeries().front()->createSeamLink(origSeams.front(), QStringLiteral("5"));
    QCOMPARE(origSeams.back()->metaObject()->inherits(&LinkedSeam::staticMetaObject), true);
    QCOMPARE(origSeams.at(0)->linkedSeams().size(), 1);
    auto const * origLinked1 = origSeams.at(0)->linkedSeams().at(0);

    auto fps1 = origProduct->filterParameterSets();
    QCOMPARE(fps1.size(), 2u);

    std::unique_ptr<Product> product = std::make_unique<Product>(QUuid{});
    QVERIFY(product);
    QVERIFY(product->uuid() != QUuid("2F086211-FBD4-4493-A580-6FF11E4925DE"));
    QCOMPARE(product->seamSeries().size(), 0);
    auto& fps = product->filterParameterSets();
    QCOMPARE(fps.size(), 0u);

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    product->createFirstSeamSeries();
    product->seamSeries().front()->createSeamCopy(copyMode, origSeams.at(0));
    QCOMPARE(fps.size(), 1u);

    auto& seams = product->seamSeries().front()->seams();
    QCOMPARE(seams.size(), 1u);

    auto seam1 = seams.at(0);
    QCOMPARE(seam1->number(), 0);
    QCOMPARE(seam1->visualNumber(), 1);
    QCOMPARE(seam1->triggerDelta(), 1000);
    QCOMPARE(seam1->velocity(), 100000);
    QCOMPARE(seam1->graphParamSet(), fps.at(0)->uuid());
    QCOMPARE(seam1->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(seam1->subGraphs().empty(), true);
    std::size_t copiedLinkedSeamSize = changeUuid ? 0 : 1;
    QCOMPARE(seam1->linkedSeams().size(), copiedLinkedSeamSize);

    if(!changeUuid) {
        auto const * linked1 = seam1->linkedSeams().at(0);
        QCOMPARE(linked1->uuid(), origLinked1->uuid());
        QCOMPARE(linked1->name(), origLinked1->name());
        QCOMPARE(linked1->label(), origLinked1->label());
        QCOMPARE(linked1->linkTo(), seam1);
    }

    auto * hwParams = seam1->hardwareParameters();
    QVERIFY(hwParams);
    QCOMPARE(hwParams->uuid() != QUuid{"c648c43d-340b-4707-bdb4-9e0879adaaaa"}, changeUuid );

    auto intervals = seam1->seamIntervals();
    QCOMPARE(intervals.size(), 3u);
    auto interval1 = intervals.at(0);
    auto interval2 = intervals.at(1);
    auto interval3 = intervals.at(2);

    QCOMPARE(interval1->uuid() != QUuid("d10cd9fe-36f0-44c3-9398-828c3df2df08"), changeUuid);
    QCOMPARE(interval2->uuid() != QUuid("127b19cf-996b-4fec-a1ea-ba58f4fc387e"), changeUuid);
    QCOMPARE(interval3->uuid() != QUuid("1ffcaded-6088-4da0-b99c-6d6bcb01ecf0"), changeUuid);

    QCOMPARE(interval1->number(), 0);
    QCOMPARE(interval2->number(), 1);
    QCOMPARE(interval3->number(), 2);

    QCOMPARE(interval1->visualNumber(), 1);
    QCOMPARE(interval2->visualNumber(), 2);
    QCOMPARE(interval3->visualNumber(), 3);

    QCOMPARE(interval1->length(), 100000000);
    QCOMPARE(interval2->length(), 100000000);
    QCOMPARE(interval3->length(), 100000000);

    QCOMPARE(interval1->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(interval2->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));
    QCOMPARE(interval3->graph(), QUuid("b142af2c-cca8-4b88-828c-5e14490e7337"));

    QCOMPARE(interval1->subGraphs().empty(), true);
    QCOMPARE(interval2->subGraphs().empty(), true);
    QCOMPARE(interval3->subGraphs().empty(), true);

    QCOMPARE(interval1->graphParamSet(), fps.at(0)->uuid());
    QCOMPARE(interval2->graphParamSet(), fps.at(0)->uuid());
    QCOMPARE(interval3->graphParamSet(), fps.at(0)->uuid());

    product->seamSeries().front()->createSeamCopy(copyMode, origSeams.at(1));
    QCOMPARE(fps.size(), 2u);
    QCOMPARE(seams.size(), 2u);

    auto seam2 = seams.at(1);
    QCOMPARE(seam2->number(), 1);
    QCOMPARE(seam2->visualNumber(), 2);
    QCOMPARE(seam2->triggerDelta(), 1000);
    QCOMPARE(seam2->velocity(), 100000);
    QCOMPARE(seam2->graphParamSet(), fps.at(1)->uuid());
    QVERIFY(seam2->graph().isNull());
    QCOMPARE(seam2->subGraphs().empty(), false);
    QCOMPARE(seam2->subGraphs().size(), 4u);
    QCOMPARE(seam2->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(seam2->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(seam2->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(seam2->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    auto intervals2 = seam2->seamIntervals();
    QCOMPARE(intervals2.size(), 3u);
    auto interval4 = intervals2.at(0);
    auto interval5 = intervals2.at(1);
    auto interval6 = intervals2.at(2);

    QCOMPARE(interval4->number(), 0);
    QCOMPARE(interval5->number(), 1);
    QCOMPARE(interval6->number(), 2);

    QCOMPARE(interval4->visualNumber(), 1);
    QCOMPARE(interval5->visualNumber(), 2);
    QCOMPARE(interval6->visualNumber(), 3);

    QCOMPARE(interval4->length(), 100000000);
    QCOMPARE(interval5->length(), 100000000);
    QCOMPARE(interval6->length(), 100000000);

    QVERIFY(interval4->graph().isNull());
    QVERIFY(interval5->graph().isNull());
    QVERIFY(interval6->graph().isNull());

    QCOMPARE(interval4->subGraphs().empty(), false);
    QCOMPARE(interval4->subGraphs().size(), 4u);
    QCOMPARE(interval4->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(interval4->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(interval4->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(interval4->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    QCOMPARE(interval5->subGraphs().empty(), false);
    QCOMPARE(interval5->subGraphs().size(), 4u);
    QCOMPARE(interval5->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(interval5->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(interval5->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(interval5->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    QCOMPARE(interval6->subGraphs().empty(), false);
    QCOMPARE(interval6->subGraphs().size(), 4u);
    QCOMPARE(interval6->subGraphs().at(0), QUuid("ce515db8-45b2-446a-af96-4d3aa3b44a1c"));
    QCOMPARE(interval6->subGraphs().at(1), QUuid("4a2922e0-746e-4bed-b45e-d60792094ff3"));
    QCOMPARE(interval6->subGraphs().at(2), QUuid("1d962b39-5b70-4d7f-aa1a-b9ae3fa5e4e1"));
    QCOMPARE(interval6->subGraphs().at(3), QUuid("27aa0c5c-c151-4c64-821d-9e80a64204a8"));

    QCOMPARE(interval4->graphParamSet(), fps.at(1)->uuid());
    QCOMPARE(interval5->graphParamSet(), fps.at(1)->uuid());
    QCOMPARE(interval6->graphParamSet(), fps.at(1)->uuid());
}

void ProductTest::testCreateSeamSeries_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void ProductTest::testCreateSeamSeries()
{
    Product p{QUuid::createUuid()};
    QSignalSpy seamSeriesChangedSpy{&p, &Product::seamSeriesChanged};
    QVERIFY(seamSeriesChangedSpy.isValid());
    QSignalSpy seamsChangedSpy{&p, &Product::seamsChanged};
    QVERIFY(seamsChangedSpy.isValid());

    QCOMPARE(p.seamSeries().size(), 0u);
    auto seamSeries = p.createSeamSeries();
    QVERIFY(seamSeries);
    QCOMPARE(seamSeries->number(), 0);
    QCOMPARE(seamSeries->visualNumber(), 1);
    QCOMPARE(seamSeries->product(), &p);
    QCOMPARE(p.seamSeries().size(), 1u);
    QCOMPARE(p.seamSeries().front(), seamSeries);
    QVERIFY(seamSeries->seams().empty());
    QCOMPARE(seamSeriesChangedSpy.count(), 1);

    // once more, should get number 1
    auto seamSeries2 = p.createSeamSeries();
    QVERIFY(seamSeries2);
    QVERIFY(seamSeries2 != seamSeries);
    QVERIFY(seamSeries2->uuid() != seamSeries->uuid());
    QCOMPARE(seamSeries2->number(), 1);
    QCOMPARE(seamSeries2->visualNumber(), 2);
    QCOMPARE(seamSeries2->product(), &p);
    QVERIFY(seamSeries2->seams().empty());
    QCOMPARE(p.seamSeries().size(), 2u);
    QCOMPARE(p.seamSeries().front(), seamSeries);
    QCOMPARE(p.seamSeries().back(), seamSeries2);
    QCOMPARE(seamSeriesChangedSpy.count(), 2);
    QCOMPARE(seamsChangedSpy.count(), 0);

    // add a seam to seamseries2
    auto seam = seamSeries2->createSeam();
    QVERIFY(seam);
    QCOMPARE(seam->seamSeries(), seamSeries2);
    QVERIFY(!seamSeries2->seams().empty());
    QCOMPARE(seamsChangedSpy.count(), 1);

    // now duplicate seamSeries2
    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;

    const auto uuidToCopy = seamSeries2->uuid();
    auto seamSeriesToCopy = p.findSeamSeries(uuidToCopy);
    auto seamSeries3 = p.createSeamSeriesCopy(copyMode, seamSeriesToCopy);
    QCOMPARE(seamSeriesChangedSpy.count(), 3);
    QCOMPARE(seamsChangedSpy.count(), 2);
    QVERIFY(seamSeries3);
    QVERIFY(seamSeries3 != seamSeries2);
    QCOMPARE(seamSeries2->uuid() != seamSeries3->uuid(), changeUuid);
    QCOMPARE(seamSeries3->number(), 2);
    QCOMPARE(seamSeries3->visualNumber(), 3);
    QCOMPARE(seamSeries3->product(), &p);
    QVERIFY(!seamSeries3->seams().empty());
    QCOMPARE(p.seamSeries().size(), 3u);
    QCOMPARE(p.seamSeries().front(), seamSeries);
    QCOMPARE(p.seamSeries().back(), seamSeries3);

    QCOMPARE(seamSeries3->seams().size(), 1u);
    QVERIFY(seamSeries3->seams().front() != seam);
    QCOMPARE(seamSeries3->seams().front()->uuid() != seam->uuid(),  changeUuid);
    QCOMPARE(seamSeries3->seams().front()->number(), seam->number());
    QCOMPARE(seamSeries3->seams().front()->seamSeries(), seamSeries3);

    // now delete seamseries2
    QSignalSpy seamSeriesDestroyedSpy{seamSeries2, &QObject::destroyed};
    QVERIFY(seamSeriesDestroyedSpy.isValid());
    QSignalSpy seamDestroyedSpy{seam, &QObject::destroyed};
    QVERIFY(seamDestroyedSpy.isValid());
    p.destroySeamSeries(seamSeries2);
    QCOMPARE(seamSeriesChangedSpy.count(), 4);
    QCOMPARE(seamsChangedSpy.count(), 3);
    QCOMPARE(p.seamSeries().size(), 2u);
    QCOMPARE(p.seamSeries().front(), seamSeries);
    QCOMPARE(p.seamSeries().back(), seamSeries3);
    QVERIFY(seamSeriesDestroyedSpy.wait());
    QCOMPARE(seamSeriesDestroyedSpy.count(), 1);
    QCOMPARE(seamDestroyedSpy.count(), 1);

    auto destroyedSeries = p.findSeamSeries(uuidToCopy);

    // duplicating the destroyed seam series will fail only if the duplicate got a different UUID.
    QCOMPARE(destroyedSeries == nullptr, changeUuid);
    QCOMPARE(p.createSeamSeriesCopy(copyMode, destroyedSeries) == nullptr, changeUuid);
}

QTEST_GUILESS_MAIN(ProductTest)
#include "testProduct.moc"
