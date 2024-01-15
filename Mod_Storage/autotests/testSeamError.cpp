#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/copyMode.h"
#include "../src/seamError.h"
#include "../src/referenceCurve.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "../src/seamInterval.h"
#include "../src/attributeModel.h"
#include "common/graph.h"

using precitec::storage::CopyMode;
using precitec::storage::SeamError;
using precitec::storage::Product;
using precitec::storage::AttributeModel;
using precitec::storage::ReferenceCurve;

class TestSeamError : public QObject
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
    void testMin();
    void testMax();
    void testThreshold();
    void testSecondThreshold();
    void testEnvelope();
    void testUseMiddleCurve();
    void testInitFromAttributes();
    void getIntValue_data();
    void getIntValue();
    void testGetDoubleValue_data();
    void testGetDoubleValue();
    void getStringValue();
    void testFromJson_data();
    void testFromJson();
    void testToJson();
    void testDuplicate_data();
    void testDuplicate();
    void testParameterList_data();
    void testParameterList();
    void testUpdateLowerBounds_data();
    void testUpdateLowerBounds();
    void testUpdateUpperBounds_data();
    void testUpdateUpperBounds();
    void testTrack277();
};

void TestSeamError::testCtor()
{
    SeamError error;

    QVERIFY(!error.uuid().isNull());
    QVERIFY(error.variantId().isNull());
    QVERIFY(error.name().isEmpty());
    QVERIFY(!error.measureTask());
    QCOMPARE(error.resultValue(), 0);
    QCOMPARE(error.errorType(), 0);
    QCOMPARE(error.shift(), 0.0);
    QCOMPARE(error.minLimit(), -100000.0);
    QCOMPARE(error.maxLimit(), 100000.0);
    QCOMPARE(error.boundaryType(), SeamError::BoundaryType::Static);
    QCOMPARE(error.threshold(), 0.0);
    QCOMPARE(error.min(), 0.0);
    QCOMPARE(error.max(), 0.0);
    QVERIFY(error.useMiddleCurveAsReference());
    QVERIFY(error.envelope().isNull());
    QVERIFY(error.envelopeName().isEmpty());
    QCOMPARE(error.secondThreshold(), 0.0);
}

void TestSeamError::testCtorWithUuid()
{    
    QUuid const uuid{"55c53584-9b02-4153-9993-ec00ebd62180"};
    SeamError error{uuid};

    QCOMPARE(error.uuid(), uuid);
}

void TestSeamError::testVariantId_data()
{
    QTest::addColumn<QUuid>("variantId");
    QTest::addColumn<SeamError::BoundaryType>("boundary");

    QTest::newRow("LengthOutsideStaticBoundary") << QUuid("3B5FE50F-6FD5-4FBC-BD78-06B892E1F97D") << SeamError::BoundaryType::Static;
    QTest::newRow("LengthOutsideReferenceBoundary") << QUuid("5EB04560-2641-4E64-A016-14207E59A370") << SeamError::BoundaryType::Reference;
    QTest::newRow("AccumulatedLengthOutsideStaticBoundary") << QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6") << SeamError::BoundaryType::Static;
    QTest::newRow("AccumulatedLengthOutsideReferenceBoundary") << QUuid("F8F4E0A8-D259-40F9-B134-68AA24E0A06C") << SeamError::BoundaryType::Reference;
    QTest::newRow("LengthInsideStaticBoundary") << QUuid("3AF9EF6A-A4E9-4234-8BA5-7B42D3E58B2C") << SeamError::BoundaryType::Static;
    QTest::newRow("LengthInsideReferenceBoundary") << QUuid("4A6AE9B0-3A1A-427F-8D58-2D0205452377") << SeamError::BoundaryType::Reference;
    QTest::newRow("AreaStaticBoundary") << QUuid("73708EA1-580A-4660-8D80-63622670BC7C") << SeamError::BoundaryType::Static;
    QTest::newRow("AreaReferenceBoundary") << QUuid("D36ECEBA-286B-4D06-B596-0491B6544F40") << SeamError::BoundaryType::Reference;
    QTest::newRow("AccumulatedAreaStaticBoundary") << QUuid("740FD8B3-852C-485A-BC24-6C67A36DABD2") << SeamError::BoundaryType::Static;
    QTest::newRow("AccumulatedAreaReferenceBoundary") << QUuid("527B7421-5DDD-436C-BE33-C1A359A736F6") << SeamError::BoundaryType::Reference;
    QTest::newRow("PeakStaticBoundary") << QUuid("396CA433-AD11-4073-A2B2-5314CC41D152") << SeamError::BoundaryType::Static;
    QTest::newRow("PeakReferenceBoundary") << QUuid("7CF9F16D-36DE-4840-A2EA-C41979F91A9B") << SeamError::BoundaryType::Reference;
    QTest::newRow("DualOutlierStaticBoundary") << QUuid("55DCC3D9-FE50-4792-8E27-460AADDDD09F") << SeamError::BoundaryType::Static;
    QTest::newRow("DualOutlierReferenceBoundary") << QUuid("C0C80DA1-4E9D-4EC0-859A-8D43A0674571") << SeamError::BoundaryType::Reference;
}

