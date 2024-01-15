#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/copyMode.h"
#include "../src/seamSeriesError.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/attributeModel.h"
#include "common/graph.h"

using precitec::storage::AttributeModel;
using precitec::storage::CopyMode;
using precitec::storage::Product;
using precitec::storage::SeamSeriesError;

class TestSeamSeriesError : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testCtorWithUuid();
    void testVariantId();
    void testResultValue();
    void testErrorType();
    void testName();
    void testThreshold();
    void testMeasureTask();
    void testInitFromAttributes();
    void testIntValue_data();
    void testIntValue();
    void testDoubleValue();
    void testStringValue();
    void testFromJson();
    void testToJson();
    void testDuplicate_data();
    void testDuplicate();
    void testParameterList();
};

void TestSeamSeriesError::testCtor()
{
    SeamSeriesError error;

    QVERIFY(!error.uuid().isNull());
    QVERIFY(error.variantId().isNull());
    QVERIFY(error.name().isEmpty());
    QVERIFY(!error.measureTask());
    QCOMPARE(error.resultValue(), 0);
    QCOMPARE(error.errorType(), 0);
    QCOMPARE(error.threshold(), 0);
}

void TestSeamSeriesError::testCtorWithUuid()
{    
    QUuid const uuid{"55c53584-9b02-4153-9993-ec00ebd62180"};
    SeamSeriesError error{uuid};

    QCOMPARE(error.uuid(), uuid);
}

void TestSeamSeriesError::testVariantId()
{
    SeamSeriesError error;

    auto id = QUuid::createUuid();
    error.setVariantId(id);
    QCOMPARE(error.variantId(), id);
}

void TestSeamSeriesError::testResultValue()
{
    SeamSeriesError error;
    QCOMPARE(error.resultValue(), 0);

    QSignalSpy resultValueChangedSpy(&error, &SeamSeriesError::resultValueChanged);
    QVERIFY(resultValueChangedSpy.isValid());

    error.setResultValue(0);
    QCOMPARE(resultValueChangedSpy.count(), 0);

    error.setResultValue(501);
    QCOMPARE(error.resultValue(), 501);
    QCOMPARE(resultValueChangedSpy.count(), 1);

    error.setResultValue(501);
    QCOMPARE(resultValueChangedSpy.count(), 1);
}

void TestSeamSeriesError::testErrorType()
{
    SeamSeriesError error;
    QCOMPARE(error.errorType(), 0);

    QSignalSpy errorTypeChangedSpy(&error, &SeamSeriesError::errorTypeChanged);
    QVERIFY(errorTypeChangedSpy.isValid());

    error.setErrorType(0);
    QCOMPARE(errorTypeChangedSpy.count(), 0);

    error.setErrorType(501);
    QCOMPARE(error.errorType(), 501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);

    error.setErrorType(501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);
}

