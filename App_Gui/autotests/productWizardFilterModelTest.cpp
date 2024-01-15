#include <QTest>

#include "../src/productWizardFilterModel.h"
#include "../src/wizardModel.h"

using precitec::gui::ProductWizardFilterModel;
using precitec::gui::WizardModel;

class ProductWizardFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testSensorGrabberAvailable_data();
    void testSensorGrabberAvailable();
    void testLaserControlAvailable_data();
    void testLaserControlAvailable();
    void testScanTrackerAvailable_data();
    void testScanTrackerAvailable();
    void testLWMAvailable_data();
    void testLWMAvailable();
    void testIDMAvailable_data();
    void testIDMAvailable();
    void testZCollimatorAvailable_data();
    void testZCollimatorAvailable();
    void testScanlabScannerAvailable_data();
    void testScanlabScannerAvailable();
};

void ProductWizardFilterModelTest::testCtor()
{
    ProductWizardFilterModel filterModel;
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
    QCOMPARE(filterModel.isScanTracker2DAvailable(), false);
    QCOMPARE(filterModel.rowCount(), 0);

    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);
    QCOMPARE(filterModel.rowCount(), 4);
    QCOMPARE(filterModel.index(0, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductColorMaps);
    QCOMPARE(filterModel.index(1, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductHardwareParametersOverview);
    QCOMPARE(filterModel.index(2, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductDetectionOverview);
    QCOMPARE(filterModel.index(3, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductError);

    filterModel.setYAxisAvailable(true);
    filterModel.setSensorGrabberAvailable(true);
    filterModel.setIDMAvailable(true);
    filterModel.setCoaxCameraAvailable(true);
    filterModel.setLEDCameraAvailable(true);
    filterModel.setLEDAvailable(true);
    filterModel.setZCollimator(true);
    filterModel.setScanTracker(true);
    filterModel.setLaserControlAvailable(true);
    filterModel.setLEDCalibration(true);
    filterModel.setLWMAvailable(true);
    filterModel.setScanlabScannerAvailable(true);
    QCOMPARE(filterModel.isYAxisAvailable(), true);
    QCOMPARE(filterModel.isSensorGrabberAvailable(), true);
    QCOMPARE(filterModel.isIDMAvailable(), true);
    QCOMPARE(filterModel.isCoaxCameraAvailable(), true);
    QCOMPARE(filterModel.isLEDCameraAvailable(), true);
    QCOMPARE(filterModel.isLEDAvailable(), true);
    QCOMPARE(filterModel.zCollimator(), true);
    QCOMPARE(filterModel.scanTracker(), true);
    QCOMPARE(filterModel.isLaserControlAvailable(), true);
    QCOMPARE(filterModel.ledCalibration(), true);
    QCOMPARE(filterModel.isLWMAvailable(), true);
    QCOMPARE(filterModel.isScanlabScannerAvailable(), true);
    QCOMPARE(filterModel.rowCount(), 11);

    int index = 0;
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductColorMaps);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductHardwareParametersOverview);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductDetectionOverview);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductCamera);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductError);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductLaserControl);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductScanTracker);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductLaserWeldingMonitor);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductScanLabScanner);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductIDM);
    QCOMPARE(filterModel.index(index++, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::ProductZCollimator);
}

void ProductWizardFilterModelTest::testSensorGrabberAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductCamera,
        WizardModel::WizardComponent::ProductError
    };

    QTest::newRow("disabled") << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testSensorGrabberAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setSensorGrabberAvailable(enabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setSensorGrabberAvailable(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

void ProductWizardFilterModelTest::testLaserControlAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductLaserControl
    };

    QTest::newRow("disabled") << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testLaserControlAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setLaserControlAvailable(enabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setLaserControlAvailable(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

void ProductWizardFilterModelTest::testScanTrackerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductScanTracker
    };

    QTest::newRow("disabled") << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testScanTrackerAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setScanTracker(enabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setScanTracker(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

void ProductWizardFilterModelTest::testLWMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductLaserWeldingMonitor
    };

    QTest::newRow("disabled") << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testLWMAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setLWMAvailable(enabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setLWMAvailable(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

void ProductWizardFilterModelTest::testIDMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductIDM
    };

    QTest::newRow("disabled") << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testIDMAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setIDMAvailable(enabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setIDMAvailable(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

void ProductWizardFilterModelTest::testZCollimatorAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductZCollimator
    };

    QTest::newRow("disabled") << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testZCollimatorAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setZCollimator(enabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setZCollimator(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

void ProductWizardFilterModelTest::testScanlabScannerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("scanTrackerEnabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled scanmaster") << true << false << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductScanLabScanner,
    };

    QTest::newRow("enabled scantracker") << true << true << 5 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError,
        WizardModel::WizardComponent::ProductScanTracker2D,
    };

    QTest::newRow("disabled") << false << false << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };

    QTest::newRow("disabled, scantracker2d enabled") << false << true << 4 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::ProductColorMaps,
        WizardModel::WizardComponent::ProductHardwareParametersOverview,
        WizardModel::WizardComponent::ProductDetectionOverview,
        WizardModel::WizardComponent::ProductError
    };
}

void ProductWizardFilterModelTest::testScanlabScannerAvailable()
{
    ProductWizardFilterModel filterModel;
    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);

    QFETCH(bool, enabled);
    filterModel.setScanlabScannerAvailable(enabled);

    QFETCH(bool, scanTrackerEnabled);
    filterModel.setScanTracker2DAvailable(scanTrackerEnabled);

    QFETCH(int, count);
    QCOMPARE(filterModel.rowCount(), count);

    QFETCH(QVector<WizardModel::WizardComponent>, components);
    QCOMPARE(components.count(), filterModel.rowCount());
    for (int i = 0; i < filterModel.rowCount(); i++)
    {
        QCOMPARE(filterModel.index(i, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), components.at(i));
    }

    filterModel.setScanlabScannerAvailable(!enabled);
    QVERIFY(filterModel.rowCount() != count);
}

QTEST_GUILESS_MAIN(ProductWizardFilterModelTest)
#include "productWizardFilterModelTest.moc"
