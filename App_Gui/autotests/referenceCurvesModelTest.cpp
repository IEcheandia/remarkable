#include <QTest>
#include <QSignalSpy>

#include "../src/referenceCurvesModel.h"
#include "referenceCurve.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"

using precitec::gui::ReferenceCurvesModel;
using precitec::storage::ReferenceCurve;
using precitec::storage::Product;
using precitec::storage::SeamSeries;
using precitec::storage::Seam;

class ReferenceCurvesModelTest: public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testSetCurrentSeam();
    void testCreateReferenceCurve();
    void testCopyFromExistingReferenceCurve();
    void testRemoveReferenceCurve();
};

void ReferenceCurvesModelTest::testCtor()
{
    ReferenceCurvesModel model{this};
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(!model.currentSeam());
}

void ReferenceCurvesModelTest::testRoleNames()
{
    ReferenceCurvesModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 3);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("curve"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("id"));
}

void ReferenceCurvesModelTest::testSetCurrentSeam()
{
    ReferenceCurvesModel model{this};
    QVERIFY(!model.currentSeam());

    QSignalSpy seamChangedSpy{&model, &ReferenceCurvesModel::currentSeamChanged};
    QVERIFY(seamChangedSpy.isValid());

    model.setCurrentSeam(nullptr);
    QVERIFY(!model.currentSeam());
    QCOMPARE(seamChangedSpy.count(), 0);

    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    const auto seam = new Seam(QUuid::createUuid(), seamSeries);
    model.setCurrentSeam(seam);
    QVERIFY(model.currentSeam());
    QCOMPARE(seam, model.currentSeam());
    QCOMPARE(seam->uuid(), model.currentSeam()->uuid());
    QCOMPARE(seamChangedSpy.count(), 1);
}

void ReferenceCurvesModelTest::testCreateReferenceCurve()
{
    ReferenceCurvesModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    const auto seam = new Seam(QUuid::createUuid(), seamSeries);
    model.setCurrentSeam(seam);

    const auto curve = model.createReferenceCurve(103);
    curve->setName(QStringLiteral("myFirstCurve"));

    QVERIFY(curve);
    QVERIFY(curve->measureTask());
    QCOMPARE(curve->measureTask(), seam);
    QCOMPARE(curve->resultType(), 103);
    QCOMPARE(seam->referenceCurves().size(), 1);
    QCOMPARE(model.rowCount(), 1);

    const auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(index.data(Qt::DisplayRole).toString(), curve->name());
    QCOMPARE(index.data(Qt::UserRole).value<ReferenceCurve*>(), curve);
    QCOMPARE(index.data(Qt::UserRole + 1).toUuid(), curve->uuid());

    const auto curve_2 = model.createReferenceCurve(301);
    curve_2->setName(QStringLiteral("mySecondCurve"));

    QVERIFY(curve_2);
    QVERIFY(curve_2->measureTask());
    QCOMPARE(curve_2->measureTask(), seam);
    QCOMPARE(curve_2->resultType(), 301);
    QCOMPARE(seam->referenceCurves().size(), 2);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(index.data(Qt::DisplayRole).toString(), curve->name());
    QCOMPARE(index.data(Qt::UserRole).value<ReferenceCurve*>(), curve);
    QCOMPARE(index.data(Qt::UserRole + 1).toUuid(), curve->uuid());

    const auto index2 = model.index(1, 0);
    QVERIFY(index2.isValid());
    QCOMPARE(index2.data(Qt::DisplayRole).toString(), curve_2->name());
    QCOMPARE(index2.data(Qt::UserRole).value<ReferenceCurve*>(), curve_2);
    QCOMPARE(index2.data(Qt::UserRole + 1).toUuid(), curve_2->uuid());
}

