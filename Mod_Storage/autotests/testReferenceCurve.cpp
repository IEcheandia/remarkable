#include <QTest>
#include <QSignalSpy>

#include "../src/copyMode.h"
#include "../src/referenceCurve.h"
#include "../src/referenceCurveData.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"

using precitec::storage::CopyMode;
using precitec::storage::ReferenceCurve;
using precitec::storage::Product;
using precitec::storage::SeamSeries;
using precitec::storage::Seam;

class TestReferenceCurve: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testName();
    void testResultType();
    void testMeasureTask();
    void testReferenceType();
    void testJitter();
    void testDuplicate_data();
    void testDuplicate();
    void testJson();
    void testSource();
    void testUsed();
};

void TestReferenceCurve::testCtor()
{
    auto curve = new ReferenceCurve{this};
    QVERIFY(!curve->uuid().isNull());
    QVERIFY(!curve->lower().isNull());
    QVERIFY(!curve->middle().isNull());
    QVERIFY(!curve->upper().isNull());
    QVERIFY(!curve->measureTask());
    QVERIFY(!curve->used());
    QVERIFY(curve->m_sourceOfCurve.empty());
    QCOMPARE(curve->name(), QStringLiteral(""));
    QCOMPARE(curve->resultType(), -1);
    QCOMPARE(curve->referenceType(), ReferenceCurve::ReferenceType::Average);
    QCOMPARE(curve->referenceTypeString(), QStringLiteral("Average"));
    QCOMPARE(curve->jitter(), 0.0f);
}

void TestReferenceCurve::testName()
{
    auto curve = new ReferenceCurve{this};
    QCOMPARE(curve->name(), QStringLiteral(""));

    QSignalSpy nameChangedSpy(curve, &ReferenceCurve::nameChanged);
    QVERIFY(nameChangedSpy.isValid());

    curve->setName(QStringLiteral(""));
    QCOMPARE(nameChangedSpy.count(), 0);

    curve->setName(QStringLiteral("Curve Name"));
    QCOMPARE(curve->name(), QStringLiteral("Curve Name"));
    QCOMPARE(nameChangedSpy.count(), 1);

    curve->setName(QStringLiteral("Curve Name"));
    QCOMPARE(nameChangedSpy.count(), 1);
}

void TestReferenceCurve::testResultType()
{
    auto curve = new ReferenceCurve{this};
    QCOMPARE(curve->resultType(), -1);

    QSignalSpy typeChangedSpy(curve, &ReferenceCurve::resultTypeChanged);
    QVERIFY(typeChangedSpy.isValid());

    curve->setResultType(-1);
    QCOMPARE(typeChangedSpy.count(), 0);

    curve->setResultType(504);
    QCOMPARE(curve->resultType(), 504);
    QCOMPARE(typeChangedSpy.count(), 1);

    curve->setResultType(504);
    QCOMPARE(typeChangedSpy.count(), 1);
}

void TestReferenceCurve::testMeasureTask()
{
    auto product = new Product(QUuid::createUuid(), this);
    auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    auto seam = new Seam(QUuid::createUuid(), seamSeries);

    auto curve = new ReferenceCurve{this};
    QVERIFY(!curve->measureTask());

    QSignalSpy taskChangedSpy(curve, &ReferenceCurve::measureTaskChanged);
    QVERIFY(taskChangedSpy.isValid());

    curve->setMeasureTask(nullptr);
    QCOMPARE(taskChangedSpy.count(), 0);

    curve->setMeasureTask(seam);
    QCOMPARE(curve->measureTask(), seam);
    QCOMPARE(taskChangedSpy.count(), 1);

    curve->setMeasureTask(seam);
    QCOMPARE(taskChangedSpy.count(), 1);

    const auto seam2 = new Seam(QUuid::createUuid(), seamSeries);

    curve->setMeasureTask(seam2);
    QCOMPARE(curve->measureTask(), seam2);
    QCOMPARE(taskChangedSpy.count(), 2);
}