void TestSeamError::testVariantId()
{
    SeamError error;

    QFETCH(QUuid, variantId);
    error.setVariantId(variantId);

    QTEST(error.variantId(), "variantId");
    QTEST(error.boundaryType(), "boundary");
}

void TestSeamError::testResultValue()
{
    SeamError error;
    QCOMPARE(error.resultValue(), 0);

    QSignalSpy resultValueChangedSpy(&error, &SeamError::resultValueChanged);
    QVERIFY(resultValueChangedSpy.isValid());

    error.setResultValue(0);
    QCOMPARE(resultValueChangedSpy.count(), 0);

    error.setResultValue(501);
    QCOMPARE(error.resultValue(), 501);
    QCOMPARE(resultValueChangedSpy.count(), 1);

    error.setResultValue(501);
    QCOMPARE(resultValueChangedSpy.count(), 1);
}

void TestSeamError::testErrorType()
{
    SeamError error;
    QCOMPARE(error.errorType(), 0);

    QSignalSpy errorTypeChangedSpy(&error, &SeamError::errorTypeChanged);
    QVERIFY(errorTypeChangedSpy.isValid());

    error.setErrorType(0);
    QCOMPARE(errorTypeChangedSpy.count(), 0);

    error.setErrorType(501);
    QCOMPARE(error.errorType(), 501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);

    error.setErrorType(501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);
}

void TestSeamError::testShift()
{
    SeamError error;
    QCOMPARE(error.shift(), 0);

    QSignalSpy shiftChangedSpy(&error, &SeamError::shiftChanged);
    QVERIFY(shiftChangedSpy.isValid());

    error.setShift(0.0);
    QCOMPARE(shiftChangedSpy.count(), 0);

    error.setShift(3.4);
    QCOMPARE(error.shift(), 3.4);
    QCOMPARE(shiftChangedSpy.count(), 1);

    error.setShift(3.4);
    QCOMPARE(shiftChangedSpy.count(), 1);
}

void TestSeamError::testMinLimit()
{
    SeamError error;
    QCOMPARE(error.minLimit(), -100000.0);

    QSignalSpy minLimitChangedSpy(&error, &SeamError::minLimitChanged);
    QVERIFY(minLimitChangedSpy.isValid());

    error.setMinLimit(-100000.0);
    QCOMPARE(minLimitChangedSpy.count(), 0);

    error.setMinLimit(-4000.0);
    QCOMPARE(error.minLimit(), -4000.0);
    QCOMPARE(minLimitChangedSpy.count(), 1);

    error.setMinLimit(-4000.0);
    QCOMPARE(minLimitChangedSpy.count(), 1);
}

