#include <QTest>
#include <QSignalSpy>

#include "../src/seamProduceInstanceCopyHandler.h"

using precitec::gui::SeamProduceInstanceCopyHandler;

class SeamProductInstanceCopyHandlerTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSourceProductInstanceStructure();

private:
};

void SeamProductInstanceCopyHandlerTest::testCtor()
{
    precitec::gui::SeamProduceInstanceCopyHandler handler{this};
}

void SeamProductInstanceCopyHandlerTest::testSourceProductInstanceStructure()
{
    // prepare user case with source product instance seam folders
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
    const auto sourceProductInstanceSubDirectory =
        sourceProductSubDirectory + "/" + sourceProductInstanceIdString + "-SN-" + sourceSerialNumber;
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

    SeamProduceInstanceCopyHandler::SeamProduceInstanceInfo source;
    source.productUuid = QUuid::fromString(sourceProductIdString);
    source.productName = "iron_product";
    source.productInstanceDirectory = dir.path() + "/" + sourceProductInstanceSubDirectory;
    source.productInstanceUuid = QUuid::fromString(sourceProductInstanceIdString);
    source.serialNumber = sourceSerialNumber.toUInt();
    source.productType = 3;
    source.seamSeriesNumber = 0;
    source.seamNumber = 0;

    SeamProduceInstanceCopyHandler::SeamProduceInstanceInfo target;
    target.productUuid = QUuid::fromString(targetProductIdString);
    target.productName = "two_line_product";
    target.productInstanceDirectory = dir.path() + "/" + targetProductInstanceSubDirectory;
    target.productInstanceUuid = QUuid::fromString(targetProductInstanceIdString);
    target.serialNumber = targetSerialNumber.toUInt();
    target.productType = 1002;
    target.seamSeriesNumber = 9;
    target.seamNumber = 9;
    auto seamProduceInstanceCopyHandler = std::make_unique<SeamProduceInstanceCopyHandler>();

    seamProduceInstanceCopyHandler->setSource(source);
    seamProduceInstanceCopyHandler->setTarget(target);
    seamProduceInstanceCopyHandler->copy();

    QDir targetPath =
        dir.path() + "/" + productInstancesSubDirectory + "/" + target.productUuid.toString(QUuid::WithoutBraces) +
        "/" + target.productInstanceUuid.toString(QUuid::WithoutBraces) + "-SN-" +
        QString::number(target.serialNumber) + "/" + "seam_series" + "000" + QString::number(target.seamSeriesNumber) +
        "/" + "seam" + "000" + QString::number(target.seamNumber);
    QVERIFY(targetPath.exists(sourceMetaFile));
    QVERIFY(targetPath.exists(sourceImageFile));
}

QTEST_GUILESS_MAIN(SeamProductInstanceCopyHandlerTest)
#include "seamProductInstanceCopyHandlerTest.moc"