void TestReferenceCurve::testReferenceType()
{
    auto curve = new ReferenceCurve{this};
    QCOMPARE(curve->referenceType(), ReferenceCurve::ReferenceType::Average);
    QCOMPARE(curve->referenceTypeString(), QStringLiteral("Average"));

    QSignalSpy referenceTypeChangedSpy(curve, &ReferenceCurve::referenceTypeChanged);
    QVERIFY(referenceTypeChangedSpy.isValid());

    curve->setReferenceType(ReferenceCurve::ReferenceType::Average);
    QCOMPARE(referenceTypeChangedSpy.count(), 0);

    curve->setReferenceType(ReferenceCurve::ReferenceType::Median);
    QCOMPARE(curve->referenceType(), ReferenceCurve::ReferenceType::Median);
    QCOMPARE(curve->referenceTypeString(), QStringLiteral("Median"));
    QCOMPARE(referenceTypeChangedSpy.count(), 1);

    curve->setReferenceType(ReferenceCurve::ReferenceType::Median);
    QCOMPARE(referenceTypeChangedSpy.count(), 1);

    curve->setReferenceType(ReferenceCurve::ReferenceType::MinMax);
    QCOMPARE(curve->referenceType(), ReferenceCurve::ReferenceType::MinMax);
    QCOMPARE(curve->referenceTypeString(), QStringLiteral("Minimum and Maximum"));
    QCOMPARE(referenceTypeChangedSpy.count(), 2);
}

void TestReferenceCurve::testJitter()
{
    auto curve = new ReferenceCurve{this};
    QCOMPARE(curve->jitter(), 0.0f);

    QSignalSpy jitterChangedSpy(curve, &ReferenceCurve::jitterChanged);
    QVERIFY(jitterChangedSpy.isValid());

    curve->setJitter(0.0f);
    QCOMPARE(jitterChangedSpy.count(), 0);

    curve->setJitter(1.0f);
    QCOMPARE(curve->jitter(), 1.0f);
    QCOMPARE(jitterChangedSpy.count(), 1);

    curve->setJitter(1.0f);
    QCOMPARE(jitterChangedSpy.count(), 1);
}

void TestReferenceCurve::testDuplicate_data()
{
    QTest::addColumn<bool>("changeUuid");

    QTest::newRow("duplicate and change UUID") << true;
    QTest::newRow("duplicate and keep UUID") << false;
}

