#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/copyMode.h"
#include "../src/intervalError.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "../src/seamInterval.h"
#include "../src/attributeModel.h"
#include "common/graph.h"

using precitec::storage::IntervalError;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::AttributeModel;
using precitec::storage::CopyMode;

class TestIntervalError : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testCtorWithUuid();
    void testVariantId_data();
    void testVariantId();
    void testResultValue();
    void testErrorType();
    void testShift();
    void testMinLimit();
    void testMaxLimit();
    void testName();
    void testMeasureTask();
    void testIntervals();
    void testMin();
    void testMax();
    void testThreshold();
    void testInitFromAttributes();
    void getIntValue_data();
    void getIntValue();
    void getStringValue();
    void testFromJson_data();
    void testFromJson();
    void testToJson();
    void testDuplicate_data();
    void testDuplicate();
    void testParameterList();
    void testUpdateLowerBounds_data();
    void testUpdateLowerBounds();
    void testUpdateUpperBounds_data();
    void testUpdateUpperBounds();
    void testLowestMin();
    void testHighestMax();
    void testTrack277();
};

void TestIntervalError::testCtor()
{
    IntervalError error;

    QVERIFY(!error.uuid().isNull());
    QVERIFY(error.variantId().isNull());
    QVERIFY(error.name().isEmpty());
    QVERIFY(!error.measureTask());
    QCOMPARE(error.resultValue(), 0);
    QCOMPARE(error.errorType(), 0);
    QCOMPARE(error.shift(), 0.0);
    QCOMPARE(error.minLimit(), -100000.0);
    QCOMPARE(error.maxLimit(), 100000.0);
    QCOMPARE(error.boundaryType(), IntervalError::BoundaryType::Static);
    QCOMPARE(Seam::maxLevel(), 3);
    QCOMPARE(error.m_levels.size(), Seam::maxLevel());
    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error.threshold(i), 0.0);
        QCOMPARE(error.min(i), 0.0);
        QCOMPARE(error.max(i), 0.0);
    }
}

void TestIntervalError::testCtorWithUuid()
{    
    QUuid const uuid{"55c53584-9b02-4153-9993-ec00ebd62180"};
    IntervalError error{uuid};

    QCOMPARE(error.uuid(), uuid);
}

void TestIntervalError::testVariantId_data()
{
    QTest::addColumn<QUuid>("variantId");
    QTest::addColumn<IntervalError::BoundaryType>("boundary");

    QTest::newRow("LengthOutsideStaticBoundary") << QUuid("3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D") << IntervalError::BoundaryType::Static;
    QTest::newRow("AccumulatedLengthOutsideStaticBoundary") << QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6") << IntervalError::BoundaryType::Static;
    QTest::newRow("LengthInsideStaticBoundary") << QUuid("3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C") << IntervalError::BoundaryType::Static;
    QTest::newRow("AreaStaticBoundary") << QUuid("73708EA1-580A-4660-8D80-63622670BC7C") << IntervalError::BoundaryType::Static;
    QTest::newRow("AccumulatedAreaStaticBoundary") << QUuid("740FD8B3-852C-485A-BC24-6C67A36DABD2") << IntervalError::BoundaryType::Static;
    QTest::newRow("PeakStaticBoundary") << QUuid("396CA433-AD11-4073-A2B2-5314CC41D152") << IntervalError::BoundaryType::Static;
}

void TestIntervalError::testVariantId()
{
    IntervalError error;

    QFETCH(QUuid, variantId);
    error.setVariantId(variantId);

    QTEST(error.variantId(), "variantId");
    QTEST(error.boundaryType(), "boundary");
}

