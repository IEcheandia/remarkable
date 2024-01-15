#include <QTest>

#include "../src/wizardModel.h"

using precitec::gui::WizardModel;

class WizardModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testRoleNames();
    void testData_data();
    void testData();
};

void WizardModelTest::testCtor()
{
    WizardModel model;
    QCOMPARE(model.rowCount(), 55);
    const auto index = model.index(0, 0);
    QVERIFY(index.isValid());
    QCOMPARE(model.rowCount(index), 0);
}

void WizardModelTest::testRoleNames()
{
    WizardModel model;
    const auto roleNames = model.roleNames();
    QCOMPARE(roleNames.size(), 6);
    QCOMPARE(roleNames[Qt::DisplayRole], QByteArrayLiteral("display"));
    QCOMPARE(roleNames[Qt::DecorationRole], QByteArrayLiteral("icon"));
    QCOMPARE(roleNames[Qt::UserRole + 1], QByteArrayLiteral("subitem"));
    QCOMPARE(roleNames[Qt::UserRole + 2], QByteArrayLiteral("component"));
    QCOMPARE(roleNames[Qt::UserRole + 3], QByteArrayLiteral("productItem"));
    QCOMPARE(roleNames[Qt::UserRole + 4], QByteArrayLiteral("seamSeriesItem"));
}