void TestReferenceCurve::testDuplicate()
{
    auto product = new Product(QUuid::createUuid(), this);
    auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    auto seam = new Seam(QUuid::createUuid(), seamSeries);

    auto curve = new ReferenceCurve{this};

    curve->setMeasureTask(seam);
    curve->setResultType(105);
    curve->setName(QStringLiteral("MyName"));
    curve->setJitter(1.5);
    curve->setReferenceType(ReferenceCurve::ReferenceType::MinMax);
    curve->setSourceOfCurve({QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")});

    QFETCH(bool, changeUuid);
    const auto copyMode = changeUuid ? CopyMode::WithDifferentIds : CopyMode::Identical;
    auto newCurve = curve->duplicate(copyMode, seam);

    QCOMPARE(curve->uuid() != newCurve->uuid(), changeUuid);
    QCOMPARE(curve->upper() != newCurve->upper(), changeUuid);
    QCOMPARE(curve->middle() != newCurve->middle(), changeUuid);
    QCOMPARE(curve->lower() != newCurve->lower(), changeUuid);
    QCOMPARE(curve->measureTask(), newCurve->measureTask());
    QCOMPARE(curve->resultType(), newCurve->resultType());
    QCOMPARE(curve->name(), newCurve->name());
    QCOMPARE(curve->jitter(), newCurve->jitter());
    QCOMPARE(curve->referenceType(), newCurve->referenceType());
    QVERIFY(newCurve->m_sourceOfCurve.empty());

    const auto upperData = std::vector<QVector2D>{{0, 0} ,{1, 1}, {2, 2}};
    const auto middleData = std::vector<QVector2D>{{3, 3} ,{4, 4}, {5, 5}};
    const auto lowerData = std::vector<QVector2D>{{6, 6} ,{7, 7}, {8, 8}};

    product->setReferenceCurveData(curve->upper(), upperData);
    product->setReferenceCurveData(curve->middle(), middleData);
    product->setReferenceCurveData(curve->lower(), lowerData);

    auto seam2 = new Seam(QUuid::createUuid(), seamSeries);

    auto seamCurve = seam2->copyReferenceCurve(curve);

    QVERIFY(curve->uuid() != seamCurve->uuid());
    QVERIFY(curve->upper() != seamCurve->upper());
    QVERIFY(curve->middle() != seamCurve->middle());
    QVERIFY(curve->lower() != seamCurve->lower());
    QCOMPARE(seamCurve->measureTask(), seam2);
    QCOMPARE(curve->resultType(), seamCurve->resultType());
    QCOMPARE(curve->name(), seamCurve->name());
    QCOMPARE(curve->jitter(), seamCurve->jitter());
    QCOMPARE(curve->referenceType(), seamCurve->referenceType());
    QVERIFY(seamCurve->m_sourceOfCurve.empty());

    QVERIFY(product->referenceCurveData(seamCurve->upper()));
    QCOMPARE(product->referenceCurveData(seamCurve->upper())->samples(), upperData);
    QVERIFY(product->referenceCurveData(seamCurve->middle()));
    QCOMPARE(product->referenceCurveData(seamCurve->middle())->samples(), middleData);
    QVERIFY(product->referenceCurveData(seamCurve->lower()));
    QCOMPARE(product->referenceCurveData(seamCurve->lower())->samples(), lowerData);
}

void TestReferenceCurve::testJson()
{
    auto product = new Product(QUuid::createUuid(), this);
    auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    auto seam = new Seam(QUuid::createUuid(), seamSeries);

    auto curve = new ReferenceCurve{this};

    curve->setMeasureTask(seam);
    curve->setResultType(105);
    curve->setName(QStringLiteral("MyName"));
    curve->setJitter(1.5);
    curve->setReferenceType(ReferenceCurve::ReferenceType::MinMax);
    curve->setSourceOfCurve({QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")});

    const auto json = curve->toJson();

    auto newCurve = ReferenceCurve::fromJson(json, seam);

    QCOMPARE(curve->uuid(), newCurve->uuid());
    QCOMPARE(curve->upper(), newCurve->upper());
    QCOMPARE(curve->middle(), newCurve->middle());
    QCOMPARE(curve->lower(), newCurve->lower());
    QCOMPARE(curve->measureTask(), newCurve->measureTask());
    QCOMPARE(curve->resultType(), newCurve->resultType());
    QCOMPARE(curve->name(), newCurve->name());
    QCOMPARE(curve->jitter(), newCurve->jitter());
    QCOMPARE(curve->referenceType(), newCurve->referenceType());
    QVERIFY(newCurve->m_sourceOfCurve.empty());
}

void TestReferenceCurve::testSource()
{
    auto curve = new ReferenceCurve{this};
    QVERIFY(curve->m_sourceOfCurve.empty());

    const auto source = std::vector<QString>{QStringLiteral("1"), QStringLiteral("2"), QStringLiteral("3")};

    curve->setSourceOfCurve(source);
    QCOMPARE(curve->sourceOfCurve(), source);

    QVERIFY(!curve->isSource(QStringLiteral("0")));
    QVERIFY(curve->isSource(QStringLiteral("1")));
    QVERIFY(curve->isSource(QStringLiteral("2")));
    QVERIFY(curve->isSource(QStringLiteral("3")));
}

void TestReferenceCurve::testUsed()
{
    auto curve = new ReferenceCurve{this};
    QVERIFY(!curve->used());

    QSignalSpy usedChangedSpy(curve, &ReferenceCurve::usedChanged);
    QVERIFY(usedChangedSpy.isValid());

    curve->subscribe({});
    QCOMPARE(usedChangedSpy.count(), 0);

    auto id1 = QUuid::createUuid();
    std::vector<QUuid> subscibtions = {id1};

    curve->subscribe(id1);
    QVERIFY(curve->used());
    QCOMPARE(curve->m_subscriptions, subscibtions);
    QCOMPARE(usedChangedSpy.count(), 1);

    curve->subscribe(id1);
    QCOMPARE(usedChangedSpy.count(), 1);

    auto id2 = QUuid::createUuid();
    subscibtions = {id1, id2};

    curve->subscribe(id2);
    QVERIFY(curve->used());
    QCOMPARE(curve->m_subscriptions, subscibtions);
    QCOMPARE(usedChangedSpy.count(), 2);

    curve->unsubscribe({});
    QCOMPARE(usedChangedSpy.count(), 2);

    curve->unsubscribe(QUuid::createUuid());
    QCOMPARE(usedChangedSpy.count(), 2);

    subscibtions = {id2};

    curve->unsubscribe(id1);
    QVERIFY(curve->used());
    QCOMPARE(curve->m_subscriptions, subscibtions);
    QCOMPARE(usedChangedSpy.count(), 3);

    curve->unsubscribe(id1);
    QCOMPARE(usedChangedSpy.count(), 3);

    curve->unsubscribe(id2);
    QVERIFY(!curve->used());
    QCOMPARE(curve->m_subscriptions, {});
    QCOMPARE(usedChangedSpy.count(), 4);
}

QTEST_GUILESS_MAIN(TestReferenceCurve)
#include "testReferenceCurve.moc"
