#include <QTest>
#include <QSignalSpy>
#include <QFileInfo>
#include <QDir>
#include <QSortFilterProxyModel>

#include "../src/metaDataWriterCommand.h"
#include "../src/product.h"
#include "../src/productInstanceModel.h"
#include "../src/extendedProductInfoHelper.h"
#include "precitec/downloadService.h"
#include "precitec/downloadManagerModel.h"

using precitec::storage::MetaDataWriterCommand;
using precitec::storage::Product;
using precitec::storage::ProductInstanceModel;
using precitec::gui::components::removableDevices::DownloadService;

class TestProductInstanceModel : public QObject
{
    Q_OBJECT
private:
    //temporary files for testing the download
    QTemporaryDir m_sourceFolder;
private Q_SLOTS:
    void initTestCase();
    void cleanupTestCase();
    void testCtor();
    void testSetProduct();
    void testSetDirectory();
    void testSetProductDirectoryName();
    void testLoading_data();
    void testLoading();
    void testLoadMetaData_data();
    void testLoadMetaData();
    void testExtendedProductInfo_data();
    void testExtendedProductInfo();
    void testMonitoring();
    void testDirectoryMonitoring_data();
    void testDirectoryMonitoring();
    void testDownloadChecked_data();
    void testDownloadChecked();
};

void TestProductInstanceModel::initTestCase()
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

void TestProductInstanceModel::cleanupTestCase()
{
    QVERIFY(m_sourceFolder.remove());
}

