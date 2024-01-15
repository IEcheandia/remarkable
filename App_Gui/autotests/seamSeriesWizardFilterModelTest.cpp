#include <QTest>

#include "../src/seamSeriesWizardFilterModel.h"
#include "../src/wizardModel.h"

using precitec::gui::SeamSeriesWizardFilterModel;
using precitec::gui::WizardModel;

class SeamSeriesWizardFilterModelTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void testCtor();
    void testScannerAvailable_data();
    void testScannerAvailable();
    void testScanTrackerAvailable_data();
    void testScanTrackerAvailable();
    void testIDMAvailable_data();
    void testIDMAvailable();
    void testLaserControlAvailable_data();
    void testLaserControlAvailable();
    void testLWMAvailable_data();
    void testLWMAvailable();
    void testZCollimatorAvailable_data();
    void testZCollimatorAvailable();
};

void SeamSeriesWizardFilterModelTest::testCtor()
{
    SeamSeriesWizardFilterModel filterModel;
    QCOMPARE(filterModel.sourceModel(), nullptr);
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

    WizardModel wizardModel;
    filterModel.setSourceModel(&wizardModel);
    QCOMPARE(filterModel.rowCount(), 1);
    QCOMPARE(filterModel.index(0, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesError);

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
    QCOMPARE(filterModel.rowCount(), 8);

    QCOMPARE(filterModel.index(0, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesAcquireScanField);
    QCOMPARE(filterModel.index(1, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesError);
    QCOMPARE(filterModel.index(2, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesLaserControl);
    QCOMPARE(filterModel.index(3, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesScanTracker);
    QCOMPARE(filterModel.index(4, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesLaserWeldingMonitor);
    QCOMPARE(filterModel.index(5, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesScanLabScanner);
    QCOMPARE(filterModel.index(6, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesIDM);
    QCOMPARE(filterModel.index(7, 0).data(Qt::UserRole + 2).value<WizardModel::WizardComponent>(), WizardModel::WizardComponent::SeamSeriesZCollimator);
}

void SeamSeriesWizardFilterModelTest::testScannerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<bool>("scanTrackerEnabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled scanmaster") << true << false << 3 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesAcquireScanField,
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesScanLabScanner,
    };

    QTest::newRow("enabled scantracker2d") << true << true << 2 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesScanTracker2D,
    };

    QTest::newRow("disabled") << false << false << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };

    QTest::newRow("disabled, scantracker2d enabled") << false << true << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };
}

void SeamSeriesWizardFilterModelTest::testScannerAvailable()
{
    SeamSeriesWizardFilterModel filterModel;
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

void SeamSeriesWizardFilterModelTest::testScanTrackerAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 2 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesScanTracker
    };

    QTest::newRow("disabled") << false << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };
}

void SeamSeriesWizardFilterModelTest::testScanTrackerAvailable()
{
    SeamSeriesWizardFilterModel filterModel;
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

void SeamSeriesWizardFilterModelTest::testIDMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 2 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesIDM
    };

    QTest::newRow("disabled") << false << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };
}

void SeamSeriesWizardFilterModelTest::testIDMAvailable()
{
    SeamSeriesWizardFilterModel filterModel;
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

void SeamSeriesWizardFilterModelTest::testLaserControlAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 2 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesLaserControl
    };

    QTest::newRow("disabled") << false << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };
}

void SeamSeriesWizardFilterModelTest::testLaserControlAvailable()
{
    SeamSeriesWizardFilterModel filterModel;
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

void SeamSeriesWizardFilterModelTest::testLWMAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 2 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesLaserWeldingMonitor
    };

    QTest::newRow("disabled") << false << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };
}

void SeamSeriesWizardFilterModelTest::testLWMAvailable()
{
    SeamSeriesWizardFilterModel filterModel;
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

void SeamSeriesWizardFilterModelTest::testZCollimatorAvailable_data()
{
    QTest::addColumn<bool>("enabled");
    QTest::addColumn<int>("count");
    QTest::addColumn<QVector<WizardModel::WizardComponent>>("components");

    QTest::newRow("enabled") << true << 2 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError,
        WizardModel::WizardComponent::SeamSeriesZCollimator
    };

    QTest::newRow("disabled") << false << 1 << QVector<WizardModel::WizardComponent>{
        WizardModel::WizardComponent::SeamSeriesError
    };
}

void SeamSeriesWizardFilterModelTest::testZCollimatorAvailable()
{
    SeamSeriesWizardFilterModel filterModel;
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

QTEST_GUILESS_MAIN(SeamSeriesWizardFilterModelTest)
#include "seamSeriesWizardFilterModelTest.moc"
