#include <QTest>
#include <QSignalSpy>
#include <QJsonObject>

#include "../src/gaugeRange.h"

using precitec::storage::GaugeRange;

class TestGaugeRange : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testRange();
    void testMinFactor();
    void testMinOffset();
    void testMaxFactor();
    void testMaxOffset();
    void testLength();
    void testMinThreshold();
    void testMaxThreshold();
    void testJson();
};

void TestGaugeRange::testCtor()
{
    GaugeRange gauge{this};

    QCOMPARE(gauge.range(), 0.0);
    QCOMPARE(gauge.minFactor(), 1.0);
    QCOMPARE(gauge.minOffset(), 0.0);
    QCOMPARE(gauge.maxFactor(), 1.0);
    QCOMPARE(gauge.maxOffset(), 0.0);
    QCOMPARE(gauge.length(), 1.0);
}

void TestGaugeRange::testRange()
{
    GaugeRange gauge{this};
    QCOMPARE(gauge.range(), 0.0);

    QSignalSpy rangeChangedSpy(&gauge, &GaugeRange::rangeChanged);
    QVERIFY(rangeChangedSpy.isValid());

    gauge.setRange(0.0);
    QCOMPARE(rangeChangedSpy.count(), 0);

    gauge.setRange(13.7);
    QCOMPARE(gauge.range(), 13.7);
    QCOMPARE(rangeChangedSpy.count(), 1);

    gauge.setRange(13.7);
    QCOMPARE(rangeChangedSpy.count(), 1);
}

void TestGaugeRange::testMinFactor()
{
    GaugeRange gauge{this};
    QCOMPARE(gauge.minFactor(), 1.0);

    QSignalSpy minFactorChangedSpy(&gauge, &GaugeRange::minFactorChanged);
    QVERIFY(minFactorChangedSpy.isValid());

    gauge.setMinFactor(1.0);
    QCOMPARE(minFactorChangedSpy.count(), 0);

    gauge.setMinFactor(13.7);
    QCOMPARE(gauge.minFactor(), 13.7);
    QCOMPARE(minFactorChangedSpy.count(), 1);

    gauge.setMinFactor(13.7);
    QCOMPARE(minFactorChangedSpy.count(), 1);
}

void TestGaugeRange::testMinOffset()
{
    GaugeRange gauge{this};
    QCOMPARE(gauge.minOffset(), 0.0);

    QSignalSpy minOffsetChangedSpy(&gauge, &GaugeRange::minOffsetChanged);
    QVERIFY(minOffsetChangedSpy.isValid());

    gauge.setMinOffset(0.0);
    QCOMPARE(minOffsetChangedSpy.count(), 0);

    gauge.setMinOffset(13.7);
    QCOMPARE(gauge.minOffset(), 13.7);
    QCOMPARE(minOffsetChangedSpy.count(), 1);

    gauge.setMinOffset(13.7);
    QCOMPARE(minOffsetChangedSpy.count(), 1);
}

void TestGaugeRange::testMaxFactor()
{
    GaugeRange gauge{this};
    QCOMPARE(gauge.maxFactor(), 1.0);

    QSignalSpy maxFactorChangedSpy(&gauge, &GaugeRange::maxFactorChanged);
    QVERIFY(maxFactorChangedSpy.isValid());

    gauge.setMaxFactor(1.0);
    QCOMPARE(maxFactorChangedSpy.count(), 0);

    gauge.setMaxFactor(13.7);
    QCOMPARE(gauge.maxFactor(), 13.7);
    QCOMPARE(maxFactorChangedSpy.count(), 1);

    gauge.setMaxFactor(13.7);
    QCOMPARE(maxFactorChangedSpy.count(), 1);
}

void TestGaugeRange::testMaxOffset()
{
    GaugeRange gauge{this};
    QCOMPARE(gauge.maxOffset(), 0.0);

    QSignalSpy maxOffsetChangedSpy(&gauge, &GaugeRange::maxOffsetChanged);
    QVERIFY(maxOffsetChangedSpy.isValid());

    gauge.setMaxOffset(0.0);
    QCOMPARE(maxOffsetChangedSpy.count(), 0);

    gauge.setMaxOffset(13.7);
    QCOMPARE(gauge.maxOffset(), 13.7);
    QCOMPARE(maxOffsetChangedSpy.count(), 1);

    gauge.setMaxOffset(13.7);
    QCOMPARE(maxOffsetChangedSpy.count(), 1);
}

void TestGaugeRange::testLength()
{
    GaugeRange gauge{this};
    QCOMPARE(gauge.length(), 1.0);

    QSignalSpy lengthChangedSpy(&gauge, &GaugeRange::lengthChanged);
    QVERIFY(lengthChangedSpy.isValid());

    gauge.setLength(1.0);
    QCOMPARE(lengthChangedSpy.count(), 0);

    gauge.setLength(13.7);
    QCOMPARE(gauge.length(), 13.7);
    QCOMPARE(lengthChangedSpy.count(), 1);

    gauge.setLength(13.7);
    QCOMPARE(lengthChangedSpy.count(), 1);
}

void TestGaugeRange::testMinThreshold()
{
    GaugeRange gauge{this};

    QCOMPARE(gauge.minThreshold(4), 4.0);

    gauge.setMinFactor(2.5);
    QCOMPARE(gauge.minThreshold(4), 10.0);

    gauge.setMinOffset(7);
    QCOMPARE(gauge.minThreshold(4), 17.0);
}

void TestGaugeRange::testMaxThreshold()
{
    GaugeRange gauge{this};

    QCOMPARE(gauge.maxThreshold(4), 4.0);

    gauge.setMaxFactor(2.5);
    QCOMPARE(gauge.maxThreshold(4), 10.0);

    gauge.setMaxOffset(7);
    QCOMPARE(gauge.maxThreshold(4), 17.0);
}

void TestGaugeRange::testJson()
{
    GaugeRange gauge{this};

    gauge.setRange(5.0);
    gauge.setMinFactor(6.0);
    gauge.setMinOffset(7.0);
    gauge.setMaxFactor(8.0);
    gauge.setMaxOffset(9.0);
    gauge.setLength(10.0);

    const auto json = gauge.toJson();

    const auto new_gauge = GaugeRange::fromJson(json, this);

    QCOMPARE(gauge.range(), new_gauge->range());
    QCOMPARE(gauge.minFactor(), new_gauge->minFactor());
    QCOMPARE(gauge.minOffset(), new_gauge->minOffset());
    QCOMPARE(gauge.maxFactor(), new_gauge->maxFactor());
    QCOMPARE(gauge.maxOffset(), new_gauge->maxOffset());
    QCOMPARE(gauge.length(), new_gauge->length());
}

QTEST_GUILESS_MAIN(TestGaugeRange)
#include "testGaugeRange.moc"

