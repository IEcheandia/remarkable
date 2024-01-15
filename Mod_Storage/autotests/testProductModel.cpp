#include <QTest>
#include <QSignalSpy>
#include <QDir>
#include <QSaveFile>

#include "../src/product.h"
#include "../src/productModel.h"
#include "../src/abstractMeasureTask.h"

using precitec::storage::ProductModel;
using precitec::storage::Product;

class TestProductModel : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testRoles();
    void testLoadEmptyFolder();
    void testLoadWithProducts();
    void testProductFileChanges();
    void testNewProducts();
    void testReloadProduct();
    void testSetReferenceCurveStorageDir();
};

void TestProductModel::testRoles()
{
    ProductModel model;
    const auto roles = model.roleNames();
    QCOMPARE(roles.count(), 3);
    QCOMPARE(roles.value(Qt::DisplayRole), QByteArrayLiteral("display"));
    QCOMPARE(roles.value(Qt::UserRole), QByteArrayLiteral("uuid"));
    QCOMPARE(roles.value(Qt::UserRole+1), QByteArrayLiteral("product"));
}

void TestProductModel::testLoadEmptyFolder()
{
    ProductModel model;
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QDir dir(QFINDTESTDATA("testdata"));
    QVERIFY(dir.exists());
    model.loadProducts(dir);
    QCOMPARE(model.productStorageDirectory(), dir);
    QCOMPARE(model.referenceCurveStorageDirectory(), QStringLiteral(""));
    QCOMPARE(modelResetSpy.count(), 1);
    QCOMPARE(model.rowCount(), 0);
}

void TestProductModel::testLoadWithProducts()
{
    ProductModel model;
    QSignalSpy modelResetSpy(&model, &QAbstractItemModel::modelReset);
    QVERIFY(modelResetSpy.isValid());
    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());
    QDir referenceDir(QFINDTESTDATA("testdata/reference_curves/"));
    QVERIFY(referenceDir.exists());
    QVERIFY(!model.defaultProduct());
    model.setReferenceStorageDirectory(referenceDir.absolutePath());
    model.loadProducts(dir);
    QCOMPARE(modelResetSpy.count(), 1);

    QCOMPARE(model.rowCount(), 30);
    QCOMPARE(model.uuid(model.index(0, 0)), QUuid(QByteArrayLiteral("1F086211-FBD4-4493-A580-6FF11E4925DD")));
    QCOMPARE(model.uuid(model.index(28, 0)), QUuid(QByteArrayLiteral("59D29F32-F1C9-4BD7-A0C8-D1E1493C1CF6")));
    QCOMPARE(model.data(model.index(29, 0), Qt::DisplayRole).toString(), QStringLiteral("TriggerNotObject"));
    QCOMPARE(model.data(model.index(30, 0), Qt::DisplayRole).isValid(), false);

    QCOMPARE(model.findProduct(QUuid(QByteArrayLiteral("59D29F32-F1C9-4BD7-A0C8-D1E1493C1CF6"))), model.products().at(28));
    QVERIFY(!model.findProduct(QUuid::createUuid()));
    QCOMPARE(model.data(model.index(0, 0), Qt::UserRole + 1).value<precitec::storage::Product*>(), model.products().front());
    QVERIFY(model.defaultProduct());
    QCOMPARE(model.defaultProduct()->isDefaultProduct(), true);

    QVERIFY(model.findFilterParameterSet(QUuid(QByteArrayLiteral("6F086211-FBD4-4493-A580-6FF11E4925DD"))));
    QVERIFY(!model.findFilterParameterSet(QUuid(QByteArrayLiteral("9F086211-FBD4-4493-A580-6FF11E4925DE"))));

    QVERIFY(!model.findMeasureTask(QUuid::createUuid()));
    auto measureTask = model.findMeasureTask({QByteArrayLiteral("AF086211-FBD4-4493-A580-6FF11E4925DE")});
    QVERIFY(measureTask);
    QCOMPARE(measureTask->number(), 2);

    for (auto i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.data(model.index(i, 0), Qt::UserRole + 1).value<precitec::storage::Product*>()->referenceCruveStorageDir(), referenceDir.absolutePath());
    }

    // and load from another directory
    QDir dir2(QFINDTESTDATA("testdata"));
    QVERIFY(dir2.exists());
    model.loadProducts(dir2);
    QCOMPARE(model.rowCount(), 0);
}

