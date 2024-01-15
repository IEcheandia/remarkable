#include <QTest>
#include <QSignalSpy>
#include <QSortFilterProxyModel>

#include "../src/productInstanceTableModel.h"
#include "../src/product.h"
#include "../src/seamSeries.h"
#include "../src/seam.h"
#include "../src/metaDataWriterCommand.h"
#include "../src/extendedProductInfoHelper.h"
#include "precitec/downloadService.h"
#include "precitec/downloadManagerModel.h"

using precitec::storage::ProductInstanceTableModel;
using precitec::storage::Product;
using precitec::storage::MetaDataWriterCommand;
using precitec::gui::components::removableDevices::DownloadService;

class TestProductInstanceTableModel : public QObject
{
    Q_OBJECT
private:
    //temporary files for testing the download
    QTemporaryDir m_sourceFolder;
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void init();
    void testCtor();
    void testRoleNames();
    void testSetProduct();
    void testSetDirectory();
    void testLoading_data();
    void testLoading();
    void testStationName();
    void testSetData();
    void testCheckAllNone();
    void testMonitoring();
    void testSeams();
    void testLoadMetaData_data();
    void testLoadMetaData();
    void testExtendedProductInfo_data();
    void testExtendedProductInfo();
    void testDownloadChecked_data();
    void testDownloadChecked();
};

void TestProductInstanceTableModel::initTestCase()
{
    QVERIFY(m_sourceFolder.isValid());

    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    //create 9 folders like 1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1, with some files in it so that the download is not immediate
    for (int i = 1; i <= 9; i++)
    {
        auto folder = m_sourceFolder.path() + productDir + QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1/").arg(i);
        QVERIFY(QDir{}.mkpath(folder));
        for (int counter = 0; counter < 50; counter++)
        {
            QFile file{QDir{folder}.filePath(QStringLiteral("bar%1").arg(counter))};
            QVERIFY(file.open(QIODevice::WriteOnly));
            file.write(QByteArrayLiteral("Some data in a subfolder"));
            auto fileSize = 100000;
            QVERIFY(file.write(std::string(fileSize, '*').c_str(), fileSize));
        }
    }
}

void TestProductInstanceTableModel::cleanupTestCase()
{
    QVERIFY(m_sourceFolder.remove());
}

void TestProductInstanceTableModel::init()
{
    auto dm = precitec::gui::components::removableDevices::DownloadManagerModel::instance();
    QTRY_VERIFY(dm->isEmpty());
}

void TestProductInstanceTableModel::testCtor()
{
    auto model = new ProductInstanceTableModel{this};

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    QVERIFY(!model->product());
    QCOMPARE(model->directory(), QString{});
    QVERIFY(!model->monitoring());
    QVERIFY(!model->loading());
    QCOMPARE(model->productDirectoryName(), ProductInstanceTableModel::ProductDirectoryName::Uuid);
    QCOMPARE(model->stationName(), QString{});
    QVERIFY(model->m_productSeams.empty());
    QVERIFY(model->m_productInstances.empty());
}

void TestProductInstanceTableModel::testRoleNames()
{
    auto model = new ProductInstanceTableModel{this};

    const auto& roleNames = model->roleNames();
    QCOMPARE(roleNames.count(), 13);
    QCOMPARE(roleNames.value(Qt::DisplayRole), QByteArrayLiteral("display"));
    QCOMPARE(roleNames.value(Qt::UserRole), QByteArrayLiteral("fileInfo"));
    QCOMPARE(roleNames.value(Qt::UserRole + 1), QByteArrayLiteral("date"));
    QCOMPARE(roleNames.value(Qt::UserRole + 2), QByteArrayLiteral("nio"));
    QCOMPARE(roleNames.value(Qt::UserRole + 3), QByteArrayLiteral("nioResultsSwitchedOff"));
    QCOMPARE(roleNames.value(Qt::UserRole + 4), QByteArrayLiteral("relativePath"));
    QCOMPARE(roleNames.value(Qt::UserRole + 5), QByteArrayLiteral("downloadService"));
    QCOMPARE(roleNames.value(Qt::UserRole + 6), QByteArrayLiteral("path"));
    QCOMPARE(roleNames.value(Qt::UserRole + 7), QByteArrayLiteral("checked"));
    QCOMPARE(roleNames.value(Qt::UserRole + 8), QByteArrayLiteral("directoryName"));
    QCOMPARE(roleNames.value(Qt::UserRole + 9), QByteArrayLiteral("seamSeries"));
    QCOMPARE(roleNames.value(Qt::UserRole + 10), QByteArrayLiteral("seam"));
    QCOMPARE(roleNames.value(Qt::UserRole + 11), QByteArrayLiteral("partNumber"));
}