void TestIntervalError::testResultValue()
{
    IntervalError error;
    QCOMPARE(error.resultValue(), 0);

    QSignalSpy resultValueChangedSpy(&error, &IntervalError::resultValueChanged);
    QVERIFY(resultValueChangedSpy.isValid());

    error.setResultValue(0);
    QCOMPARE(resultValueChangedSpy.count(), 0);

    error.setResultValue(501);
    QCOMPARE(error.resultValue(), 501);
    QCOMPARE(resultValueChangedSpy.count(), 1);

    error.setResultValue(501);
    QCOMPARE(resultValueChangedSpy.count(), 1);
}

void TestIntervalError::testErrorType()
{
    IntervalError error;
    QCOMPARE(error.errorType(), 0);

    QSignalSpy errorTypeChangedSpy(&error, &IntervalError::errorTypeChanged);
    QVERIFY(errorTypeChangedSpy.isValid());

    error.setErrorType(0);
    QCOMPARE(errorTypeChangedSpy.count(), 0);

    error.setErrorType(501);
    QCOMPARE(error.errorType(), 501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);

    error.setErrorType(501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);
}

void TestIntervalError::testShift()
{
    IntervalError error;
    QCOMPARE(error.shift(), 0);

    QSignalSpy shiftChangedSpy(&error, &IntervalError::shiftChanged);
    QVERIFY(shiftChangedSpy.isValid());

    error.setShift(0.0);
    QCOMPARE(shiftChangedSpy.count(), 0);

    error.setShift(3.4);
    QCOMPARE(error.shift(), 3.4);
    QCOMPARE(shiftChangedSpy.count(), 1);

    error.setShift(3.4);
    QCOMPARE(shiftChangedSpy.count(), 1);
}

void TestIntervalError::testMinLimit()
{
    IntervalError error;
    QCOMPARE(error.minLimit(), -100000.0);

    QSignalSpy minLimitChangedSpy(&error, &IntervalError::minLimitChanged);
    QVERIFY(minLimitChangedSpy.isValid());

    error.setMinLimit(-100000.0);
    QCOMPARE(minLimitChangedSpy.count(), 0);

    error.setMinLimit(-4000.0);
    QCOMPARE(error.minLimit(), -4000.0);
    QCOMPARE(minLimitChangedSpy.count(), 1);

    error.setMinLimit(-4000.0);
    QCOMPARE(minLimitChangedSpy.count(), 1);
}

void TestIntervalError::testMaxLimit()
{
    IntervalError error;
    QCOMPARE(error.maxLimit(), 100000.0);

    QSignalSpy maxLimitChangedSpy(&error, &IntervalError::maxLimitChanged);
    QVERIFY(maxLimitChangedSpy.isValid());

    error.setMaxLimit(100000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 0);

    error.setMaxLimit(4000.0);
    QCOMPARE(error.maxLimit(), 4000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 1);

    error.setMaxLimit(4000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 1);
}

