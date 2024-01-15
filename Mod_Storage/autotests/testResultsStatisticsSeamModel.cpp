#include <QTest>
#include <QSignalSpy>

#include "../src/product.h"
#include "../src/resultsStatisticsController.h"
#include "../src/resultsStatisticsSeamModel.h"

using precitec::storage::Product;
using precitec::storage::ResultsStatisticsController;
using precitec::storage::ResultsStatisticsSeamModel;

class TestResultsStatisticsSeamModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoles();
    void testResultsStatisticsController();
    void testSeamSeriesId();
    void testSeamId();
    void testLinkedSeamId();
    void testData_data();
    void testData();
    void testSeam_data();
    void testSeam();
    void testLinkedSeam_data();
    void testLinkedSeam();
};

void TestResultsStatisticsSeamModel::testCtor()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QCOMPARE(model->roleNames().size(), 7);
    QCOMPARE(model->rowCount(), 0);
    QVERIFY(!model->resultsStatisticsController());
    QVERIFY(model->seamSeriesId().isNull());
    QVERIFY(model->seamId().isNull());
    QVERIFY(model->linkedSeamId().isNull());

    QCOMPARE(model->seamIoInPercent(), 0.0);
    QCOMPARE(model->seamNioInPercent(), 0.0);
    QCOMPARE(model->seamIo(), 0);
    QCOMPARE(model->seamNio(), 0);
    QCOMPARE(model->seamIncludesLinkedSeams(), false);
    QCOMPARE(model->seamIoInPercentIncludingLinkedSeams(), 0.0);
    QCOMPARE(model->seamNioInPercentIncludingLinkedSeams(), 0.0);
    QCOMPARE(model->seamIoIncludingLinkedSeams(), 0);
    QCOMPARE(model->seamNioIncludingLinkedSeams(), 0);
}