void TestProductInstanceTableModel::testSetProduct()
{
    auto model = new ProductInstanceTableModel{this};

    QSignalSpy productChangedSpy(model, &ProductInstanceTableModel::productChanged);
    QVERIFY(productChangedSpy.isValid());

    QSignalSpy seamsChangedSpy(model, &ProductInstanceTableModel::seamsChanged);
    QVERIFY(seamsChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QVERIFY(!model->product());
    QCOMPARE(model->columnCount(), 1);

    model->setProduct(nullptr);
    QVERIFY(!model->product());
    QCOMPARE(productChangedSpy.count(), 0);
    QCOMPARE(seamsChangedSpy.count(), 0);

    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);

    model->setProduct(product);
    QCOMPARE(model->product(), product);
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(model->columnCount(), 7);
    QCOMPARE(seamsChangedSpy.count(), 1);

    model->setProduct(product);
    QCOMPARE(productChangedSpy.count(), 1);
    QCOMPARE(seamsChangedSpy.count(), 1);

    product->deleteLater();
    QVERIFY(productChangedSpy.wait());
    QVERIFY(!model->product());
    QCOMPARE(productChangedSpy.count(), 2);
    QCOMPARE(model->columnCount(), 1);
    QCOMPARE(seamsChangedSpy.count(), 2);

    // loading should not have started, as we did not have a directory
    QCOMPARE(loadingChangedSpy.count(), 0);
}

void TestProductInstanceTableModel::testSetDirectory()
{
    auto model = new ProductInstanceTableModel{this};

    QSignalSpy directoryChangedSpy(model, &ProductInstanceTableModel::directoryChanged);
    QVERIFY(directoryChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QCOMPARE(model->directory(), QString{});

    model->setDirectory({});
    QCOMPARE(directoryChangedSpy.count(), 0);

    model->setDirectory(QStringLiteral("foo"));
    QCOMPARE(model->directory(), QStringLiteral("foo"));
    QCOMPARE(directoryChangedSpy.count(), 1);

    model->setDirectory(QStringLiteral("foo"));
    QCOMPARE(directoryChangedSpy.count(), 1);

    // loading should not have started, as we did not have a product
    QCOMPARE(loadingChangedSpy.count(), 0);
}

void TestProductInstanceTableModel::testLoading_data()
{
    QTest::addColumn<QString>("productDir");
    QTest::addColumn<ProductInstanceTableModel::ProductDirectoryName>("productDirectoryName");

    QTest::newRow("uuid") << QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/") << ProductInstanceTableModel::ProductDirectoryName::Uuid;
    QTest::newRow("name") << QStringLiteral("/FooBar/") << ProductInstanceTableModel::ProductDirectoryName::ProductName;
}

void TestProductInstanceTableModel::testLoading()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    QFETCH(QString, productDir);
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("notaprodinstance/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1234-SN-1234/")));

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy rowInsertedSpy(model, &ProductInstanceTableModel::rowsInserted);
    QVERIFY(rowInsertedSpy.isValid());

    QSignalSpy columnsInsertedSpy(model, &ProductInstanceTableModel::columnsInserted);
    QVERIFY(columnsInsertedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    QFETCH(ProductInstanceTableModel::ProductDirectoryName, productDirectoryName);
    model->setProductDirectoryName(productDirectoryName);

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(rowInsertedSpy.count(), 1);
    QCOMPARE(model->rowCount(), 9);

    QCOMPARE(columnsInsertedSpy.count(), 1);
    QCOMPARE(model->columnCount(), 7);

    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel(model);
    sortModel.setSortRole(Qt::DisplayRole);
    sortModel.sort(0);

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::DisplayRole).toString(), QString::number(row + 1));
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole).value<QFileInfo>(), QFileInfo(dir.path() + productDir + dirName + QStringLiteral("/")));
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 1).toDateTime(), QFileInfo(dir.path() + productDir + dirName).lastModified());
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Unknown);
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 3).toBool(), false);
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/") + dirName);
            QVERIFY(sortModel.index(row, column).data(Qt::UserRole + 5).value<DownloadService*>());
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 6).toString(), dir.path() + productDir + dirName);
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 7).toBool(), false);
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 8).toString(), dirName);
        }
    }
}

