#include <QTest>
#include <QSignalSpy>

#include "../src/valueConverter.h"

using precitec::scanmaster::components::wobbleFigureEditor::ValueConverter;

class ValueConverterTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testConvertDoubleToPercent();
    void testConvertFromPercentDouble();
};

void ValueConverterTest::testCtor()
{
    auto converter = ValueConverter::instance();
    QVERIFY(converter);
}

void ValueConverterTest::testConvertDoubleToPercent()
{
    auto converter = ValueConverter::instance();

    double test = 0.1;
    QCOMPARE(converter->convertDoubleToPercent(test), 10.0);
    test = 0.0;
    QCOMPARE(converter->convertDoubleToPercent(test), 0.0);
    test = 0.25;
    QCOMPARE(converter->convertDoubleToPercent(test), 25.0);
    test = 0.3;
    QCOMPARE(converter->convertDoubleToPercent(test), 30.0);
    test = 0.6;
    QCOMPARE(converter->convertDoubleToPercent(test), 60.0);
    test = 0.75;
    QCOMPARE(converter->convertDoubleToPercent(test), 75.0);
    test = 1.0;
    QCOMPARE(converter->convertDoubleToPercent(test), 100.0);
}

void ValueConverterTest::testConvertFromPercentDouble()
{
    auto converter = ValueConverter::instance();

    double test = 10.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 0.1);
    test = 0.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 0.0);
    test = 25.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 0.25);
    test = 30.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 0.3);
    test = 60.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 0.6);
    test = 75.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 0.75);
    test = 100.0;
    QCOMPARE(converter->convertFromPercentDouble(test), 1.0);
}


QTEST_GUILESS_MAIN(ValueConverterTest)
#include "valueConverterTest.moc"