void TestProductInstanceModel::testCtor()
{
    ProductInstanceModel model;
    QCOMPARE(model.directory(), QString());
    QCOMPARE(model.rowCount(), 0);
    QVERIFY(!model.product());
    QCOMPARE(model.isLoading(), false);
    QCOMPARE(model.stationName(), QString{});
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.count(), 12);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("fileInfo"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("date"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("nio"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("nioResultsSwitchedOff"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("relativePath"));
    QCOMPARE(roleNames[Qt::UserRole + 5], QByteArrayLiteral("downloadService"));
    QCOMPARE(roleNames[Qt::UserRole + 6], QByteArrayLiteral("path"));
    QCOMPARE(roleNames[Qt::UserRole + 7], QByteArrayLiteral("checked"));
    QCOMPARE(roleNames[Qt::UserRole + 8], QByteArrayLiteral("directoryName"));
    QCOMPARE(roleNames[Qt::UserRole + 9], QByteArrayLiteral("uuid"));
    QCOMPARE(roleNames[Qt::UserRole + 11], QByteArrayLiteral("partNumber"));
    QCOMPARE(model.productDirectoryName(), ProductInstanceModel::ProductDirectoryName::Uuid);
}

void TestProductInstanceModel::testSetProduct()
{
    ProductInstanceModel model;
    QSignalSpy productChangedSpy(&model, &ProductInstanceModel::productChanged);
    QVERIFY(productChangedSpy.isValid());
    QSignalSpy loadingChangedSpy(&model, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());
    QVERIFY(!model.product());
    // setting same (null) product should not emit signal
    model.setProduct(nullptr);
    QVERIFY(!model.product());
    QCOMPARE(productChangedSpy.count(), 0);

    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);

    model.setProduct(product.get());
    QCOMPARE(model.product(), product.get());
    QCOMPARE(productChangedSpy.count(), 1);

    // setting same should not emit signal
    model.setProduct(product.get());
    QCOMPARE(productChangedSpy.count(), 1);

    // deleting the Product should emit signal
    product.reset();
    QCOMPARE(productChangedSpy.count(), 2);
    QVERIFY(!model.product());

    // loading should not have been started as we did not have a directory
    QCOMPARE(loadingChangedSpy.count(), 0);
}

void TestProductInstanceModel::testSetDirectory()
{
    ProductInstanceModel model;
    QSignalSpy directoryChangedSpy(&model, &ProductInstanceModel::directoryChanged);
    QVERIFY(directoryChangedSpy.isValid());
    QSignalSpy loadingChangedSpy(&model, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());
    QCOMPARE(model.directory(), QString());

    // setting to same directory should not emit signal
    model.setDirectory(QString());
    QCOMPARE(directoryChangedSpy.count(), 0);

    // set a proper directory
    model.setDirectory(QStringLiteral("foo"));
    QCOMPARE(model.directory(), QStringLiteral("foo"));
    QCOMPARE(directoryChangedSpy.count(), 1);

    // setting same should not change
    model.setDirectory(QStringLiteral("foo"));
    QCOMPARE(directoryChangedSpy.count(), 1);

    // and we should not have any loading as we did not have a product
    QCOMPARE(loadingChangedSpy.count(), 0);
}

void TestProductInstanceModel::testSetProductDirectoryName()
{
    ProductInstanceModel model;
    QSignalSpy productDirectoryNameChangedSpy(&model, &ProductInstanceModel::productDirectoryNameChanged);
    QVERIFY(productDirectoryNameChangedSpy.isValid());
    QCOMPARE(model.productDirectoryName(), ProductInstanceModel::ProductDirectoryName::Uuid);
    // setting same should not change
    model.setProductDirectoryName(ProductInstanceModel::ProductDirectoryName::Uuid);
    QCOMPARE(model.productDirectoryName(), ProductInstanceModel::ProductDirectoryName::Uuid);
    QCOMPARE(productDirectoryNameChangedSpy.count(), 0);
    // setting other should change
    model.setProductDirectoryName(ProductInstanceModel::ProductDirectoryName::ProductName);
    QCOMPARE(model.productDirectoryName(), ProductInstanceModel::ProductDirectoryName::ProductName);
    QCOMPARE(productDirectoryNameChangedSpy.count(), 1);
    // setting same should not change
    model.setProductDirectoryName(ProductInstanceModel::ProductDirectoryName::ProductName);
    QCOMPARE(productDirectoryNameChangedSpy.count(), 1);
    // back to start
    model.setProductDirectoryName(ProductInstanceModel::ProductDirectoryName::Uuid);
    QCOMPARE(model.productDirectoryName(), ProductInstanceModel::ProductDirectoryName::Uuid);
    QCOMPARE(productDirectoryNameChangedSpy.count(), 2);
}

void TestProductInstanceModel::testLoading_data()
{
    QTest::addColumn<QString>("productDir");
    QTest::addColumn<ProductInstanceModel::ProductDirectoryName>("productDirectoryName");

    QTest::newRow("uuid") << QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/") << ProductInstanceModel::ProductDirectoryName::Uuid;
    QTest::newRow("name") << QStringLiteral("/FooBar/") << ProductInstanceModel::ProductDirectoryName::ProductName;
}

void TestProductInstanceModel::testLoading()
{
    // first create some mock data
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
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

    ProductInstanceModel model;
    QSignalSpy rowsRemovedSpy{&model, &QAbstractItemModel::rowsRemoved};
    QVERIFY(rowsRemovedSpy.isValid());

    QSignalSpy dataChangedSpy{&model, &QAbstractItemModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());
    model.setStationName(QStringLiteral("Test"));
    model.setStationName({});
    QVERIFY(dataChangedSpy.isEmpty());

    QFETCH(ProductInstanceModel::ProductDirectoryName, productDirectoryName);
    model.setProductDirectoryName(productDirectoryName);
    qRegisterMetaType<QFileInfo>();
    QSignalSpy loadingChangedSpy(&model, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());
    QSignalSpy rowInsertedSpy(&model, &ProductInstanceModel::rowsInserted);
    QVERIFY(rowInsertedSpy.isValid());

    model.setDirectory(dir.path());
    model.setProduct(product.get());
    if (model.isLoading())
    {
        QCOMPARE(loadingChangedSpy.count(), 1);
        QVERIFY(loadingChangedSpy.wait());
    }
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(model.isLoading(), false);

    // now compare what we have
    QTRY_COMPARE(rowInsertedSpy.count(), 1);

    QCOMPARE(model.rowCount(), 9);
    QCOMPARE(model.rowCount(model.index(0, 0)), 0);

    QSortFilterProxyModel sortModel;
    sortModel.setSourceModel(&model);
    sortModel.setSortRole(Qt::DisplayRole);
    sortModel.sort(0);

    QCOMPARE(sortModel.index(0, 0).data().toString(), QStringLiteral("1"));
    QCOMPARE(sortModel.index(1, 0).data().toString(), QStringLiteral("2"));
    QCOMPARE(sortModel.index(2, 0).data().toString(), QStringLiteral("3"));
    QCOMPARE(sortModel.index(3, 0).data().toString(), QStringLiteral("4"));
    QCOMPARE(sortModel.index(4, 0).data().toString(), QStringLiteral("5"));
    QCOMPARE(sortModel.index(5, 0).data().toString(), QStringLiteral("6"));
    QCOMPARE(sortModel.index(6, 0).data().toString(), QStringLiteral("7"));
    QCOMPARE(sortModel.index(7, 0).data().toString(), QStringLiteral("8"));
    QCOMPARE(sortModel.index(8, 0).data().toString(), QStringLiteral("9"));
    QCOMPARE(sortModel.index(9, 0).data(), QVariant());

    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole).value<QFileInfo>(), QFileInfo(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QCOMPARE(sortModel.index(8, 0).data(Qt::UserRole).value<QFileInfo>(), QFileInfo(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")));

    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 1).toDateTime(), QFileInfo(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")).lastModified());
    QCOMPARE(sortModel.index(8, 0).data(Qt::UserRole + 1).toDateTime(), QFileInfo(dir.path() + productDir + QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9/")).lastModified());

    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 2).value<ProductInstanceModel::State>(), ProductInstanceModel::State::Unknown);
    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 3).toBool(), false);

    productDir.remove(0, 1);
    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1"));
    QCOMPARE(sortModel.index(1, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2"));
    QCOMPARE(sortModel.index(2, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3"));
    QCOMPARE(sortModel.index(3, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4"));
    QCOMPARE(sortModel.index(4, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5"));
    QCOMPARE(sortModel.index(5, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6"));
    QCOMPARE(sortModel.index(6, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7"));
    QCOMPARE(sortModel.index(7, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8"));
    QCOMPARE(sortModel.index(8, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("FooBar/9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9"));

    QVERIFY(sortModel.index(0, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(1, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(2, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(3, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(4, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(5, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(6, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(7, 0).data(Qt::UserRole + 5).value<DownloadService*>());
    QVERIFY(sortModel.index(8, 0).data(Qt::UserRole + 5).value<DownloadService*>());

    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 6).toString(), dir.path() + QStringLiteral("/") + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1"));

    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1"));
    QCOMPARE(sortModel.index(1, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2"));
    QCOMPARE(sortModel.index(2, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3"));
    QCOMPARE(sortModel.index(3, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4"));
    QCOMPARE(sortModel.index(4, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5"));
    QCOMPARE(sortModel.index(5, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6"));
    QCOMPARE(sortModel.index(6, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7"));
    QCOMPARE(sortModel.index(7, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8"));
    QCOMPARE(sortModel.index(8, 0).data(Qt::UserRole + 8).toString(), QStringLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9"));

    QSignalSpy stationNameChangedSpy{&model, &ProductInstanceModel::stationNameChanged};
    QVERIFY(stationNameChangedSpy.isValid());
    model.setStationName(QStringLiteral("test"));
    QCOMPARE(model.stationName(), QStringLiteral("test"));
    QCOMPARE(stationNameChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(8, 0));
    QCOMPARE(dataChangedSpy.first().at(2).value<QVector<int>>(), QVector<int>() << Qt::UserRole + 4);
    // setting same name should not emit signals
    model.setStationName(QStringLiteral("test"));
    QCOMPARE(stationNameChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(sortModel.index(0, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1"));
    QCOMPARE(sortModel.index(1, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/2F086211-FBD4-4493-A580-6FF11E4925DE-SN-2"));
    QCOMPARE(sortModel.index(2, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/3F086211-FBD4-4493-A580-6FF11E4925DE-SN-3"));
    QCOMPARE(sortModel.index(3, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/4F086211-FBD4-4493-A580-6FF11E4925DE-SN-4"));
    QCOMPARE(sortModel.index(4, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5"));
    QCOMPARE(sortModel.index(5, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/6F086211-FBD4-4493-A580-6FF11E4925DE-SN-6"));
    QCOMPARE(sortModel.index(6, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/7F086211-FBD4-4493-A580-6FF11E4925DE-SN-7"));
    QCOMPARE(sortModel.index(7, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/8F086211-FBD4-4493-A580-6FF11E4925DE-SN-8"));
    QCOMPARE(sortModel.index(8, 0).data(Qt::UserRole + 4).toString(), QStringLiteral("test/FooBar/9F086211-FBD4-4493-A580-6FF11E4925DE-SN-9"));

    // test checked role
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(1, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(2, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(3, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(4, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(5, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(6, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(7, 0).data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(model.index(8, 0).data(Qt::UserRole + 7).toBool(), false);

    dataChangedSpy.clear();
    auto index4 = model.index(4, 0);
    model.toggleChecked(4);
    QCOMPARE(index4.data(Qt::UserRole + 7).toBool(), true);
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), index4);
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), index4);
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 7});

    model.toggleChecked(4);
    QCOMPARE(index4.data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(dataChangedSpy.count(), 2);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), index4);
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), index4);
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 7});

    // with invalid index
    model.toggleChecked(-1);
    QCOMPARE(dataChangedSpy.count(), 2);

    // test uncheck all
    model.toggleChecked(4);
    QCOMPARE(index4.data(Qt::UserRole + 7).toBool(), true);
    QCOMPARE(dataChangedSpy.count(), 3);
    // now uncheck
    model.uncheckAll();
    QCOMPARE(index4.data(Qt::UserRole + 7).toBool(), false);
    QCOMPARE(dataChangedSpy.count(), 4);
    QCOMPARE(dataChangedSpy.last().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.last().at(1).toModelIndex(), model.index(model.rowCount() - 1, 0));
    QCOMPARE(dataChangedSpy.last().at(2).value<QVector<int>>(), QVector<int>{Qt::UserRole + 7});

    // test delete of one item
    QVERIFY(rowsRemovedSpy.isEmpty());
    int row = sortModel.mapToSource(sortModel.index(4, 0)).row();
    QVERIFY(QDir{dir.path() + QStringLiteral("/") + productDir + QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5/")}.removeRecursively());
    QVERIFY(!QDir{dir.path() + QStringLiteral("/") + productDir}.exists(QStringLiteral("5F086211-FBD4-4493-A580-6FF11E4925DE-SN-5")));
    model.setMonitoring(true);
    QTRY_COMPARE(rowsRemovedSpy.count(), 1);
    QCOMPARE(rowsRemovedSpy.last().at(0).toModelIndex(), QModelIndex{});
    QCOMPARE(rowsRemovedSpy.last().at(1).toInt(), row);
    QCOMPARE(rowsRemovedSpy.last().at(2).toInt(), row);
}


void TestProductInstanceModel::testLoadMetaData_data()
{
    QTest::addColumn<bool>("nio");
    QTest::addColumn<bool>("nioSwitchedOff");
    QTest::addColumn<QDateTime>("dateTime");
    QTest::addColumn<QVector<int>>("expectedRoles");

    QTest::newRow("io|nioon") << false << false << QDateTime() << QVector<int>{Qt::UserRole + 2};
    QTest::newRow("nio|niooff") << true << true << QDateTime() << QVector<int>{Qt::UserRole + 2, Qt::UserRole + 3};
    QTest::newRow("time") << false << false << QDateTime::currentDateTime() << QVector<int>{Qt::UserRole + 1, Qt::UserRole + 2};
}

void TestProductInstanceModel::testLoadMetaData()
{
    // first create some mock data
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));
    // and a directory
    QTemporaryDir dir;
    const auto path = dir.path() + QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/");
    QVERIFY(QDir{}.mkpath(path));

    // create metadata
    QFETCH(bool, nio);
    QFETCH(bool, nioSwitchedOff);
    QFETCH(QDateTime, dateTime);
    MetaDataWriterCommand writer{path, {
        {QStringLiteral("nio"), nio},
        {QStringLiteral("nioSwitchedOff"), nioSwitchedOff},
        {QStringLiteral("date"), dateTime.toString(Qt::ISODateWithMs)}
    }};
    writer.execute();

    ProductInstanceModel model;
    QSignalSpy dataChangedSpy(&model, &ProductInstanceModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy rowsInsertedSpy{&model, &ProductInstanceModel::rowsInserted};
    QVERIFY(rowsInsertedSpy.isValid());
    model.setDirectory(dir.path());
    model.setProduct(product.get());
    QVERIFY(rowsInsertedSpy.wait());

    model.ensureMetaDataLoaded(0);
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(0, 0));
    QTEST(dataChangedSpy.first().at(2).value<QVector<int>>(), "expectedRoles");
    if (dateTime != QDateTime{})
    {
        QCOMPARE(model.index(0, 0).data(Qt::UserRole + 1).toDateTime(), dateTime);
    }
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 2).value<ProductInstanceModel::State>(), nio ? ProductInstanceModel::State::Nio : ProductInstanceModel::State::Io);
    QCOMPARE(model.index(0, 0).data(Qt::UserRole + 3).toBool(), nioSwitchedOff);
}

void TestProductInstanceModel::testExtendedProductInfo_data()
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

void TestProductInstanceModel::testExtendedProductInfo()
{
    // first create some mock data
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));
    // and a directory
    QTemporaryDir dir;
    const auto path = dir.path() + QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/");
    QVERIFY(QDir{}.mkpath(path));

    // create metadata
    QFETCH(QString, extendedProductInfo);
    MetaDataWriterCommand writer{path, {
        {QStringLiteral("nio"), false},
        {QStringLiteral("nioSwitchedOff"), false},
        {QStringLiteral("date"), QDateTime{}.toString(Qt::ISODateWithMs)},
        {QStringLiteral("extendedProductInfo"), extendedProductInfo}
    }};
    writer.execute();

    ProductInstanceModel model;
    auto helper = model.extendedProductInfoHelper();
    QFETCH(bool, serialNumber);
    helper->setSerialNumberFromExtendedProductInfo(serialNumber);
    QFETCH(quint32, serialNumberField);
    helper->setSerialNumberFromExtendedProductInfoField(serialNumberField);
    QFETCH(bool, partNumber);
    helper->setPartNumberFromExtendedProductInfo(partNumber);
    QFETCH(quint32, partNumberField);
    helper->setPartNumberFromExtendedProductInfoField(partNumberField);

    QSignalSpy dataChangedSpy(&model, &ProductInstanceModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy rowsInsertedSpy{&model, &ProductInstanceModel::rowsInserted};
    QVERIFY(rowsInsertedSpy.isValid());
    model.setDirectory(dir.path());
    model.setProduct(product.get());
    QVERIFY(rowsInsertedSpy.wait());

    model.ensureMetaDataLoaded(0);
    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.first().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.first().at(1).toModelIndex(), model.index(0, 0));
    QTEST(model.data(model.index(0, 0), Qt::DisplayRole).toString(), "expectedSerialNumber");
    QTEST(model.data(model.index(0, 0), Qt::UserRole + 11).toString(), "expectedPartNumber");
}

void TestProductInstanceModel::testMonitoring()
{
    ProductInstanceModel model;
    QSignalSpy monitoringChangedSpy(&model, &ProductInstanceModel::monitoringChanged);
    QVERIFY(monitoringChangedSpy.isValid());

    QCOMPARE(model.isMonitoring(), false);
    QCOMPARE(model.property("monitoring").toBool(), false);

    model.setMonitoring(true);
    QCOMPARE(model.isMonitoring(), true);
    QCOMPARE(model.property("monitoring").toBool(), true);
    QCOMPARE(monitoringChangedSpy.count(), 1);

    // setting again should not change
    model.setMonitoring(true);
    QCOMPARE(monitoringChangedSpy.count(), 1);

    // and back to false
    model.setMonitoring(false);
    QCOMPARE(model.isMonitoring(), false);
    QCOMPARE(model.property("monitoring").toBool(), false);
    QCOMPARE(monitoringChangedSpy.count(), 2);
}

void TestProductInstanceModel::testDirectoryMonitoring_data()
{
    QTest::addColumn<QString>("productDir");
    QTest::addColumn<ProductInstanceModel::ProductDirectoryName>("productDirectoryName");

    QTest::newRow("uuid") << QStringLiteral("/1f086211-fbd4-4493-a580-6ff11e4925de/") << ProductInstanceModel::ProductDirectoryName::Uuid;
    QTest::newRow("name") << QStringLiteral("/FooBar/") << ProductInstanceModel::ProductDirectoryName::ProductName;
}

void TestProductInstanceModel::testDirectoryMonitoring()
{
    // create a Product
    const QString fileName = QFINDTESTDATA("testdata/products/seamuuid.json");
    QVERIFY(!fileName.isEmpty());
    std::unique_ptr<Product> product{Product::fromJson(fileName)};
    QVERIFY(product);
    QCOMPARE(product->uuid().toString(), QStringLiteral("{1f086211-fbd4-4493-a580-6ff11e4925de}"));
    // and a directory
    QTemporaryDir dir;
    QFETCH(QString, productDir);
    QVERIFY(QDir{}.mkpath(dir.path() + productDir));

    ProductInstanceModel model;
    QFETCH(ProductInstanceModel::ProductDirectoryName, productDirectoryName);
    model.setProductDirectoryName(productDirectoryName);
    QSignalSpy loadingChangedSpy(&model, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());
    QSignalSpy rowInsertedSpy(&model, &ProductInstanceModel::rowsInserted);
    QVERIFY(rowInsertedSpy.isValid());

    model.setMonitoring(true);
    model.setDirectory(dir.path());
    model.setProduct(product.get());

    if (model.isLoading())
    {
        QCOMPARE(loadingChangedSpy.count(), 1);
        QVERIFY(loadingChangedSpy.wait());
    }
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(rowInsertedSpy.count(), 0);
    QCOMPARE(model.rowCount(), 0);

    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral(".1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1/")));
    QVERIFY(!rowInsertedSpy.wait());
    QCOMPARE(rowInsertedSpy.count(), 0);
    QCOMPARE(model.rowCount(), 0);

    QVERIFY(QDir{dir.path() + productDir}.rename(QStringLiteral(".1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1"), QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DE-SN-1")));
    QVERIFY(rowInsertedSpy.wait());
    QCOMPARE(rowInsertedSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);

    model.setMonitoring(false);
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DF-SN-2/")));
    QVERIFY(!rowInsertedSpy.wait(200));

    model.setMonitoring(true);
    QVERIFY(rowInsertedSpy.wait());
    QCOMPARE(rowInsertedSpy.count(), 2);
    QCOMPARE(model.rowCount(), 2);

    // change product
    loadingChangedSpy.clear();
    Product product2{QUuid::createUuid()};
    model.setProduct(&product2);
    QCOMPARE(model.rowCount(), 0);
    if (model.isLoading())
    {
        QCOMPARE(loadingChangedSpy.count(), 1);
        QVERIFY(loadingChangedSpy.wait());
    }
    QCOMPARE(loadingChangedSpy.count(), 2);

    // don't crash on enable monitoring with null product
    QTRY_COMPARE(model.isLoading(), false);
    loadingChangedSpy.clear();
    model.setProduct(product.get());
    if (model.isLoading())
    {
        QCOMPARE(loadingChangedSpy.count(), 1);
        QVERIFY(loadingChangedSpy.wait());
    }
    QCOMPARE(loadingChangedSpy.count(), 2);

    model.setMonitoring(false);
    QVERIFY(QDir{}.mkpath(dir.path() + productDir + QStringLiteral("1F086211-FBD4-4493-A580-6FF11E4925DF-SN-3/")));
    // give file system watcher some time to notify new product instance
    QTest::qWait(200);
    // now set product to null and re-enable monitoring, this resulted in crash
    model.setProduct(nullptr);
    QTRY_COMPARE(model.isLoading(), false);
    model.setMonitoring(true);
}

void TestProductInstanceModel::testDownloadChecked_data()
{
    QTest::addColumn<int>("cancelMode");
    QTest::newRow("Not canceled") << 0;
    QTest::newRow("Cancel while preparing download") << 1;
    QTest::newRow("Cancel after download has started") << 2;
    QTest::newRow("Cancel after download has ended") << 3;
}


void TestProductInstanceModel::testDownloadChecked()
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
    auto model = new ProductInstanceModel{this};

    QSignalSpy loadingChangedSpy(model, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());

    QSignalSpy dataChangedSpy{model, &ProductInstanceModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    QCOMPARE(model->rowCount(), 0);

    model->setDirectory(m_sourceFolder.path());
    model->setProduct(product);

    QCOMPARE(loadingChangedSpy.count(), 1);
    QVERIFY(loadingChangedSpy.wait());

    QCOMPARE(loadingChangedSpy.count(), 2);

    QCOMPARE(model->rowCount(), 9);

    model->checkAll();
    QSignalSpy downloadingChangedSpy{model, &ProductInstanceModel::downloadingChanged};

    QString downloadDir("video");
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
            model->cancelDownload();
            downloadCanceled = true;
            QVERIFY(!dm->isEmpty());
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

    QTRY_COMPARE(downloadingChangedSpy.count(), 2);
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
QTEST_GUILESS_MAIN(TestProductInstanceModel)
#include "testProductInstanceModel.moc"
