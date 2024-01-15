#include <QTest>
#include <QSignalSpy>

#include "../src/extendedProductInfoHelper.h"

using precitec::storage::ExtendedProductInfoHelper;

class TestExtendedProductInfoHelper : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSerialNumberFromExtendedProductInfo();
    void testSerialNumberFromExtendedProductInfoField();
    void testPartNumberFromExtendedProductInfo();
    void testPartNumberFromExtendedProductInfoField();

    void testSerialNumber_data();
    void testSerialNumber();
    void testPartNumber_data();
    void testPartNumber();
};

void TestExtendedProductInfoHelper::testCtor()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();
    QVERIFY(!helper->serialNumberFromExtendedProductInfo());
    QVERIFY(!helper->partNumberFromExtendedProductInfo());
    QCOMPARE(helper->serialNumberFromExtendedProductInfoField(), 0);
    QCOMPARE(helper->partNumberFromExtendedProductInfoField(), 0);
}

void TestExtendedProductInfoHelper::testSerialNumberFromExtendedProductInfo()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();
    QCOMPARE(helper->serialNumberFromExtendedProductInfo(), false);

    QSignalSpy changedSpy{helper.get(), &ExtendedProductInfoHelper::serialNumberFromExtendedProductInfoChanged};
    QVERIFY(changedSpy.isValid());

    helper->setSerialNumberFromExtendedProductInfo(true);
    QCOMPARE(helper->serialNumberFromExtendedProductInfo(), true);
    QCOMPARE(changedSpy.count(), 1);
    // setting same should not change
    helper->setSerialNumberFromExtendedProductInfo(true);
    QCOMPARE(changedSpy.count(), 1);

    helper->setSerialNumberFromExtendedProductInfo(false);
    QCOMPARE(helper->serialNumberFromExtendedProductInfo(), false);
    QCOMPARE(changedSpy.count(), 2);
}

void TestExtendedProductInfoHelper::testSerialNumberFromExtendedProductInfoField()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();
    QCOMPARE(helper->serialNumberFromExtendedProductInfoField(), 0u);

    QSignalSpy changedSpy{helper.get(), &ExtendedProductInfoHelper::serialNumberFromExtendedProductInfoFieldChanged};
    QVERIFY(changedSpy.isValid());

    helper->setSerialNumberFromExtendedProductInfoField(1);
    QCOMPARE(helper->serialNumberFromExtendedProductInfoField(), 1u);
    QCOMPARE(changedSpy.count(), 1);
    // setting same should not change
    helper->setSerialNumberFromExtendedProductInfoField(1);
    QCOMPARE(changedSpy.count(), 1);
}

void TestExtendedProductInfoHelper::testPartNumberFromExtendedProductInfo()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();
    QCOMPARE(helper->partNumberFromExtendedProductInfo(), false);

    QSignalSpy changedSpy{helper.get(), &ExtendedProductInfoHelper::partNumberFromExtendedProductInfoChanged};
    QVERIFY(changedSpy.isValid());

    helper->setPartNumberFromExtendedProductInfo(true);
    QCOMPARE(helper->partNumberFromExtendedProductInfo(), true);
    QCOMPARE(changedSpy.count(), 1);
    // setting same should not change
    helper->setPartNumberFromExtendedProductInfo(true);
    QCOMPARE(changedSpy.count(), 1);

    helper->setPartNumberFromExtendedProductInfo(false);
    QCOMPARE(helper->partNumberFromExtendedProductInfo(), false);
    QCOMPARE(changedSpy.count(), 2);
}

void TestExtendedProductInfoHelper::testPartNumberFromExtendedProductInfoField()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();
    QCOMPARE(helper->partNumberFromExtendedProductInfoField(), 0u);

    QSignalSpy changedSpy{helper.get(), &ExtendedProductInfoHelper::partNumberFromExtendedProductInfoFieldChanged};
    QVERIFY(changedSpy.isValid());

    helper->setPartNumberFromExtendedProductInfoField(1);
    QCOMPARE(helper->partNumberFromExtendedProductInfoField(), 1u);
    QCOMPARE(changedSpy.count(), 1);
    // setting same should not change
    helper->setPartNumberFromExtendedProductInfoField(1);
    QCOMPARE(changedSpy.count(), 1);
}

void TestExtendedProductInfoHelper::testSerialNumber_data()
{
    QTest::addColumn<QString>("extendedProductInfo");
    QTest::addColumn<bool>("set");
    QTest::addColumn<uint>("field");
    QTest::addColumn<bool>("expectedIsSet");
    QTest::addColumn<QString>("expectedValue");

    QTest::newRow("disabled") << QStringLiteral("STFR21336002351\nP2623301-35-B") << false << 0u << false << QString{};
    QTest::newRow("enabled") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 0u << true << QStringLiteral("STFR21336002351");
    QTest::newRow("wrong index") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 2u << false << QString{};
    QTest::newRow("empty") << QString{} << true << 0u << false << QString{};
}

void TestExtendedProductInfoHelper::testSerialNumber()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();

    QFETCH(bool, set);
    helper->setSerialNumberFromExtendedProductInfo(set);
    QFETCH(quint32, field);
    helper->setSerialNumberFromExtendedProductInfoField(field);
    QFETCH(QString, extendedProductInfo);
    const auto value = helper->serialNumber(extendedProductInfo);
    QTEST(value.has_value(), "expectedIsSet");
    if (value)
    {
        QTEST(value.value(), "expectedValue");
    }
}

void TestExtendedProductInfoHelper::testPartNumber_data()
{
    QTest::addColumn<QString>("extendedProductInfo");
    QTest::addColumn<bool>("set");
    QTest::addColumn<uint>("field");
    QTest::addColumn<bool>("expectedIsSet");
    QTest::addColumn<QString>("expectedValue");

    QTest::newRow("disabled") << QStringLiteral("STFR21336002351\nP2623301-35-B") << false << 0u << false << QString{};
    QTest::newRow("enabled") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 1u << true << QStringLiteral("P2623301-35-B");
    QTest::newRow("wrong index") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 2u << false << QString{};
    QTest::newRow("empty") << QString{} << true << 0u << false << QString{};
}

void TestExtendedProductInfoHelper::testPartNumber()
{
    auto helper = std::make_unique<ExtendedProductInfoHelper>();

    QFETCH(bool, set);
    helper->setPartNumberFromExtendedProductInfo(set);
    QFETCH(quint32, field);
    helper->setPartNumberFromExtendedProductInfoField(field);
    QFETCH(QString, extendedProductInfo);
    const auto value = helper->partNumber(extendedProductInfo);
    QTEST(value.has_value(), "expectedIsSet");
    if (value)
    {
        QTEST(value.value(), "expectedValue");
    }
}

QTEST_GUILESS_MAIN(TestExtendedProductInfoHelper)
#include "testExtendedProductInfoHelper.moc"
