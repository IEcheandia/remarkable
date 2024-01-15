#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/qualityNormResult.h"
#include "../src/qualityNorm.h"
#include "../src/qualityNormLevel.h"
#include "../src/gaugeRange.h"

using precitec::storage::QualityNormResult;
using precitec::storage::QualityNorm;
using precitec::storage::QualityNormLevel;
using precitec::storage::GaugeRange;

class TestQualityNormResult : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testResultType();
    void testJson();
};

void TestQualityNormResult::testCtor()
{
    QualityNormResult qnResult{this};
    QCOMPARE(qnResult.resultType(), -1);
    QCOMPARE(qnResult.m_levels.size(), 3);
    QCOMPARE(qnResult.levelAt(0)->level(), 1);
    QCOMPARE(qnResult.levelAt(1)->level(), 2);
    QCOMPARE(qnResult.levelAt(2)->level(), 3);
}

void TestQualityNormResult::testResultType()
{
    QualityNormResult qnResult{this};
    QCOMPARE(qnResult.resultType(), -1);

    QSignalSpy resultTypeChangedSpy(&qnResult, &QualityNormResult::resultTypeChanged);
    QVERIFY(resultTypeChangedSpy.isValid());

    qnResult.setResultType(-1);
    QCOMPARE(resultTypeChangedSpy.count(), 0);

    qnResult.setResultType(501);
    QCOMPARE(qnResult.resultType(), 501);
    QCOMPARE(resultTypeChangedSpy.count(), 1);

    qnResult.setResultType(501);
    QCOMPARE(resultTypeChangedSpy.count(), 1);
}

void TestQualityNormResult::testJson()
{
    const QString dir = QFINDTESTDATA("testdata/quality_norms/qualityNormResult.json");

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

    auto qnResult = QualityNormResult::fromJson(document.object(), new QualityNorm{this});
    QCOMPARE(qnResult->resultType(), 115);

    auto lvl_1 = qnResult->levelAt(0);
    QCOMPARE(lvl_1->level(), 1);
    QCOMPARE(lvl_1->m_gaugeRanges.size(), 1);

    const auto gauge_0 = lvl_1->m_gaugeRanges.at(0);
    QCOMPARE(gauge_0->range(), 99999.0);
    QCOMPARE(gauge_0->minFactor(), 0.1);
    QCOMPARE(gauge_0->minOffset(), 0.0);
    QCOMPARE(gauge_0->maxFactor(), 0.1);
    QCOMPARE(gauge_0->maxOffset(), 0.0);
    QCOMPARE(gauge_0->length(), 2.0);

    auto lvl_2 = qnResult->levelAt(1);
    QCOMPARE(lvl_2->level(), 2);
    QCOMPARE(lvl_2->m_gaugeRanges.size(), 2);

    const auto gauge_1 = lvl_2->m_gaugeRanges.at(0);
    QCOMPARE(gauge_1->range(), 1.0);
    QCOMPARE(gauge_1->minFactor(), 0.0);
    QCOMPARE(gauge_1->minOffset(), 0.0);
    QCOMPARE(gauge_1->maxFactor(), 0.1);
    QCOMPARE(gauge_1->maxOffset(), 0.0);
    QCOMPARE(gauge_1->length(), 2.0);

    const auto gauge_2 = lvl_2->m_gaugeRanges.at(1);
    QCOMPARE(gauge_2->range(), 99999.0);
    QCOMPARE(gauge_2->minFactor(), 0.0);
    QCOMPARE(gauge_2->minOffset(), 0.0);
    QCOMPARE(gauge_2->maxFactor(), 0.2);
    QCOMPARE(gauge_2->maxOffset(), 0.0);
    QCOMPARE(gauge_2->length(), 2.0);

    auto lvl_3 = qnResult->levelAt(2);
    QCOMPARE(lvl_3->level(), 3);
    QCOMPARE(lvl_3->m_gaugeRanges.size(), 1);

    const auto gauge_3 = lvl_3->m_gaugeRanges.at(0);
    QCOMPARE(gauge_3->range(), 99999.0);
    QCOMPARE(gauge_3->minFactor(), 0.0);
    QCOMPARE(gauge_3->minOffset(), 0.2);
    QCOMPARE(gauge_3->maxFactor(), 0.3);
    QCOMPARE(gauge_3->maxOffset(), 0.4);
    QCOMPARE(gauge_3->length(), 1.0);
}

QTEST_GUILESS_MAIN(TestQualityNormResult)
#include "testQualityNormResult.moc"



