#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>

#include "../src/copyMode.h"
#include "../src/productError.h"
#include "../src/product.h"
#include "../src/attributeModel.h"
#include "common/graph.h"

using precitec::storage::CopyMode;
using precitec::storage::ProductError;
using precitec::storage::Product;
using precitec::storage::AttributeModel;

class TestProductError : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testVariantId();
    void testResultValue();
    void testErrorType();
    void testName();
    void testThreshold();
    void testProduct();
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

void TestProductError::testCtor()
{
    ProductError error;

    QVERIFY(!error.uuid().isNull());
    QVERIFY(error.variantId().isNull());
    QVERIFY(error.name().isEmpty());
    QVERIFY(!error.product());
    QCOMPARE(error.resultValue(), 0);
    QCOMPARE(error.errorType(), 0);
    QCOMPARE(error.threshold(), 0);
}

void TestProductError::testVariantId()
{
    ProductError error;

    auto id = QUuid::createUuid();
    error.setVariantId(id);
    QCOMPARE(error.variantId(), id);
}

void TestProductError::testResultValue()
{
    ProductError error;
    QCOMPARE(error.resultValue(), 0);

    QSignalSpy resultValueChangedSpy(&error, &ProductError::resultValueChanged);
    QVERIFY(resultValueChangedSpy.isValid());

    error.setResultValue(0);
    QCOMPARE(resultValueChangedSpy.count(), 0);

    error.setResultValue(501);
    QCOMPARE(error.resultValue(), 501);
    QCOMPARE(resultValueChangedSpy.count(), 1);

    error.setResultValue(501);
    QCOMPARE(resultValueChangedSpy.count(), 1);
}

void TestProductError::testErrorType()
{
    ProductError error;
    QCOMPARE(error.errorType(), 0);

    QSignalSpy errorTypeChangedSpy(&error, &ProductError::errorTypeChanged);
    QVERIFY(errorTypeChangedSpy.isValid());

    error.setErrorType(0);
    QCOMPARE(errorTypeChangedSpy.count(), 0);

    error.setErrorType(501);
    QCOMPARE(error.errorType(), 501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);

    error.setErrorType(501);
    QCOMPARE(errorTypeChangedSpy.count(), 1);
}

