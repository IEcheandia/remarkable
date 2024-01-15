#include <QTest>
#include <QSignalSpy>

#include "../src/historyModel.h"
#include "resultsServer.h"
#include "product.h"
#include "extendedProductInfoHelper.h"

using precitec::gui::HistoryModel;
using precitec::interface::ImageContext;
using precitec::interface::ResultArgs;
using precitec::storage::Product;
using precitec::storage::ResultsServer;
class HistoryModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testSetResultsServer();
    void testSetMax();
    void testHistory();
    void testMaxHistory();
    void testAbstractListModelFunctionality();
    void testExtendedProductInfo_data();
    void testExtendedProductInfo();
    void testStopProductInspectionWithoutResults();
};

void HistoryModelTest::testCtor()
{
    HistoryModel model{this};
    QVERIFY(!model.resultsServer());
    QCOMPARE(model.max(), 100);
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.roleNames().size(), 5);
    QVERIFY(model.m_history.empty());
}

void HistoryModelTest::testRoleNames()
{
    HistoryModel model{this};
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 5);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("serialNumber"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("productName"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("partNumber"));
}

void HistoryModelTest::testSetResultsServer()
{
    HistoryModel model{this};
    QSignalSpy resultsServerChangedSpy{&model, &HistoryModel::resultsServerChanged};
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

void HistoryModelTest::testSetMax()
{
    HistoryModel model{this};
    QSignalSpy maxChangedSpy{&model, &HistoryModel::maxChanged};
    QVERIFY(maxChangedSpy.isValid());

    QCOMPARE(model.max(), 100);
    const auto max = 11;
    model.setMax(max);
    QCOMPARE(maxChangedSpy.count(), 1);
    QCOMPARE(model.max(), 11);
}

void HistoryModelTest::testHistory()
{
    HistoryModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);

    Product p{QUuid::createUuid()};
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();
    auto seam2 = p.createSeam();
    auto seam3 = p.createSeam();

    ResultArgs arg1;
    arg1.setResultType(precitec::interface::XCoordOutOfLimits);
    ImageContext context1;
    context1.setPosition(int(5 * 1000.0));
    arg1.setContext(context1);
    arg1.setNio(true);
    const auto uuidProductInstance = QUuid::createUuid();
    emit server.productInspectionStarted(&p, QUuid::createUuid(), {});
    emit server.seamInspectionStarted(seam1, uuidProductInstance, 0u);
    emit server.nioReceived(arg1);

    emit server.nioReceived(arg1);

    ResultArgs arg2;
    arg2.setResultType(precitec::interface::GapPositionError);
    ImageContext context2;
    context2.setPosition(int(5 * 1000.0));
    arg2.setContext(context2);
    arg2.setNio(true);

    emit server.nioReceived(arg2);

    precitec::interface::GeoDoublearray values;
    values.ref().getData().push_back(0.0);
    values.ref().getData().push_back(0.1);
    values.ref().getData().push_back(1.0);
    values.ref().getRank().resize(values.ref().getData().size());
    values.ref().getData().push_back(3.0);
    values.ref().getRank().push_back(255);
    precitec::interface::ResultDoubleArray result3{Poco::UUIDGenerator().createRandom(),
                                                   precitec::interface::AnalysisOK,
                                                   precitec::interface::AnalysisErrBadROI,
                                                   ImageContext{},
                                                   values,
                                                   precitec::geo2d::TRange<double>(1.0, 3.5),
                                                   false};

    emit server.seamInspectionStarted(seam2, uuidProductInstance, 0u);
    emit server.resultsReceived(result3);
    emit server.combinedResultsReceived({result3, result3});

    QCOMPARE(model.m_history.size(), 1u);
    emit server.seamInspectionStarted(seam3, QUuid::createUuid(), 0u);
    emit server.resultsReceived(result3);
    emit server.productInspectionStopped(&p);

    QCOMPARE(model.m_history.size(), 3u);
    QCOMPARE(model.m_history.front().serialNumber, QStringLiteral("0"));
    QCOMPARE(model.m_history.front().isNio, true);

    QCOMPARE(model.m_history.back().serialNumber, QStringLiteral("0"));
    QCOMPARE(model.m_history.back().isNio, false);
}