void TestProductInstanceTableModel::testStationName()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("notaprodinstance/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1234-SN-1234/")));

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy stationNameChangedSpy(model, &ProductInstanceTableModel::stationNameChanged);
    QVERIFY(stationNameChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ProductInstanceTableModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    QCOMPARE(model->stationName(), QString{});

    model->setStationName({});
    QCOMPARE(stationNameChangedSpy.count(), 0);
    QCOMPARE(dataChangedSpy.count(), 0);

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 9);
    QCOMPARE(model->columnCount(), 7);

    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel(model);
    sortModel.setSortRole(Qt::DisplayRole);
    sortModel.sort(0);

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/") + dirName);
        }
    }

    model->setStationName(QStringLiteral("test"));
    QCOMPARE(model->stationName(), QStringLiteral("test"));
    QCOMPARE(stationNameChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    const auto& args = dataChangedSpy.takeFirst();
    QCOMPARE(args.size(), 3);

    const auto& topLeft = args.at(0).toModelIndex();
    QVERIFY(topLeft.isValid());
    QCOMPARE(topLeft.row(), 0);
    QCOMPARE(topLeft.column(), 0);

    const auto& bottomRight = args.at(1).toModelIndex();
    QVERIFY(bottomRight.isValid());
    QCOMPARE(bottomRight.row(), 8);
    QCOMPARE(bottomRight.column(), 0);

    const auto& roles = args.at(2).value<QVector<int>>();
    QCOMPARE(roles, QVector<int>{Qt::UserRole + 4});

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/") + dirName);
        }
    }
}

void TestProductInstanceTableModel::testSetData()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("notaprodinstance/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1234-SN-1234/")));

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ProductInstanceTableModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 9);
    QCOMPARE(model->columnCount(), 7);

    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel(model);
    sortModel.setSortRole(Qt::DisplayRole);
    sortModel.sort(0);

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QVERIFY(sortModel.index(row, column).data(Qt::UserRole + 5).value<DownloadService*>());
            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 7).toBool(), false);
        }
    }

    // setData defined only for Qt::UserRole + 7
    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::DisplayRole));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 1));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 2));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 3));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 4));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 5));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 6));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 8));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 9));
            QVERIFY(!sortModel.setData(sortModel.index(row, column), {}, Qt::UserRole + 10));
        }
    }

    // test with invalid index
    QVERIFY(!sortModel.setData({}, {}));

    QCOMPARE(dataChangedSpy.count(), 0);

    // set checked
    QVERIFY(sortModel.setData(sortModel.index(5, 0), true, Qt::UserRole + 7));

    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.at(0).at(0).toModelIndex(), sortModel.mapToSource(sortModel.index(5, 0)));
    QCOMPARE(dataChangedSpy.at(0).at(1).toModelIndex(), sortModel.mapToSource(sortModel.index(5, 0)));
    QCOMPARE(dataChangedSpy.at(0).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 7});

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 7).toBool(), row == 5);
        }
    }
}

void TestProductInstanceTableModel::testCheckAllNone()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("notaprodinstance/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1234-SN-1234/")));

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ProductInstanceTableModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 9);
    QCOMPARE(model->columnCount(), 7);

    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel(model);
    sortModel.setSortRole(Qt::DisplayRole);
    sortModel.sort(0);

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 7).toBool(), false);
        }
    }

    model->checkAll();

    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model->index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model->index(8, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 7});

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 7).toBool(), true);
        }
    }

    model->uncheckAll();

    QCOMPARE(dataChangedSpy.count(), 2);

    QCOMPARE(dataChangedSpy.at(1).at(0).toModelIndex(), model->index(0, 0));
    QCOMPARE(dataChangedSpy.at(1).at(1).toModelIndex(), model->index(8, 0));
    QCOMPARE(dataChangedSpy.at(1).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 7});

    for (auto row = 0; row < model->rowCount(); row++)
    {
        for (auto column = 0; column < model->columnCount(); column++)
        {
            const auto dirName = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);

            QCOMPARE(sortModel.index(row, column).data(Qt::UserRole + 7).toBool(), false);
        }
    }

}

