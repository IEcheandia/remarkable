#include <QTest>
#include <QSignalSpy>

#include "../src/latestProductErrorsModel.h"
#include "resultsServer.h"
#include "errorSettingModel.h"
#include "product.h"

using precitec::gui::LatestProductErrorsModel;
using precitec::storage::ResultsServer;
using precitec::storage::ErrorSettingModel;
using precitec::storage::Product;
using precitec::interface::ResultArgs;
using precitec::interface::ImageContext;

class LatestProductErrorsModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testSetResultsServer();
    void testSetErrorConfigModel();
    void testUpdateSeamNumber();
    void testAddToQueue();
    void testUpdate();
    void testClear();
};

void LatestProductErrorsModelTest::testCtor()
{
    LatestProductErrorsModel model{this};
    QVERIFY(!model.resultsServer());
    QVERIFY(!model.errorConfigModel());
    QVERIFY(!model.liveUpdate());
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.roleNames().size(), 4);
    QCOMPARE(model.m_currentSeamNumber, -1);
    QVERIFY(model.m_queue.empty());
    QVERIFY(model.m_errors.empty());
}

void LatestProductErrorsModelTest::testRoleNames()
{
    LatestProductErrorsModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("seam"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("name"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("color"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("position"));
}

void LatestProductErrorsModelTest::testSetResultsServer()
{
    LatestProductErrorsModel model{this};
    QSignalSpy resultsServerChangedSpy{&model, &LatestProductErrorsModel::resultsServerChanged};
    QVERIFY(resultsServerChangedSpy.isValid());

    ResultsServer *server = new ResultsServer(this);
    model.setResultsServer(server);
    QCOMPARE(resultsServerChangedSpy.count(), 1);
    QCOMPARE(model.resultsServer(), server);

    model.setResultsServer(server);
    QCOMPARE(resultsServerChangedSpy.count(), 1);

    server->deleteLater();
    QVERIFY(resultsServerChangedSpy.wait());
    QVERIFY(!model.resultsServer());
}

void LatestProductErrorsModelTest::testSetErrorConfigModel()
{
    LatestProductErrorsModel model{this};
    QSignalSpy errorConfigChangedSpy{&model, &LatestProductErrorsModel::errorConfigModelChanged};
    QVERIFY(errorConfigChangedSpy.isValid());

    ErrorSettingModel *errorConfig = new ErrorSettingModel{this};
    model.setErrorConfigModel(errorConfig);
    QCOMPARE(errorConfigChangedSpy.count(), 1);
    QCOMPARE(model.errorConfigModel(), errorConfig);

    model.setErrorConfigModel(errorConfig);
    QCOMPARE(errorConfigChangedSpy.count(), 1);

    errorConfig->deleteLater();
    QVERIFY(errorConfigChangedSpy.wait());
    QVERIFY(!model.errorConfigModel());
}

void LatestProductErrorsModelTest::testUpdateSeamNumber()
{
    LatestProductErrorsModel model{this};
    ResultsServer server{this};
    model.setResultsServer(&server);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();
    auto seam2 = p.createSeam();
    auto seam3 = p.createSeam();

    QSignalSpy seamNumberChangedSpy{&model, &LatestProductErrorsModel::seamNumberChanged};
    QVERIFY(seamNumberChangedSpy.isValid());

    QCOMPARE(model.m_currentSeamNumber, -1);

    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 0u);
    QCOMPARE(seamNumberChangedSpy.count(), 1);
    QCOMPARE(model.m_currentSeamNumber, 1);

    emit server.seamInspectionStarted(seam3, QUuid::createUuid(), 0u);
    QCOMPARE(seamNumberChangedSpy.count(), 2);
    QCOMPARE(model.m_currentSeamNumber, 3);

    emit server.seamInspectionStarted(seam2, QUuid::createUuid(), 0u);
    QCOMPARE(seamNumberChangedSpy.count(), 3);
    QCOMPARE(model.m_currentSeamNumber, 2);
}

void LatestProductErrorsModelTest::testAddToQueue()
{
    LatestProductErrorsModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();
    auto seam2 = p.createSeam();
    auto seam3 = p.createSeam();

    ResultArgs arg1;
    arg1.setResultType(precitec::interface::XCoordOutOfLimits);
    arg1.setNio(true);
    ImageContext context1;
    context1.setPosition(int(0.25 * 1000.0));
    arg1.setContext(context1);

    emit server.nioReceived(arg1);
    QVERIFY(model.m_queue.empty());

    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg1);
    QCOMPARE(model.m_queue.size(), 1u);

    ResultArgs arg2;
    arg2.setNio(false);
    arg2.setResultType(precitec::interface::GapPositionError);
    ImageContext context2;
    context2.setPosition(int(5 * 1000.0));
    arg2.setContext(context2);

    emit server.seamInspectionStarted(seam2, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg2);
    QCOMPARE(model.m_queue.size(), 1u);

    ResultArgs arg3;
    arg3.setResultType(precitec::interface::GapPositionError);
    arg3.setContext(context2);
    arg3.setNio(true);

    emit server.seamInspectionStarted(seam3, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg3);
    QCOMPARE(model.m_queue.size(), 2u);

    auto it = model.m_queue.begin();

    QCOMPARE((*it).first, 1);
    QCOMPARE((*it).second.isNio(), true);
    QCOMPARE((*it).second.resultType(), precitec::interface::XCoordOutOfLimits);
    QCOMPARE((*it).second.context().position() / 1000.0, 0.25);

    it++;

    QCOMPARE((*it).first, 3);
    QCOMPARE((*it).second.isNio(), true);
    QCOMPARE((*it).second.resultType(), precitec::interface::GapPositionError);
    QCOMPARE((*it).second.context().position() / 1000.0, 5);
}

