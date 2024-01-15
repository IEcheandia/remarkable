#include <QTest>
#include <QSignalSpy>

#include "../src/productController.h"

#include "product.h"
#include "productModel.h"

using precitec::gui::ProductController;
using precitec::storage::Product;
using precitec::storage::ProductModel;

class ProductControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testData();
    void testExportSeparatedProduct();
    void testExportSeparatedProductWithoutScanfieldImages();
    void testImportSeparatedProduct();
};

void ProductControllerTest::testCtor()
{
    auto productController = std::make_unique<ProductController>();
    QVERIFY(!productController->productModel());
    QVERIFY(!productController->currentProduct());
    QVERIFY(productController->currentProductIndex() == -1);
}

void ProductControllerTest::testRoleNames()
{
    auto productController = std::make_unique<ProductController>();
    const auto roleNames = productController->roleNames();
    QCOMPARE(roleNames.size(), 4);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::UserRole], QByteArrayLiteral("uuid"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("product"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("changed"));
}

void ProductControllerTest::testData()
{
    auto productModel = new ProductModel{this};
    QVERIFY(productModel);
    QSignalSpy productModelResetSpy(productModel, &ProductModel::modelReset);
    QVERIFY(productModelResetSpy.isValid());

    QTemporaryDir dir;
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_1.json"),
                        dir.filePath(QStringLiteral("product1.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_2.json"),
                        dir.filePath(QStringLiteral("product2.json"))));
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_3.json"),
                        dir.filePath(QStringLiteral("product3.json"))));

    QVERIFY(!productModel->defaultProduct());
    productModel->loadProducts(dir.path());
    QCOMPARE(productModelResetSpy.count(), 1);
    QCOMPARE(productModel->rowCount(), 3);

    std::unique_ptr<ProductController> controller = std::make_unique<ProductController>();

    QVERIFY(controller);
    QSignalSpy controllerChangedSpy(controller.get(), &ProductController::hasChangesChanged);
    QVERIFY(controllerChangedSpy.isValid());
    controller->setProductModel(productModel);

    QVERIFY(controller->productModel() != nullptr);
    QCOMPARE(controller->index(0, 0).data(Qt::DisplayRole).value<QString>(), QString("FooBar_0"));
    QCOMPARE(controller->index(1, 0).data(Qt::DisplayRole).value<QString>(), QString("FooBar_1"));
    QCOMPARE(controller->index(2, 0).data(Qt::DisplayRole).value<QString>(), QString("FooBar_2"));
    QCOMPARE(controller->index(0, 1).data(Qt::DisplayRole).value<QString>(), QString(""));
    QCOMPARE(controller->index(0, 0).data(Qt::UserRole).value<QString>(),
             QString("{1f086211-fbd4-4493-a580-6ff11e4925de}"));
    QCOMPARE(controller->index(1, 0).data(Qt::UserRole).value<QString>(),
             QString("{bc5c3130-14bf-4924-9aed-bd92d0fbc885}"));
    QCOMPARE(controller->index(2, 0).data(Qt::UserRole).value<QString>(),
             QString("{c70768ed-823b-44ad-8ab2-60a551cfba66}"));
    QCOMPARE(controller->index(2, 1).data(Qt::UserRole).value<QString>(), QString(""));
    QCOMPARE(controller->index(0, 0).data(Qt::UserRole + 1).value<Product *>()->name(), QString("FooBar_0"));
    QCOMPARE(controller->index(1, 0).data(Qt::UserRole + 1).value<Product *>()->name(), QString("FooBar_1"));
    QCOMPARE(controller->index(2, 0).data(Qt::UserRole + 1).value<Product *>()->name(), QString("FooBar_2"));
    QCOMPARE(controller->index(0, 0).data(Qt::UserRole + 2).value<bool>(), 0);
    QCOMPARE(controller->index(1, 0).data(Qt::UserRole + 2).value<bool>(), 0);
    QCOMPARE(controller->index(2, 0).data(Qt::UserRole + 2).value<bool>(), 0);
}