void TestProductInstanceTableModel::testMonitoring()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("notaprodinstance/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1234-SN-1234/")));

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy monitoringChangedSpy(model, &ProductInstanceTableModel::monitoringChanged);
    QVERIFY(monitoringChangedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ProductInstanceTableModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy rowsRemovedSpy{model, &ProductInstanceTableModel::rowsRemoved};
    QVERIFY(rowsRemovedSpy.isValid());

    QVERIFY(!model->monitoring());
    QVERIFY(!model->m_updatePending);

    model->setMonitoring(false);
    QCOMPARE(monitoringChangedSpy.count(), 0);

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 9);
    QCOMPARE(model->columnCount(), 7);

    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel(model);
    sortModel.setSortRole(Qt::DisplayRole);
    sortModel.sort(0);

    QCOMPARE(rowsRemovedSpy.count(), 0);
    QVERIFY(!model->m_updatePending);

    const auto& row = sortModel.mapToSource(sortModel.index(4, 0)).row();
    QVERIFY(QDir{dir.path() + QStringLiteral("/") + productDir}.exists(QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5")));
    QVERIFY(QDir{dir.path() + QStringLiteral("/") + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")}.removeRecursively());
    QVERIFY(!QDir{dir.path() + QStringLiteral("/") + productDir}.exists(QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5")));

    QCOMPARE(rowsRemovedSpy.count(), 0);
    QTRY_VERIFY(model->m_updatePending);

    model->setMonitoring(true);
    QVERIFY(model->monitoring());
    QCOMPARE(monitoringChangedSpy.count(), 1);

    QTRY_COMPARE(rowsRemovedSpy.count(), 1);
    QVERIFY(!model->m_updatePending);
    QCOMPARE(rowsRemovedSpy.first().at(0).toModelIndex(), QModelIndex{});
    QCOMPARE(rowsRemovedSpy.first().at(1).toInt(), row);
    QCOMPARE(rowsRemovedSpy.first().at(2).toInt(), row);
}

void TestProductInstanceTableModel::testSeams()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("notaprodinstance/")));
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1234-SN-1234/")));

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy rowInsertedSpy(model, &ProductInstanceTableModel::rowsInserted);
    QVERIFY(rowInsertedSpy.isValid());

    QSignalSpy columnsInsertedSpy(model, &ProductInstanceTableModel::columnsInserted);
    QVERIFY(columnsInsertedSpy.isValid());

    QSignalSpy columnsRemovedSpy{model, &ProductInstanceTableModel::columnsRemoved};
    QVERIFY(columnsRemovedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(rowInsertedSpy.count(), 1);
    QCOMPARE(model->rowCount(), 9);

    QCOMPARE(columnsInsertedSpy.count(), 1);

    QCOMPARE(columnsInsertedSpy.first().at(0).toModelIndex(), QModelIndex{});
    QCOMPARE(columnsInsertedSpy.first().at(1).toInt(), 1);
    QCOMPARE(columnsInsertedSpy.first().at(2).toInt(), 6);

    QCOMPARE(columnsRemovedSpy.count(), 0);
    QCOMPARE(model->columnCount(), 7);

    auto seams = product->seams();
    QCOMPARE(seams.size(), 6);

    std::sort(seams.begin(), seams.end(), [] (auto s1, auto s2) {
        if (s1->seamSeries()->uuid() == s2->seamSeries()->uuid())
        {
            return s1->number() < s2->number();
        }

        return s1->seamSeries()->number() < s2->seamSeries()->number();
    });

    auto thirdSeam = seams.at(2);
    thirdSeam->seamSeries()->destroySeam(thirdSeam);

    QCOMPARE(columnsInsertedSpy.count(), 1);
    QCOMPARE(columnsRemovedSpy.count(), 1);

    QCOMPARE(model->columnCount(), 6);

    // 1 column offset, as first column is always available and displays the instance info
    QCOMPARE(columnsRemovedSpy.first().at(0).toModelIndex(), QModelIndex{});
    QCOMPARE(columnsRemovedSpy.first().at(1).toInt(), 3);
    QCOMPARE(columnsRemovedSpy.first().at(2).toInt(), 3);

    auto series = product->seamSeries();

    std::sort(series.begin(), series.end(), [] (auto s1, auto s2) {
        return s1->number() < s2->number();
    });

    product->destroySeamSeries(series.back());

    QCOMPARE(columnsInsertedSpy.count(), 1);
    QCOMPARE(columnsRemovedSpy.count(), 4);

    QCOMPARE(model->columnCount(), 3);

    // columns are removed one by one, therefore every one is located at index 3 at the moment of removal
    for (auto idx = 1; idx < 4; idx++)
    {
        QCOMPARE(columnsRemovedSpy.at(idx).at(0).toModelIndex(), QModelIndex{});
        QCOMPARE(columnsRemovedSpy.at(idx).at(1).toInt(), 3);
        QCOMPARE(columnsRemovedSpy.at(idx).at(2).toInt(), 3);
    }

    product->seamSeries().front()->createSeam();

    QCOMPARE(columnsInsertedSpy.count(), 2);
    QCOMPARE(columnsRemovedSpy.count(), 4);

    QCOMPARE(model->columnCount(), 4);

    QCOMPARE(columnsInsertedSpy.at(1).at(0).toModelIndex(), QModelIndex{});
    QCOMPARE(columnsInsertedSpy.at(1).at(1).toInt(), 2);
    QCOMPARE(columnsInsertedSpy.at(1).at(2).toInt(), 2);

    model->setProduct(nullptr);

    QCOMPARE(columnsInsertedSpy.count(), 2);
    QCOMPARE(columnsRemovedSpy.count(), 5);

    QCOMPARE(columnsRemovedSpy.at(4).at(0).toModelIndex(), QModelIndex{});
    QCOMPARE(columnsRemovedSpy.at(4).at(1).toInt(), 1);
    QCOMPARE(columnsRemovedSpy.at(4).at(2).toInt(), 3);
}

void TestProductInstanceTableModel::testLoadMetaData_data()
{
    QTest::addColumn<bool>("nio");
    QTest::addColumn<bool>("nioSwitchedOff");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QVector<int>>("expectedRoles");

    QTest::newRow("io|nioon") << false << false << QDateTime() << QVector<int>{Qt::UserRole + 2};
    QTest::newRow("nio|niooff") << true << true << QDateTime() << QVector<int>{Qt::UserRole + 2, Qt::UserRole + 3};
    QTest::newRow("time") << false << false << QDateTime::currentDateTime() << QVector<int>{Qt::UserRole + 1, Qt::UserRole + 2};
}

void TestProductInstanceTableModel::testLoadMetaData()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto path = dir.path() + QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/");
    QVERIFY(QDir{}.mkpath(path));

    QJsonArray seamJson {
        QJsonObject {
            {QStringLiteral("uuid"), "99f71378-ef2a-4ada-8c1f-b022c53505e4"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "8471ced1-ab55-4138-90a8-20fe49426acc"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "56178094-2d5c-47ed-81e5-528094e5f8fe"},
            {QStringLiteral("nio"), true}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "b7872d0b-e19f-4f28-b731-3f6931b5199c"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "68575ccc-3c34-4dc0-94ee-c92bb8811e53"},
            {QStringLiteral("nio"), true}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "3F086211-FBD4-4493-A580-6FF11E4925DF"},
            {QStringLiteral("nio"), false}
        }
    };

    QJsonArray seriesJson {
        QJsonObject {
            {QStringLiteral("uuid"), "bcc3332d-dc4d-4b94-9dc9-604262ec8666"},
            {QStringLiteral("nio"), false}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "f9d9a202-965c-4500-9bc4-41a829072d15"},
            {QStringLiteral("nio"), true}
        },
        QJsonObject {
            {QStringLiteral("uuid"), "2F086211-FBD4-4493-A580-6FF11E4925DE"},
            {QStringLiteral("nio"), true}
        }
    };

    // create metadata
    QFETCH(bool, nio);
    QFETCH(bool, nioSwitchedOff);
    QFETCH(QDateTime, dateTime);
    MetaDataWriterCommand writer{path, {
        {QStringLiteral("serialNumber"), 1},
        {QStringLiteral("nio"), nio},
        {QStringLiteral("nioSwitchedOff"), nioSwitchedOff},
        {QStringLiteral("date"), dateTime.toString(Qt::ISODateWithMs)},
        {QStringLiteral("processedSeamSeries"), seriesJson},
        {QStringLiteral("processedSeams"), seamJson}
    }};
    writer.execute();

    auto model = new ProductInstanceTableModel{this};

    QSignalSpy dataChangedSpy(model, &ProductInstanceTableModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy{model, &ProductInstanceTableModel::rowsInserted};
    QVERIFY(rowsInsertedSpy.isValid());

    QSignalSpy columnsInsertedSpy{model, &ProductInstanceTableModel::columnsInserted};
    QVERIFY(columnsInsertedSpy.isValid());

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->columnCount(), 7);

    QTRY_COMPARE(dataChangedSpy.count(), 7);

    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model->index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model->index(0, 0));
    QTEST(dataChangedSpy.first().at(2).value<QVector<int>>(), "expectedRoles");

    for (auto i = 1; i < 7; i++)
    {
        QCOMPARE(dataChangedSpy.at(i).at(0).toModelIndex(), model->index(0, i));
        QCOMPARE(dataChangedSpy.at(i).at(1).toModelIndex(), model->index(0, i));
        QCOMPARE(dataChangedSpy.at(i).at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 2});
    }
    if (dateTime != QDateTime{})
    {
        QCOMPARE(model->index(0, 0).data(Qt::UserRole + 1).toDateTime(), dateTime);
    }
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), nio ? ProductInstanceTableModel::State::Nio : ProductInstanceTableModel::State::Io);
    QCOMPARE(model->index(0, 1).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Io);
    QCOMPARE(model->index(0, 2).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Io);
    QCOMPARE(model->index(0, 3).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Nio);
    QCOMPARE(model->index(0, 4).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Io);
    QCOMPARE(model->index(0, 5).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Nio);
    QCOMPARE(model->index(0, 6).data(Qt::UserRole + 2).value<ProductInstanceTableModel::State>(), ProductInstanceTableModel::State::Io);
    QCOMPARE(model->index(0, 0).data(Qt::UserRole + 3).toBool(), nioSwitchedOff);
}

