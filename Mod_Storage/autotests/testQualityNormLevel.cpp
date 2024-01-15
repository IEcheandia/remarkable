#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/qualityNormLevel.h"
#include "../src/qualityNormResult.h"
#include "../src/gaugeRange.h"

using precitec::storage::QualityNormLevel;
using precitec::storage::QualityNormResult;
using precitec::storage::GaugeRange;

class TestQualityNormLevel : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testRange();
    void testJson();
};

void TestQualityNormLevel::testCtor()
{
    QualityNormLevel qnLevel{2, this};
    QCOMPARE(qnLevel.level(), 2);
    QVERIFY(qnLevel.m_gaugeRanges.empty());
}

void TestQualityNormLevel::testRange()
{
    QualityNormLevel qnLevel{2, this};
    QVERIFY(qnLevel.m_gaugeRanges.empty());

    auto null_gauge = qnLevel.range(5);
    QVERIFY(!null_gauge);

    auto gauge_5 = new GaugeRange(this);
    gauge_5->setRange(5.0);
    qnLevel.m_gaugeRanges.push_back(gauge_5);

    auto gauge_at_0 = qnLevel.range(0);
    auto gauge_at_5 = qnLevel.range(5);
    auto gauge_at_7 = qnLevel.range(7);

    QCOMPARE(gauge_at_0, gauge_5);
    QCOMPARE(gauge_at_5, gauge_5);
    QCOMPARE(gauge_at_7, gauge_5);

    auto gauge_7 = new GaugeRange(this);
    gauge_7->setRange(5.0);
    qnLevel.m_gaugeRanges.push_back(gauge_7);

    auto gauge_at_0_new = qnLevel.range(0);
    auto gauge_at_5_new = qnLevel.range(5);
    auto gauge_at_7_new = qnLevel.range(7);
    auto gauge_at_9_new = qnLevel.range(9);

    QCOMPARE(gauge_at_0_new, gauge_5);
    QCOMPARE(gauge_at_5_new, gauge_5);
    QCOMPARE(gauge_at_7_new, gauge_7);
    QCOMPARE(gauge_at_9_new, gauge_7);
}

void TestQualityNormLevel::testJson()
{
    const QString dir = QFINDTESTDATA("testdata/quality_norms/qualityNormLevel.json");

    QFile file(dir);
    if (!file.open(QIODevice::ReadOnly))
    {
        QVERIFY(false);
    }
    const QByteArray data = file.readAll();
    if (data.isEmpty())
    {
        QVERIFY(false);
    }
    const auto document = QJsonDocument::fromJson(data);
    if (document.isNull())
    {
        QVERIFY(false);
    }

    auto qnLevel = QualityNormLevel::fromJson(document.object(), new QualityNormResult{this}, 2);
    QCOMPARE(qnLevel->level(), 2);
    QCOMPARE(qnLevel->m_gaugeRanges.size(), 2);

    const auto gauge_1 = qnLevel->m_gaugeRanges.at(0);
    QCOMPARE(gauge_1->range(), 1.0);
    QCOMPARE(gauge_1->minFactor(), 0.0);
    QCOMPARE(gauge_1->minOffset(), 0.0);
    QCOMPARE(gauge_1->maxFactor(), 0.1);
    QCOMPARE(gauge_1->maxOffset(), 0.0);
    QCOMPARE(gauge_1->length(), 2.0);

    const auto gauge_2 = qnLevel->m_gaugeRanges.at(1);
    QCOMPARE(gauge_2->range(), 99999.0);
    QCOMPARE(gauge_2->minFactor(), 0.0);
    QCOMPARE(gauge_2->minOffset(), 0.0);
    QCOMPARE(gauge_2->maxFactor(), 0.2);
    QCOMPARE(gauge_2->maxOffset(), 0.0);
    QCOMPARE(gauge_2->length(), 2.0);
}

QTEST_GUILESS_MAIN(TestQualityNormLevel)
#include "testQualityNormLevel.moc"


