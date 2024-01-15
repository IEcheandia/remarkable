#include <QTest>
#include <QSignalSpy>

#include "../src/assemblyImageInspectionModel.h"
#include "product.h"
#include "resultsServer.h"
#include "seam.h"
#include "seamSeries.h"

using precitec::gui::AssemblyImageInspectionModel;
using precitec::storage::Product;
using precitec::storage::ResultsServer;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

class AssemblyImageInspectionModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSetResultsServer();
    void testInspection();
};

void AssemblyImageInspectionModelTest::testCtor()
{
    AssemblyImageInspectionModel model;
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(model.product() == nullptr);
    QVERIFY(model.resultsServer() == nullptr);
}

void AssemblyImageInspectionModelTest::testSetResultsServer()
{
    AssemblyImageInspectionModel model;
    QSignalSpy resultsServerChangedSpy{&model, &AssemblyImageInspectionModel::resultsServerChanged};
    QVERIFY(resultsServerChangedSpy.isValid());

    ResultsServer *server = new ResultsServer(this);
    model.setResultsServer(server);
    QCOMPARE(resultsServerChangedSpy.count(), 1);
    QCOMPARE(model.resultsServer(), server);

    // setting same should not change
    model.setResultsServer(server);
    QCOMPARE(resultsServerChangedSpy.count(), 1);

    // and delete
    server->deleteLater();
    QVERIFY(resultsServerChangedSpy.wait());
    QVERIFY(model.resultsServer() == nullptr);
}

void AssemblyImageInspectionModelTest::testInspection()
{
    AssemblyImageInspectionModel model;
    ResultsServer server;
    model.setResultsServer(&server);

    // create a Product
    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();
    auto seam2 = p.createSeam();
    auto seam3 = p.createSeam();

    seam1->setName(QStringLiteral("foo"));
    seam2->setName(QStringLiteral("bar"));
    seam3->setName(QStringLiteral("fooBar"));

    seam1->setPositionInAssemblyImage({2, 3});

    QSignalSpy productChangedSpy{&model, &AssemblyImageInspectionModel::productChanged};
    QVERIFY(productChangedSpy.isValid());
    emit server.productInspectionStarted(&p, QUuid::createUuid(), {});
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(model.product(), &p);
    QCOMPARE(model.rowCount(), 3);

    QCOMPARE(model.index(0, 0).data().toString(), QStringLiteral("foo"));
    QCOMPARE(model.index(1, 0).data().toString(), QStringLiteral("bar"));
    QCOMPARE(model.index(2, 0).data().toString(), QStringLiteral("fooBar"));

    QCOMPARE(model.index(0, 0).data(Qt::UserRole).toPointF(), QPointF(2, 3));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole).toPointF(), QPointF(-1, -1));
    QCOMPARE(model.index(2, 0).data(Qt::UserRole).toPointF(), QPointF(-1, -1));

    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 1).toBool(), false);

    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Uninspected);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Uninspected);
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Uninspected);

    QSignalSpy dataChangedSpy{&model, &AssemblyImageInspectionModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    emit server.seamInspectionStarted(seam2, QUuid::createUuid(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().count(), 1);
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().at(0), int(Qt::UserRole + 1));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toBool(), true);

    // let's end the inspection
    emit server.seamInspectionEnded();
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(1, 0));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().count(), 2);
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().at(0), int(Qt::UserRole + 1));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().at(1), int(Qt::UserRole + 2));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Success);

    // let's start a new product
    emit server.productInspectionStarted(&p, QUuid::createUuid(), {});
    QCOMPARE(dataChangedSpy.count(), 3);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(2, 0));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().count(), 1);
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>().at(0), int(Qt::UserRole + 2));
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Uninspected);

    // send a start
    emit server.seamInspectionStarted(seam3, QUuid::createUuid(), 1);
    QCOMPARE(dataChangedSpy.count(), 4);
    // send an nio
    emit server.nioReceived(precitec::interface::ResultArgs{});
    QCOMPARE(dataChangedSpy.count(), 5);
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 2).value<AssemblyImageInspectionModel::SeamState>(), AssemblyImageInspectionModel::SeamState::Failure);
}

QTEST_GUILESS_MAIN(AssemblyImageInspectionModelTest)
#include "assemblyImageInspectionModelTest.moc"