void ReferenceCurvesModelTest::testCopyFromExistingReferenceCurve()
{
    ReferenceCurvesModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    const auto seam = new Seam(QUuid::createUuid(), seamSeries);
    model.setCurrentSeam(seam);

    const auto curve = model.createReferenceCurve(103);
    curve->setName(QStringLiteral("myFirstCurve"));

    QVERIFY(curve);
    QVERIFY(curve->measureTask());
    QCOMPARE(curve->measureTask(), seam);
    QCOMPARE(curve->resultType(), 103);
    QCOMPARE(seam->referenceCurves().size(), 1);
    QCOMPARE(model.rowCount(), 1);

    const auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(index.data(Qt::DisplayRole).toString(), curve->name());
    QCOMPARE(index.data(Qt::UserRole).value<ReferenceCurve*>(), curve);
    QCOMPARE(index.data(Qt::UserRole + 1).toUuid(), curve->uuid());

    const auto curve_2 = model.copyReferenceCurve(curve);

    QVERIFY(curve_2);
    QVERIFY(curve_2->measureTask());
    QCOMPARE(curve_2->measureTask(), curve->measureTask());
    QCOMPARE(curve_2->resultType(), curve->resultType());
    QCOMPARE(seam->referenceCurves().size(), 2);
    QCOMPARE(model.rowCount(), 2);

    const auto seam2 = new Seam(QUuid::createUuid(), seamSeries);
    model.setCurrentSeam(seam2);

    const auto curve_3 = model.copyReferenceCurve(curve);

    QVERIFY(curve_3);
    QVERIFY(curve_3->measureTask());
    QCOMPARE(curve_3->measureTask(), seam2);
    QCOMPARE(curve_3->resultType(), curve->resultType());
    QCOMPARE(seam2->referenceCurves().size(), 1);
    QCOMPARE(model.rowCount(), 1);
}

void ReferenceCurvesModelTest::testRemoveReferenceCurve()
{
    ReferenceCurvesModel model{this};
    const auto product = new Product(QUuid::createUuid(), this);
    const auto seamSeries = new SeamSeries(QUuid::createUuid(), product);
    const auto seam = new Seam(QUuid::createUuid(), seamSeries);
    model.setCurrentSeam(seam);

    const auto curve = model.createReferenceCurve(103);
    curve->setName(QStringLiteral("myFirstCurve"));

    QVERIFY(curve);
    QVERIFY(curve->measureTask());
    QCOMPARE(curve->measureTask(), seam);
    QCOMPARE(curve->resultType(), 103);
    QCOMPARE(seam->referenceCurves().size(), 1);
    QCOMPARE(model.rowCount(), 1);

    const auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(index.data(Qt::DisplayRole).toString(), curve->name());
    QCOMPARE(index.data(Qt::UserRole).value<ReferenceCurve*>(), curve);
    QCOMPARE(index.data(Qt::UserRole + 1).toUuid(), curve->uuid());

    const auto curve_2 = model.createReferenceCurve(301);
    curve->setName(QStringLiteral("mySecondCurve"));

    QVERIFY(curve_2);
    QVERIFY(curve_2->measureTask());
    QCOMPARE(curve_2->measureTask(), seam);
    QCOMPARE(curve_2->resultType(), 301);
    QCOMPARE(seam->referenceCurves().size(), 2);
    QCOMPARE(model.rowCount(), 2);

    QCOMPARE(index.data(Qt::DisplayRole).toString(), curve->name());
    QCOMPARE(index.data(Qt::UserRole).value<ReferenceCurve*>(), curve);
    QCOMPARE(index.data(Qt::UserRole + 1).toUuid(), curve->uuid());

    const auto index2 = model.index(1, 0);
    QVERIFY(index2.isValid());
    QCOMPARE(index2.data(Qt::DisplayRole).toString(), curve_2->name());
    QCOMPARE(index2.data(Qt::UserRole).value<ReferenceCurve*>(), curve_2);
    QCOMPARE(index2.data(Qt::UserRole + 1).toUuid(), curve_2->uuid());

    model.removeReferenceCurve(curve);
    QCOMPARE(seam->referenceCurves().size(), 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(index.data(Qt::DisplayRole).toString(), curve_2->name());
    QCOMPARE(index.data(Qt::UserRole).value<ReferenceCurve*>(), curve_2);
    QCOMPARE(index.data(Qt::UserRole + 1).toUuid(), curve_2->uuid());

    model.removeReferenceCurve(curve_2);
    QVERIFY(seam->referenceCurves().empty());
    QCOMPARE(model.rowCount(), 0);
}

QTEST_GUILESS_MAIN(ReferenceCurvesModelTest)
#include "referenceCurvesModelTest.moc"
