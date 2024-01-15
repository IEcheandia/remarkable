#include <QTest>
#include <QSignalSpy>

#include "../src/seamsOnAssemblyImageModel.h"
#include "../src/seamsOnAssemblyImageFilterModel.h"

#include "product.h"
#include "seam.h"
#include "seamSeries.h"

using precitec::gui::SeamsOnAssemblyImageModel;
using precitec::gui::SeamsOnAssemblyImageFilterModel;
using precitec::storage::Product;
using precitec::storage::Seam;

class SeamsOnAssemblyImageModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetSeam();
    void testMultipleSeams();
};

void SeamsOnAssemblyImageModelTest::testCtor()
{
    SeamsOnAssemblyImageModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.seam() == nullptr);

    QSignalSpy changedSpy{&model, &SeamsOnAssemblyImageModel::markAsChanged};
    QVERIFY(changedSpy.isValid());
    model.setSeamPosition(1, 2);
    QCOMPARE(changedSpy.count(), 0);
}

void SeamsOnAssemblyImageModelTest::testSetSeam()
{
    SeamsOnAssemblyImageModel model;
    QSignalSpy seamChangedSpy{&model, &SeamsOnAssemblyImageModel::seamChanged};
    QVERIFY(seamChangedSpy.isValid());
    QSignalSpy modelResetSpy{&model, &SeamsOnAssemblyImageModel::modelReset};
    QVERIFY(modelResetSpy.isValid());

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    seam->setName(QStringLiteral("foo"));

    model.setSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    // setting same seam should not change
    model.setSeam(seam);
    QCOMPARE(seamChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.seam(), seam);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("foo"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toBool(), true);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toBool(), false);

    // now delete the seam
    QSignalSpy seamDestroyedSpy{seam, &QObject::destroyed};
    QVERIFY(seamDestroyedSpy.isValid());
    p.seamSeries().front()->destroySeam(seam);
    QCOMPARE(modelResetSpy.count(), 2);
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(seamDestroyedSpy.wait());
    QVERIFY(model.seam() == nullptr);
}

void SeamsOnAssemblyImageModelTest::testMultipleSeams()
{
    SeamsOnAssemblyImageModel model;

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam = p.createSeam();
    seam->setName(QStringLiteral("foo"));
    seam->setPositionInAssemblyImage({1, 2});

    auto seam2 = p.createSeam();
    seam2->setName(QStringLiteral("bar"));
    seam2->setPositionInAssemblyImage({3, 4});

    auto seam3 = seam2->seamSeries()->createSeamLink(seam2, QStringLiteral("3"));
    seam3->setName(QStringLiteral("fooBar"));
    seam3->setPositionInAssemblyImage({5, 6});

    model.setSeam(seam2);

    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toString(), QStringLiteral("foo"));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toPointF(), QPointF(1, 2));
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toBool(), false);

    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toString(), QStringLiteral("bar"));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole).toPointF(), QPointF(3, 4));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toBool(), true);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 2).toBool(), false);

    QCOMPARE(model.index(2, 0).data(Qt::DisplayRole).toString(), QStringLiteral("fooBar"));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole).toPointF(), QPointF(5, 6));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 2).toBool(), true);

    // test change
    QSignalSpy hasChangesChangedSpy{&model, &SeamsOnAssemblyImageModel::markAsChanged};
    QVERIFY(hasChangesChangedSpy.isValid());
    QSignalSpy dataChangedSpy{&model, &SeamsOnAssemblyImageModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    model.setSeamPosition(6, 7);
    QCOMPARE(hasChangesChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole).toPointF(), QPointF(6, 7));

    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>().count(), 1);
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>().at(0), int(Qt::UserRole));

    // let's test the filter model
    SeamsOnAssemblyImageFilterModel filterModel;
    filterModel.setSourceModel(&model);
    // everything should be forwarded
    QCOMPARE(filterModel.rowCount(), 3);
    // let's set the seam to -1, -1
    model.setSeamPosition(-1, -1);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(filterModel.rowCount(), 2);
}

QTEST_GUILESS_MAIN(SeamsOnAssemblyImageModelTest)
#include "seamsOnAssemblyImageModelTest.moc"
