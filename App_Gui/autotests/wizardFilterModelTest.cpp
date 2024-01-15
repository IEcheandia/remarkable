#include <QTest>

#include "../src/wizardFilterModel.h"
#include "../src/wizardModel.h"

using precitec::gui::AbstractWizardFilterModel;
using precitec::gui::WizardFilterModel;
using precitec::gui::WizardModel;

class WizardFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testYAxisAvailable_data();
    void testYAxisAvailable();
    void testScanTrackerAvailable_data();
    void testScanTrackerAvailable();
    void testSensorGrabberAvailable_data();
    void testSensorGrabberAvailable();
    void testIDMAvailable_data();
    void testIDMAvailable();
    void testCameraAvailable_data();
    void testCameraAvailable();
    void testZCollimatorAvailable_data();
    void testZCollimatorAvailable();
    void testLEDAvailable_data();
    void testLEDAvailable();
    void testLaserControlAvailable_data();
    void testLaserControlAvailable();
    void testLWMAvailable_data();
    void testLWMAvailable();
    void testLineLaserAvailable_data();
    void testLineLaserAvailable();
    void testNewsonScannerAvailable_data();
    void testNewsonScannerAvailable();
    void testScanlabScannerAvailable_data();
    void testScanlabScannerAvailable();
    void testCameraInterfaceType_data();
    void testCameraInterfaceType();
};

void WizardFilterModelTest::testCtor()
{
    WizardFilterModel filterModel;
    QCOMPARE(filterModel.isYAxisAvailable(), false);
    QCOMPARE(filterModel.isSensorGrabberAvailable(), false);
    QCOMPARE(filterModel.isIDMAvailable(), false);
    QCOMPARE(filterModel.isCoaxCameraAvailable(), false);
    QCOMPARE(filterModel.isLEDCameraAvailable(), false);
    QCOMPARE(filterModel.isLEDAvailable(), false);
    QCOMPARE(filterModel.zCollimator(), false);
    QCOMPARE(filterModel.scanTracker(), false);
    QCOMPARE(filterModel.isLaserControlAvailable(), false);
    QCOMPARE(filterModel.ledCalibration(), false);
    QCOMPARE(filterModel.isLWMAvailable(), false);
    QCOMPARE(filterModel.isLineLaser1Available(), false);
    QCOMPARE(filterModel.isLineLaser2Available(), false);
    QCOMPARE(filterModel.isLineLaser3Available(), false);
    QCOMPARE(filterModel.isNewsonScannerAvailable(), false);
    QCOMPARE(filterModel.isScanlabScannerAvailable(), false);
    QCOMPARE(filterModel.rowCount(), 0);
}