void HistoryModelTest::testMaxHistory()
{
    HistoryModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);
    auto max = 5u;
    model.setMax(max);

    auto const numberOfProducts = 10;
    for (auto item = 0u; item < numberOfProducts; item++)
    {
        Product p{QUuid::createUuid()};
        p.createFirstSeamSeries();
        auto seam1 = p.createSeam();
        auto seam2 = p.createSeam();
        auto seam3 = p.createSeam();

        ResultArgs arg1;
        arg1.setResultType(precitec::interface::XCoordOutOfLimits);
        ImageContext context1;
        context1.setPosition(int(5 * 1000.0));
        arg1.setContext(context1);
        arg1.setNio(true);

        emit server.productInspectionStarted(&p, QUuid::createUuid(), {});
        emit server.seamInspectionStarted(seam1, QUuid::createUuid(), item);
        emit server.nioReceived(arg1);
        emit server.nioReceived(arg1);

        ResultArgs arg2;
        arg2.setResultType(precitec::interface::GapPositionError);
        ImageContext context2;
        context2.setPosition(int(5 * 1000.0));
        arg2.setContext(context2);
        arg2.setNio(true);
        emit server.nioReceived(arg2);

        precitec::interface::GeoDoublearray values;
        values.ref().getData().push_back(0.0);
        values.ref().getData().push_back(0.1);
        values.ref().getData().push_back(1.0);
        values.ref().getRank().resize(values.ref().getData().size());
        values.ref().getData().push_back(3.0);
        values.ref().getRank().push_back(255);
        precitec::interface::ResultDoubleArray result3{Poco::UUIDGenerator().createRandom(),
                                                       precitec::interface::AnalysisOK,
                                                       precitec::interface::AnalysisErrBadROI,
                                                       ImageContext{},
                                                       values,
                                                       precitec::geo2d::TRange<double>(1.0, 3.5),
                                                       false};

        emit server.seamInspectionStarted(seam2, QUuid::createUuid(), item + 1);
        emit server.resultsReceived(result3);
        emit server.combinedResultsReceived({result3, result3});

        emit server.seamInspectionStarted(seam3, QUuid::createUuid(), item + 1);
        emit server.resultsReceived(result3);
        emit server.productInspectionStopped(&p);
    }

    QCOMPARE(model.m_history.size(), max);

    QCOMPARE(model.m_history.front().serialNumber, QStringLiteral("9"));
    QCOMPARE(model.m_history.front().isNio, false);

    QCOMPARE(model.m_history.back().serialNumber, QString::number(numberOfProducts));
    QCOMPARE(model.m_history.back().isNio, 0);
}

void HistoryModelTest::testAbstractListModelFunctionality()
{
    HistoryModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);
    auto max = 10u;
    model.setMax(max);

    auto const numberOfProducts = 10;
    for (auto item = 0u; item < numberOfProducts; item++)
    {
        Product p{QUuid::createUuid()};
        p.setName("product_" + QString::number(item));
        p.createFirstSeamSeries();
        auto seam1 = p.createSeam();
        auto seam2 = p.createSeam();
        auto seam3 = p.createSeam();

        ResultArgs arg1;
        arg1.setResultType(precitec::interface::XCoordOutOfLimits);
        ImageContext context1;
        context1.setPosition(int(5 * 1000.0));
        arg1.setContext(context1);
        arg1.setNio(true);

        emit server.productInspectionStarted(&p, QUuid::createUuid(), {});
        emit server.seamInspectionStarted(seam1, QUuid::createUuid(), item);
        emit server.nioReceived(arg1);
        emit server.nioReceived(arg1);

        ResultArgs arg2;
        arg2.setResultType(precitec::interface::GapPositionError);
        ImageContext context2;
        context2.setPosition(int(5 * 1000.0));
        arg2.setContext(context2);
        arg2.setNio(true);
        emit server.nioReceived(arg2);

        precitec::interface::GeoDoublearray values;
        values.ref().getData().push_back(0.0);
        values.ref().getData().push_back(0.1);
        values.ref().getData().push_back(1.0);
        values.ref().getRank().resize(values.ref().getData().size());
        values.ref().getData().push_back(3.0);
        values.ref().getRank().push_back(255);
        precitec::interface::ResultDoubleArray result3{Poco::UUIDGenerator().createRandom(),
                                                       precitec::interface::AnalysisOK,
                                                       precitec::interface::AnalysisErrBadROI,
                                                       ImageContext{},
                                                       values,
                                                       precitec::geo2d::TRange<double>(1.0, 3.5),
                                                       false};

        emit server.seamInspectionStarted(seam2, QUuid::createUuid(), item + 1);
        emit server.resultsReceived(result3);
        emit server.combinedResultsReceived({result3, result3});

        emit server.seamInspectionStarted(seam3, QUuid::createUuid(), item + 1);
        emit server.resultsReceived(result3);
        emit server.productInspectionStopped(&p);
    }

    QCOMPARE(model.rowCount(), 10);

    QCOMPARE(model.data(model.index(0), Qt::DisplayRole).toInt(), 7);
    QCOMPARE(model.data(model.index(0), Qt::UserRole).toString(), "product_6");
    QCOMPARE(model.data(model.index(0), Qt::UserRole + 2).toBool(), false);
    QCOMPARE(model.data(model.index(0), Qt::UserRole + 3).toString(), QString{});

    QCOMPARE(model.data(model.index(max - 1), Qt::DisplayRole).toInt(), 10);
    QCOMPARE(model.data(model.index(max - 1), Qt::UserRole).toString(), "product_9");
    QCOMPARE(model.data(model.index(max - 1), Qt::UserRole + 2).toBool(), false);
    QCOMPARE(model.data(model.index(max - 1), Qt::UserRole + 3).toString(), QString{});
}

