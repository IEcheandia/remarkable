#include <QTest>

#include "../src/productInstanceTransferHandler.h"
#include "seam.h"
#include "seamSeries.h"
#include "product.h"

using precitec::gui::ProductInstanceTransferHandler;

using precitec::storage::Product;
using precitec::storage::Seam;
using precitec::storage::SeamSeries;

class ProductInstanceTransferHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testTransferWithoutProperties();
    void testTransferProductInstanceWithoutSeams();
    void testTransferProductInstanceWithOneSeam();

private:
};

void ProductInstanceTransferHandlerTest::testCtor()
{
    precitec::gui::ProductInstanceTransferHandler transferHandler{this};
}

void ProductInstanceTransferHandlerTest::testTransferWithoutProperties()
{
    precitec::gui::ProductInstanceTransferHandler transferHandler{this};
    QVERIFY(!transferHandler.transfer());
}

void ProductInstanceTransferHandlerTest::testTransferProductInstanceWithoutSeams()
{
    // create moc product instance files
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    Product sourceProduct(QUuid::createUuid());
    QString sourceProductDirectoryName(sourceProduct.uuid().toString(QUuid::WithoutBraces));
    QDir(dir.path()).mkdir(sourceProductDirectoryName);
    QString sourceProductDirectory(dir.path() + "/" + sourceProductDirectoryName);

    QFile sourceProductMetaUuidFile(sourceProductDirectory + "/" + sourceProduct.uuid().toString(QUuid::WithoutBraces) +
                                    ".id");
    QVERIFY(sourceProductMetaUuidFile.open(QIODevice::WriteOnly));
    sourceProductMetaUuidFile.close();

    QUuid sourceProductInstanceId(QUuid::createUuid());
    quint32 sourceProductInstanceSerialNumber = 123;
    QString sourceProductInstanceDirectoryName(sourceProductInstanceId.toString(QUuid::WithoutBraces) + "-SN-" +
                                               QString::number(sourceProductInstanceSerialNumber));
    QVERIFY(QDir(sourceProductDirectory).mkdir(sourceProductInstanceDirectoryName));

    Product targetProduct(QUuid::createUuid());

    QString targetProductDirectoryName(targetProduct.uuid().toString(QUuid::WithoutBraces));
    QString targetProductDirectory(dir.path() + "/" + targetProductDirectoryName);
    QDir(dir.path()).mkdir(targetProductDirectoryName);

    QFile targetProductMetaUuidFile(targetProductDirectory + "/" + targetProduct.uuid().toString(QUuid::WithoutBraces) +
                                    ".id");
    QVERIFY(targetProductMetaUuidFile.open(QIODevice::WriteOnly));
    targetProductMetaUuidFile.close();

    // initialization
    precitec::gui::ProductInstanceTransferHandler transferHandler{this};
    transferHandler.setDirectory(dir.path());
    transferHandler.setSourceProduct(&sourceProduct);
    transferHandler.setSourceProductInstanceId(sourceProductInstanceId);
    transferHandler.setSourceSerialNumber(sourceProductInstanceSerialNumber);
    transferHandler.setTargetProduct(&targetProduct);

    QVERIFY(transferHandler.transfer());

    // check results
    const auto expectedTargetProductInstanceDirectory = QDir(transferHandler.targetFullProductInstanceDirectory());

    QFile expectedTargetProductInstanceUuidFile(
        expectedTargetProductInstanceDirectory.path() + "/" +
        transferHandler.targetProductInstanceId().toString(QUuid::WithoutBraces) + ".id");

    QVERIFY(expectedTargetProductInstanceDirectory.exists());
    QVERIFY(expectedTargetProductInstanceUuidFile.exists());
}

void ProductInstanceTransferHandlerTest::testTransferProductInstanceWithOneSeam()
{
    // prepare user case with source product instance seam directory
    QTemporaryDir dir;
    QVERIFY(dir.isValid());

    QDir(dir.path()).mkdir("video");
    const auto productInstancesSubDirectory = QStringLiteral("video/WM_QNX_PC");
    QDir(dir.path()).mkdir(productInstancesSubDirectory);

    const auto sourceProductIdString = QStringLiteral("a4f5b5ce-d50d-45d6-bfe5-cfd620bba0ff");
    const auto sourceProductSubDirectory = productInstancesSubDirectory + "/" + sourceProductIdString;
    QDir(dir.path()).mkdir(sourceProductSubDirectory);

    // move product id file to temporal directory
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

    // move product id file to temporal directory
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/" + targetProductSubDirectory + "/" + targetProductIdString + ".id"),
                        dir.filePath(targetProductSubDirectory + "/" + targetProductIdString + ".id")));
    const auto targetProductInstanceIdString = QStringLiteral("02cf29c6-5c21-11ec-adf3-e0d4643b6870");
    const auto targetSerialNumber = QStringLiteral("1213152859");
    const auto targetProductInstanceSubDirectory =
        targetProductSubDirectory + "/" + targetProductInstanceIdString + "-SN-" + targetSerialNumber;
    QDir(dir.path()).mkdir(targetProductInstanceSubDirectory);

    Product sourceProduct(QUuid::fromString(sourceProductIdString));

    Product targetProduct(QUuid::fromString(targetProductIdString));
    targetProduct.setName("Big product");

    precitec::gui::ProductInstanceTransferHandler transferHandler{this};
    transferHandler.setDirectory(dir.path() + "/" + productInstancesSubDirectory);
    transferHandler.setSourceProduct(&sourceProduct);
    transferHandler.setSourceProductInstanceId(QUuid::fromString(sourceProductInstanceIdString));
    transferHandler.setSourceSerialNumber(sourceSerialNumber.toUInt());
    transferHandler.setTargetProduct(&targetProduct);

    sourceProduct.createFirstSeamSeries();
    sourceProduct.createSeam();
    sourceProduct.seams();

    targetProduct.createFirstSeamSeries();
    targetProduct.createSeam();
    targetProduct.seams();

    QVERIFY(transferHandler.transfer());

    const auto expectedTargetProductInstanceDirectory = QDir(transferHandler.targetFullProductInstanceDirectory());
    QFile expectedTargetProductInstanceUuidFile(
        expectedTargetProductInstanceDirectory.path() + "/" +
        transferHandler.targetProductInstanceId().toString(QUuid::WithoutBraces) + ".id");
    QFile expectedTargetProductMetaFile(expectedTargetProductInstanceDirectory.path() + "/" +
                                        seamSeriesSeamSubDirectory + "/" + sourceMetaFile);
    QFile expectedTargetProductImageFile(expectedTargetProductInstanceDirectory.path() + "/" +
                                         seamSeriesSeamSubDirectory + "/" + sourceImageFile);
    QVERIFY(expectedTargetProductInstanceUuidFile.exists());
    QVERIFY(expectedTargetProductMetaFile.exists());
    QVERIFY(expectedTargetProductImageFile.exists());
}

QTEST_GUILESS_MAIN(ProductInstanceTransferHandlerTest)
#include "productInstanceTransferHandlerTest.moc"