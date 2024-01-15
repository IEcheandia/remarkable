#include <QTest>
#include <QSignalSpy>
#include <QUuid>

#include "../src/productInstanceSeamModel.h"
#include "../src/metaDataWriterCommand.h"
#include "../src/product.h"
#include "../src/productInstanceSeamSortModel.h"

using precitec::storage::MetaDataWriterCommand;
using precitec::storage::Product;
using precitec::storage::ProductInstanceSeamModel;
using precitec::storage::ProductInstanceSeamSortModel;


class TestProductInstanceSeamModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testLoading();
    void testLoadMetaData_data();
    void testLoadMetaData();
    void testSortModel();
};

void TestProductInstanceSeamModel::testCtor()
{
    ProductInstanceSeamModel model;
    QCOMPARE(model.rowCount(), 0);
    QCOMPARE(model.productInstance().exists(), false);
    const auto roles = model.roleNames();
    QCOMPARE(roles.count(), 6);
    QCOMPARE(roles[Qt::DisplayRole], QByteArrayLiteral("number"));
    QCOMPARE(roles[Qt::UserRole], QByteArrayLiteral("nio"));
    QCOMPARE(roles[Qt::UserRole + 1], QByteArrayLiteral("nioResultsSwitchedOff"));
    QCOMPARE(roles[Qt::UserRole + 2], QByteArrayLiteral("length"));
    QCOMPARE(roles[Qt::UserRole + 3], QByteArrayLiteral("visualNumber"));
    QCOMPARE(roles[Qt::UserRole + 4], QByteArrayLiteral("uuid"));
}

void TestProductInstanceSeamModel::testLoading()
{
    QTemporaryDir dir;
    QVERIFY(QDir{}.mkpath(dir.path() + QStringLiteral("/seam_series0001/seam0001/")));
    QVERIFY(QDir{}.mkpath(dir.path() + QStringLiteral("/seam_series0001/seam0002/")));
    QVERIFY(QDir{}.mkpath(dir.path() + QStringLiteral("/seam_series0002/seam0001/")));
    QVERIFY(QDir{}.mkpath(dir.path() + QStringLiteral("/seam_series0002/seamabcd/")));
    QVERIFY(QDir{}.mkpath(dir.path() + QStringLiteral("/seamSeries0002/seam0002/")));
    QVERIFY(QDir{}.mkpath(dir.path() + QStringLiteral("/seam_seriesabcd/seam0002/")));
    QFileInfo info(dir.path());
    QVERIFY(info.exists());
    QVERIFY(info.isDir());

    ProductInstanceSeamModel model;
    QSignalSpy productInstanceChangedSpy(&model, &ProductInstanceSeamModel::productInstanceChanged);
    QVERIFY(productInstanceChangedSpy.isValid());
    QSignalSpy seamSeriesChangedSpy(&model, &ProductInstanceSeamModel::seamSeriesChanged);
    QVERIFY(seamSeriesChangedSpy.isValid());
    QSignalSpy modelResetSpy(&model, &ProductInstanceSeamModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    model.setProductInstance(info);
    QCOMPARE(model.productInstance(), info);
    QCOMPARE(productInstanceChangedSpy.count(), 1);
    QCOMPARE(seamSeriesChangedSpy.count(), 0);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 0);

    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto series1 = product.createSeamSeries();
    auto series2 = product.createSeamSeries();

    model.setSeamSeries(series1);
    QCOMPARE(model.seamSeries(), series1);
    QCOMPARE(seamSeriesChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 2);

    QCOMPARE(model.rowCount(), 2);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toInt(), 1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).value<ProductInstanceSeamModel::State>(), ProductInstanceSeamModel::State::Unknown);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toInt(), -1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 3).toInt(), 2);

    QCOMPARE(model.index(1, 0).data(Qt::DisplayRole).toInt(), 2);

    model.setSeamSeries(series2);
    QCOMPARE(model.seamSeries(), series2);
    QCOMPARE(seamSeriesChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 3);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data(Qt::DisplayRole).toInt(), 1);

    // setting same product instance should not change
    model.setProductInstance(info);
    QCOMPARE(productInstanceChangedSpy.count(), 1);
    QCOMPARE(modelResetSpy.count(), 3);

    // reset model
    model.setProductInstance(QFileInfo());
    QCOMPARE(productInstanceChangedSpy.count(), 2);
    QCOMPARE(modelResetSpy.count(), 4);
    QCOMPARE(model.rowCount(), 0);
}