void TestSeamError::testMaxLimit()
{
    SeamError error;
    QCOMPARE(error.maxLimit(), 100000.0);

    QSignalSpy maxLimitChangedSpy(&error, &SeamError::maxLimitChanged);
    QVERIFY(maxLimitChangedSpy.isValid());

    error.setMaxLimit(100000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 0);

    error.setMaxLimit(4000.0);
    QCOMPARE(error.maxLimit(), 4000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 1);

    error.setMaxLimit(4000.0);
    QCOMPARE(maxLimitChangedSpy.count(), 1);
}

void TestSeamError::testName()
{
    SeamError error;
    QCOMPARE(error.name(), QStringLiteral(""));

    QSignalSpy nameChangedSpy(&error, &SeamError::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    error.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(error.name(), QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestSeamError::testMeasureTask()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    SeamError error(this);
    QVERIFY(!error.measureTask());

    QSignalSpy measureTaskChangedSpy(&error, &SeamError::measureTaskChanged);
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

    error.setResultValue(15);
    const auto reference = seam2->createReferenceCurve(15);
    QSignalSpy usedChangedSpy(reference, &ReferenceCurve::usedChanged);
    QVERIFY(usedChangedSpy.isValid());
    QCOMPARE(reference->used(), false);

    error.setEnvelope(reference->uuid());
    QCOMPARE(error.envelope(), reference->uuid());
    QCOMPARE(usedChangedSpy.count(), 1);
    QCOMPARE(reference->used(), true);

    error.setMeasureTask(seam);
    QCOMPARE(error.measureTask(), seam);
    QCOMPARE(measureTaskChangedSpy.count(), 3);
    QCOMPARE(usedChangedSpy.count(), 2);
    QCOMPARE(reference->used(), false);
    QCOMPARE(error.envelope(), QUuid{});
}

void TestSeamError::testMin()
{
    SeamError error;
    QCOMPARE(error.min(), 0.0);

    QSignalSpy minChangedSpy(&error, &SeamError::minChanged);
    QVERIFY(minChangedSpy.isValid());

    error.setMin(0.0);
    QCOMPARE(minChangedSpy.count(), 0);

    error.setMin(40.0);
    QCOMPARE(error.min(), 40.0);
    QCOMPARE(minChangedSpy.count(), 1);

    error.setMin(40.0);
    QCOMPARE(minChangedSpy.count(), 1);
}

void TestSeamError::testMax()
{
    SeamError error;
    QCOMPARE(error.max(), 0.0);

    QSignalSpy maxChangedSpy(&error, &SeamError::maxChanged);
    QVERIFY(maxChangedSpy.isValid());

    error.setMax(0.0);
    QCOMPARE(maxChangedSpy.count(), 0);

    error.setMax(40.0);
    QCOMPARE(error.max(), 40.0);
    QCOMPARE(maxChangedSpy.count(), 1);

    error.setMax(40.0);
    QCOMPARE(maxChangedSpy.count(), 1);
}

void TestSeamError::testThreshold()
{
    SeamError error;
    QCOMPARE(error.threshold(), 0.0);

    QSignalSpy thresholdChangedSpy(&error, &SeamError::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    error.setThreshold(0.0);
    QCOMPARE(thresholdChangedSpy.count(), 0);

    error.setThreshold(40.0);
    QCOMPARE(error.threshold(), 40.0);
    QCOMPARE(thresholdChangedSpy.count(), 1);

    error.setThreshold(40.0);
    QCOMPARE(thresholdChangedSpy.count(), 1);
}

void TestSeamError::testSecondThreshold()
{
    SeamError error;
    QCOMPARE(error.secondThreshold(), 0.0);

    QSignalSpy secondThresholdChangedSpy(&error, &SeamError::secondThresholdChanged);
    QVERIFY(secondThresholdChangedSpy.isValid());

    error.setSecondThreshold(0.0);
    QCOMPARE(secondThresholdChangedSpy.count(), 0);

    error.setSecondThreshold(40.0);
    QCOMPARE(error.secondThreshold(), 40.0);
    QCOMPARE(secondThresholdChangedSpy.count(), 1);

    error.setSecondThreshold(40.0);
    QCOMPARE(secondThresholdChangedSpy.count(), 1);
}

void TestSeamError::testEnvelope()
{
    SeamError error;
    QCOMPARE(error.envelope(), QUuid{});
    QCOMPARE(error.envelopeName(), QString{});

    QSignalSpy envelopeChangedSpy(&error, &SeamError::envelopeChanged);
    QVERIFY(envelopeChangedSpy.isValid());

    QSignalSpy envelopeNameChangedSpy(&error, &SeamError::envelopeNameChanged);
    QVERIFY(envelopeNameChangedSpy.isValid());

    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = seamSeries->createSeam();
    const auto reference = seam->createReferenceCurve(15);
    reference->setName(QStringLiteral("Some Name"));
    QSignalSpy usedChangedSpy(reference, &ReferenceCurve::usedChanged);
    QVERIFY(usedChangedSpy.isValid());
    QCOMPARE(reference->used(), false);

    error.setEnvelope(reference->uuid());
    QCOMPARE(envelopeChangedSpy.count(), 0);
    QCOMPARE(envelopeNameChangedSpy.count(), 0);
    QCOMPARE(usedChangedSpy.count(), 0);

    error.setMeasureTask(seam);
    QCOMPARE(envelopeChangedSpy.count(), 0);
    QCOMPARE(envelopeNameChangedSpy.count(), 0);
    QCOMPARE(usedChangedSpy.count(), 0);

    error.setEnvelope(reference->uuid());
    QCOMPARE(envelopeChangedSpy.count(), 1);
    QCOMPARE(envelopeNameChangedSpy.count(), 0);
    QCOMPARE(usedChangedSpy.count(), 0);
    QCOMPARE(error.envelope(), QUuid{});
    QCOMPARE(error.envelopeName(), QString{});

    error.setResultValue(15);
    QCOMPARE(envelopeChangedSpy.count(), 1);
    QCOMPARE(envelopeNameChangedSpy.count(), 0);
    QCOMPARE(usedChangedSpy.count(), 0);

    error.setEnvelope(reference->uuid());
    QCOMPARE(envelopeChangedSpy.count(), 2);
    QCOMPARE(envelopeNameChangedSpy.count(), 1);
    QCOMPARE(usedChangedSpy.count(), 1);
    QCOMPARE(error.envelope(), reference->uuid());
    QCOMPARE(error.envelopeName(), QStringLiteral("Some Name"));
    QCOMPARE(reference->used(), true);

    reference->setName(QStringLiteral("Some Other Name"));
    QCOMPARE(envelopeNameChangedSpy.count(), 2);
    QCOMPARE(error.envelope(), reference->uuid());
    QCOMPARE(error.envelopeName(), QStringLiteral("Some Other Name"));

    error.setResultValue(10);
    QCOMPARE(envelopeChangedSpy.count(), 3);
    QCOMPARE(envelopeNameChangedSpy.count(), 3);
    QCOMPARE(usedChangedSpy.count(), 2);
    QCOMPARE(error.envelope(), QUuid{});
    QCOMPARE(error.envelopeName(), QString{});
    QCOMPARE(reference->used(), false);

    error.setResultValue(15);
    QCOMPARE(envelopeChangedSpy.count(), 3);
    QCOMPARE(envelopeNameChangedSpy.count(), 3);
    QCOMPARE(usedChangedSpy.count(), 2);

    error.setEnvelope(reference->uuid());
    QCOMPARE(envelopeChangedSpy.count(), 4);
    QCOMPARE(envelopeNameChangedSpy.count(), 4);
    QCOMPARE(usedChangedSpy.count(), 3);
    QCOMPARE(error.envelope(), reference->uuid());
    QCOMPARE(error.envelopeName(), QStringLiteral("Some Other Name"));
    QCOMPARE(reference->used(), true);

    error.setMeasureTask(seamSeries);
    QCOMPARE(envelopeChangedSpy.count(), 5);
    QCOMPARE(envelopeNameChangedSpy.count(), 5);
    QCOMPARE(usedChangedSpy.count(), 4);
    QCOMPARE(error.envelope(), QUuid{});
    QCOMPARE(error.envelopeName(), QString{});
    QCOMPARE(reference->used(), false);
}

void TestSeamError::testUseMiddleCurve()
{
    SeamError error;
    QCOMPARE(error.useMiddleCurveAsReference(), true);

    QSignalSpy useMiddleCurveAsReferenceChangedSpy(&error, &SeamError::useMiddleCurveAsReferenceChanged);
    QVERIFY(useMiddleCurveAsReferenceChangedSpy.isValid());

    error.setUseMiddleCurveAsReference(true);
    QCOMPARE(useMiddleCurveAsReferenceChangedSpy.count(), 0);

    error.setUseMiddleCurveAsReference(false);
    QCOMPARE(error.useMiddleCurveAsReference(), false);
    QCOMPARE(useMiddleCurveAsReferenceChangedSpy.count(), 1);

    error.setUseMiddleCurveAsReference(false);
    QCOMPARE(useMiddleCurveAsReferenceChangedSpy.count(), 1);
}

void TestSeamError::testInitFromAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    SeamError error(this);
    error.setVariantId("F8F4E0A8-D259-40F9-B134-68AA24E0A06C");
    error.setMinLimit(0.0);
    QCOMPARE(error.minLimit(), 0.0);
    error.initFromAttributes(&model);
    QCOMPARE(error.minLimit(), -100000);
    QCOMPARE(error.threshold(), 10);
}

void TestSeamError::getIntValue_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("series");
    QTest::addColumn<int>("seam");
    QTest::addColumn<int>("interval");

    QTest::newRow("Measure Series") << QStringLiteral("SeamSeries") << 3 << 3 << 3;
    QTest::newRow("Measure Seam") << QStringLiteral("Seam") << 0 << 5 << 5;
    QTest::newRow("Measure Interval") << QStringLiteral("SeamInterval") << 0 << 0 << 7;
}

void TestSeamError::getIntValue()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    seamSeries->setNumber(3);
    const auto seam = seamSeries->createSeam();
    seam->setNumber(5);
    const auto interval = seam->createSeamInterval();
    interval->setNumber(7);

    SeamError error;
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

void TestSeamError::testGetDoubleValue_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<double>("result");
    QTest::addColumn<double>("shifted");

    QTest::newRow("min") << QStringLiteral("Min") << -25.3 << -21.9;
    QTest::newRow("max") << QStringLiteral("Max") << 17.26 << 20.66;
    QTest::newRow("threshold") << QStringLiteral("Threshold") << 9.7 << 9.7;
    QTest::newRow("other") << QStringLiteral("Other") << 0.0 << 0.0;
}