void ProductControllerTest::testExportSeparatedProduct()
{
    // prepare user case with 1 product with reference_curves and scanfieldimages and 1 product without the artifacts
    QTemporaryDir dir;
    QDir(dir.path()).mkdir("products");
    QDir(dir.path()).mkdir("reference_curves");
    QDir(dir.path()).mkdir("scanfieldimage");
    QDir(dir.path()).mkdir("scanfieldimage/4583c4cb-bf22-4863-aaa6-bb86c7b7c9ea");
    const auto productIdString = QStringLiteral("a4f5b5ce-d50d-45d6-bfe5-cfd620bba0ff");
    const auto productFileName = productIdString + QStringLiteral(".json");
    const auto referenceCurvesName = productIdString + QStringLiteral(".ref");
    const auto scanfieldImageFolder = QStringLiteral("4583c4cb-bf22-4863-aaa6-bb86c7b7c9ea");

    // move first product file to temporal directory
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_1.json"),
                        dir.filePath(QStringLiteral("products/product1.json"))));

    // move second product files to temporal directory
    const auto sourcePathPrefix = QStringLiteral("testdata/config_folder_data");
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + QStringLiteral("/products/") + productFileName),
                        dir.filePath(QStringLiteral("products/") + productFileName)));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + QStringLiteral("/reference_curves/") + referenceCurvesName),
                        dir.filePath(QStringLiteral("reference_curves/") + referenceCurvesName)));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + QStringLiteral("/scanfieldimage/") + scanfieldImageFolder +
                                      QStringLiteral("/ScanFieldImage.bmp")),
                        dir.filePath(QStringLiteral("scanfieldimage/") + scanfieldImageFolder +
                                     QStringLiteral("/ScanFieldImage.bmp"))));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + QStringLiteral("/scanfieldimage/") + scanfieldImageFolder +
                                      QStringLiteral("/ScanFieldImage.ini")),
                        dir.filePath(QStringLiteral("scanfieldimage/") + scanfieldImageFolder +
                                     QStringLiteral("/ScanFieldImage.ini"))));

    // prepare product model
    auto productModel = new ProductModel{this};
    QVERIFY(productModel);
    QSignalSpy productModelResetSpy(productModel, &ProductModel::modelReset);
    QVERIFY(productModelResetSpy.isValid());

    // load these two products
    productModel->setReferenceStorageDirectory(dir.path() + QStringLiteral("/reference_curves"));
    productModel->setScanfieldImageStorageDirectory(dir.path() + QStringLiteral("/scanfieldimage"));
    productModel->loadProducts(dir.path() + QStringLiteral("/products"));
    QCOMPARE(productModelResetSpy.count(), 1);
    QCOMPARE(productModel->rowCount(), 2);

    // set up removable device folder
    const auto targetDir = QStringLiteral("removable_device");
    std::unique_ptr<ProductController> controller = std::make_unique<ProductController>();
    controller->setProductModel(productModel);

    // setup current product and export
    controller->selectProduct(QUuid::fromString(productIdString));
    QDir(dir.path()).mkdir(targetDir);
    controller->setScanfieldPath(dir.path() + QStringLiteral("/scanfieldimage/"));
    QSignalSpy copyInProgressSpy(controller.get(), &ProductController::copyInProgressChanged);
    controller->exportCurrentProductSeparately(dir.path() + QString("/removable_device/"));
    copyInProgressSpy.wait();
    // check results
    auto productTargetDir = targetDir + "/" + controller->currentProduct()->name();
    QVERIFY(QDir(dir.path()).exists(productTargetDir + QStringLiteral("/product/") + productFileName));
    QVERIFY(QDir(dir.path()).exists(productTargetDir + QStringLiteral("/reference_curve/") + referenceCurvesName));

    QVERIFY(QDir(dir.path()).exists(productTargetDir));
    QVERIFY(QDir(dir.path()).exists("removable_device/images"));
    QVERIFY(QDir(dir.path()).exists("removable_device/images/scanfieldimages"));
    QVERIFY(QDir(dir.path())
                .exists(productTargetDir + QStringLiteral("/scanfieldimages/") + scanfieldImageFolder +
                        QStringLiteral("/ScanFieldImage.ini")));
    QVERIFY(QDir(dir.path())
                .exists(productTargetDir + QStringLiteral("/scanfieldimages/") + scanfieldImageFolder +
                        QStringLiteral("/ScanFieldImage.bmp")));
}