void TestIntervalError::testName()
{
    IntervalError error;
    QCOMPARE(error.name(), QStringLiteral(""));

    QSignalSpy nameChangedSpy(&error, &IntervalError::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    error.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(error.name(), QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestIntervalError::testMeasureTask()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    IntervalError error(this);
    QVERIFY(!error.measureTask());

    QSignalSpy measureTaskChangedSpy(&error, &IntervalError::measureTaskChanged);
    QVERIFY(measureTaskChangedSpy.isValid());

    error.setMeasureTask(seam);
    QVERIFY(error.measureTask());
    QCOMPARE(error.measureTask(), seam);

    QCOMPARE(measureTaskChangedSpy.count(), 1);

    error.setMeasureTask(seam);
    QCOMPARE(measureTaskChangedSpy.count(), 1);

    const auto seam2 = product->createSeam();

    error.setMeasureTask(seam2);
    QCOMPARE(error.measureTask(), seam2);
    QCOMPARE(measureTaskChangedSpy.count(), 2);
}

void TestIntervalError::testIntervals()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();
    QCOMPARE(seam->seamIntervalsCount(), 1);
    const auto interval1 = seam->firstSeamInterval();
    QVERIFY(interval1);

    auto error = seam->addIntervalError(QUuid("3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D"));
    QCOMPARE(error->m_errorIds.size(), 1);
    QVERIFY(error->m_errorIds.find(interval1->uuid()) != error->m_errorIds.end());

    const auto interval2 = seam->createSeamInterval();
    QCOMPARE(error->m_errorIds.size(), 2);
    QVERIFY(error->m_errorIds.find(interval1->uuid()) != error->m_errorIds.end());
    QVERIFY(error->m_errorIds.find(interval2->uuid()) != error->m_errorIds.end());

    const auto interval3 = seam->createSeamInterval();
    QCOMPARE(error->m_errorIds.size(), 3);
    QVERIFY(error->m_errorIds.find(interval1->uuid()) != error->m_errorIds.end());
    QVERIFY(error->m_errorIds.find(interval2->uuid()) != error->m_errorIds.end());
    QVERIFY(error->m_errorIds.find(interval3->uuid()) != error->m_errorIds.end());

    seam->destroySeamInterval(interval2);
    QCOMPARE(error->m_errorIds.size(), 2);
    QVERIFY(error->m_errorIds.find(interval1->uuid()) != error->m_errorIds.end());
    QVERIFY(error->m_errorIds.find(interval2->uuid()) == error->m_errorIds.end());
    QVERIFY(error->m_errorIds.find(interval3->uuid()) != error->m_errorIds.end());
}

void TestIntervalError::testMin()
{
    IntervalError error(this);
    QSignalSpy minChangedSpy(&error, &IntervalError::minChanged);
    QVERIFY(minChangedSpy.isValid());

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error.min(i), 0.0);
    }

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setMin(i, 0.0);
    }
    QCOMPARE(minChangedSpy.count(), 0);

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setMin(i, i + 0.5);
        QCOMPARE(error.min(i), i + 0.5);
        QCOMPARE(minChangedSpy.count(), i + 1);
    }

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setMin(i, i + 0.5);
        QCOMPARE(error.min(i), i + 0.5);
    }
    QCOMPARE(minChangedSpy.count(), 3);
}

void TestIntervalError::testMax()
{
    IntervalError error(this);
    QSignalSpy maxChangedSpy(&error, &IntervalError::maxChanged);
    QVERIFY(maxChangedSpy.isValid());

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error.max(i), 0.0);
    }

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setMax(i, 0.0);
    }
    QCOMPARE(maxChangedSpy.count(), 0);

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setMax(i, i + 0.5);
        QCOMPARE(error.max(i), i + 0.5);
        QCOMPARE(maxChangedSpy.count(), i + 1);
    }

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setMax(i, i + 0.5);
        QCOMPARE(error.max(i), i + 0.5);
    }
    QCOMPARE(maxChangedSpy.count(), 3);
}

void TestIntervalError::testThreshold()
{
    IntervalError error(this);
    QSignalSpy thresholdChangedSpy(&error, &IntervalError::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error.threshold(i), 0.0);
    }

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setThreshold(i, 0.0);
    }
    QCOMPARE(thresholdChangedSpy.count(), 0);

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setThreshold(i, i + 0.5);
        QCOMPARE(error.threshold(i), i + 0.5);
        QCOMPARE(thresholdChangedSpy.count(), i + 1);
    }

    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        error.setThreshold(i, i + 0.5);
        QCOMPARE(error.threshold(i), i + 0.5);
    }
    QCOMPARE(thresholdChangedSpy.count(), 3);
}

void TestIntervalError::testInitFromAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    IntervalError error(this);
    error.setVariantId("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6");
    error.setMinLimit(0.0);
    QCOMPARE(error.minLimit(), 0.0);
    error.initFromAttributes(&model);
    QCOMPARE(error.minLimit(), -100000);
    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error.max(i), 3);
    }
}