void TestProductModel::testProductFileChanges()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/product.json"), dir.filePath(QStringLiteral("product.json"))));

    ProductModel model;
    model.loadProducts(QDir(dir.path()));
    QCOMPARE(model.rowCount(), 1);
    QSignalSpy dataChangedSpy(&model, &ProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());
    QSignalSpy rowsRemovedSpy(&model, &ProductModel::rowsRemoved);
    QVERIFY(rowsRemovedSpy.isValid());

    // let's save some other product into the file
    QScopedPointer<Product> product{Product::fromJson(QFINDTESTDATA("testdata/products/seamuuid.json"))};
    QVERIFY(!product.isNull());
    QVERIFY(model.uuid(model.index(0, 0)) != product->uuid());

    QSaveFile saveFile{dir.filePath(QStringLiteral("product.json"))};
    QVERIFY(saveFile.open(QIODevice::WriteOnly));
    product->toJson(&saveFile);
    QVERIFY(saveFile.commit());

    QVERIFY(dataChangedSpy.wait());
    QCOMPARE(model.uuid(model.index(0, 0)), product->uuid());

    QCOMPARE(rowsRemovedSpy.count(), 0);
    QVERIFY(QFile::remove(dir.filePath(QStringLiteral("product.json"))));
    QVERIFY(rowsRemovedSpy.wait());
    QCOMPARE(rowsRemovedSpy.count(), 1);
}

void TestProductModel::testNewProducts()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    ProductModel model;
    model.loadProducts(QDir(dir.path()));
    QCOMPARE(model.rowCount(), 0);
    QSignalSpy rowsInsertedSpy(&model, &ProductModel::rowsInserted);
    QVERIFY(rowsInsertedSpy.isValid());

    Product p{QUuid::createUuid()};
    p.setFilePath(dir.filePath(QStringLiteral("foo.json")));
    QVERIFY(p.save());

    QVERIFY(rowsInsertedSpy.wait());
    QCOMPARE(rowsInsertedSpy.count(), 1);
    QCOMPARE(model.rowCount(), 1);
    QCOMPARE(model.uuid(model.index(0, 0)), p.uuid());

    // saving product again should not trigger the changed signal
    QVERIFY(!rowsInsertedSpy.wait(200));

    // a non json file should not trigger
    p.setFilePath(dir.filePath(QStringLiteral("foo.bar")));
    QVERIFY(p.save());
    QVERIFY(!rowsInsertedSpy.wait(200));
}

void TestProductModel::testReloadProduct()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/products/product.json"), dir.filePath(QStringLiteral("product.json"))));

    ProductModel model;
    model.loadProducts(QDir(dir.path()));
    QCOMPARE(model.rowCount(), 1);
    QSignalSpy dataChangedSpy(&model, &ProductModel::dataChanged);
    QVERIFY(dataChangedSpy.isValid());

    auto p = model.products().front();
    QVERIFY(p);
    QSignalSpy productDeletedSpy{p, &QObject::destroyed};
    QVERIFY(productDeletedSpy.isValid());

    model.reloadProduct(p->uuid());
    QCOMPARE(dataChangedSpy.count(), 1);
    QCOMPARE(dataChangedSpy.front().at(0).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.front().at(1).toModelIndex(), model.index(0, 0));
    QCOMPARE(dataChangedSpy.front().at(2).value<QVector<int>>(), QVector<int>());
    QVERIFY(model.products().front() != p);
    QCOMPARE(model.products().front()->uuid(), p->uuid());

    // existing product should be deleted
    QVERIFY(productDeletedSpy.wait());

    // try reloading not existing product
    model.reloadProduct(QUuid::createUuid());
    QCOMPARE(dataChangedSpy.count(), 1);
}

void TestProductModel::testSetReferenceCurveStorageDir()
{
    ProductModel model;
    QCOMPARE(model.referenceCurveStorageDirectory(), QStringLiteral(""));

    QDir dir(QFINDTESTDATA("testdata/products/"));
    QVERIFY(dir.exists());

    QDir referenceDir(QFINDTESTDATA("testdata/reference_curves/"));
    QVERIFY(referenceDir.exists());
    model.loadProducts(dir);
    QCOMPARE(model.referenceCurveStorageDirectory(), QStringLiteral(""));

    for (auto i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.data(model.index(i, 0), Qt::UserRole + 1).value<precitec::storage::Product*>()->referenceCruveStorageDir(), QString());
    }

    QCOMPARE(model.referenceCurveStorageDirectory(), QStringLiteral(""));
    model.setReferenceStorageDirectory(referenceDir.absolutePath());

    for (auto i = 0; i < model.rowCount(); i++)
    {
        QCOMPARE(model.data(model.index(i, 0), Qt::UserRole + 1).value<precitec::storage::Product*>()->referenceCruveStorageDir(), referenceDir.absolutePath());
    }
}


QTEST_GUILESS_MAIN(TestProductModel)
#include "testProductModel.moc"
