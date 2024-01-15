#include <QTest>
#include <QSignalSpy>

#include "../src/product.h"
#include "../src/resultsStatisticsController.h"
#include "../src/resultsStatisticsSeamSeriesModel.h"

using precitec::storage::Product;
using precitec::storage::ResultsStatisticsController;
using precitec::storage::ResultsStatisticsSeamSeriesModel;

class TestResultsStatisticsSeamSeriesModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoles();
    void testResultsStatisticsController();
    void testData_data();
    void testData();
};

void TestResultsStatisticsSeamSeriesModel::testCtor()
{
    auto model = new ResultsStatisticsSeamSeriesModel{this};
    QCOMPARE(model->roleNames().size(), 7);
    QCOMPARE(model->rowCount(), 0);
    QVERIFY(!model->resultsStatisticsController());
}

void TestResultsStatisticsSeamSeriesModel::testRoles()
{
    auto model = new ResultsStatisticsSeamSeriesModel{this};

    const auto& roles = model->roleNames();
    QCOMPARE(roles.count(), 7);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("name"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("id"));
    QCOMPARE(roles.value(Qt::UserRole + 1), QByteArrayLiteral("ioInPercent"));
    QCOMPARE(roles.value(Qt::UserRole + 2), QByteArrayLiteral("nioInPercent"));
    QCOMPARE(roles.value(Qt::UserRole + 3), QByteArrayLiteral("io"));
    QCOMPARE(roles.value(Qt::UserRole + 4), QByteArrayLiteral("nio"));
    QCOMPARE(roles.value(Qt::UserRole + 5), QByteArrayLiteral("visualNumber"));
}

void TestResultsStatisticsSeamSeriesModel::testResultsStatisticsController()
{
    auto model = new ResultsStatisticsSeamSeriesModel{this};
    QVERIFY(!model->resultsStatisticsController());

    QSignalSpy controllerChangedSpy(model, &ResultsStatisticsSeamSeriesModel::resultsStatisticsControllerChanged);
    QVERIFY(controllerChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamSeriesModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    model->setResultsStatisticsController(nullptr);
    QCOMPARE(controllerChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    auto controller = new ResultsStatisticsController{this};

    model->setResultsStatisticsController(controller);
    QCOMPARE(model->resultsStatisticsController(), controller);
    QCOMPARE(controllerChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model->setResultsStatisticsController(controller);
    QCOMPARE(controllerChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    controller->update();
    QCOMPARE(controllerChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 2);

    controller->deleteLater();
    QTRY_COMPARE(controllerChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 3);
    QVERIFY(!model->resultsStatisticsController());
}

void TestResultsStatisticsSeamSeriesModel::testData_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<double>("ioInPercent");
    QTest::addColumn<double>("nioInPercent");
    QTest::addColumn<int>("io");
    QTest::addColumn<int>("nio");
    QTest::addColumn<int>("visualNumber");

    QTest::newRow("Series_1") << 0 << QStringLiteral("Series_1") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << 1.0 / 7.0 << 6.0 / 7.0 << 1 << 6 << 1;
    QTest::newRow("Series_2") << 1 << QStringLiteral("Series_2") << QUuid{"071ffab7-5161-4e92-a3d5-1e50b4227ab1"} << 1.0 / 3.0 << 2.0 / 3.0 << 1 << 2 << 2;
}

void TestResultsStatisticsSeamSeriesModel::testData()
{
    auto model = new ResultsStatisticsSeamSeriesModel{this};
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamSeriesModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    auto controller = new ResultsStatisticsController{this};

    QSignalSpy updateSpy(controller, &ResultsStatisticsController::update);
    QVERIFY(updateSpy.isValid());

    model->setResultsStatisticsController(controller);
    QCOMPARE(modelResetSpy.count(), 1);

    const auto& productJson = QFINDTESTDATA("testdata/statistics/statistics_product.json");

    auto product = Product::fromJson(productJson, this);
    QVERIFY(product);

    controller->setCurrentProduct(product);
    controller->setResultsStoragePath(QFINDTESTDATA("testdata/statistics"));
    QCOMPARE(updateSpy.count(), 1);

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(modelResetSpy.count(), 2);

    controller->calculate(QDate{2021, 5, 25}, QDate{2021, 6, 1});
    QTRY_COMPARE(updateSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 3);
    QCOMPARE(model->rowCount(), 2);

    QFETCH(int, index);

    const auto& idx = model->index(index);
    QVERIFY(idx.isValid());

    QTEST(idx.data(Qt::DisplayRole).toString(), "name");
    QTEST(idx.data(Qt::UserRole).toUuid(), "uuid");
    QTEST(idx.data(Qt::UserRole + 1).toDouble(), "ioInPercent");
    QTEST(idx.data(Qt::UserRole + 2).toDouble(), "nioInPercent");
    QTEST(idx.data(Qt::UserRole + 3).toInt(), "io");
    QTEST(idx.data(Qt::UserRole + 4).toInt(), "nio");
    QTEST(idx.data(Qt::UserRole + 5).toInt(), "visualNumber");

    controller->deleteLater();
    QTRY_COMPARE(modelResetSpy.count(), 4);
    QCOMPARE(model->rowCount(), 0);
}

QTEST_GUILESS_MAIN(TestResultsStatisticsSeamSeriesModel)
#include "testResultsStatisticsSeamSeriesModel.moc"