void TestIntervalError::getIntValue_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("series");
    QTest::addColumn<int>("seam");
    QTest::addColumn<int>("interval");

    QTest::newRow("Measure Series") << QStringLiteral("SeamSeries") << 3 << 3 << 3;
    QTest::newRow("Measure Seam") << QStringLiteral("Seam") << 0 << 5 << 5;
    QTest::newRow("Measure Interval") << QStringLiteral("SeamInterval") << 0 << 0 << 7;
}

void TestIntervalError::getIntValue()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    seamSeries->setNumber(3);
    const auto seam = seamSeries->createSeam();
    seam->setNumber(5);
    const auto interval = seam->createSeamInterval();
    interval->setNumber(7);

    IntervalError error;
    QFETCH(QString, name);

    error.setMeasureTask(interval);
    QTEST(error.getIntValue(name), "interval");

    error.setMeasureTask(seam);
    QTEST(error.getIntValue(name), "seam");

    error.setMeasureTask(seamSeries);
    QTEST(error.getIntValue(name), "series");

    error.setResultValue(501);
    QCOMPARE(error.getIntValue(QStringLiteral("Result")), error.resultValue());

    error.setErrorType(17);
    QCOMPARE(error.getIntValue(QStringLiteral("Error")), error.errorType());

    QCOMPARE(error.getIntValue(QStringLiteral("Other")), -1);
}

void TestIntervalError::getStringValue()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = seamSeries->createSeam();
    const auto interval = seam->createSeamInterval();

    IntervalError error;
    error.setMeasureTask(interval);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamInterval"));

    error.setMeasureTask(seam);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamInterval"));

    error.setMeasureTask(seamSeries);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamInterval"));

    QCOMPARE(error.getStringValue(QStringLiteral("Other")), QUuid{}.toString(QUuid::WithoutBraces).toStdString());
}

void TestIntervalError::testFromJson_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("new") << "intervalError";
    QTest::newRow("legacy") << "legacyIntervalError";
}

void TestIntervalError::testFromJson()
{
    QFETCH(QString, fileName);
    const QString dir = QFINDTESTDATA(QStringLiteral("testdata/errors/%1.json").arg(fileName));

    QFile file(dir);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    QVERIFY(!data.isEmpty());
    const auto document = QJsonDocument::fromJson(data);
    QVERIFY(!document.isNull());

    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    auto error = IntervalError::fromJson(document.object(), seam);

    QCOMPARE(error->uuid(), QUuid("94f507f1-4215-4fbd-afd1-fe34b77347e5"));
    QCOMPARE(error->variantId(), QUuid("cedeacb4-d4bb-4fdc-945a-bdc54e5848a6"));
    QCOMPARE(error->name(), QStringLiteral("My Interval Error"));
    QCOMPARE(error->resultValue(), 566);
    QCOMPARE(error->errorType(), 1006);
    QCOMPARE(error->minLimit(), -99000);
    QCOMPARE(error->maxLimit(), 99000);
    QCOMPARE(error->shift(), 3.7);
    QCOMPARE(error->boundaryType(), IntervalError::BoundaryType::Static);
    QCOMPARE(error->m_errorIds.size(), 2);
    const auto error1It = error->m_errorIds.find(QUuid("dc35b02a-a943-428b-8ec2-f40e1ec5e42f"));
    QVERIFY(error1It != error->m_errorIds.end());
    QCOMPARE((*error1It).second, QUuid("d22b18aa-90d9-48b4-86b4-fe2d5a138ee8"));
    const auto error12It = error->m_errorIds.find(QUuid("e292aca2-f97d-497b-a082-154043d5eb85"));
    QVERIFY(error12It != error->m_errorIds.end());
    QCOMPARE((*error12It).second, QUuid("831a17ce-2356-46a1-b2bc-9886385290a8"));
    QCOMPARE(error->min(0), 13.4);
    QCOMPARE(error->max(0), 14.0);
    QCOMPARE(error->threshold(0), 15.0);
    QCOMPARE(error->min(1), 16.5);
    QCOMPARE(error->max(1), 17.8);
    QCOMPARE(error->threshold(1), 18);
    QCOMPARE(error->min(2), 0);
    QCOMPARE(error->max(2), 0);
    QCOMPARE(error->threshold(2), 0);

    // test if the connections have been properly restored
    QSignalSpy minChangedSpy(error, &IntervalError::minChanged);
    QVERIFY(minChangedSpy.isValid());
    QCOMPARE(minChangedSpy.count(), 0);
    QSignalSpy maxChangedSpy(error, &IntervalError::maxChanged);
    QVERIFY(maxChangedSpy.isValid());
    QCOMPARE(maxChangedSpy.count(), 0);
    QSignalSpy thresholdChangedSpy(error, &IntervalError::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());
    QCOMPARE(thresholdChangedSpy.count(), 0);

    error->setMin(1, 0.5);
    error->setMax(1, 0.5);
    error->setThreshold(1, 0.5);

    QCOMPARE(minChangedSpy.count(), 1);
    QCOMPARE(maxChangedSpy.count(), 1);
    QCOMPARE(thresholdChangedSpy.count(), 1);
}

