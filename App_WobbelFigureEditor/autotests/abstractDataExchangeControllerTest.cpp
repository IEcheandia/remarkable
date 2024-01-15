#include <QTest>
#include <QSignalSpy>

#include "../src/abstractDataExchangeController.h"
#include "../../Mod_Storage/src/productModel.h"
#include "../../Mod_Storage/src/product.h"
#include "../../Mod_Storage/src/seam.h"
#include "../../Mod_Storage/src/seamSeries.h"

using precitec::scanmaster::components::wobbleFigureEditor::AbstractDataExchangeController;
using precitec::storage::ProductModel;
using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

class AbstractDataExchangeControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testProductModel();
    void testProductModelIndex();
    void testProduct();
    void testSeams();
    void testSeamListIndex();
    void testSeam();
    void testSetObjects();

private:
    QTemporaryDir m_dir;
};

void AbstractDataExchangeControllerTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void AbstractDataExchangeControllerTest::testCtor()
{
    AbstractDataExchangeController dataExchangeController;

    QCOMPARE(dataExchangeController.productModel(), nullptr);
    QCOMPARE(dataExchangeController.productModelIndex(), QModelIndex());
    QCOMPARE(dataExchangeController.product(), nullptr);
    QVERIFY(dataExchangeController.seams().isEmpty());
    QCOMPARE(dataExchangeController.seamListIndex(), -1);
    QCOMPARE(dataExchangeController.seam(), nullptr);
}

void AbstractDataExchangeControllerTest::testProductModel()
{
    AbstractDataExchangeController dataExchangeController;
    ProductModel productModel;

    QSignalSpy productModelChanged{&dataExchangeController, &AbstractDataExchangeController::productModelChanged};
    QVERIFY(productModelChanged.isValid());
    QCOMPARE(productModelChanged.count(), 0);

    dataExchangeController.setProductModel(&productModel);
    QCOMPARE(dataExchangeController.productModel(), &productModel);
    QCOMPARE(productModelChanged.count(), 1);

    dataExchangeController.setProductModel(&productModel);
    QCOMPARE(productModelChanged.count(), 1);

    dataExchangeController.setProductModel(nullptr);
    QCOMPARE(productModelChanged.count(), 2);
    QCOMPARE(dataExchangeController.productModel(), nullptr);
}

void AbstractDataExchangeControllerTest::testProductModelIndex()
{
    QDir dir(QFINDTESTDATA("../../Mod_Storage/autotests/testdata/products/"));
    QVERIFY(dir.exists());

    AbstractDataExchangeController dataExchangeController;
    ProductModel productModel;

    productModel.loadProducts(dir);
    QCOMPARE(productModel.rowCount(), 30);

    QSignalSpy productModelIndexChanged{&dataExchangeController, &AbstractDataExchangeController::productModelIndexChanged};
    QVERIFY(productModelIndexChanged.isValid());
    QCOMPARE(productModelIndexChanged.count(), 0);

    dataExchangeController.setProductModelIndex(productModel.index(1, 0));
    QCOMPARE(dataExchangeController.productModelIndex(), productModel.index(1, 0));
    QCOMPARE(productModelIndexChanged.count(), 1);
}

void AbstractDataExchangeControllerTest::testProduct()
{
    AbstractDataExchangeController dataExchangeController;
    Product product(QUuid::createUuid());

    QSignalSpy productChanged{&dataExchangeController, &AbstractDataExchangeController::productChanged};
    QVERIFY(productChanged.isValid());
    QCOMPARE(productChanged.count(), 0);

    dataExchangeController.setProduct(&product);
    QCOMPARE(dataExchangeController.product(), &product);
    QCOMPARE(productChanged.count(), 1);

    dataExchangeController.setProduct(&product);
    QCOMPARE(productChanged.count(), 1);

    dataExchangeController.setProduct(nullptr);
    QCOMPARE(dataExchangeController.product(), nullptr);
    QCOMPARE(productChanged.count(), 2);
}

void AbstractDataExchangeControllerTest::testSeams()
{
    AbstractDataExchangeController dataExchangeController;
    QVariantList list;
    list.push_back(QVariant());

    QSignalSpy seamsChanged{&dataExchangeController, &AbstractDataExchangeController::seamsChanged};
    QVERIFY(seamsChanged.isValid());
    QCOMPARE(seamsChanged.count(), 0);

    dataExchangeController.setSeams(list);
    QCOMPARE(dataExchangeController.seams(), list);
    QCOMPARE(seamsChanged.count(), 1);

    dataExchangeController.setSeams(list);
    QCOMPARE(seamsChanged.count(), 1);

    QVariantList list2;
    list2.push_back(QVariant());
    list2.push_back(QVariant());

    dataExchangeController.setSeams(list2);
    QCOMPARE(dataExchangeController.seams(), list2);
    QCOMPARE(seamsChanged.count(), 2);
}