void TestProductError::testName()
{
    ProductError error;
    QCOMPARE(error.name(), QStringLiteral(""));

    QSignalSpy nameChangedSpy(&error, &ProductError::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    error.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(error.name(), QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);

    error.setName(QStringLiteral("My Error"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestProductError::testThreshold()
{
    ProductError error;
    QCOMPARE(error.threshold(), 0);

    QSignalSpy thresholdChangedSpy(&error, &ProductError::thresholdChanged);
    QVERIFY(thresholdChangedSpy.isValid());

    error.setThreshold(0);
    QCOMPARE(thresholdChangedSpy.count(), 0);

    error.setThreshold(40);
    QCOMPARE(error.threshold(), 40);
    QCOMPARE(thresholdChangedSpy.count(), 1);

    error.setThreshold(40);
    QCOMPARE(thresholdChangedSpy.count(), 1);
}

void TestProductError::testProduct()
{
    const auto product = new Product(QUuid::createUuid(), this);

    ProductError error(this);
    QVERIFY(!error.product());

    QSignalSpy productChangedSpy(&error, &ProductError::productChanged);
    QVERIFY(productChangedSpy.isValid());

    error.setProduct(product);
    QVERIFY(error.product());
    QCOMPARE(error.product(), product);

    QCOMPARE(productChangedSpy.count(), 1);

    error.setProduct(product);
    QCOMPARE(productChangedSpy.count(), 1);

    const auto product2 = new Product(QUuid::createUuid(), this);

    error.setProduct(product2);
    QCOMPARE(error.product(), product2);
    QCOMPARE(productChangedSpy.count(), 2);
}

void TestProductError::testInitFromAttributes()
{
    AttributeModel model;
    QSignalSpy modelResetSpy{&model, &AttributeModel::modelReset};
    QVERIFY(modelResetSpy.isValid());
    model.load(QFINDTESTDATA("testdata/attributes/attributes.json"));
    QVERIFY(modelResetSpy.wait());
    QCOMPARE(modelResetSpy.count(), 1);

    ProductError error(this);
    error.setVariantId("37E21057-EFD4-4C18-A298-BE9F804C6C04");
    error.initFromAttributes(&model);
    QCOMPARE(error.threshold(), 10);
}

void TestProductError::testIntValue_data()
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

void TestProductError::testIntValue()
{
    ProductError error;
    error.setResultValue(105);
    error.setErrorType(64);

    QFETCH(QString, name);
    QTEST(error.getIntValue(name), "result");
}

void TestProductError::testDoubleValue()
{
    ProductError error;
    error.setThreshold(15);

    QCOMPARE(error.getDoubleValue(QStringLiteral("Threshold")), 15.0);
    QCOMPARE(error.getDoubleValue(QStringLiteral("Other")), 0.0);
}

void TestProductError::testStringValue()
{
    ProductError error;
    QCOMPARE(error.getStringValue(QStringLiteral("Scope")), std::string("Product"));

    QCOMPARE(error.getStringValue(QStringLiteral("Other")), QUuid{}.toString(QUuid::WithoutBraces).toStdString());
}

void TestProductError::testFromJson()
{
    const QString dir = QFINDTESTDATA(QStringLiteral("testdata/errors/productError.json"));

    QFile file(dir);

    QVERIFY(file.open(QIODevice::ReadOnly));
    const QByteArray data = file.readAll();
    QVERIFY(!data.isEmpty());
    const auto document = QJsonDocument::fromJson(data);
    QVERIFY(!document.isNull());

    const auto product = new Product(QUuid::createUuid(), this);
    auto error = ProductError::fromJson(document.object(), product);

    QCOMPARE(error->uuid(), QUuid("08cdfc15-e6a3-4308-9550-0e9984ce665c"));
    QCOMPARE(error->variantId(), QUuid("37e21057-efd4-4c18-a298-be9f804c6c04"));
    QCOMPARE(error->name(), QStringLiteral("My Product Error"));
    QCOMPARE(error->resultValue(), 65);
    QCOMPARE(error->errorType(), 17);
    QCOMPARE(error->threshold(), 9);
}

void TestProductError::testToJson()
{
    const auto product = new Product(QUuid::createUuid(), this);

    ProductError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Overlying Error"));
    error.setProduct(product);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setThreshold(7);

    auto json = error.toJson();
    QCOMPARE(json.count(), 6);

    auto error2 = ProductError::fromJson(json, product);
    QCOMPARE(error.uuid(), error2->uuid());
    QCOMPARE(error.variantId(), error2->variantId());
    QCOMPARE(error.name(), error2->name());
    QCOMPARE(error.product(), error2->product());
    QCOMPARE(error.resultValue(), error2->resultValue());
    QCOMPARE(error.errorType(), error2->errorType());
    QCOMPARE(error.threshold(), error2->threshold());
}

void TestProductError::testDuplicate_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestProductError::testDuplicate()
{
    const auto product = new Product(QUuid::createUuid(), this);

    ProductError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Another Overlying Error"));
    error.setProduct(product);
    error.setResultValue(204);
    error.setErrorType(1016);
    error.setThreshold(7);

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto duplicated = error.duplicate(copyMode, product);
    QVERIFY(duplicated);
    bool const hasDifferentUuid = duplicated->uuid() != error.uuid();
    QCOMPARE(hasDifferentUuid, changeUuid);
    QCOMPARE(duplicated->product(), product);
    QCOMPARE(error.variantId(), duplicated->variantId());
    QCOMPARE(error.name(), duplicated->name());
    QCOMPARE(error.resultValue(), duplicated->resultValue());
    QCOMPARE(error.errorType(), duplicated->errorType());
    QCOMPARE(error.threshold(), duplicated->threshold());
}

void TestProductError::testParameterList()
{
    const auto product = new Product(QUuid::createUuid(), this);
    product->setLwmTriggerSignalThreshold(2.5);

    ProductError error;
    error.setVariantId(QUuid("CEDEACB4-D4BB-4FDC-945A-BDC54E5848A6"));
    error.setName(QStringLiteral("Overlying Error"));
    error.setProduct(product);
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
            QCOMPARE(QString::fromStdString(parameter->value<std::string>()), QStringLiteral("Product"));
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
        else if (QString::fromStdString(parameter->name()).compare(QStringLiteral("SecondThreshold")) == 0)
        {
            secondThresholdFound = true;
            QCOMPARE(parameter->value<double>(), 0.0);
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
    QVERIFY(secondThresholdFound);
    QVERIFY(lwmTriggerSignalThresholdFound);
}

QTEST_GUILESS_MAIN(TestProductError)
#include "testProductError.moc"