void TestResultsStatisticsSeamModel::testRoles()
{
    auto model = new ResultsStatisticsSeamModel{this};

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

void TestResultsStatisticsSeamModel::testResultsStatisticsController()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QVERIFY(!model->resultsStatisticsController());

    QSignalSpy controllerChangedSpy(model, &ResultsStatisticsSeamModel::resultsStatisticsControllerChanged);
    QVERIFY(controllerChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
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

void TestResultsStatisticsSeamModel::testSeamSeriesId()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QVERIFY(model->seamSeriesId().isNull());

    QSignalSpy seamSeriesIdChangedSpy(model, &ResultsStatisticsSeamModel::seamSeriesIdChanged);
    QVERIFY(seamSeriesIdChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
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

void TestResultsStatisticsSeamModel::testSeamId()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QVERIFY(model->seamId().isNull());

    QSignalSpy seamIdChangedSpy(model, &ResultsStatisticsSeamModel::seamIdChanged);
    QVERIFY(seamIdChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    model->setSeamId({});
    QCOMPARE(seamIdChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    const auto& uuid = QUuid::createUuid();

    model->setSeamId(uuid);
    QCOMPARE(model->seamId(), uuid);
    QCOMPARE(seamIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model->setSeamId(uuid);
    QCOMPARE(seamIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
}

void TestResultsStatisticsSeamModel::testLinkedSeamId()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QVERIFY(model->linkedSeamId().isNull());

    QSignalSpy linkedSeamIdChangedSpy(model, &ResultsStatisticsSeamModel::linkedSeamIdChanged);
    QVERIFY(linkedSeamIdChangedSpy.isValid());

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
    QVERIFY(modelResetSpy.isValid());

    model->setLinkedSeamId({});
    QCOMPARE(linkedSeamIdChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 0);

    const auto& uuid = QUuid::createUuid();

    model->setLinkedSeamId(uuid);
    QCOMPARE(model->linkedSeamId(), uuid);
    QCOMPARE(linkedSeamIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);

    model->setLinkedSeamId(uuid);
    QCOMPARE(linkedSeamIdChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 1);
}

void TestResultsStatisticsSeamModel::testData_data()
{
    QTest::addColumn<int>("index");
    QTest::addColumn<QUuid>("series");
    QTest::addColumn<QUuid>("seam");
    QTest::addColumn<QString>("name");
    QTest::addColumn<QUuid>("uuid");
    QTest::addColumn<double>("ioInPercent");
    QTest::addColumn<double>("nioInPercent");
    QTest::addColumn<int>("io");
    QTest::addColumn<int>("nio");
    QTest::addColumn<int>("visualNumber");

    QTest::newRow("LinkedSeam_1") << 0 << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"e0677e0d-0887-43d1-b3d5-c6554dccd81a"} << QStringLiteral("5") << QUuid{"6db0ba11-c553-46fb-bc1d-9b14c5a51f76"} << 1.0 << 0.0 << 1 << 0 << 5;

    QTest::newRow("LinkedSeam_2") << 1 << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"e0677e0d-0887-43d1-b3d5-c6554dccd81a"} << QStringLiteral("6") << QUuid{"9738a3d9-ea7e-4740-87f6-205cef07e40a"} << 1.0 << 0.0 << 1 << 0 << 6;
}

void TestResultsStatisticsSeamModel::testData()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
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

    QFETCH(int, index);

    QFETCH(QUuid, series);
    model->setSeamSeriesId(series);
    QCOMPARE(modelResetSpy.count(), 4);
    QCOMPARE(model->rowCount(), 0);

    QFETCH(QUuid, seam);
    model->setSeamId(seam);
    QCOMPARE(modelResetSpy.count(), 5);
    QCOMPARE(model->rowCount(), 2);

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
    QTRY_COMPARE(modelResetSpy.count(), 6);
    QCOMPARE(model->rowCount(), 0);
}

void TestResultsStatisticsSeamModel::testSeam_data()
{
    QTest::addColumn<QUuid>("series");
    QTest::addColumn<QUuid>("seam");
    QTest::addColumn<double>("ioInPercent");
    QTest::addColumn<double>("nioInPercent");
    QTest::addColumn<unsigned int>("io");
    QTest::addColumn<unsigned int>("nio");
    QTest::addColumn<bool>("includesLinkedSeams");
    QTest::addColumn<double>("ioInPercentIncludingLinkedSeams");
    QTest::addColumn<double>("nioInPercentIncludingLinkedSeams");
    QTest::addColumn<unsigned int>("ioIncludingLinkedSeams");
    QTest::addColumn<unsigned int>("nioIncludingLinkedSeams");

    QTest::newRow("Series_1_Seam_1") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"e0677e0d-0887-43d1-b3d5-c6554dccd81a"} << 1.0 / 6.0 << 5.0 / 6.0 << 1u << 5u << true << 3.0 / 8.0 << 5.0 / 8.0 << 3u << 5u;

    QTest::newRow("Series_1_Seam_2") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"0f24e004-ece6-43ad-a94a-1b8ee4658821"} << 0.0 << 1.0 << 0u << 3u << false << 0.0 << 1.0 << 0u << 3u;

    QTest::newRow("Series_1_Seam_3") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"ddc8b6d9-5f88-45b1-af30-818bc232ccd5"} << 0.0 << 1.0 << 0u << 4u << false << 0.0 << 1.0 << 0u << 4u;

    QTest::newRow("Series_1_Seam_4") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"12ac8037-36a6-4d2b-a135-2f8e49ab9988"} << 0.0 << 1.0 << 0u << 4u << false << 0.0 << 1.0 << 0u << 4u;

    QTest::newRow("Series_2_Seam_1") << QUuid{"071ffab7-5161-4e92-a3d5-1e50b4227ab1"} << QUuid{"43783412-44a0-42fa-b9a3-9edf0cd98175"} << 1.0 / 3.0 << 2.0 / 3.0 << 1u << 2u << false << 1.0 / 3.0 << 2.0 / 3.0 << 1u << 2u;
}

void TestResultsStatisticsSeamModel::testSeam()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
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
    QCOMPARE(model->rowCount(), 0);

    QFETCH(QUuid, seam);
    model->setSeamId(seam);
    QCOMPARE(modelResetSpy.count(), 5);

    QTEST(model->seamIoInPercent(), "ioInPercent");
    QTEST(model->seamNioInPercent(), "nioInPercent");
    QTEST(model->seamIo(), "io");
    QTEST(model->seamNio(), "nio");
    QTEST(model->seamIncludesLinkedSeams(), "includesLinkedSeams");
    QTEST(model->seamIoInPercentIncludingLinkedSeams(), "ioInPercentIncludingLinkedSeams");
    QTEST(model->seamNioInPercentIncludingLinkedSeams(), "nioInPercentIncludingLinkedSeams");
    QTEST(model->seamIoIncludingLinkedSeams(), "ioIncludingLinkedSeams");
    QTEST(model->seamNioIncludingLinkedSeams(), "nioIncludingLinkedSeams");

    controller->deleteLater();
    QTRY_COMPARE(modelResetSpy.count(), 6);
    QCOMPARE(model->rowCount(), 0);
}