void ProductControllerTest::testExportSeparatedProductWithoutScanfieldImages()
{
    // prepare user case with 1 product with reference_curves and scanfieldimages and 1 product without the artifacts
    QTemporaryDir dir;
    QDir(dir.path()).mkdir("products");
    QDir(dir.path()).mkdir("reference_curves");
    const auto productIdString = QStringLiteral("a4f5b5ce-d50d-45d6-bfe5-cfd620bba0ff");
    const auto productFileName = productIdString + QStringLiteral(".json");
    const auto referenceCurvesName = productIdString + QStringLiteral(".ref");

    // move first product file to temporal directory
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_1.json"),
                        dir.filePath(QStringLiteral("products/product1.json"))));

    // move second product files to temporal directory
    const auto sourcePathPrefix = QStringLiteral("testdata/config_folder_data");
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + QStringLiteral("/products/") + productFileName),
                        dir.filePath(QStringLiteral("products/") + productFileName)));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + QStringLiteral("/reference_curves/") + referenceCurvesName),
                        dir.filePath(QStringLiteral("reference_curves/") + referenceCurvesName)));

    // prepare product model
    auto productModel = new ProductModel{this};
    QVERIFY(productModel);
    QSignalSpy productModelResetSpy(productModel, &ProductModel::modelReset);
    QVERIFY(productModelResetSpy.isValid());

    // load these two products
    productModel->setReferenceStorageDirectory(dir.path() + QStringLiteral("/reference_curves"));
    productModel->loadProducts(dir.path() + QStringLiteral("/products"));
    QCOMPARE(productModelResetSpy.count(), 1);
    QCOMPARE(productModel->rowCount(), 2);

    // set up removable device folder
    const auto targetDir = QStringLiteral("removable_device");
    std::unique_ptr<ProductController> controller = std::make_unique<ProductController>();
    controller->setProductModel(productModel);

    // setup current product and export
    controller->selectProduct(QUuid::fromString(productIdString));
    QDir(dir.path()).mkdir(targetDir);
    QSignalSpy copyInProgressSpy(controller.get(), &ProductController::copyInProgressChanged);
    controller->exportCurrentProductSeparately(dir.path() + QString("/removable_device/"));
    copyInProgressSpy.wait();
    // check results
    auto productTargetDir = targetDir + "/" + controller->currentProduct()->name();
    QVERIFY(QDir(dir.path()).exists(productTargetDir + QStringLiteral("/product/") + productFileName));
    QVERIFY(QDir(dir.path()).exists(productTargetDir + QStringLiteral("/reference_curve/") + referenceCurvesName));

    QVERIFY(QDir(dir.path()).exists(productTargetDir));
    QVERIFY(QDir(dir.path()).exists("removable_device/images"));
    QVERIFY(!QDir(dir.path()).exists("removable_device/images/scanfieldimages"));
}