void TestProductInstanceTableModel::testExtendedProductInfo_data()
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

void TestProductInstanceTableModel::testExtendedProductInfo()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    // and a directory
    QTemporaryDir dir;
    const auto path = dir.path() + QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/");
    QVERIFY(QDir{}.mkpath(path));

    // create metadata
    QFETCH(QString, extendedProductInfo);
    MetaDataWriterCommand writer{path, {
        {QStringLiteral("serialNumber"), 1},
        {QStringLiteral("nio"), false},
        {QStringLiteral("nioSwitchedOff"), false},
        {QStringLiteral("date"), QDateTime::currentDateTime().toString(Qt::ISODateWithMs)},
        {QStringLiteral("processedSeamSeries"), QJsonArray{}},
        {QStringLiteral("processedSeams"), QJsonArray{}},
        {QStringLiteral("extendedProductInfo"), extendedProductInfo}
    }};
    writer.execute();

    auto model = std::make_unique<ProductInstanceTableModel>();
    auto helper = model->extendedProductInfoHelper();
    QFETCH(bool, serialNumber);
    helper->setSerialNumberFromExtendedProductInfo(serialNumber);
    QFETCH(quint32, serialNumberField);
    helper->setSerialNumberFromExtendedProductInfoField(serialNumberField);
    QFETCH(bool, partNumber);
    helper->setPartNumberFromExtendedProductInfo(partNumber);
    QFETCH(quint32, partNumberField);
    helper->setPartNumberFromExtendedProductInfoField(partNumberField);

    QSignalSpy dataChangedSpy(model.get(), &ProductInstanceTableModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    QSignalSpy rowsInsertedSpy{model.get(), &ProductInstanceTableModel::rowsInserted};
    QVERIFY(rowsInsertedSpy.isValid());

    QSignalSpy columnsInsertedSpy{model.get(), &ProductInstanceTableModel::columnsInserted};
    QVERIFY(columnsInsertedSpy.isValid());

    QSignalSpy loadingChangedSpy(model.get(), &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    model->setDirectory(dir.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 1);
    QCOMPARE(model->columnCount(), 7);

    QVERIFY(dataChangedSpy.wait());

    QTEST(model->data(model->index(0, 0), Qt::DisplayRole).toString(), "expectedSerialNumber");
    QTEST(model->data(model->index(0, 0), Qt::UserRole + 11).toString(), "expectedPartNumber");
}

void TestProductInstanceTableModel::testDownloadChecked_data()
{
    QTest::addColumn<int>("cancelMode");
    QTest::newRow("Not canceled") << 0;
    QTest::newRow("Cancel while preparing download") << 1;
    QTest::newRow("Cancel after download has started") << 2;
    QTest::newRow("Cancel after download has ended") << 3;
}

void TestProductInstanceTableModel::testDownloadChecked()
{
    auto dm = precitec::gui::components::removableDevices::DownloadManagerModel::instance();
    QTRY_COMPARE(dm->isEmpty(), true);
    enum CancelMode
    {
        WhilePreparingDownload = 1,
        AfterDownloadHasStarted = 2,
        AfterDownloadHasEnded = 3
    };
    QFETCH(int, cancelMode);
    if (cancelMode == AfterDownloadHasStarted)
    {
        QSKIP("Test is racy on filesystem and fails sporadically on CI system");
        return;
    }

    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());

    auto product = Product::fromJson(fileName);
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));

    const auto& productDir = QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/");
    // the directory with the files to download had been created in initTestCase
    for (int i = 1; i <= 9; i++)
    {
        auto folder = m_sourceFolder.path() + productDir + QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1/").arg(i);
        QFileInfo::exists(folder);
    }
    auto model = new ProductInstanceTableModel{this};

    QSignalSpy loadingChangedSpy(model, &ProductInstanceTableModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ProductInstanceTableModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);
    QCOMPARE(model->columnCount(), 1);

    model->setDirectory(m_sourceFolder.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model->loading(), false);

    QCOMPARE(model->rowCount(), 9);
    QCOMPARE(model->columnCount(), 7);

    model->checkAll();
    QSignalSpy downloadingChangedSpy{model, &ProductInstanceTableModel::downloadingChanged};

    QString downloadDir("results");
    QTemporaryDir targetDir;
    model->downloadChecked(targetDir.path(), downloadDir);
    bool downloadCanceled = false;
    switch(cancelMode)
    {
        case WhilePreparingDownload:
            QTest::qWait(5);
            model->cancelDownload();
            downloadCanceled = true;
            break;
        case AfterDownloadHasStarted:
            QTRY_COMPARE_WITH_TIMEOUT(downloadingChangedSpy.count(), 1, 100);
            QTRY_VERIFY(!dm->isEmpty());
            model->cancelDownload();
            QVERIFY(!dm->isEmpty());
            downloadCanceled = true;
            break;
        case AfterDownloadHasEnded:
            QTRY_COMPARE_WITH_TIMEOUT(downloadingChangedSpy.count(), 2, 2000);
            QVERIFY(dm->isEmpty());
            model->cancelDownload();
            downloadCanceled = false;
        default:
            downloadCanceled = false;
            break;
    }

    QTRY_COMPARE_WITH_TIMEOUT(downloadingChangedSpy.count(), 2, 1500);
    QVERIFY(!model->downloading());
    QTRY_VERIFY(dm->isEmpty());

    //when the download is complete, all items are unchecked
    int missingDirectories = 0;
    int checkedItems = 0;
    auto expectedTargetFolder = targetDir.filePath(QCoreApplication::applicationName() +"/"+ downloadDir+"/"+product->name());
    if (!downloadCanceled)
    {
        QVERIFY(QFileInfo{expectedTargetFolder}.exists());
    }
    for (auto row = 0; row < model->rowCount(); row++)
    {
        const auto subFolder = QStringLiteral("%1F086211-FBD4-4493-A580-6FF11E4925DE-SN-%1").arg(row + 1);
        auto targetFullPath = expectedTargetFolder+"/"+ subFolder;
        if (model->index(row, 0).data(Qt::UserRole + 7).toBool())
        {
            checkedItems++;
        }
        if (!QFileInfo{targetFullPath}.exists())
        {
            missingDirectories++;
        }
    }

    if (!downloadCanceled)
    {
        QCOMPARE(checkedItems, 0);
        QCOMPARE(missingDirectories, 0);
    }
    else
    {
        QVERIFY(missingDirectories > 0);
    }

    QVERIFY(dm->isEmpty());
    delete model;
    QVERIFY(targetDir.remove());

}
QTEST_GUILESS_MAIN(TestProductInstanceTableModel)
#include "testProductInstanceTableModel.moc"