void TestProductInstanceSeamModel::testLoadMetaData_data()
{
    QTest::addColumn<bool>("nio");
    QTest::addColumn<bool>("nioSwitchedOff");
    QTest::addColumn<int>("length");
    QTest::addColumn<QVector<int>>("expectedRoles");

    QTest::newRow("io|nioon") << false << false << -1 << QVector<int>{Qt::UserRole, Qt::UserRole + 4};
    QTest::newRow("nio|niooff") << true << true << -1 << QVector<int>{Qt::UserRole, Qt::UserRole + 1, Qt::UserRole + 4};
    QTest::newRow("length") << false << false << 100 << QVector<int>{Qt::UserRole, Qt::UserRole + 2, Qt::UserRole + 4};
}

void TestProductInstanceSeamModel::testLoadMetaData()
{
    QTemporaryDir dir;
    const auto path = dir.path() + QStringLiteral("/seam_series0001/seam0001/");
    QVERIFY(QDir{}.mkpath(path));
    QFileInfo info(dir.path());
    QVERIFY(info.exists());
    QVERIFY(info.isDir());

    // create metadata
    QFETCH(bool, nio);
    QFETCH(bool, nioSwitchedOff);
    QFETCH(int, length);
    QUuid uuid = QUuid::createUuid();
    MetaDataWriterCommand writer{QDir{path}, {
        {QStringLiteral("nio"), nio},
        {QStringLiteral("nioSwitchedOff"), nioSwitchedOff},
        {QStringLiteral("length"), length},
        {QStringLiteral("uuid"), uuid.toString(QUuid::WithoutBraces)}
    }};
    writer.execute();

    ProductInstanceSeamModel model;
    QSignalSpy dataChangedSpy(&model, &ProductInstanceSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto series1 = product.createSeamSeries();

    model.setProductInstance(info);
    model.setSeamSeries(series1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole).value<ProductInstanceSeamModel::State>(), ProductInstanceSeamModel::State::Unknown);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toBool(), false);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toInt(), -1);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 4).toUuid(), QUuid());
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(0, 0));
    QTEST(dataChangedSpy.first().at(2).value<QVector<int>>(), "expectedRoles");

    QCOMPARE(model.index(0, 0).data(Qt::UserRole).value<ProductInstanceSeamModel::State>(), nio ? ProductInstanceSeamModel::State::Nio : ProductInstanceSeamModel::State::Io);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toBool(), nioSwitchedOff);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).toInt(), length);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 4).toUuid(), uuid);
}