void TestIntervalError::testToJson()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();
    QCOMPARE(seam->seamIntervalsCount(), 1);
    const auto interval1 = seam->firstSeamInterval();
    QVERIFY(interval1);
    const auto interval2 = seam->createSeamInterval();
    QVERIFY(interval2);

    auto error = seam->addIntervalError(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error->setName(QStringLiteral("Interval Error"));
    error->setResultValue(204);
    error->setErrorType(1016);
    error->setMinLimit(-43000);
    error->setMaxLimit(37000);
    error->setShift(2.6);
    error->setMin(0, 3.7);
    error->setMax(0, 12.64);
    error->setThreshold(0, 4.74);
    error->setMin(1, 6.52);
    error->setMax(1, 9.32);
    error->setThreshold(1, 8.434);
    error->setMin(2, 9.498);
    error->setMax(2, 12.85);
    error->setThreshold(2, 7.95421);

    auto json = error->toJson();
    QCOMPARE(json.count(), 10);

    auto error2 = IntervalError::fromJson(json, seam);
    QCOMPARE(error->uuid(), error2->uuid());
    QCOMPARE(error->variantId(), error2->variantId());
    QCOMPARE(error->name(), error2->name());
    QCOMPARE(error->measureTask(), error2->measureTask());
    QCOMPARE(error->resultValue(), error2->resultValue());
    QCOMPARE(error->errorType(), error2->errorType());
    QCOMPARE(error->minLimit(), error2->minLimit());
    QCOMPARE(error->maxLimit(), error2->maxLimit());
    QCOMPARE(error->shift(), error2->shift());
    QCOMPARE(error->boundaryType(), error2->boundaryType());
    QCOMPARE(error->m_errorIds.size(), error2->m_errorIds.size());
    const auto error11It = error->m_errorIds.find(interval1->uuid());
    QVERIFY(error11It != error->m_errorIds.end());
    const auto error21It = error2->m_errorIds.find(interval1->uuid());
    QVERIFY(error21It != error2->m_errorIds.end());
    QCOMPARE((*error11It).second, (*error21It).second);
    const auto error12It = error->m_errorIds.find(interval2->uuid());
    QVERIFY(error12It != error->m_errorIds.end());
    const auto error22It = error2->m_errorIds.find(interval2->uuid());
    QVERIFY(error22It != error2->m_errorIds.end());
    QCOMPARE((*error12It).second, (*error22It).second);
    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error->min(i), error2->min(i));
        QCOMPARE(error->max(i), error2->max(i));
        QCOMPARE(error->threshold(i), error2->threshold(i));
    }
}