void ProductControllerTest::testImportSeparatedProduct()
{
    // prepare usa case with a separated product on removable device (temporary directory)
    QTemporaryDir dir;
    const auto productFolder = QStringLiteral("images");
    const auto productSubFolder = QStringLiteral("/product");
    const auto referenceCurveSubFolder = QStringLiteral("/reference_curve");
    const auto scanFieldImagesSubFolder = QStringLiteral("/scanfieldimages");

    QDir(dir.path()).mkdir(productFolder);
    QDir(dir.path()).mkdir(productFolder + productSubFolder);
    QDir(dir.path()).mkdir(productFolder + referenceCurveSubFolder);
    QDir(dir.path()).mkdir(productFolder + scanFieldImagesSubFolder);
    QDir(dir.path())
        .mkdir(productFolder + scanFieldImagesSubFolder + QStringLiteral("/4583c4cb-bf22-4863-aaa6-bb86c7b7c9ea"));

    const auto productIdString = QStringLiteral("a4f5b5ce-d50d-45d6-bfe5-cfd620bba0ff");
    const auto productFileName = productIdString + QStringLiteral(".json");
    const auto referenceCurvesName = productIdString + QStringLiteral(".ref");
    const auto scanfieldImageFolder = QStringLiteral("4583c4cb-bf22-4863-aaa6-bb86c7b7c9ea");
    const auto sourcePathPrefix = QStringLiteral("testdata/separated_products/images");
    const auto slash = QStringLiteral("/");
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + productSubFolder + slash + productFileName),
                        dir.filePath(productFolder + productSubFolder + slash + productFileName)));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + referenceCurveSubFolder + slash + referenceCurvesName),
                        dir.filePath(productFolder + referenceCurveSubFolder + slash + referenceCurvesName)));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + scanFieldImagesSubFolder + slash + scanfieldImageFolder +
                                      QStringLiteral("/ScanFieldImage.bmp")),
                        dir.filePath(productFolder + scanFieldImagesSubFolder + slash + scanfieldImageFolder +
                                     QStringLiteral("/ScanFieldImage.bmp"))));
    QVERIFY(QFile::copy(QFINDTESTDATA(sourcePathPrefix + scanFieldImagesSubFolder + slash + scanfieldImageFolder +
                                      QStringLiteral("/ScanFieldImage.ini")),
                        dir.filePath(productFolder + scanFieldImagesSubFolder + slash + scanfieldImageFolder +
                                     QStringLiteral("/ScanFieldImage.ini"))));

    // create and load an augment product in temporal directory
    // to set up product storage directory for a productModel
    QDir(dir.path()).mkdir("products");
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/product_data/product_1.json"),
                        dir.filePath(QStringLiteral("products/product1.json"))));
    auto productModel = new ProductModel{this};
    QVERIFY(productModel);
    QSignalSpy productModelResetSpy(productModel, &ProductModel::modelReset);
    QVERIFY(productModelResetSpy.isValid());
    productModel->loadProducts(dir.path() + QStringLiteral("/products"));
    QCOMPARE(productModelResetSpy.count(), 1);
    QCOMPARE(productModel->rowCount(), 1);

    // set reference storage
    QDir(dir.path()).mkdir("reference_curves");
    productModel->setReferenceStorageDirectory(dir.path() + QStringLiteral("/reference_curves/"));
    std::unique_ptr<ProductController> controller = std::make_unique<ProductController>();
    controller->setProductModel(productModel);

    // set scanfield storage
    QDir(dir.path()).mkdir("scanfieldimage");
    controller->setScanfieldPath(dir.path() + QString("/scanfieldimage/"));
    productModel->setScanfieldImageStorageDirectory(dir.path() + QStringLiteral("/scanfieldimage/"));

    // perform import
    QSignalSpy copyInProgressSpy(controller.get(), &ProductController::copyInProgressChanged);
    controller->importSeparatedProduct(dir.path() + QStringLiteral("/") + productFolder);
    copyInProgressSpy.wait();

    // check results
    QVERIFY(QDir(dir.path()).exists(QStringLiteral("products/") + productFileName));
    QVERIFY(QDir(dir.path()).exists(QStringLiteral("reference_curves/") + referenceCurvesName));

    QVERIFY(
        QDir(dir.path())
            .exists(QStringLiteral("scanfieldimage/") + scanfieldImageFolder + QStringLiteral("/ScanFieldImage.bmp")));
    QVERIFY(
        QDir(dir.path())
            .exists(QStringLiteral("scanfieldimage/") + scanfieldImageFolder + QStringLiteral("/ScanFieldImage.ini")));
}

QTEST_GUILESS_MAIN(ProductControllerTest)
#include "productControllerTest.moc"
