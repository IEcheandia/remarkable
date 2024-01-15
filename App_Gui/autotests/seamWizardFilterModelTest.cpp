#include <QTest>
#include <QSignalSpy>

#include "../src/linkedSeamWizardFilterModel.h"
#include "../src/seamWizardFilterModel.h"
#include "../src/wizardModel.h"
#include "guiConfiguration.h"

using precitec::gui::LinkedSeamWizardFilterModel;
using precitec::gui::SeamWizardFilterModel;
using precitec::gui::WizardModel;
using precitec::gui::GuiConfiguration;

class SeamWizardFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
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
    void testScanLabScanner_data();
    void testScanLabScanner();
    void testZCollimator_data();
    void testZCollimator();
    void testSeamIntervalErrors();
    void testLinkedSeam();
};

void SeamWizardFilterModelTest::initTestCase()
{
    GuiConfiguration::instance()->setSeamIntervalsOnProductStructure(false);
}

void SeamWizardFilterModelTest::testCtor()
{
    SeamWizardFilterModel filterModel;
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
    QCOMPARE(filterModel.isScanlabScannerAvailable(), false);
    QCOMPARE(filterModel.rowCount(), 0);

    LinkedSeamWizardFilterModel linkedFilterModel;
    QCOMPARE(linkedFilterModel.isYAxisAvailable(), false);
    QCOMPARE(linkedFilterModel.isSensorGrabberAvailable(), false);
    QCOMPARE(linkedFilterModel.isIDMAvailable(), false);
    QCOMPARE(linkedFilterModel.isCoaxCameraAvailable(), false);
    QCOMPARE(linkedFilterModel.isLEDCameraAvailable(), false);
    QCOMPARE(linkedFilterModel.isLEDAvailable(), false);
    QCOMPARE(linkedFilterModel.zCollimator(), false);
    QCOMPARE(linkedFilterModel.scanTracker(), false);
    QCOMPARE(linkedFilterModel.isLaserControlAvailable(), false);
    QCOMPARE(linkedFilterModel.ledCalibration(), false);
    QCOMPARE(linkedFilterModel.isLWMAvailable(), false);
    QCOMPARE(linkedFilterModel.isScanlabScannerAvailable(), false);
    QCOMPARE(linkedFilterModel.rowCount(), 0);
}