void LatestProductErrorsModelTest::testUpdate()
{
    LatestProductErrorsModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();
    auto seam2 = p.createSeam();

    ResultArgs arg1;
    arg1.setResultType(precitec::interface::XCoordOutOfLimits);
    ImageContext context1;
    context1.setPosition(int(0.25 * 1000.0));
    arg1.setContext(context1);
    arg1.setNio(true);

    emit server.nioReceived(arg1);
    QVERIFY(model.m_queue.empty());

    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg1);
    QCOMPARE(model.m_queue.size(), 1u);

    ResultArgs arg2;
    arg2.setResultType(precitec::interface::GapPositionError);
    ImageContext context2;
    context2.setPosition(int(5 * 1000.0));
    arg2.setContext(context2);
    arg2.setNio(true);

    emit server.seamInspectionStarted(seam2, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg2);
    QCOMPARE(model.m_queue.size(), 2u);

    model.update();
    QCOMPARE(model.m_errors.size(), 2u);
    QCOMPARE(model.m_queue.size(), 0u);

    const auto row0 = model.index(0, 0);
    QCOMPARE(row0.data(Qt::DisplayRole).toString(), QStringLiteral("Seam 1"));
    QCOMPARE(row0.data(Qt::UserRole).toString(), QStringLiteral("X position out of limits"));
    QCOMPARE(row0.data(Qt::UserRole + 1).value<QColor>(), QColor(Qt::red));
    QCOMPARE(row0.data(Qt::UserRole + 2).toDouble(), 0.25);

    const auto row1 = model.index(1, 0);
    QCOMPARE(row1.data(Qt::DisplayRole).toString(), QStringLiteral("Seam 2"));
    QCOMPARE(row1.data(Qt::UserRole).toString(), QStringLiteral("GapPositionError"));
    QCOMPARE(row1.data(Qt::UserRole + 1).value<QColor>(), QColor(Qt::red));
    QCOMPARE(row1.data(Qt::UserRole + 2).toDouble(), 5.0);

    ErrorSettingModel errorConfig{this};
    model.setErrorConfigModel(&errorConfig);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/errors_data/errorsconfig.json"), dir.filePath(QStringLiteral("errorsconfig.json"))));
    QString tempErrorStorageFile = dir.filePath(QStringLiteral("errorsconfig.json"));
    QFINDTESTDATA(tempErrorStorageFile);
    errorConfig.m_errorStorageFile = tempErrorStorageFile;
    errorConfig.loadErrors();
    QCOMPARE(errorConfig.rowCount(), 15);

    model.clear();

    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg1);
    emit server.seamInspectionStarted(seam2, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg2);

    model.update();

    QCOMPARE(row0.data(Qt::DisplayRole).toString(), QStringLiteral("Seam 1"));
    QCOMPARE(row0.data(Qt::UserRole).toString(), QStringLiteral("X position out of limits"));
    QCOMPARE(row0.data(Qt::UserRole + 1).value<QColor>(), QColor("#FF0000"));
    QCOMPARE(row0.data(Qt::UserRole + 2).toDouble(), 0.25);

    QCOMPARE(row1.data(Qt::DisplayRole).toString(), QStringLiteral("Seam 2"));
    QCOMPARE(row1.data(Qt::UserRole).toString(), QStringLiteral("GapPositionError"));
    QCOMPARE(row1.data(Qt::UserRole + 1).value<QColor>(), QColor("#FF0000"));
    QCOMPARE(row1.data(Qt::UserRole + 2).toDouble(), 5.0);
}

void LatestProductErrorsModelTest::testClear()
{
    LatestProductErrorsModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();
    auto seam2 = p.createSeam();

    ResultArgs arg1;
    arg1.setResultType(precitec::interface::XCoordOutOfLimits);
    ImageContext context1;
    context1.setPosition(int(0.25 * 1000.0));
    arg1.setContext(context1);
    arg1.setNio(true);

    emit server.nioReceived(arg1);
    QVERIFY(model.m_queue.empty());

    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg1);
    QCOMPARE(model.m_queue.size(), 1u);

    model.update();
    QCOMPARE(model.m_errors.size(), 1u);
    QCOMPARE(model.m_queue.size(), 0u);

    ResultArgs arg2;
    arg2.setResultType(precitec::interface::GapPositionError);
    ImageContext context2;
    context2.setPosition(int(5 * 1000.0));
    arg2.setContext(context2);
    arg2.setNio(true);

    emit server.seamInspectionStarted(seam2, QUuid::createUuid(), 0u);
    emit server.nioReceived(arg2);
    QCOMPARE(model.m_errors.size(), 1u);
    QCOMPARE(model.m_queue.size(), 1u);

    model.clear();

    QCOMPARE(model.m_errors.size(), 0u);
    QCOMPARE(model.m_queue.size(), 0u);
}

QTEST_GUILESS_MAIN(LatestProductErrorsModelTest)
#include "latestProductErrorsModelTest.moc"