void AbstractDataExchangeControllerTest::testSeamListIndex()
{
    AbstractDataExchangeController dataExchangeController;

    QSignalSpy seamListIndexChanged{&dataExchangeController, &AbstractDataExchangeController::seamListIndexChanged};
    QVERIFY(seamListIndexChanged.isValid());
    QCOMPARE(seamListIndexChanged.count(), 0);

    dataExchangeController.setSeamListIndex(0);
    QCOMPARE(dataExchangeController.seamListIndex(), 0);
    QCOMPARE(seamListIndexChanged.count(), 1);

    dataExchangeController.setSeamListIndex(0);
    QCOMPARE(seamListIndexChanged.count(), 1);

    dataExchangeController.setSeamListIndex(5);
    QCOMPARE(dataExchangeController.seamListIndex(), 5);
    QCOMPARE(seamListIndexChanged.count(), 2);
}

void AbstractDataExchangeControllerTest::testSeam()
{
    AbstractDataExchangeController dataExchangeController;
    Product product(QUuid::createUuid());
    SeamSeries seamSeries(QUuid::createUuid(), &product);
    Seam seam1(QUuid::createUuid(), &seamSeries);
    Seam seam2(QUuid::createUuid(), &seamSeries);

    QSignalSpy seamChanged{&dataExchangeController, &AbstractDataExchangeController::seamChanged};
    QVERIFY(seamChanged.isValid());
    QCOMPARE(seamChanged.count(), 0);

    dataExchangeController.setSeam(&seam1);
    QCOMPARE(dataExchangeController.seam(), &seam1);
    QCOMPARE(seamChanged.count(), 1);

    dataExchangeController.setSeam(&seam1);
    QCOMPARE(seamChanged.count(), 1);

    dataExchangeController.setSeam(&seam2);
    QCOMPARE(dataExchangeController.seam(), &seam2);
    QCOMPARE(seamChanged.count(), 2);
}

void AbstractDataExchangeControllerTest::testSetObjects()
{
    QDir dir(QFINDTESTDATA("../../Mod_Storage/autotests/testdata/products/"));
    QVERIFY(dir.exists());

    AbstractDataExchangeController dataExchangeController;
    ProductModel productModel;

    QSignalSpy productChanged{&dataExchangeController, &AbstractDataExchangeController::productChanged};
    QVERIFY(productChanged.isValid());
    QCOMPARE(productChanged.count(), 0);

    QSignalSpy seamsChanged{&dataExchangeController, &AbstractDataExchangeController::seamsChanged};
    QVERIFY(seamsChanged.isValid());
    QCOMPARE(seamsChanged.count(), 0);

    QSignalSpy seamChanged{&dataExchangeController, &AbstractDataExchangeController::seamChanged};
    QVERIFY(seamChanged.isValid());
    QCOMPARE(seamChanged.count(), 0);

    productModel.loadProducts(dir);
    QCOMPARE(productModel.rowCount(), 30);
    dataExchangeController.setProductModel(&productModel);

    const auto& index = productModel.index(12, 0);
    dataExchangeController.setProductModelIndex(index);
    QCOMPARE(productChanged.count(), 1);
    QCOMPARE(dataExchangeController.product()->name(), productModel.data(index, Qt::DisplayRole));

    /*for (int i = 0; i < productModel.rowCount(); i++)
    {
        const auto& index = productModel.index(i, 0);
        qDebug() << i;
        dataExchangeController.setProductModelIndex(index);
        //QCOMPARE(productChanged.count(), 1);
        QCOMPARE(dataExchangeController.product()->name(), productModel.data(index, Qt::DisplayRole));
    }*/

    QCOMPARE(seamsChanged.count(), 1);
    QCOMPARE(dataExchangeController.seams(), dataExchangeController.product()->allSeams());

    const auto& seamIndex = 1;
    dataExchangeController.setSeamListIndex(seamIndex);
    QCOMPARE(seamChanged.count(), 1);
    QCOMPARE(dataExchangeController.seam(), dataExchangeController.seams().at(seamIndex).value<Seam*>());
}

QTEST_GUILESS_MAIN(AbstractDataExchangeControllerTest)
#include "abstractDataExchangeControllerTest.moc"