void TestSeamError::testGetDoubleValue()
{
    SeamError error;
    error.setMin(-25.3);
    error.setMax(17.26);
    error.setThreshold(9.7);

    QFETCH(QString, name);
    QTEST(error.getDoubleValue(name), "result");

    error.setShift(3.4);

    QTEST(error.getDoubleValue(name), "shifted");
}

void TestSeamError::getStringValue()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = seamSeries->createSeam();
    const auto interval = seam->createSeamInterval();

    SeamError error;
    error.setResultValue(15);
    error.setMeasureTask(interval);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamInterval"));

    error.setMeasureTask(seam);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("Seam"));

    error.setMeasureTask(seamSeries);
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamSeries"));

    QCOMPARE(error.getStringValue(QStringLiteral("Other")), std::string("Product"));

    auto id = QUuid::createUuid();
    error.setEnvelope(id);
    QCOMPARE(error.getStringValue(QStringLiteral("Reference")), QUuid{}.toString(QUuid::WithoutBraces).toStdString());

    const auto reference = seamSeries->createReferenceCurve(15);
    error.setEnvelope(reference->uuid());
    QCOMPARE(error.getStringValue(QStringLiteral("Reference")), reference->uuid().toString(QUuid::WithoutBraces).toStdString());
}

void TestSeamError::testFromJson_data()
{
    QTest::addColumn<QString>("fileName");

    QTest::newRow("new") << "seamError";
    QTest::newRow("legacy") << "legacySeamError";
}