void HistoryModelTest::testExtendedProductInfo_data()
{
    QTest::addColumn<QString>("extendedProductInfo");
    QTest::addColumn<bool>("serialNumber");
    QTest::addColumn<quint32>("serialNumberField");
    QTest::addColumn<QString>("expectedSerialNumber");
    QTest::addColumn<bool>("partNumber");
    QTest::addColumn<quint32>("partNumberField");
    QTest::addColumn<QString>("expectedPartNumber");

    QTest::newRow("disabled") << QStringLiteral("STFR21336002351\nP2623301-35-B") << false << 0u << QStringLiteral("1") << false << 1u << QString{};
    QTest::newRow("enabled") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 0u << QStringLiteral("STFR21336002351") << true << 1u << QStringLiteral("P2623301-35-B");
    QTest::newRow("wrong index serial") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 2u << QStringLiteral("1") << true << 1u << QStringLiteral("P2623301-35-B");
    QTest::newRow("wrong index part") << QStringLiteral("STFR21336002351\nP2623301-35-B") << true << 0u << QStringLiteral("STFR21336002351") << true << 2u << QString{};
    QTest::newRow("empty") << QString{} << true << 0u << QStringLiteral("1") << true << 1u << QString{};
}

void HistoryModelTest::testExtendedProductInfo()
{
    HistoryModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);
    auto max = 10u;
    model.setMax(max);

    Product p{QUuid::createUuid()};
    p.setName("product");
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();

    ResultArgs arg1;
    arg1.setResultType(precitec::interface::XCoordOutOfLimits);
    ImageContext context1;
    context1.setPosition(int(5 * 1000.0));
    arg1.setContext(context1);
    arg1.setNio(true);

    QFETCH(QString, extendedProductInfo);
    QFETCH(bool, serialNumber);
    QFETCH(quint32, serialNumberField);
    QFETCH(bool, partNumber);
    QFETCH(quint32, partNumberField);
    model.extendedProductInfoHelper()->setSerialNumberFromExtendedProductInfo(serialNumber);
    model.extendedProductInfoHelper()->setSerialNumberFromExtendedProductInfoField(serialNumberField);
    model.extendedProductInfoHelper()->setPartNumberFromExtendedProductInfo(partNumber);
    model.extendedProductInfoHelper()->setPartNumberFromExtendedProductInfoField(partNumberField);

    emit server.productInspectionStarted(&p, QUuid::createUuid(), extendedProductInfo);
    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 1);

    emit server.nioReceived(arg1);

    emit server.productInspectionStopped(&p);

    QCOMPARE(model.rowCount(), 1);

    QTEST(model.data(model.index(0), Qt::DisplayRole).toString(), "expectedSerialNumber");
    QCOMPARE(model.data(model.index(0), Qt::UserRole).toString(), "product");
    QTEST(model.data(model.index(0), Qt::UserRole + 3).toString(), "expectedPartNumber");
}

void HistoryModelTest::testStopProductInspectionWithoutResults()
{
    HistoryModel model{this};
    ResultsServer server;
    model.setResultsServer(&server);
    auto max = 5u;
    model.setMax(max);
    Product p{QUuid::createUuid()};
    p.setName("product");
    p.createFirstSeamSeries();
    auto seam1 = p.createSeam();

    ResultArgs arg1;
    arg1.setResultType(precitec::interface::XCoordOutOfLimits);
    ImageContext context1;
    context1.setPosition(int(5 * 1000.0));
    arg1.setContext(context1);
    arg1.setNio(false);

    emit server.productInspectionStarted(&p, QUuid::createUuid(), {});
    emit server.seamInspectionStarted(seam1, QUuid::createUuid(), 1);
    emit server.productInspectionStopped(&p);

    QCOMPARE(model.m_history.size(), 1);

    QCOMPARE(model.m_history.front().serialNumber, QStringLiteral("1"));
    QCOMPARE(model.m_history.front().isNio, true);
    QCOMPARE(model.m_history.front().productName, "product");
    QCOMPARE(model.m_history.front().partNumber, "");
}

QTEST_GUILESS_MAIN(HistoryModelTest)
#include "historyModelTest.moc"