void TestProductInstanceSeamModel::testSortModel()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QDir seamSeriesDir{dir.filePath(QStringLiteral("seam_series0001"))};
    QVERIFY(QDir{}.mkpath(seamSeriesDir.absolutePath()));

    std::vector<QUuid> uuids{
        QUuid::createUuid(),
        QUuid::createUuid(),
        QUuid::createUuid()
    };

    for (std::size_t i = 0; i < uuids.size(); i++)
    {
        QDir seamDir{seamSeriesDir.filePath(QStringLiteral("seam000%1").arg(i))};
        QVERIFY(QDir{}.mkpath(seamDir.absolutePath()));

        // create metadata
        MetaDataWriterCommand writer{seamDir, {
            {QStringLiteral("nio"), false},
            {QStringLiteral("nioSwitchedOff"), true},
            {QStringLiteral("length"), int(i)},
            {QStringLiteral("uuid"), uuids.at(i).toString(QUuid::WithoutBraces)}
        }};
        writer.execute();
    }

    ProductInstanceSeamModel model;
    QSignalSpy dataChangedSpy(&model, &ProductInstanceSeamModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    Product product{QUuid::createUuid()};
    product.createFirstSeamSeries();
    auto series1 = product.createSeamSeries();

    model.setProductInstance(QFileInfo{dir.path()});
    model.setSeamSeries(series1);
    QCOMPARE(model.rowCount(), 3);
    QVERIFY(dataChangedSpy.wait());
    QTRY_COMPARE(dataChangedSpy.count(), 3);
    QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 4).toUuid(), uuids.at(0));
    QCOMPARE(model.data(model.index(1, 0), Qt::UserRole + 4).toUuid(), uuids.at(1));
    QCOMPARE(model.data(model.index(2, 0), Qt::UserRole + 4).toUuid(), uuids.at(2));

    ProductInstanceSeamSortModel sortModel;
    QVERIFY(!sortModel.productInstanceSeamModel());
    QSignalSpy sortModelChangedSpy{&sortModel, &ProductInstanceSeamSortModel::productInstanceSeamModelChanged};
    QVERIFY(sortModelChangedSpy.isValid());
    sortModel.setProductInstanceSeamModel(&model);
    QCOMPARE(sortModel.productInstanceSeamModel(), &model);
    QCOMPARE(sortModelChangedSpy.count(), 1);
    // setting same should not change
    sortModel.setProductInstanceSeamModel(&model);
    QCOMPARE(sortModelChangedSpy.count(), 1);

    QCOMPARE(sortModel.rowCount(), 3);
    // there is no metadata.json yet, thus it should be sorted by seam number
    QCOMPARE(sortModel.data(sortModel.index(0, 0), Qt::UserRole + 4).toUuid(), model.data(model.index(0, 0), Qt::UserRole + 4).toUuid());
    QCOMPARE(sortModel.data(sortModel.index(1, 0), Qt::UserRole + 4).toUuid(), model.data(model.index(1, 0), Qt::UserRole + 4).toUuid());
    QCOMPARE(sortModel.data(sortModel.index(2, 0), Qt::UserRole + 4).toUuid(), model.data(model.index(2, 0), Qt::UserRole + 4).toUuid());

    // now reset the sort model, create metadata and set again
    sortModel.setProductInstanceSeamModel(nullptr);
    QCOMPARE(sortModelChangedSpy.count(), 2);
    QCOMPARE(sortModel.rowCount(), 0);

    // create metadata
    MetaDataWriterCommand writer{seamSeriesDir, {
        {QStringLiteral("nio"), false},
        {QStringLiteral("nioSwitchedOff"), true},
        {QStringLiteral("processedSeams"), QJsonArray {
            QJsonObject{
                {QStringLiteral("uuid"), uuids.at(2).toString(QUuid::WithoutBraces)},
                {QStringLiteral("nio"), false}
            },
            QJsonObject{
                {QStringLiteral("uuid"), uuids.at(2).toString(QUuid::WithoutBraces)},
                {QStringLiteral("nio"), false}
            },
            QJsonObject{
                {QStringLiteral("uuid"), uuids.at(2).toString(QUuid::WithoutBraces)},
                {QStringLiteral("nio"), false}
            }
        }}
    }};
    writer.execute();

    sortModel.setProductInstanceSeamModel(&model);
    QCOMPARE(sortModel.rowCount(), 3);
    QCOMPARE(sortModel.data(sortModel.index(0, 0), Qt::UserRole + 4).toUuid(), model.data(model.index(2, 0), Qt::UserRole + 4).toUuid());
    QCOMPARE(sortModel.data(sortModel.index(1, 0), Qt::UserRole + 4).toUuid(), model.data(model.index(0, 0), Qt::UserRole + 4).toUuid());
    QCOMPARE(sortModel.data(sortModel.index(2, 0), Qt::UserRole + 4).toUuid(), model.data(model.index(1, 0), Qt::UserRole + 4).toUuid());
}

QTEST_GUILESS_MAIN(TestProductInstanceSeamModel)
#include "testProductInstanceSeamModel.moc"