void TestSeamError::testFromJson()
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

    auto error = SeamError::fromJson(document.object(), seam);

    QCOMPARE(error->uuid(), QUuid("94f507f1-4215-4fbd-afd1-fe34b77347e5"));
    QCOMPARE(error->variantId(), QUuid("f8f4e0a8-d259-40f9-b134-68aa24e0a06c"));
    QCOMPARE(error->name(), QStringLiteral("My Seam Error"));
    QCOMPARE(error->resultValue(), 517);
    QCOMPARE(error->errorType(), 1007);
    QCOMPARE(error->minLimit(), -99000);
    QCOMPARE(error->maxLimit(), 99000);
    QCOMPARE(error->shift(), 3.7);
    QCOMPARE(error->boundaryType(), SeamError::BoundaryType::Reference);
    QCOMPARE(error->max(), 1.2);
    QCOMPARE(error->min(), 0.99);
    QCOMPARE(error->threshold(), 13);
    QCOMPARE(error->secondThreshold(), 2.4);
    QCOMPARE(error->useMiddleCurveAsReference(), false);
}

void TestSeamError::testToJson()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->createFirstSeamSeries();
    const auto seam = product->createSeam();

    SeamError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(seam);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setMinLimit(-43000);
    error.setMaxLimit(37000);
    error.setShift(2.6);
    error.setMin(-3.7);
    error.setMax(12.3);
    error.setThreshold(7.4);
    error.setSecondThreshold(4.6);
    error.setEnvelope(QUuid::createUuid());
    error.setUseMiddleCurveAsReference(false);

    // static error
    auto json = error.toJson();
    QCOMPARE(json.count(), 12);

    auto error2 = SeamError::fromJson(json, seam);
    QCOMPARE(error.uuid(), error2->uuid());
    QCOMPARE(error.variantId(), error2->variantId());
    QCOMPARE(error.name(), error2->name());
    QCOMPARE(error.measureTask(), error2->measureTask());
    QCOMPARE(error.resultValue(), error2->resultValue());
    QCOMPARE(error.errorType(), error2->errorType());
    QCOMPARE(error.minLimit(), error2->minLimit());
    QCOMPARE(error.maxLimit(), error2->maxLimit());
    QCOMPARE(error.shift(), error2->shift());
    QCOMPARE(error.boundaryType(), error2->boundaryType());
    QCOMPARE(error.min(), error2->min());
    QCOMPARE(error.max(), error2->max());
    QCOMPARE(error.threshold(), error2->threshold());
    QCOMPARE(error.secondThreshold(), error2->secondThreshold());
    QCOMPARE(error2->envelope(), QUuid{});
    QCOMPARE(error2->useMiddleCurveAsReference(), true);

    // reference error
    error.setVariantId("f8f4e0a8-d259-40f9-b134-68aa24e0a06c");
    auto json2 = error.toJson();
    QCOMPARE(json2.count(), 14);

    auto error3 = SeamError::fromJson(json2, seam);
    QCOMPARE(error.uuid(), error3->uuid());
    QCOMPARE(error.variantId(), error3->variantId());
    QCOMPARE(error.name(), error3->name());
    QCOMPARE(error.measureTask(), error3->measureTask());
    QCOMPARE(error.resultValue(), error3->resultValue());
    QCOMPARE(error.errorType(), error3->errorType());
    QCOMPARE(error.minLimit(), error3->minLimit());
    QCOMPARE(error.maxLimit(), error3->maxLimit());
    QCOMPARE(error.shift(), error3->shift());
    QCOMPARE(error.boundaryType(), error3->boundaryType());
    QCOMPARE(error.min(), error3->min());
    QCOMPARE(error.max(), error3->max());
    QCOMPARE(error.threshold(), error3->threshold());
    QCOMPARE(error.secondThreshold(), error3->secondThreshold());
    QCOMPARE(error.envelope(), error3->envelope());
    QCOMPARE(error.useMiddleCurveAsReference(), error3->useMiddleCurveAsReference());
}

