#include <QTest>
#include <QTemporaryFile>

#include "../guiConfiguration.h"

using precitec::gui::GuiConfiguration;

class GuiConfigurationTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testDefaultValues();
    void testDefaultValuesSouvis6000();
    void testDefaultValuesScanmaster();
};

void GuiConfigurationTest::testCtor()
{
    auto gc = GuiConfiguration::instance();

    QCOMPARE(gc->seamSeriesOnProductStructure(), false);
    QCOMPARE(gc->seamIntervalsOnProductStructure(), false);
    QCOMPARE(gc->configureBlackLevelOffsetVoltagesOnCamera(), false);
    QCOMPARE(gc->configureLinLogOnCamera(), false);
    QCOMPARE(gc->configureThicknessOnSeam(), false);
    QCOMPARE(gc->configureMovingDirectionOnSeam(), false);
    QCOMPARE(gc->ledCalibration(), false);
    QCOMPARE(gc->quickEditFilterParametersOnSeam(), false);
    QCOMPARE(gc->qualityFaultCategory2(), false);
    QCOMPARE(gc->scalePlotterFromSettings(), false);
    QCOMPARE(gc->colorSignalsByQuality(), false);
    QCOMPARE(gc->displayErrorBoundariesInPlotter(), true);
    QCOMPARE(gc->formatHardDisk(), false);
    QCOMPARE(gc->maximumNumberOfScreenshots(), 100);
    QCOMPARE(gc->maximumNumberOfSeamsOnOverview(), 100);
    QCOMPARE(gc->numberOfSeamsInPlotter(), 1);
    QCOMPARE(gc->serialNumberFromExtendedProductInfo(), 0u);
    QCOMPARE(gc->partNumberFromExtendedProductInfo(), 0u);
}

void GuiConfigurationTest::testDefaultValues()
{
    auto gc = GuiConfiguration::instance();
    QTemporaryFile file;
    QVERIFY(file.open());
    gc->setConfigFilePath(file.fileName());
    QCOMPARE(gc->seamSeriesOnProductStructure(), false);
    QCOMPARE(gc->seamIntervalsOnProductStructure(), false);
    QCOMPARE(gc->configureBlackLevelOffsetVoltagesOnCamera(), false);
    QCOMPARE(gc->configureLinLogOnCamera(), false);
    QCOMPARE(gc->configureThicknessOnSeam(), false);
    QCOMPARE(gc->configureMovingDirectionOnSeam(), false);
    QCOMPARE(gc->ledCalibration(), false);
    QCOMPARE(gc->quickEditFilterParametersOnSeam(), false);
    QCOMPARE(gc->qualityFaultCategory2(), false);
    QCOMPARE(gc->scalePlotterFromSettings(), false);
    QCOMPARE(gc->colorSignalsByQuality(), false);
    QCOMPARE(gc->displayErrorBoundariesInPlotter(), true);
    QCOMPARE(gc->formatHardDisk(), false);
    QCOMPARE(gc->maximumNumberOfScreenshots(), 100);
    QCOMPARE(gc->maximumNumberOfSeamsOnOverview(), 100);
    QCOMPARE(gc->numberOfSeamsInPlotter(), 1);
    QCOMPARE(gc->serialNumberFromExtendedProductInfo(), 0u);
    QCOMPARE(gc->partNumberFromExtendedProductInfo(), 0u);
}

void GuiConfigurationTest::testDefaultValuesScanmaster()
{
    auto gc = GuiConfiguration::instance();
    QTemporaryFile file;
    QVERIFY(file.open());
    gc->setDefaultConfigFilePath(QFINDTESTDATA("../../../../wm_inst/config_templates/scanmaster/uiSettings"));
    gc->setConfigFilePath(file.fileName());
    QCOMPARE(gc->seamSeriesOnProductStructure(), true);
    QCOMPARE(gc->seamIntervalsOnProductStructure(), false);
    QCOMPARE(gc->configureBlackLevelOffsetVoltagesOnCamera(), false);
    QCOMPARE(gc->configureLinLogOnCamera(), false);
    QCOMPARE(gc->configureThicknessOnSeam(), false);
    QCOMPARE(gc->configureMovingDirectionOnSeam(), false);
    QCOMPARE(gc->ledCalibration(), false);
    QCOMPARE(gc->quickEditFilterParametersOnSeam(), false);
    QCOMPARE(gc->qualityFaultCategory2(), false);
    QCOMPARE(gc->scalePlotterFromSettings(), false);
    QCOMPARE(gc->colorSignalsByQuality(), false);
    QCOMPARE(gc->displayErrorBoundariesInPlotter(), true);
    QCOMPARE(gc->formatHardDisk(), false);
    QCOMPARE(gc->maximumNumberOfScreenshots(), 100);
    QCOMPARE(gc->maximumNumberOfSeamsOnOverview(), 100);
    QCOMPARE(gc->numberOfSeamsInPlotter(), 1);
    QCOMPARE(gc->serialNumberFromExtendedProductInfo(), 0u);
    QCOMPARE(gc->partNumberFromExtendedProductInfo(), 0u);
}

void GuiConfigurationTest::testDefaultValuesSouvis6000()
{
    auto gc = GuiConfiguration::instance();
    QTemporaryFile file;
    QVERIFY(file.open());
    gc->setDefaultConfigFilePath(QFINDTESTDATA("../../../../wm_inst/config_templates/souvis6000/uiSettings"));
    gc->setConfigFilePath(file.fileName());
    QCOMPARE(gc->seamSeriesOnProductStructure(), false);
    QCOMPARE(gc->seamIntervalsOnProductStructure(), true);
    QCOMPARE(gc->configureBlackLevelOffsetVoltagesOnCamera(), true);
    QCOMPARE(gc->configureLinLogOnCamera(), true);
    QCOMPARE(gc->configureThicknessOnSeam(), true);
    QCOMPARE(gc->configureMovingDirectionOnSeam(), true);
    QCOMPARE(gc->ledCalibration(), true);
    QCOMPARE(gc->quickEditFilterParametersOnSeam(), false);
    QCOMPARE(gc->qualityFaultCategory2(), true);
    QCOMPARE(gc->scalePlotterFromSettings(), true);
    QCOMPARE(gc->colorSignalsByQuality(), false);
    QCOMPARE(gc->displayErrorBoundariesInPlotter(), true);
    QCOMPARE(gc->formatHardDisk(), false);
    QCOMPARE(gc->maximumNumberOfScreenshots(), 100);
    QCOMPARE(gc->maximumNumberOfSeamsOnOverview(), 100);
    QCOMPARE(gc->numberOfSeamsInPlotter(), 1);
    QCOMPARE(gc->serialNumberFromExtendedProductInfo(), 0u);
    QCOMPARE(gc->partNumberFromExtendedProductInfo(), 0u);
}

QTEST_GUILESS_MAIN(GuiConfigurationTest)
#include "guiConfigurationTest.moc"