void WizardFilterModelTest::testYAxisAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure

    };
    QTest::newRow("disabled") << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testYAxisAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(enabled);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isYAxisAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testScanTrackerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testScanTrackerAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(enabled);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.scanTracker(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testSensorGrabberAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 15 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testSensorGrabberAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(enabled);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isSensorGrabberAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testIDMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("camera_enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << true << 14 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled_no_camera") << false << false << 13 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testIDMAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    QFETCH(bool, camera_enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(enabled);
    filterModel.setCoaxCameraAvailable(camera_enabled);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isIDMAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testCameraAvailable_data()
{
    QTest::addColumn<bool>("enabledCoax");
    QTest::addColumn<bool>("enabledLED");
    QTest::addColumn<bool>("enabledIDM");
    QTest::addColumn<bool>("enabledScheimpflug");
    QTest::addColumn<AbstractWizardFilterModel::CameraInterfaceType>("interfaceType");
    QTest::addColumn<bool>("showProductionSetup");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled Coax") << true << false << true << false << AbstractWizardFilterModel::CameraInterfaceType::GigE << false
    << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("enabled Scheimpflug user view") << true << false << true << true << AbstractWizardFilterModel::CameraInterfaceType::GigE << false
    << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("enabled Scheimpflug production view") << true << false << true << true << AbstractWizardFilterModel::CameraInterfaceType::GigE << true
    << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraChessboardCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("enabled LED") << false << true << true << false << AbstractWizardFilterModel::CameraInterfaceType::GigE << false
    << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << false << true << false << AbstractWizardFilterModel::CameraInterfaceType::GigE << false
    << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled_no_IDM") << false << false << false << false << AbstractWizardFilterModel::CameraInterfaceType::GigE << false
    << 13 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testCameraAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabledCoax);
    QFETCH(bool, enabledLED);
    QFETCH(bool, enabledIDM);
    QFETCH(bool, enabledScheimpflug);
    QFETCH(AbstractWizardFilterModel::CameraInterfaceType, interfaceType);
    QFETCH(bool, showProductionSetup);
    
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(enabledIDM);
    filterModel.setCoaxCameraAvailable(enabledCoax);
    filterModel.setLEDCameraAvailable(enabledLED);
    filterModel.setScheimpflugCameraAvailable(enabledScheimpflug);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    filterModel.setCameraInterfaceType(interfaceType);
    filterModel.setShowProductionSetup(showProductionSetup);
    QCOMPARE(filterModel.isCoaxCameraAvailable(), enabledCoax);
    QCOMPARE(filterModel.isLEDCameraAvailable(), enabledLED);
    QCOMPARE(filterModel.isIDMAvailable(), enabledIDM);
    QCOMPARE(filterModel.cameraInterfaceType(), interfaceType);
    QCOMPARE(filterModel.showProductionSetup(), showProductionSetup);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testZCollimatorAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testZCollimatorAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setZCollimator(enabled);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.zCollimator(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testLEDAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("souvis");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled with souvis") << true << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
     QTest::newRow("disabled with souvis") << false << true << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
     QTest::newRow("enabled without souvis") << true << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled without souvis") << false << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testLEDAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    QFETCH(bool, souvis);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(enabled);
    filterModel.setLEDCalibration(souvis);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isLEDAvailable(), enabled);
    QCOMPARE(filterModel.ledCalibration(), souvis);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testLaserControlAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 15 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testLaserControlAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setLaserControlAvailable(enabled);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isLaserControlAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testLWMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testLWMAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(enabled);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isLWMAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testLineLaserAvailable_data()
{
    QTest::addColumn<bool>("line1enabled");
    QTest::addColumn<bool>("line2enabled");
    QTest::addColumn<bool>("line3enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("disabled") << false << false << false << 16 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("line1enabled") << true << false << false << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("line2enabled") << false << true << false << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("line3enabled") << false << false << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testLineLaserAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, line1enabled);
    QFETCH(bool, line2enabled);
    QFETCH(bool, line3enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setLineLaser1Available(line1enabled);
    filterModel.setLineLaser2Available(line2enabled);
    filterModel.setLineLaser3Available(line3enabled);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isLineLaser1Available(), line1enabled);
    QCOMPARE(filterModel.isLineLaser2Available(), line2enabled);
    QCOMPARE(filterModel.isLineLaser3Available(), line3enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testNewsonScannerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 15 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testNewsonScannerAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(enabled);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isNewsonScannerAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testScanlabScannerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
    QTest::newRow("disabled") << false << 14 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testScanlabScannerAvailable()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(enabled);
    QCOMPARE(filterModel.isScanlabScannerAvailable(), enabled);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void WizardFilterModelTest::testCameraInterfaceType_data()
{
    QTest::addColumn<AbstractWizardFilterModel::CameraInterfaceType>("interfaceType");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("FrameGrabber") << AbstractWizardFilterModel::CameraInterfaceType::FrameGrabber << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };

    QTest::newRow("GigE") << AbstractWizardFilterModel::CameraInterfaceType::GigE << 17 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::Axis,
        WizardModel::WizardComponent::Camera,
        WizardModel::WizardComponent::IDM,
        WizardModel::WizardComponent::LWM,
        WizardModel::WizardComponent::ScanLabScanner,
        WizardModel::WizardComponent::ToolCenterPointOCT,
        WizardModel::WizardComponent::IDMCalibration,
        WizardModel::WizardComponent::ScanfieldCalibration,
        WizardModel::WizardComponent::CameraCalibration,
        WizardModel::WizardComponent::LEDCalibration,
        WizardModel::WizardComponent::FocusPosition,
        WizardModel::WizardComponent::ToolCenterPoint,
        WizardModel::WizardComponent::LaserControl,
        WizardModel::WizardComponent::LaserControlDelay,
        WizardModel::WizardComponent::ScanTracker,
        WizardModel::WizardComponent::FigureEditor,
        WizardModel::WizardComponent::ProductStructure
    };
}

void WizardFilterModelTest::testCameraInterfaceType()
{
    WizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(AbstractWizardFilterModel::CameraInterfaceType, interfaceType);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setLineLaser1Available(true);
    filterModel.setLineLaser2Available(true);
    filterModel.setLineLaser3Available(true);
    filterModel.setNewsonScannerAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    filterModel.setCameraInterfaceType(interfaceType);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

QTEST_GUILESS_MAIN(WizardFilterModelTest)
#include "wizardFilterModelTest.moc"