void TestSeamError::testDuplicate_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestSeamError::testDuplicate()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = product->createSeamSeries();
    const auto seam = product->createSeam();

    SeamError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(seam);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setMinLimit(-43000);
    error.setMaxLimit(37000);
    error.setShift(2.6);
    error.setMin(-3.7);
    error.setMax(12.3);
    error.setThreshold(7.4);
    error.setUseMiddleCurveAsReference(false);

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;

    auto duplicated = error.duplicate(copyMode, seamSeries);
    QVERIFY(duplicated);
    QCOMPARE(duplicated->uuid() != error.uuid(), changeUuid);
    QCOMPARE(duplicated->measureTask(), seamSeries);
    QCOMPARE(error.variantId(), duplicated->variantId());
    QCOMPARE(error.name(), duplicated->name());
    QCOMPARE(error.resultValue(), duplicated->resultValue());
    QCOMPARE(error.errorType(), duplicated->errorType());
    QCOMPARE(error.minLimit(), duplicated->minLimit());
    QCOMPARE(error.maxLimit(), duplicated->maxLimit());
    QCOMPARE(error.shift(), duplicated->shift());
    QCOMPARE(error.min(), duplicated->min());
    QCOMPARE(error.max(), duplicated->max());
    QCOMPARE(error.threshold(), duplicated->threshold());
    QCOMPARE(error.useMiddleCurveAsReference(), duplicated->useMiddleCurveAsReference());
    // envelope is not duplicated, as the duplicated reference has a new uuid
}

