#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/simpleError.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "../src/seamInterval.h"
#include "../src/attributeModel.h"

using precitec::storage::SimpleError;
using precitec::storage::Product;
using precitec::storage::AttributeModel;

class TestSimpleError : public QObject
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
    void testInitFromAttributes();
    void getIntValue_data();
    void getIntValue();
    void getStringValue();
    void testFromJson();
    void testToJson();
};

void TestSimpleError::testCtor()
{
    SimpleError error;

    QVERIFY(!error.uuid().isNull());
    QVERIFY(error.variantId().isNull());
    QVERIFY(error.name().isEmpty());
    QVERIFY(!error.measureTask());
    QCOMPARE(error.resultValue(), 0);
    QCOMPARE(error.errorType(), 0);
    QCOMPARE(error.shift(), 0.0);
    QCOMPARE(error.minLimit(), -100000.0);
    QCOMPARE(error.maxLimit(), 100000.0);
    QCOMPARE(error.boundaryType(), SimpleError::BoundaryType::Static);
}

void TestSimpleError::testCtorWithUuid()
{    
    QUuid const uuid{"55c53584-9b02-4153-9993-ec00ebd62180"};
    SimpleError error{uuid};

    QCOMPARE(error.uuid(), uuid);
}

void TestSimpleError::testVariantId_data()
{
    QTest::addColumn<QUuid>("variantId");
    QTest::addColumn<SimpleError::BoundaryType>("boundary");

    QTest::newRow("LengthOutsideStaticBoundary") << QUuid("3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D") << SimpleError::BoundaryType::Static;
    QTest::newRow("LengthOutsideReferenceBoundary") << QUuid("5EB04560-2641-4E64-A016-14207E59A370") << SimpleError::BoundaryType::Reference;
    QTest::newRow("AccumulatedLengthOutsideStaticBoundary") << QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6") << SimpleError::BoundaryType::Static;
    QTest::newRow("AccumulatedLengthOutsideReferenceBoundary") << QUuid("F8F4E0A8-D259-40F9-B134-68AA24E0A06C") << SimpleError::BoundaryType::Reference;
    QTest::newRow("LengthInsideStaticBoundary") << QUuid("3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C") << SimpleError::BoundaryType::Static;
    QTest::newRow("LengthInsideReferenceBoundary") << QUuid("4A6AE9B0-3A1A-427F-8D58-2D0205452377") << SimpleError::BoundaryType::Reference;
    QTest::newRow("AreaStaticBoundary") << QUuid("73708EA1-580A-4660-8D80-63622670BC7C") << SimpleError::BoundaryType::Static;
    QTest::newRow("AreaReferenceBoundary") << QUuid("D36ECEBA-286B-4D06-B596-0491B6544F40") << SimpleError::BoundaryType::Reference;
    QTest::newRow("AccumulatedAreaStaticBoundary") << QUuid("740FD8B3-852C-485A-BC24-6C67A36DABD2") << SimpleError::BoundaryType::Static;
    QTest::newRow("AccumulatedAreaReferenceBoundary") << QUuid("527B7421-5DDD-436C-BE33-C1A359A736F6") << SimpleError::BoundaryType::Reference;
    QTest::newRow("PeakStaticBoundary") << QUuid("396CA433-AD11-4073-A2B2-5314CC41D152") << SimpleError::BoundaryType::Static;
    QTest::newRow("PeakReferenceBoundary") << QUuid("7CF9F16D-36DE-4840-A2EA-C41979F91A9B") << SimpleError::BoundaryType::Reference;
}

void TestSimpleError::testVariantId()
{
    SimpleError error;

    QFETCH(QUuid, variantId);
    error.setVariantId(variantId);

    QTEST(error.variantId(), "variantId");
    QTEST(error.boundaryType(), "boundary");
}

void TestSimpleError::testResultValue()
{
    SimpleError error;
    QCOMPARE(error.resultValue(), 0);

    QSignalSpy resultValueChangedSpy(&error, &SimpleError::resultValueChanged);
    QVERIFY(resultValueChangedSpy.isValid());

    error.setResultValue(0);
    QCOMPARE(resultValueChangedSpy.count(), 0);

    error.setResultValue(501);
    QCOMPARE(error.resultValue(), 501);
    QCOMPARE(resultValueChangedSpy.count(), 1);

    error.setResultValue(501);
    QCOMPARE(resultValueChangedSpy.count(), 1);
}

void TestSimpleError::testErrorType()
{
    SimpleError error;
    QCOMPARE(error.errorType(), 0);

    QSignalSpy errorTypeChangedSpy(&error, &SimpleError::errorTypeChanged);
    QVERIFY(errorTypeChangedSpy.isValid());

    error.setErrorType(0);
    QCOMPARE(errorTypeChangedSpy.count(), 0);

    error.setErrorType(501);
    QCOMPARE(error.errorType(), 501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);

    error.setErrorType(501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);
}

void TestSimpleError::testShift()
{
    SimpleError error;
    QCOMPARE(error.shift(), 0);

    QSignalSpy shiftChangedSpy(&error, &SimpleError::shiftChanged);
    QVERIFY(shiftChangedSpy.isValid());

    error.setShift(0.0);
    QCOMPARE(shiftChangedSpy.count(), 0);

    error.setShift(3.4);
    QCOMPARE(error.shift(), 3.4);
    QCOMPARE(shiftChangedSpy.count(), 1);

    error.setShift(3.4);
    QCOMPARE(shiftChangedSpy.count(), 1);
}

