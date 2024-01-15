#include <QTest>
#include <QSignalSpy>

#include "../src/product.h"
#include "../src/resultsStatisticsController.h"
#include "../src/resultsStatisticsSeamsModel.h"

using precitec::storage::Product;
using precitec::storage::ResultsStatisticsController;
using precitec::storage::ResultsStatisticsSeamsModel;

class TestResultsStatisticsSeamsModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoles();
    void testResultsStatisticsController();
    void testSeamSeriesId();
    void testData_data();
    void testData();
};

void TestResultsStatisticsSeamsModel::testCtor()
{
    auto model = new ResultsStatisticsSeamsModel{this};
    QCOMPARE(model->roleNames().size(), 12);
    QCOMPARE(model->rowCount(), 0);
    QVERIFY(!model->resultsStatisticsController());
    QVERIFY(model->seamSeriesId().isNull());
}

void TestResultsStatisticsSeamsModel::testRoles()
{
    auto model = new ResultsStatisticsSeamsModel{this};

    const auto& roles = model->roleNames();
    QCOMPARE(roles.count(), 12);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("name"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("id"));
    QCOMPARE(roles.value(Qt::UserRole + 1), QByteArrayLiteral("ioInPercent"));
    QCOMPARE(roles.value(Qt::UserRole + 2), QByteArrayLiteral("nioInPercent"));
    QCOMPARE(roles.value(Qt::UserRole + 3), QByteArrayLiteral("io"));
    QCOMPARE(roles.value(Qt::UserRole + 4), QByteArrayLiteral("nio"));
    QCOMPARE(roles.value(Qt::UserRole + 5), QByteArrayLiteral("visualNumber"));
    QCOMPARE(roles.value(Qt::UserRole + 6), QByteArrayLiteral("includesLinkedSeams"));
    QCOMPARE(roles.value(Qt::UserRole + 7), QByteArrayLiteral("ioInPercentIncludingLinkedSeams"));
    QCOMPARE(roles.value(Qt::UserRole + 8), QByteArrayLiteral("nioInPercentIncludingLinkedSeams"));
    QCOMPARE(roles.value(Qt::UserRole + 9), QByteArrayLiteral("ioIncludingLinkedSeams"));
    QCOMPARE(roles.value(Qt::UserRole + 10), QByteArrayLiteral("nioIncludingLinkedSeams"));
}

void TestResultsStatisticsSeamsModel::testResultsStatisticsController()
{
    auto model = new ResultsStatisticsSeamsModel{this};
    QVERIFY(!model->resultsStatisticsController());

    QSignalSpy controllerChangedSpy(model, &ResultsStatisticsSeamsModel::resultsStatisticsControllerChanged);
    QVERIFY(controllerChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamsModel::modelReset);
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

void TestResultsStatisticsSeamsModel::testSeamSeriesId()
{
    auto model = new ResultsStatisticsSeamsModel{this};
    QVERIFY(model->seamSeriesId().isNull());

    QSignalSpy seamSeriesIdChangedSpy(model, &ResultsStatisticsSeamsModel::seamSeriesIdChanged);
    QVERIFY(seamSeriesIdChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamsModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    model->setSeamSeriesId({});
    QCOMPARE(seamSeriesIdChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    const auto& uuid = QUuid::createUuid();

    model->setSeamSeriesId(uuid);
    QCOMPARE(model->seamSeriesId(), uuid);
    QCOMPARE(seamSeriesIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model->setSeamSeriesId(uuid);
    QCOMPARE(seamSeriesIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
}

void TestResultsStatisticsSeamsModel::testData_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<QUuid>("series");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<int>("rowCount");
    QTest::addColumn<double>("ioInPercent");
    QTest::addColumn<double>("nioInPercent");
    QTest::addColumn<int>("io");
    QTest::addColumn<int>("nio");
    QTest::addColumn<int>("visualNumber");
    QTest::addColumn<bool>("includesLinkedSeams");
    QTest::addColumn<double>("ioInPercentIncludingLinkedSeams");
    QTest::addColumn<double>("nioInPercentIncludingLinkedSeams");
    QTest::addColumn<int>("ioIncludingLinkedSeams");
    QTest::addColumn<int>("nioIncludingLinkedSeams");

    QTest::newRow("Series_1_Seam_1") << 0 << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QStringLiteral("Seam_1") << QUuid{"e0677e0d-0887-43d1-b3d5-c6554dccd81a"} << 4 << 1.0 / 6.0 << 5.0 / 6.0 << 1 << 5 << 1 << true << 3.0 / 8.0 << 5.0 / 8.0 << 3 << 5;

    QTest::newRow("Series_1_Seam_2") << 1 << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QStringLiteral("Seam_2") << QUuid{"0f24e004-ece6-43ad-a94a-1b8ee4658821"} << 4 << 0.0 << 1.0 << 0 << 3 << 2 << false << 0.0 << 1.0 << 0 << 3;

    QTest::newRow("Series_1_Seam_3") << 2 << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QStringLiteral("Seam_3") << QUuid{"ddc8b6d9-5f88-45b1-af30-818bc232ccd5"} << 4 << 0.0 << 1.0 << 0 << 4 << 3 << false << 0.0 << 1.0 << 0 << 4;

    QTest::newRow("Series_1_Seam_4") << 3 << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QStringLiteral("Seam_4") << QUuid{"12ac8037-36a6-4d2b-a135-2f8e49ab9988"} << 4 << 0.0 << 1.0 << 0 << 4 << 4 << false << 0.0 << 1.0 << 0 << 4;

    QTest::newRow("Series_2_Seam_1") << 0 << QUuid{"071ffab7-5161-4e92-a3d5-1e50b4227ab1"} << QStringLiteral("Seam_5") << QUuid{"43783412-44a0-42fa-b9a3-9edf0cd98175"} << 1 << 1.0 / 3.0 << 2.0 / 3.0 << 1 << 2 << 1 << false << 1.0 / 3.0 << 2.0 / 3.0 << 1 << 2;
}

void TestResultsStatisticsSeamsModel::testData()
{
    auto model = new ResultsStatisticsSeamsModel{this};
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamsModel::modelReset);
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
    QCOMPARE(model->rowCount(), 0);

    QFETCH(QUuid, series);
    model->setSeamSeriesId(series);
    QCOMPARE(modelResetSpy.count(), 4);
    QTEST(model->rowCount(), "rowCount");

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
    QTEST(idx.data(Qt::UserRole + 6).toBool(), "includesLinkedSeams");
    QTEST(idx.data(Qt::UserRole + 7).toDouble(), "ioInPercentIncludingLinkedSeams");
    QTEST(idx.data(Qt::UserRole + 8).toDouble(), "nioInPercentIncludingLinkedSeams");
    QTEST(idx.data(Qt::UserRole + 9).toInt(), "ioIncludingLinkedSeams");
    QTEST(idx.data(Qt::UserRole + 10).toInt(), "nioIncludingLinkedSeams");

    controller->deleteLater();
    QTRY_COMPARE(modelResetSpy.count(), 5);
    QCOMPARE(model->rowCount(), 0);
}

QTEST_GUILESS_MAIN(TestResultsStatisticsSeamsModel)
#include "testResultsStatisticsSeamsModel.moc"