void TestIntervalError::testDuplicate_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestIntervalError::testDuplicate()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();
    QCOMPARE(seam->seamIntervalsCount(), 1);
    const auto interval1 = seam->firstSeamInterval();
    QVERIFY(interval1);
    const auto interval2 = seam->createSeamInterval();
    QVERIFY(interval2);

    auto error = seam->addIntervalError(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error->setName(QStringLiteral("Interval Error"));
    error->setResultValue(204);
    error->setErrorType(1016);
    error->setMinLimit(-43000);
    error->setMaxLimit(37000);
    error->setShift(2.6);
    error->setMin(0, 3.7);
    error->setMax(0, 12.64);
    error->setThreshold(0, 4.74);
    error->setMin(1, 6.52);
    error->setMax(1, 9.32);
    error->setThreshold(1, 8.434);
    error->setMin(2, 9.498);
    error->setMax(2, 12.85);
    error->setThreshold(2, 7.95421);

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto duplicated = error->duplicate(copyMode, seam);
    QVERIFY(duplicated);
    QCOMPARE(duplicated->uuid() != error->uuid(), changeUuid);
    QCOMPARE(duplicated->measureTask(), seam);
    QCOMPARE(error->variantId(), duplicated->variantId());
    QCOMPARE(error->name(), duplicated->name());
    QCOMPARE(error->resultValue(), duplicated->resultValue());
    QCOMPARE(error->errorType(), duplicated->errorType());
    QCOMPARE(error->minLimit(), duplicated->minLimit());
    QCOMPARE(error->maxLimit(), duplicated->maxLimit());
    QCOMPARE(error->shift(), duplicated->shift());
    QCOMPARE(error->m_errorIds.size(), duplicated->m_errorIds.size());
    // error ids must be different only when we change all UUIDs.
    const auto error11It = error->m_errorIds.find(interval1->uuid());
    QVERIFY(error11It != error->m_errorIds.end());
    const auto error21It = duplicated->m_errorIds.find(interval1->uuid());
    QVERIFY(error21It != duplicated->m_errorIds.end());
    QCOMPARE((*error11It).second != (*error21It).second, changeUuid);
    const auto error12It = error->m_errorIds.find(interval2->uuid());
    QVERIFY(error12It != error->m_errorIds.end());
    const auto error22It = duplicated->m_errorIds.find(interval2->uuid());
    QVERIFY(error22It != duplicated->m_errorIds.end());
    QCOMPARE((*error12It).second != (*error22It).second, changeUuid);
    for (auto i = 0u; i < Seam::maxLevel(); i++)
    {
        QCOMPARE(error->min(i), duplicated->min(i));
        QCOMPARE(error->max(i), duplicated->max(i));
        QCOMPARE(error->threshold(i), duplicated->threshold(i));
    }
}