void TestSimpleError::testMinLimit()
{
    SimpleError error;
    QCOMPARE(error.minLimit(), -100000.0);

    QSignalSpy minLimitChangedSpy(&error, &SimpleError::minLimitChanged);
    QVERIFY(minLimitChangedSpy.isValid());

    error.setMinLimit(-100000.0);
    QCOMPARE(minLimitChangedSpy.count(), 0);

    error.setMinLimit(-4000.0);
    QCOMPARE(error.minLimit(), -4000.0);
    QCOMPARE(minLimitChangedSpy.count(), 1);

    error.setMinLimit(-4000.0);
    QCOMPARE(minLimitChangedSpy.count(), 1);
}

void TestSimpleError::testMaxLimit()
{
    SimpleError error;
    QCOMPARE(error.maxLimit(), 100000.0);

    QSignalSpy maxLimitChangedSpy(&error, &SimpleError::maxLimitChanged);
    QVERIFY(maxLimitChangedSpy.isValid());

    error.setMaxLimit(100000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 0);

    error.setMaxLimit(4000.0);
    QCOMPARE(error.maxLimit(), 4000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 1);

    error.setMaxLimit(4000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 1);
}

void TestSimpleError::testName()
{
    SimpleError error;
    QCOMPARE(error.name(), QStringLiteral(""));

    QSignalSpy nameChangedSpy(&error, &SimpleError::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    error.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(error.name(), QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestSimpleError::testMeasureTask()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    SimpleError error(this);
    QVERIFY(!error.measureTask());

    QSignalSpy measureTaskChangedSpy(&error, &SimpleError::measureTaskChanged);
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

void TestSimpleError::testInitFromAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    SimpleError error(this);
    error.setVariantId("F8F4E0A8-D259-40F9-B134-68AA24E0A06C");
    error.setMinLimit(0.0);
    QCOMPARE(error.minLimit(), 0.0);
    error.initFromAttributes(&model);
    QCOMPARE(error.minLimit(), -100000);
}

void TestSimpleError::getIntValue_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("series");
    QTest::addColumn<int>("seam");
    QTest::addColumn<int>("interval");

    QTest::newRow("Measure Series") << QStringLiteral("SeamSeries") << 3 << 3 << 3;
    QTest::newRow("Measure Seam") << QStringLiteral("Seam") << 0 << 5 << 5;
    QTest::newRow("Measure Interval") << QStringLiteral("SeamInterval") << 0 << 0 << 7;
}

void TestSimpleError::getIntValue()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    seamSeries->setNumber(3);
    const auto seam = seamSeries->createSeam();
    seam->setNumber(5);
    const auto interval = seam->createSeamInterval();
    interval->setNumber(7);

    SimpleError error;
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

void TestSimpleError::getStringValue()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = seamSeries->createSeam();
    const auto interval = seam->createSeamInterval();

    SimpleError error;
    error.setMeasureTask(interval);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamInterval"));

    error.setMeasureTask(seam);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("Seam"));

    error.setMeasureTask(seamSeries);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamSeries"));

    QCOMPARE(error.getStringValue(QStringLiteral("Other")), std::string("Product"));
}

void TestSimpleError::testFromJson()
{
    const QString dir = QFINDTESTDATA("testdata/errors/simpleError.json");

    QFile file(dir);
    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    QVERIFY(!data.isEmpty());
    const auto document = QJsonDocument::fromJson(data);
    QVERIFY(!document.isNull());

    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    SimpleError error;
    error.fromJson(document.object(), seam);

    QCOMPARE(error.uuid(), QUuid("94f507f1-4215-4fbd-afd1-fe34b77347e5"));
    QCOMPARE(error.variantId(), QUuid("3b5fe50f-6fd5-4fbc-bd78-06b892e1f97d"));
    QCOMPARE(error.name(), QStringLiteral("My Simple Error"));
    QCOMPARE(error.resultValue(), 517);
    QCOMPARE(error.errorType(), 1007);
    QCOMPARE(error.minLimit(), -99000);
    QCOMPARE(error.maxLimit(), 99000);
    QCOMPARE(error.shift(), 3.7);
    QCOMPARE(error.boundaryType(), SimpleError::BoundaryType::Static);
}

void TestSimpleError::testToJson()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    SimpleError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(seam);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setMinLimit(-43000);
    error.setMaxLimit(37000);
    error.setShift(2.6);

    auto json = error.toJson();

    SimpleError error2;
    error2.fromJson(json, seam);
    QCOMPARE(error.uuid(), error2.uuid());
    QCOMPARE(error.variantId(), error2.variantId());
    QCOMPARE(error.name(), error2.name());
    QCOMPARE(error.measureTask(), error2.measureTask());
    QCOMPARE(error.resultValue(), error2.resultValue());
    QCOMPARE(error.errorType(), error2.errorType());
    QCOMPARE(error.minLimit(), error2.minLimit());
    QCOMPARE(error.maxLimit(), error2.maxLimit());
    QCOMPARE(error.shift(), error2.shift());
    QCOMPARE(error.boundaryType(), error2.boundaryType());
}

QTEST_GUILESS_MAIN(TestSimpleError)
#include "testSimpleError.moc"