void TestSeamError::testParameterList_data()
{
    QTest::addColumn<QUuid>("variantId");
    QTest::addColumn<double>("min");
    QTest::addColumn<double>("max");
    QTest::addColumn<double>("reference");

    QTest::newRow("static") << QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6") << -3.7 + 2.6 << 12.3 + 2.6;
    QTest::newRow("reference") << QUuid("D36ECEBA-286B-4D06-B596-0491B6544F40") << -3.7 << 12.3;
}

void TestSeamError::testParameterList()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->setLwmTriggerSignalThreshold(2.5);
    const auto seamSeries = product->createSeamSeries();
    seamSeries->setNumber(3);
    const auto seam = seamSeries->createSeam();
    seam->setNumber(5);
    const auto reference = seam->createReferenceCurve(15);

    SeamError error;
    QFETCH(QUuid, variantId);
    error.setVariantId(variantId);
    error.setName(QStringLiteral("Seam Error"));
    error.setMeasureTask(seam);
    error.setResultValue(15);
    error.setErrorType(1016);
    error.setMinLimit(-43000);
    error.setMaxLimit(37000);
    error.setShift(2.6);
    error.setMin(-3.7);
    error.setMax(12.3);
    error.setThreshold(7.4);
    error.setSecondThreshold(3.8);
    error.setUseMiddleCurveAsReference(false);
    error.setEnvelope(reference->uuid());

    auto parameterList = error.toParameterList();
    QCOMPARE(parameterList.size(), 13);

    auto resultFound = false;
    auto errorFound = false;
    auto minFound = false;
    auto maxFound = false;
    auto thresholdFound = false;
    auto scopeFound = false;
    auto seamSeriesFound = false;
    auto seamFound = false;
    auto seamIntervalFound = false;
    auto referenceFound = false;
    auto middleReferenceFound = false;
    auto secondThresholdFound = false;
    auto lwmTriggerSignalThresholdFound = false;

    for (auto &parameter : parameterList)
    {
        if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Result")) == 0)
        {
            resultFound = true;
            QCOMPARE(parameter->value<int>(), 15);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Error")) == 0)
        {
            errorFound = true;
            QCOMPARE(parameter->value<int>(), 1016);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Min")) == 0)
        {
            minFound = true;
            QTEST(parameter->value<double>(), "min");
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Max")) == 0)
        {
            maxFound = true;
            QTEST(parameter->value<double>(), "max");
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Threshold")) == 0)
        {
            thresholdFound = true;
            QCOMPARE(parameter->value<double>(), 7.4);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Scope")) == 0)
        {
            scopeFound = true;
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), QStringLiteral("Seam"));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SeamSeries")) == 0)
        {
            seamSeriesFound = true;
            QCOMPARE(parameter->value<int>(), 3);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Seam")) == 0)
        {
            seamFound = true;
            QCOMPARE(parameter->value<int>(), 5);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SeamInterval")) == 0)
        {
            seamIntervalFound = true;
            QCOMPARE(parameter->value<int>(), 0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Reference")) == 0)
        {
            referenceFound = true;
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), error.envelope().toString(QUuid::WithoutBraces));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("MiddleReference")) == 0)
        {
            middleReferenceFound = true;
            QCOMPARE(parameter->value<bool>(), error.useMiddleCurveAsReference());
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SecondThreshold")) == 0)
        {
            secondThresholdFound = true;
            QCOMPARE(parameter->value<double>(), 3.8);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("LwmSignalThreshold")) == 0)
        {
            lwmTriggerSignalThresholdFound = true;
            QCOMPARE(parameter->value<double>(), 2.5);
        }

        QCOMPARE(QString::fromStdString(parameter->instanceID().toString()), error.uuid().toString(QUuid::WithoutBraces));
        QCOMPARE(QString::fromStdString(parameter->typID().toString()), error.variantId().toString(QUuid::WithoutBraces));
    }

    QVERIFY(resultFound);
    QVERIFY(errorFound);
    QVERIFY(minFound);
    QVERIFY(maxFound);
    QVERIFY(thresholdFound);
    QVERIFY(scopeFound);
    QVERIFY(seamSeriesFound);
    QVERIFY(seamFound);
    QVERIFY(seamIntervalFound);
    QVERIFY(referenceFound);
    QVERIFY(middleReferenceFound);
    QVERIFY(secondThresholdFound);
    QVERIFY(lwmTriggerSignalThresholdFound);
}

void TestSeamError::testUpdateLowerBounds_data()
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

void TestSeamError::testUpdateLowerBounds()
{
    SeamError error;
    error.setMaxLimit(10.0);
    error.setMax(5.0);
    error.setMin(3.0);
    error.setShift(-3.0);

    QFETCH(double, limitValue);
    error.setMinLimit(limitValue);

    QTEST(error.shift(), "shift");
    QTEST(error.min(), "min");
    QTEST(error.max(), "max");
    QTEST(error.maxLimit(), "maxLimit");
}

void TestSeamError::testUpdateUpperBounds_data()
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

void TestSeamError::testUpdateUpperBounds()
{
    SeamError error;
    error.setMinLimit(2.0);
    error.setMin(5.0);
    error.setMax(7.0);
    error.setShift(3.0);

    QFETCH(double, limitValue);
    error.setMaxLimit(limitValue);

    QTEST(error.shift(), "shift");
    QTEST(error.max(), "max");
    QTEST(error.min(), "min");
    QTEST(error.minLimit(), "minLimit");
}

void TestSeamError::testTrack277()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seam = product->createSeam();

    SeamError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(seam);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setMinLimit(-5000);
    error.setMaxLimit(-1000);
    error.setShift(402.1);
    error.setMin(-3405);
    error.setMax(-3395);
    error.setThreshold(10);
    error.setUseMiddleCurveAsReference(false);

    std::unique_ptr<SeamError> error2{SeamError::fromJson(error.toJson(), seam)};
    QCOMPARE(error2->shift(), error.shift());
}

QTEST_GUILESS_MAIN(TestSeamError)
#include "testSeamError.moc"