void TestIntervalError::testParameterList()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->setLwmTriggerSignalThreshold(2.5);
    const auto seamSeries = product->createSeamSeries();
    seamSeries->setNumber(3);
    const auto seam = seamSeries->createSeam();
    seam->setNumber(5);
    const auto interval1 = seam->firstSeamInterval();
    interval1->setLevel(0);
    const auto interval2 = seam->createSeamInterval();
    interval2->setLevel(1);
    const auto interval3 = seam->createSeamInterval();
    interval3->setLevel(2);

    auto error = seam->addIntervalError(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error->setName(QStringLiteral("Interval Error"));
    error->setResultValue(15);
    error->setErrorType(1016);
    error->setMinLimit(-43000);
    error->setMaxLimit(37000);
    error->setShift(2.6);
    error->setMin(0, -3.7);
    error->setMax(0, 12.3);
    error->setThreshold(0, 7.4);
    error->setSecondThreshold(0, 7.4);
    error->setMin(1, 2.5);
    error->setMax(1, 13.8);
    error->setThreshold(1, 6.4);
    error->setSecondThreshold(1, 7.4);
    error->setMin(2, -12.9);
    error->setMax(2, -2);
    error->setThreshold(2, 17.4);
    error->setSecondThreshold(2, 7.4);

    auto getIntervalError = [error, interval1, interval2, interval3] (int index)
    {
        switch (index)
        {
            case 0:
                return (*error->m_errorIds.find(interval1->uuid())).second;
            case 1:
                return (*error->m_errorIds.find(interval2->uuid())).second;
            case 2:
                return (*error->m_errorIds.find(interval3->uuid())).second;
            default:
                return (*error->m_errorIds.find(interval1->uuid())).second;
        }
    };

    auto parameterList = error->toParameterList();
    QCOMPARE(parameterList.size(), 3 * 13);

    auto resultFound = 0;
    auto errorFound = 0;
    auto minFound = 0;
    auto maxFound = 0;
    auto thresholdFound = 0;
    auto scopeFound = 0;
    auto seamSeriesFound = 0;
    auto seamFound = 0;
    auto seamIntervalFound = 0;
    auto referenceFound = 0;
    auto middleReferenceFound = 0;
    auto secondThresholdFound = 0;
    auto lwmTriggerSignalThresholdFound = 0;

    const auto parameterCount = 13;

    auto counter = 0;
    for (auto &parameter : parameterList)
    {
        if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Result")) == 0)
        {
            resultFound++;
            QCOMPARE(parameter->value<int>(), 15);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Error")) == 0)
        {
            errorFound++;
            QCOMPARE(parameter->value<int>(), 1016);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Min")) == 0)
        {
            minFound++;
            QCOMPARE(parameter->value<double>(), error->min(counter / parameterCount) + 2.6);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Max")) == 0)
        {
            maxFound++;
            QCOMPARE(parameter->value<double>(),  error->max(counter / parameterCount) + 2.6);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Threshold")) == 0)
        {
            thresholdFound++;
            QCOMPARE(parameter->value<double>(),  error->threshold(counter / parameterCount));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Scope")) == 0)
        {
            scopeFound++;
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), QStringLiteral("SeamInterval"));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SeamSeries")) == 0)
        {
            seamSeriesFound++;
            QCOMPARE(parameter->value<int>(), 3);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Seam")) == 0)
        {
            seamFound++;
            QCOMPARE(parameter->value<int>(), 5);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SeamInterval")) == 0)
        {
            seamIntervalFound++;
            QCOMPARE(parameter->value<int>(), counter / parameterCount);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Reference")) == 0)
        {
            referenceFound++;
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), QUuid{}.toString(QUuid::WithoutBraces));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("MiddleReference")) == 0)
        {
            middleReferenceFound++;
            QCOMPARE(parameter->value<bool>(), false);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SecondThreshold")) == 0)
        {
            secondThresholdFound++;
            QCOMPARE(parameter->value<double>(),  error->secondThreshold(counter / parameterCount));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("LwmSignalThreshold")) == 0)
        {
            lwmTriggerSignalThresholdFound++;
            QCOMPARE(parameter->value<double>(), 2.5);
        }

        QCOMPARE(QString::fromStdString(parameter->instanceID().toString()), getIntervalError(counter / parameterCount).toString(QUuid::WithoutBraces));
        QCOMPARE(QString::fromStdString(parameter->typID().toString()), error->variantId().toString(QUuid::WithoutBraces));
        counter++;
    }

    QVERIFY(resultFound == 3);
    QVERIFY(errorFound == 3);
    QVERIFY(minFound == 3);
    QVERIFY(maxFound == 3);
    QVERIFY(thresholdFound == 3);
    QVERIFY(scopeFound == 3);
    QVERIFY(seamSeriesFound == 3);
    QVERIFY(seamFound == 3);
    QVERIFY(seamIntervalFound == 3);
    QVERIFY(referenceFound == 3);
    QVERIFY(middleReferenceFound == 3);
    QVERIFY(secondThresholdFound == 3);
    QVERIFY(lwmTriggerSignalThresholdFound == 3);
}

void TestIntervalError::testUpdateLowerBounds_data()
{
    QTest::addColumn<double>("limitValue");
    QTest::addColumn<double>("shift");
    QTest::addColumn<double>("min");
    QTest::addColumn<double>("max");
    QTest::addColumn<double>("maxLimit");

    QTest::newRow("ok") << -5.0 << -3.0 << 3.0 << 5.0 << 10.0;
    QTest::newRow("shiftConflict") << 0.5 << 0.0 << 3.0 << 5.0 << 10.0;
    QTest::newRow("minConflict") << 3.5 << 0.0 << 3.5 << 5.0 << 10.0;
    QTest::newRow("maxConflict") << 5.5 << 0.0 << 5.5 << 5.5 << 10.0;
    QTest::newRow("maxLimitConflict") << 10.5 << 0.0 << 10.5 << 10.5 << 10.5;
}

void TestIntervalError::testUpdateLowerBounds()
{
    IntervalError error;
    error.setMaxLimit(10.0);
    error.setMax(1, 5.0);
    error.setMin(1, 3.0);
    error.setShift(-3.0);

    QFETCH(double, limitValue);
    error.setMinLimit(limitValue);

    QTEST(error.shift(), "shift");
    QTEST(error.min(1), "min");
    QTEST(error.max(1), "max");
    QTEST(error.maxLimit(), "maxLimit");
}

void TestIntervalError::testUpdateUpperBounds_data()
{
    QTest::addColumn<double>("limitValue");
    QTest::addColumn<double>("shift");
    QTest::addColumn<double>("minLimit");
    QTest::addColumn<double>("min");
    QTest::addColumn<double>("max");

    QTest::newRow("ok") << 15.0 << 3.0 << 2.0 << 5.0 << 7.0;
    QTest::newRow("shiftConflict") << 9.5 << 0.0 << 2.0 << 5.0 << 7.0;
    QTest::newRow("maxConflict") << 6.5 << 0.0 << 2.0 << 5.0 << 6.5;
    QTest::newRow("minConflict") << 4.5 << 0.0 << 2.0 << 4.5 << 4.5;
    QTest::newRow("minLimitConflict") << 0.5 << 0.0 << 0.5 << 0.5 << 0.5;
}

void TestIntervalError::testUpdateUpperBounds()
{
    IntervalError error;
    error.setMinLimit(2.0);
    error.setMin(1, 5.0);
    error.setMax(1, 7.0);
    error.setShift(3.0);

    QFETCH(double, limitValue);
    error.setMaxLimit(limitValue);

    QTEST(error.shift(), "shift");
    QTEST(error.max(1), "max");
    QTEST(error.min(1), "min");
    QTEST(error.minLimit(), "minLimit");
}

void TestIntervalError::testLowestMin()
{
    IntervalError error;
    error.setMin(0, 7.0);
    error.setMin(1, 5.0);
    error.setMin(2, 6.0);

    QCOMPARE(error.lowestMin(), 5.0);
}

void TestIntervalError::testHighestMax()
{
    IntervalError error;
    error.setMax(0, 7.0);
    error.setMax(1, 5.0);
    error.setMax(2, 6.0);

    QCOMPARE(error.highestMax(), 7.0);
}

void TestIntervalError::testTrack277()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seam = product->createSeam();

    IntervalError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(seam);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setMinLimit(-5000);
    error.setMaxLimit(-1000);
    for (auto i = 0u; i < precitec::storage::AbstractMeasureTask::maxLevel(); i++)
    {
        error.setMin(i, -3405);
        error.setMax(i, -3395);
    }
    error.setShift(402.1);

    std::unique_ptr<IntervalError> error2{IntervalError::fromJson(error.toJson(), seam)};
    QCOMPARE(error2->shift(), error.shift());
}

QTEST_GUILESS_MAIN(TestIntervalError)
#include "testIntervalError.moc"