void TestSeamSeriesError::testName()
{
    SeamSeriesError error;
    QCOMPARE(error.name(), QStringLiteral(""));

    QSignalSpy nameChangedSpy(&error, &SeamSeriesError::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    error.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(error.name(), QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestSeamSeriesError::testThreshold()
{
    SeamSeriesError error;
    QCOMPARE(error.threshold(), 0);

    QSignalSpy thresholdChangedSpy(&error, &SeamSeriesError::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    error.setThreshold(0);
    QCOMPARE(thresholdChangedSpy.count(), 0);

    error.setThreshold(40);
    QCOMPARE(error.threshold(), 40);
    QCOMPARE(thresholdChangedSpy.count(), 1);

    error.setThreshold(40);
    QCOMPARE(thresholdChangedSpy.count(), 1);
}

void TestSeamSeriesError::testMeasureTask()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto series = product->createSeamSeries();

    SeamSeriesError error(this);
    QVERIFY(!error.measureTask());

    QSignalSpy measureTaskChangedSpy(&error, &SeamSeriesError::measureTaskChanged);
    QVERIFY(measureTaskChangedSpy.isValid());

    error.setMeasureTask(series);
    QVERIFY(error.measureTask());
    QCOMPARE(error.measureTask(), series);

    QCOMPARE(measureTaskChangedSpy.count(), 1);

    error.setMeasureTask(series);
    QCOMPARE(measureTaskChangedSpy.count(), 1);

    const auto series2 = product->createSeamSeries();

    error.setMeasureTask(series2);
    QCOMPARE(error.measureTask(), series2);
    QCOMPARE(measureTaskChangedSpy.count(), 2);
}

void TestSeamSeriesError::testInitFromAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    SeamSeriesError error(this);
    error.setVariantId("37E21057-EFD4-4C18-A298-BE9F804C6C04");
    error.initFromAttributes(&model);
    QCOMPARE(error.threshold(), 10);
}

void TestSeamSeriesError::testIntValue_data()
{
    QTest::addColumn<QString>("name");
    QTest::addColumn<int>("result");

    QTest::newRow("series") << QStringLiteral("SeamSeries") << 0;
    QTest::newRow("seam") << QStringLiteral("Seam") << 0;
    QTest::newRow("interval") << QStringLiteral("SeamInterval") << 0;
    QTest::newRow("result") << QStringLiteral("Result") << 105;
    QTest::newRow("error") << QStringLiteral("Error") << 64;
    QTest::newRow("other") << QStringLiteral("Something") << -1;
}

void TestSeamSeriesError::testIntValue()
{
    SeamSeriesError error;
    error.setResultValue(105);
    error.setErrorType(64);

    QFETCH(QString, name);
    QTEST(error.getIntValue(name), "result");
}

void TestSeamSeriesError::testDoubleValue()
{
    SeamSeriesError error;
    error.setThreshold(15);

    QCOMPARE(error.getDoubleValue(QStringLiteral("Threshold")), 15.0);
    QCOMPARE(error.getDoubleValue(QStringLiteral("Other")), 0.0);
}

void TestSeamSeriesError::testStringValue()
{
    SeamSeriesError error;
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("SeamSeries"));

    QCOMPARE(error.getStringValue(QStringLiteral("Other")), QUuid{}.toString(QUuid::WithoutBraces).toStdString());
}

void TestSeamSeriesError::testFromJson()
{
    const QString dir = QFINDTESTDATA(QStringLiteral("testdata/errors/productError.json"));

    QFile file(dir);

    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    QVERIFY(!data.isEmpty());
    const auto document = QJsonDocument::fromJson(data);
    QVERIFY(!document.isNull());

    const auto product = new Product(QUuid::createUuid(), this);
    const auto series = product->createSeamSeries();
    auto error = SeamSeriesError::fromJson(document.object(), series);

    QCOMPARE(error->uuid(), QUuid("08cdfc15-e6a3-4308-9550-0e9984ce665c"));
    QCOMPARE(error->variantId(), QUuid("37e21057-efd4-4c18-a298-be9f804c6c04"));
    QCOMPARE(error->name(), QStringLiteral("My Product Error"));
    QCOMPARE(error->resultValue(), 65);
    QCOMPARE(error->errorType(), 17);
    QCOMPARE(error->threshold(), 9);
}

void TestSeamSeriesError::testToJson()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto series = product->createSeamSeries();

    SeamSeriesError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(series);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setThreshold(7);

    auto json = error.toJson();
    QCOMPARE(json.count(), 6);

    auto error2 = SeamSeriesError::fromJson(json, series);
    QCOMPARE(error.uuid(), error2->uuid());
    QCOMPARE(error.variantId(), error2->variantId());
    QCOMPARE(error.name(), error2->name());
    QCOMPARE(error.measureTask(), error2->measureTask());
    QCOMPARE(error.resultValue(), error2->resultValue());
    QCOMPARE(error.errorType(), error2->errorType());
    QCOMPARE(error.threshold(), error2->threshold());
}

void TestSeamSeriesError::testDuplicate_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestSeamSeriesError::testDuplicate()
{
    const auto product = new Product(QUuid::createUuid(), this);
    const auto series = product->createSeamSeries();

    SeamSeriesError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Simple Error"));
    error.setMeasureTask(series);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setThreshold(7);

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto duplicated = error.duplicate(copyMode, series);
    QVERIFY(duplicated);
    QCOMPARE(duplicated->uuid() != error.uuid(), changeUuid);
    QCOMPARE(duplicated->measureTask(), series);
    QCOMPARE(error.variantId(), duplicated->variantId());
    QCOMPARE(error.name(), duplicated->name());
    QCOMPARE(error.resultValue(), duplicated->resultValue());
    QCOMPARE(error.errorType(), duplicated->errorType());
    QCOMPARE(error.threshold(), duplicated->threshold());
}

void TestSeamSeriesError::testParameterList()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->setLwmTriggerSignalThreshold(2.5);
    const auto series = product->createSeamSeries();

    SeamSeriesError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Seam Error"));
    error.setMeasureTask(series);
    error.setResultValue(15);
    error.setErrorType(1016);
    error.setThreshold(7);

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
            QCOMPARE(parameter->value<double>(), 0.0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Max")) == 0)
        {
            maxFound = true;
            QCOMPARE(parameter->value<double>(), 0.0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Threshold")) == 0)
        {
            thresholdFound = true;
            QCOMPARE(parameter->value<double>(), 7.0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Scope")) == 0)
        {
            scopeFound = true;
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), QStringLiteral("SeamSeries"));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SeamSeries")) == 0)
        {
            seamSeriesFound = true;
            QCOMPARE(parameter->value<int>(), 0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Seam")) == 0)
        {
            seamFound = true;
            QCOMPARE(parameter->value<int>(), 0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SeamInterval")) == 0)
        {
            seamIntervalFound = true;
            QCOMPARE(parameter->value<int>(), 0);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("Reference")) == 0)
        {
            referenceFound = true;
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), QUuid{}.toString(QUuid::WithoutBraces));
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("MiddleReference")) == 0)
        {
            middleReferenceFound = true;
            QCOMPARE(parameter->value<bool>(), false);
        }
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("LwmSignalThreshold")) == 0)
        {
            lwmTriggerSignalThresholdFound = true;
            QCOMPARE(parameter->value<double>(), 0.0);
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
    QVERIFY(lwmTriggerSignalThresholdFound);
}

QTEST_GUILESS_MAIN(TestSeamSeriesError)
#include "testSeamSeriesError.moc"