void WizardModelTest::testData_data()
{
    QTest::addColumn<int>("row");
    QTest::addColumn<QString>("display");
    QTest::addColumn<QString>("icon");
    QTest::addColumn<bool>("subitem");
    QTest::addColumn<bool>("productItem");
    QTest::addColumn<bool>("seamSeriesIem");
    QTest::addColumn<WizardModel::WizardComponent>("component");

    int row = 0;
    QTest::newRow("Axis") << row++ << QStringLiteral("Hardware") << QStringLiteral("wizard-hardware") << false << false << false << WizardModel::WizardComponent::Axis;
    QTest::newRow("Camera") << row++ << QStringLiteral("HW-ROI") << QStringLiteral("wizard-roi") << false << false << false << WizardModel::WizardComponent::Camera;
    QTest::newRow("IDM") << row++ << QStringLiteral("IDM") << QStringLiteral("menu-icon_settings") << false << false << false << WizardModel::WizardComponent::IDM;
    QTest::newRow("LWM") << row++ << QStringLiteral("Laser Welding Monitor") << QStringLiteral("wizard-lwm") << false << false << false << WizardModel::WizardComponent::LWM;
    QTest::newRow("ScanLabScanner") << row++ << QStringLiteral("Scanner") << QStringLiteral("wizard-scanner") << false << false << false << WizardModel::WizardComponent::ScanLabScanner;
    QTest::newRow("ToolCenterPointTCP") << row++ << QStringLiteral("IDM Tool Center Point") << QStringLiteral("crosshairs") << false << false << false << WizardModel::WizardComponent::ToolCenterPointOCT;
    QTest::newRow("Calibration") << row++ << QStringLiteral("Calibration IDM") << QStringLiteral("wizard-calibration") << false << false << false << WizardModel::WizardComponent::IDMCalibration;
    QTest::newRow("Calibration") << row++ << QStringLiteral("Calibration Scanfield") << QStringLiteral("wizard-calibration") << false << false << false << WizardModel::WizardComponent::ScanfieldCalibration;
    QTest::newRow("Calibration") << row++ << QStringLiteral("Calibration Camera") << QStringLiteral("wizard-calibration") << false << false << false << WizardModel::WizardComponent::CameraCalibration;
    QTest::newRow("Chessboard Calibration") << row++ << QStringLiteral("Calibration Camera") << QStringLiteral("wizard-calibration") << false << false << false << WizardModel::WizardComponent::CameraChessboardCalibration;
    QTest::newRow("Calibration") << row++ << QStringLiteral("Calibration LED") << QStringLiteral("wizard-calibration") << false << false << false << WizardModel::WizardComponent::LEDCalibration;
    QTest::newRow("FocusPosition") << row++ << QStringLiteral("Focus Position") << QStringLiteral("menu-icon_optics-set") << false << false << false << WizardModel::WizardComponent::FocusPosition;
    QTest::newRow("ToolCenterPoint") << row++ << QStringLiteral("Tool Center Point") << QStringLiteral("crosshairs") << false << false << false << WizardModel::WizardComponent::ToolCenterPoint;
    QTest::newRow("LaserControl") << row++ << QStringLiteral("Laser Control") << QStringLiteral("wizard-lasercontrol") << false << false << false << WizardModel::WizardComponent::LaserControl;
    QTest::newRow("LaserControlDelay") << row++ << QStringLiteral("Laser Control Delay") << QStringLiteral("wizard-lasercontrol") << false << false << false << WizardModel::WizardComponent::LaserControlDelay;
    QTest::newRow("ScanTracker") << row++ << QStringLiteral("Scan Tracker") << QStringLiteral("wizard-scantracker") << false << false << false << WizardModel::WizardComponent::ScanTracker;
    QTest::newRow("FigureEditor") << row++ << QStringLiteral("Figure Editor") << QStringLiteral("wizard-figure-editor") << false << false << false << WizardModel::WizardComponent::FigureEditor;
    QTest::newRow("ProductStructure") << row++ << QStringLiteral("Product Structure") << QStringLiteral("wizard-product-structure") << false << false << false << WizardModel::WizardComponent::ProductStructure;

    QTest::newRow("ProductColorMaps") << row++ << QStringLiteral("Color Maps") << QStringLiteral("view-plot") << false << true << false << WizardModel::WizardComponent::ProductColorMaps;
    QTest::newRow("ProductHardwareParametersOverview") << row++ << QStringLiteral("Hardware Parameters Overview") << QStringLiteral("wizard-hardware") << false << true << false << WizardModel::WizardComponent::ProductHardwareParametersOverview;
    QTest::newRow("ProductDetectionOverview") << row++ << QStringLiteral("Detection Overview") << QStringLiteral("wizard-detection") << false << true << false << WizardModel::WizardComponent::ProductDetectionOverview;
    QTest::newRow("ProductCamera") << row++ << QStringLiteral("Sensor") << QStringLiteral("wizard-sensor") << false << true << false << WizardModel::WizardComponent::ProductCamera;
    QTest::newRow("ProductError") << row++ << QStringLiteral("Errors") << QStringLiteral("wizard-sumerror") << false << true << false << WizardModel::WizardComponent::ProductError;
    QTest::newRow("ProductLaserControl") << row++ << QStringLiteral("Laser Control") << QStringLiteral("wizard-lasercontrol") << false << true << false << WizardModel::WizardComponent::ProductLaserControl;
    QTest::newRow("ProductScanTracker") << row++ << QStringLiteral("Scan Tracker") << QStringLiteral("wizard-scantracker") << false << true << false << WizardModel::WizardComponent::ProductScanTracker;
    QTest::newRow("ProductLaserWeldingMonitor") << row++ << QStringLiteral("Laser Welding Monitor") << QStringLiteral("wizard-lwm") << false << true << false << WizardModel::WizardComponent::ProductLaserWeldingMonitor;
    QTest::newRow("ProductScanLabScanner") << row++ << QStringLiteral("Scanner") << QStringLiteral("wizard-scanner") << false << true << false << WizardModel::WizardComponent::ProductScanLabScanner;
    QTest::newRow("ProductIDM") << row++ << QStringLiteral("IDM") << QStringLiteral("menu-icon_settings") << false << true << false << WizardModel::WizardComponent::ProductIDM;
    QTest::newRow("ProductZCollimator") << row++ << QStringLiteral("Z Collimator") << QStringLiteral("menu-icon_optics-set") << false << true << false << WizardModel::WizardComponent::ProductZCollimator;
    QTest::newRow("ProductScanTracker2D") << row++ << QStringLiteral("ScanTracker 2D") << QStringLiteral("wizard-scantracker") << false << true << false << WizardModel::WizardComponent::ProductScanTracker2D;

    QTest::newRow("SeamSeriesAcquireScanField") << row++ << QStringLiteral("Acquire Scan Field Image") << QStringLiteral("wizard-scantracker") << false << false << true << WizardModel::WizardComponent::SeamSeriesAcquireScanField;
    QTest::newRow("SeamSeriesError") << row++ << QStringLiteral("Errors") << QStringLiteral("wizard-sumerror") << false << false << true << WizardModel::WizardComponent::SeamSeriesError;
    QTest::newRow("SeamSeriesLaserControl") << row++ << QStringLiteral("Laser Control") << QStringLiteral("wizard-lasercontrol") << false << false << true << WizardModel::WizardComponent::SeamSeriesLaserControl;
    QTest::newRow("SeamSeriesScanTracker") << row++ << QStringLiteral("Scan Tracker") << QStringLiteral("wizard-scantracker") << false << false << true << WizardModel::WizardComponent::SeamSeriesScanTracker;
    QTest::newRow("SeamSeriesLaserWeldingMonitor") << row++ << QStringLiteral("Laser Welding Monitor") << QStringLiteral("wizard-lwm") << false << false << true << WizardModel::WizardComponent::SeamSeriesLaserWeldingMonitor;
    QTest::newRow("SeamSeriesScanLabScanner") << row++ << QStringLiteral("Scanner") << QStringLiteral("wizard-scanner") << false << false << true << WizardModel::WizardComponent::SeamSeriesScanLabScanner;
    QTest::newRow("SeamSeriesIDM") << row++ << QStringLiteral("IDM") << QStringLiteral("menu-icon_settings") << false << false << true << WizardModel::WizardComponent::SeamSeriesIDM;
    QTest::newRow("SeamSeriesZCollimator") << row++ << QStringLiteral("Z Collimator") << QStringLiteral("menu-icon_optics-set") << false << false << true << WizardModel::WizardComponent::SeamSeriesZCollimator;
    QTest::newRow("SeamSeriesScanTracker2D") << row++ << QStringLiteral("ScanTracker 2D") << QStringLiteral("wizard-scantracker") << false << false << true << WizardModel::WizardComponent::SeamSeriesScanTracker2D;

    QTest::newRow("SeamAssemblyImage") << row++ << QStringLiteral("Assembly Image") << QStringLiteral("view-assembly-image") << true << false << false << WizardModel::WizardComponent::SeamAssemblyImage;
    QTest::newRow("SeamCamera") << row++ << QStringLiteral("Sensor") << QStringLiteral("wizard-sensor") << true << false << false << WizardModel::WizardComponent::SeamCamera;
    QTest::newRow("SeamAxis") << row++ << QStringLiteral("Axis") << QStringLiteral("wizard-axis") << true << false << false << WizardModel::WizardComponent::SeamAxis;
    QTest::newRow("SeamDetection") << row++ << QStringLiteral("Detection") << QStringLiteral("wizard-detection") << true << false << false << WizardModel::WizardComponent::SeamDetection;
    QTest::newRow("SeamError") << row++ << QStringLiteral("Errors") << QStringLiteral("wizard-sumerror") << true << false << false << WizardModel::WizardComponent::SeamError;
    QTest::newRow("SeamReferenceCurves") << row++ << QStringLiteral("Reference Curves") << QStringLiteral("wizard-reference") << true << false << false << WizardModel::WizardComponent::SeamReferenceCurves;
    QTest::newRow("SeamLaserControl") << row++ << QStringLiteral("Laser Control") << QStringLiteral("wizard-lasercontrol") << true << false << false << WizardModel::WizardComponent::SeamLaserControl;
    QTest::newRow("SeamScanTracker") << row++ << QStringLiteral("Scan Tracker") << QStringLiteral("wizard-scantracker") << true << false << false << WizardModel::WizardComponent::SeamScanTracker;
    QTest::newRow("LWM") << row++ << QStringLiteral("Laser Welding Monitor") << QStringLiteral("wizard-lwm") << true << false << false << WizardModel::WizardComponent::SeamLaserWeldingMonitor;
    QTest::newRow("ScanLabScanner") << row++ << QStringLiteral("Scanner") << QStringLiteral("wizard-scanner") << true << false << false << WizardModel::WizardComponent::SeamScanLabScanner;
    QTest::newRow("IDM") << row++ << QStringLiteral("IDM") << QStringLiteral("menu-icon_settings") << true << false << false << WizardModel::WizardComponent::SeamIDM;
    QTest::newRow("ZCollimator") << row++ << QStringLiteral("Z Collimator") << QStringLiteral("menu-icon_optics-set") << true << false << false << WizardModel::WizardComponent::SeamZCollimator;
    QTest::newRow("Interval errors") << row++ << QStringLiteral("Interval Errors") << QStringLiteral("wizard-sumerror") << true << false << false << WizardModel::WizardComponent::SeamIntervalError;
    QTest::newRow("SeamScanTracker2D") << row++ << QStringLiteral("ScanTracker 2D") << QStringLiteral("wizard-scantracker") << true << false << false << WizardModel::WizardComponent::SeamScanTracker2D;
    QTest::newRow("ExternalLWM") << row++ << QStringLiteral("Laser Welding Monitor") << QStringLiteral("wizard-lwm") << false << false << false << WizardModel::WizardComponent::ExternalLWM;
    QTest::newRow("SeamExternalLWM") << row++ << QStringLiteral("Laser Welding Monitor") << QStringLiteral("wizard-lwm") << true << false << false << WizardModel::WizardComponent::SeamExternalLWM;
}

void WizardModelTest::testData()
{
    WizardModel model;
    QFETCH(int, row);
    const auto index = model.index(row, 0);
    QVERIFY(index.isValid());
    QTEST(index.data(Qt::DisplayRole).toString(), "display");
    QTEST(index.data(Qt::DecorationRole).toString(), "icon");
    QTEST(index.data(Qt::UserRole + 1).toBool(), "subitem");
    QTEST(index.data(Qt::UserRole + 3).toBool(), "productItem");
    QTEST(index.data(Qt::UserRole + 4).toBool(), "seamSeriesIem");
    QFETCH(WizardModel::WizardComponent, component);
    QCOMPARE(index.data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), component);
    QCOMPARE(model.indexForComponent(component), index);
}

QTEST_GUILESS_MAIN(WizardModelTest)
#include "wizardModelTest.moc"