void SeamWizardFilterModelTest::testYAxisAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testYAxisAvailable()
{
    SeamWizardFilterModel filterModel;
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

void SeamWizardFilterModelTest::testScanTrackerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testScanTrackerAvailable()
{
    SeamWizardFilterModel filterModel;
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

void SeamWizardFilterModelTest::testSensorGrabberAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testSensorGrabberAvailable()
{
    SeamWizardFilterModel filterModel;
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

void SeamWizardFilterModelTest::testIDMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testIDMAvailable()
{
    SeamWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(enabled);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
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

void SeamWizardFilterModelTest::testCameraAvailable_data()
{
    QTest::addColumn<bool>("enabledCoax");
    QTest::addColumn<bool>("enabledLED");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled Coax") << true << false << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("enabled LED") << false << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << false << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testCameraAvailable()
{
    SeamWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabledCoax);
    QFETCH(bool, enabledLED);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setYAxisAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(enabledCoax);
    filterModel.setLEDCameraAvailable(enabledLED);
    filterModel.setLEDAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setScanTracker(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
    filterModel.setZCollimator(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isCoaxCameraAvailable(), enabledCoax);
    QCOMPARE(filterModel.isLEDCameraAvailable(), enabledLED);

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void SeamWizardFilterModelTest::testZCollimatorAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
    };
}

void SeamWizardFilterModelTest::testZCollimatorAvailable()
{
    SeamWizardFilterModel filterModel;
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

void SeamWizardFilterModelTest::testLEDAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("souvis");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled with souvis") << true << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled with souvis") << false << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

     QTest::newRow("enabled without souvis") << true << false << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled without souvis") << false << false << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testLEDAvailable()
{
    SeamWizardFilterModel filterModel;
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

void SeamWizardFilterModelTest::testLaserControlAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testLaserControlAvailable()
{
    SeamWizardFilterModel filterModel;
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
    filterModel.setZCollimator(true);
    filterModel.setLWMAvailable(true);
    filterModel.setExternalLWMAvailable(false);
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


void SeamWizardFilterModelTest::testLWMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };
}

void SeamWizardFilterModelTest::testLWMAvailable()
{
    SeamWizardFilterModel filterModel;
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

void SeamWizardFilterModelTest::testScanLabScanner_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("scanTrackerEnabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << false << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("enabled, scantracker2d") << true << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
        WizardModel::WizardComponent::SeamScanTracker2D,
    };

    QTest::newRow("disabled") << false << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator
    };

    QTest::newRow("disabled, scantracker2d enabled") << false << true << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator
    };
}

void SeamWizardFilterModelTest::testScanLabScanner()
{
    SeamWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);
    QSignalSpy spy{&filterModel, &SeamWizardFilterModel::scanlabScannerAvailableChanged};
    QVERIFY(spy.isValid());

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
    QFETCH(bool, scanTrackerEnabled);
    filterModel.setScanTracker2DAvailable(scanTrackerEnabled);

    QCOMPARE(spy.count(), 0);
    filterModel.setScanlabScannerAvailable(enabled);
    if (enabled)
    {
        QCOMPARE(spy.count(), 1);
        QCOMPARE(filterModel.isScanlabScannerAvailable(), enabled);
    }

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void SeamWizardFilterModelTest::testZCollimator_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 12 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
        WizardModel::WizardComponent::SeamZCollimator,
    };

    QTest::newRow("disabled") << false << 11 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamAssemblyImage,
        WizardModel::WizardComponent::SeamCamera,
        WizardModel::WizardComponent::SeamAxis,
        WizardModel::WizardComponent::SeamDetection,
        WizardModel::WizardComponent::SeamError,
        WizardModel::WizardComponent::SeamReferenceCurves,
        WizardModel::WizardComponent::SeamLaserControl,
        WizardModel::WizardComponent::SeamScanTracker,
        WizardModel::WizardComponent::SeamLaserWeldingMonitor,
        WizardModel::WizardComponent::SeamScanLabScanner,
        WizardModel::WizardComponent::SeamIDM,
    };
}

void SeamWizardFilterModelTest::testZCollimator()
{
    SeamWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);
    QSignalSpy spy{&filterModel, &SeamWizardFilterModel::zCollimatorChanged};
    QVERIFY(spy.isValid());

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
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(spy.count(), 0);
    filterModel.setZCollimator(enabled);
    if (enabled)
    {
        QCOMPARE(spy.count(), 1);
        QCOMPARE(filterModel.zCollimator(), enabled);
    }

    QTEST(filterModel.rowCount(), "count");

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }
}

void SeamWizardFilterModelTest::testSeamIntervalErrors()
{
    SeamWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

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
    filterModel.setScanlabScannerAvailable(true);

    QCOMPARE(filterModel.rowCount(), 12);

    GuiConfiguration::instance()->setSeamIntervalsOnProductStructure(true);
    QCOMPARE(filterModel.rowCount(), 13);
    QCOMPARE(filterModel.index(12, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamIntervalError);

    GuiConfiguration::instance()->setSeamIntervalsOnProductStructure(false);
    QCOMPARE(filterModel.rowCount(), 12);
}

void SeamWizardFilterModelTest::testLinkedSeam()
{
    LinkedSeamWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QCOMPARE(filterModel.rowCount(), 1);
    QCOMPARE(filterModel.index(0, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamAssemblyImage);

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
    filterModel.setScanlabScannerAvailable(true);

    QCOMPARE(filterModel.rowCount(), 1);
    QCOMPARE(filterModel.index(0, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamAssemblyImage);
}

QTEST_GUILESS_MAIN(SeamWizardFilterModelTest)
#include "seamWizardFilterModelTest.moc"
