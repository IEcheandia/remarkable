#include <QTest>

#include "../src/productInstancesTransferController.h"
#include "../src/productInstanceModel.h"
#include "../src/checkedFilterModel.h"

#include "seam.h"
#include "seamSeries.h"
#include "product.h"

#include <QSignalSpy>

using precitec::gui::ProductInstancesTransferController;

using precitec::gui::CheckedFilterModel;
using precitec::storage::Product;
using precitec::storage::ProductInstanceModel;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

class ProductInstancesTransferControllerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testTransferWithoutProperties();
    void testTransferOneProductInstanceWithOneSeam();

private:
};

void ProductInstancesTransferControllerTest::testCtor()
{
    precitec::gui::ProductInstancesTransferController transferController{this};
}

void ProductInstancesTransferControllerTest::testTransferWithoutProperties()
{
    precitec::gui::ProductInstancesTransferController transferController{this};
    QVERIFY(!transferController.transfer());
}

void ProductInstancesTransferControllerTest::testTransferOneProductInstanceWithOneSeam()
{
    // setup moc file structure
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QDir(dir.path()).mkdir("video");
    const auto productInstancesSubDirectory = QStringLiteral("video/WM_QNX_PC");
    QDir(dir.path()).mkdir(productInstancesSubDirectory);

    const auto sourceProductIdString = QStringLiteral("a4f5b5ce-d50d-45d6-bfe5-cfd620bba0ff");
    const auto sourceProductSubDirectory = productInstancesSubDirectory + "/" + sourceProductIdString;
    QDir(dir.path()).mkdir(sourceProductSubDirectory);

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/" + sourceProductSubDirectory + "/" + sourceProductIdString + ".id"),
                        dir.filePath(sourceProductSubDirectory + "/" + sourceProductIdString + ".id")));
    const auto sourceProductInstanceIdString = QStringLiteral("01d7de9c-5db5-11ec-bca5-e0d4643b6870");
    const auto sourceSerialNumber = QStringLiteral("1215154054");
    const auto sourceProductInstanceDirectoryName = sourceProductInstanceIdString + "-SN-" + sourceSerialNumber;
    const auto sourceProductInstanceSubDirectory = sourceProductSubDirectory + "/" + sourceProductInstanceDirectoryName;
    QDir(dir.path()).mkdir(sourceProductInstanceSubDirectory);

    QDir(dir.path()).mkdir(sourceProductInstanceSubDirectory + QStringLiteral("/seam_series0000"));
    const auto seamSeriesSeamSubDirectory = QStringLiteral("seam_series0000/seam0000");
    QDir(dir.path()).mkdir(sourceProductInstanceSubDirectory + "/" + seamSeriesSeamSubDirectory);

    auto const sourceImageFile = QStringLiteral("00000.bmp");
    QVERIFY(QFile::copy(
        QFINDTESTDATA("testdata/" + sourceProductInstanceSubDirectory + "/" + seamSeriesSeamSubDirectory + "/" +
                      sourceImageFile),
        dir.filePath(sourceProductInstanceSubDirectory + "/" + seamSeriesSeamSubDirectory + "/" + sourceImageFile)));

    auto const sourceMetaFile = QStringLiteral("sequence_info.xml");
    QVERIFY(QFile::copy(
        QFINDTESTDATA("testdata/" + sourceProductInstanceSubDirectory + "/" + seamSeriesSeamSubDirectory + "/" +
                      sourceMetaFile),
        dir.filePath(sourceProductInstanceSubDirectory + "/" + seamSeriesSeamSubDirectory + "/" + sourceMetaFile)));

    const auto targetProductIdString = QStringLiteral("f439a74d-8c56-41e9-abe3-d98d5dc0718b");
    const auto targetProductSubDirectory = productInstancesSubDirectory + "/" + targetProductIdString;
    QDir(dir.path()).mkdir(targetProductSubDirectory);

    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/" + targetProductSubDirectory + "/" + targetProductIdString + ".id"),
                        dir.filePath(targetProductSubDirectory + "/" + targetProductIdString + ".id")));
    const auto additionalTargetProductInstanceIdString = QStringLiteral("02cf29c6-5c21-11ec-adf3-e0d4643b6870");
    const auto additionalTargetSerialNumber = QStringLiteral("1213152859");
    const auto additionalTargetProductInstanceSubDirectory = targetProductSubDirectory + "/" +
                                                             additionalTargetProductInstanceIdString + "-SN-" +
                                                             additionalTargetSerialNumber;
    QDir(dir.path()).mkdir(additionalTargetProductInstanceSubDirectory);

    // setup moc products
    Product sourceProduct(QUuid::fromString(sourceProductIdString));

    Product targetProduct(QUuid::fromString(targetProductIdString));
    targetProduct.setName("Big product");

    sourceProduct.createFirstSeamSeries();
    sourceProduct.createSeam();
    sourceProduct.seams();

    targetProduct.createFirstSeamSeries();
    targetProduct.createSeam();
    targetProduct.seams();

    // setup product instance model
    ProductInstanceModel sourceProductInstanceModel(this);

    QSignalSpy loadingChangedSpy(&sourceProductInstanceModel, &ProductInstanceModel::loadingChanged);
    QVERIFY(loadingChangedSpy.isValid());
    QSignalSpy rowInsertedSpy(&sourceProductInstanceModel, &ProductInstanceModel::rowsInserted);
    QVERIFY(rowInsertedSpy.isValid());

    sourceProductInstanceModel.setDirectory(dir.path() + "/" + productInstancesSubDirectory);
    sourceProductInstanceModel.setProduct(&sourceProduct);
    if (sourceProductInstanceModel.isLoading())
    {
        QCOMPARE(loadingChangedSpy.count(), 1);
        QVERIFY(loadingChangedSpy.wait());
    }
    QCOMPARE(loadingChangedSpy.count(), 2);
    QCOMPARE(sourceProductInstanceModel.isLoading(), false);

    QTRY_COMPARE(rowInsertedSpy.count(), 1);

    QCOMPARE(sourceProductInstanceModel.rowCount(), 1);
    QCOMPARE(sourceProductInstanceModel.rowCount(sourceProductInstanceModel.index(0, 0)), 0);

    QCOMPARE(sourceProductInstanceModel.index(0, 0).data().toString(), QStringLiteral("1215154054"));
    auto index0 = sourceProductInstanceModel.index(0, 0);
    sourceProductInstanceModel.toggleChecked(0);
    QCOMPARE(index0.data(Qt::UserRole + 7).toBool(), true);

    CheckedFilterModel checkedFilterModel(this);
    checkedFilterModel.setSourceModel(&sourceProductInstanceModel);
    checkedFilterModel.setRoleName("checked");

    precitec::gui::ProductInstancesTransferController transferController{this};
    transferController.setMonitoring(false);
    QSignalSpy indexFilterModelSpy(&transferController,
                                     &ProductInstancesTransferController::indexFilterModelChanged);
    QSignalSpy targetProductSpy(&transferController,
                                   &ProductInstancesTransferController::targetProductChanged);
    transferController.setIndexFilterModel(&checkedFilterModel);
    QCOMPARE(indexFilterModelSpy.count(), 1);
    transferController.setTargetProduct(&targetProduct);
    QCOMPARE(targetProductSpy.count(), 1);

    QSignalSpy transferInProgressSpy(&transferController,
                                     &ProductInstancesTransferController::transferInProgressChanged);
    transferController.transfer();
    QVERIFY(transferInProgressSpy.wait());

    QCOMPARE(transferController.targetProductInstanceDirectories().size(), 1);

    const auto expectedTargetProductInstanceDirectory = QDir(*transferController.targetProductInstanceDirectories().begin());

    QFile expectedTargetProductMetaFile(expectedTargetProductInstanceDirectory.path() + "/" +
                                        seamSeriesSeamSubDirectory + "/" + sourceMetaFile);
    QFile expectedTargetProductImageFile(expectedTargetProductInstanceDirectory.path() + "/" +
                                         seamSeriesSeamSubDirectory + "/" + sourceImageFile);
    QVERIFY(expectedTargetProductMetaFile.exists());
    QVERIFY(expectedTargetProductImageFile.exists());
}

QTEST_GUILESS_MAIN(ProductInstancesTransferControllerTest)
#include "productInstancesTransferControllerTest.moc"