void TestResultsStatisticsSeamModel::testLinkedSeam_data()
{
    QTest::addColumn<QUuid>("series");
    QTest::addColumn<QUuid>("seam");
    QTest::addColumn<QUuid>("linkedSeam");
    QTest::addColumn<double>("ioInPercent");
    QTest::addColumn<double>("nioInPercent");
    QTest::addColumn<unsigned int>("io");
    QTest::addColumn<unsigned int>("nio");
    QTest::addColumn<bool>("includesLinkedSeams");
    QTest::addColumn<double>("ioInPercentIncludingLinkedSeams");
    QTest::addColumn<double>("nioInPercentIncludingLinkedSeams");
    QTest::addColumn<unsigned int>("ioIncludingLinkedSeams");
    QTest::addColumn<unsigned int>("nioIncludingLinkedSeams");

    QTest::newRow("LinkedSeam_1") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"e0677e0d-0887-43d1-b3d5-c6554dccd81a"} << QUuid{"6db0ba11-c553-46fb-bc1d-9b14c5a51f76"} << 1.0 << 0.0 << 1u << 0u << false << 1.0 << 0.0 << 1u << 0u;

    QTest::newRow("LinkedSeam_2") << QUuid{"45bbcc22-f179-4057-86d5-a9812699e28e"} << QUuid{"e0677e0d-0887-43d1-b3d5-c6554dccd81a"} << QUuid{"9738a3d9-ea7e-4740-87f6-205cef07e40a"} << 1.0 << 0.0 << 1u << 0u << false << 1.0 << 0.0 << 1u << 0u;

    QTest::newRow("NoLinkedSeamFound") << QUuid{"071ffab7-5161-4e92-a3d5-1e50b4227ab1"} << QUuid{"43783412-44a0-42fa-b9a3-9edf0cd98175"} << QUuid{"9738a3d9-ea7e-4740-87f6-205cef07e40a"} << 0.0 << 0.0 << 0u << 0u << false << 0.0 << 0.0 << 0u << 0u;
}

void TestResultsStatisticsSeamModel::testLinkedSeam()
{
    auto model = new ResultsStatisticsSeamModel{this};
    QCOMPARE(model->rowCount(), 0);

    QSignalSpy modelResetSpy(model, &ResultsStatisticsSeamModel::modelReset);
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
    QCOMPARE(model->rowCount(), 0);

    QFETCH(QUuid, seam);
    model->setSeamId(seam);
    QCOMPARE(modelResetSpy.count(), 5);

    QFETCH(QUuid, linkedSeam);
    model->setLinkedSeamId(linkedSeam);
    QCOMPARE(modelResetSpy.count(), 6);
    QCOMPARE(model->rowCount(), 0);

    QTEST(model->seamIoInPercent(), "ioInPercent");
    QTEST(model->seamNioInPercent(), "nioInPercent");
    QTEST(model->seamIo(), "io");
    QTEST(model->seamNio(), "nio");
    QTEST(model->seamIncludesLinkedSeams(), "includesLinkedSeams");
    QTEST(model->seamIoInPercentIncludingLinkedSeams(), "ioInPercentIncludingLinkedSeams");
    QTEST(model->seamNioInPercentIncludingLinkedSeams(), "nioInPercentIncludingLinkedSeams");
    QTEST(model->seamIoIncludingLinkedSeams(), "ioIncludingLinkedSeams");
    QTEST(model->seamNioIncludingLinkedSeams(), "nioIncludingLinkedSeams");

    controller->deleteLater();
    QTRY_COMPARE(modelResetSpy.count(), 6);
    QCOMPARE(model->rowCount(), 0);
}

QTEST_GUILESS_MAIN(TestResultsStatisticsSeamModel)
#include "testResultsStatisticsSeamModel.moc"
