#include <QTest>
#include <QSignalSpy>
#include <QJsonDocument>
#include <QJsonObject>

#include "../src/qualityNorm.h"
#include "../src/qualityNormResult.h"
#include "../src/qualityNormLevel.h"
#include "../src/gaugeRange.h"

using precitec::storage::QualityNorm;
using precitec::storage::QualityNormResult;
using precitec::storage::QualityNormLevel;
using precitec::storage::GaugeRange;

class TestQualityNorm : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void testCtor();
    void testName();
    void testResult();
    void testJson();
    void testCommonResults();
};

void TestQualityNorm::testCtor()
{
    QualityNorm qualityNorm{this};
    QVERIFY(!qualityNorm.uuid().isNull());
    QVERIFY(qualityNorm.name().isNull());
    QVERIFY(qualityNorm.m_qualityNormResults.empty());

    QualityNorm qualityNorm2{QUuid{"4a8c526f-07ec-4933-8502-0282f98927ec"}, QStringLiteral("myQN"), this};
    QCOMPARE(qualityNorm2.uuid(), QUuid{"4a8c526f-07ec-4933-8502-0282f98927ec"});
    QCOMPARE(qualityNorm2.name(), QStringLiteral("myQN"));
}

void TestQualityNorm::testName()
{
    QualityNorm qualityNorm{this};
    QVERIFY(qualityNorm.name().isNull());

    QSignalSpy nameChangedSpy(&qualityNorm, &QualityNorm::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    qualityNorm.setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    qualityNorm.setName(QStringLiteral("My QN"));
    QCOMPARE(qualityNorm.name(), QStringLiteral("My QN"));
    QCOMPARE(nameChangedSpy.count(), 1);

    qualityNorm.setName(QStringLiteral("My QN"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestQualityNorm::testResult()
{
    QualityNorm qualityNorm{this};

    auto qnResult_114 = new QualityNormResult{this};
    qnResult_114->setResultType(114);
    qualityNorm.m_qualityNormResults.emplace_back(qnResult_114);

    auto qnResult_115 = new QualityNormResult{this};
    qnResult_115->setResultType(115);
    qualityNorm.m_qualityNormResults.emplace_back(qnResult_115);

    QVERIFY(!qualityNorm.qualityNormResult(35));
    QCOMPARE(qualityNorm.qualityNormResult(114), qnResult_114);
    QCOMPARE(qualityNorm.qualityNormResult(115), qnResult_115);
}

void TestQualityNorm::testJson()
{
    const QString dir = QFINDTESTDATA("testdata/quality_norms/qualityNorm.json");

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

    auto qualityNorm = QualityNorm::fromJson(document.object(), this);
    QCOMPARE(qualityNorm->name(), QStringLiteral("ISO"));
    QCOMPARE(qualityNorm->uuid(), QUuid{"0477aa1d-168a-4437-80bf-d1da322232c3"});
    QCOMPARE(qualityNorm->m_qualityNormResults.size(), 3);

    QVERIFY(qualityNorm->qualityNormResult(114));
    QVERIFY(qualityNorm->qualityNormResult(115));
    QVERIFY(qualityNorm->qualityNormResult(116));
}

void TestQualityNorm::testCommonResults()
{
    const QString dir = QFINDTESTDATA("testdata/quality_norms/qualityNorms.json");

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

    QualityNorm qualityNorm{this};
    QCOMPARE(qualityNorm.m_qualityNormResults.size(), 0);

    qualityNorm.addCommonResultsFromJson(document.object());
    QCOMPARE(qualityNorm.m_qualityNormResults.size(), 1);

    QVERIFY(qualityNorm.qualityNormResult(113));
}

QTEST_GUILESS_MAIN(TestQualityNorm)
#include "testQualityNorm.moc"




