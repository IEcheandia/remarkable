#include <QTest>
#include <QSignalSpy>

#include "../src/scanfieldModule.h"
#include "../src/deviceProxyWrapper.h"
#include "../src/permissions.h"
#include "message/calibrationCoordinatesRequest.interface.h"

using precitec::gui::ScanfieldModule;
using precitec::gui::DeviceProxyWrapper;
using precitec::gui::Permission;
using precitec::geo2d::DPoint;

typedef std::shared_ptr<precitec::interface::TCalibrationCoordinatesRequest<precitec::interface::AbstractInterface>> CalibrationCoordinatesRequestProxy;

namespace precitec
{
namespace interface
{

class MockGrabberProxy : public TDevice<AbstractInterface>
{

public:
    MockGrabberProxy() {}

    KeyHandle set(SmpKeyValue keyValue, int subDevice = 0) override
    {
        Q_UNUSED(keyValue)
        Q_UNUSED(subDevice)
        return {};
    }
    int initialize(Configuration const& config, int subDevice = 0) override
    {
        Q_UNUSED(config)
        Q_UNUSED(subDevice)
        return -1;
    }
    void uninitialize() override {}
    void reinitialize() override {}
    void set(Configuration config, int subDevice = 0) override
    {
        Q_UNUSED(config)
        Q_UNUSED(subDevice)
    }
    SmpKeyValue get(Key key, int subDevice = 0) override
    {
        Q_UNUSED(key)
        Q_UNUSED(subDevice)
        return {};
    }
    SmpKeyValue get(KeyHandle handle, int subDevice = 0) override
    {
        Q_UNUSED(handle)
        Q_UNUSED(subDevice)
        return {};
    }
    Configuration get(int subDevice = 0) override
    {
        Q_UNUSED(subDevice)

        Configuration config;

        config.push_back( SmpKeyValue( new TKeyValue<int>( "Window.WMax", m_width, 0, 2000, 1280) ) );
        config.push_back( SmpKeyValue( new TKeyValue<int>( "Window.HMax", m_height, 0, 2000, 1024) ) );

        return config;
    }

    int m_width = 1400;
    int m_height = 1600;
};

class MockCalibrationProxy : public TCalibrationCoordinatesRequest<AbstractInterface>
{
public:
    MockCalibrationProxy() {};
    Vec3D get3DCoordinates(ScreenCooordinate_t pX, ScreenCooordinate_t pY, SensorId p_oSensorID, LaserLine p_LaserLine)
    {
        Q_UNUSED(pX)
        Q_UNUSED(pY)
        Q_UNUSED(p_oSensorID)
        Q_UNUSED(p_LaserLine)
        return {};
    }
    DPoint getCoordinatesFromGrayScaleImage(ScreenCooordinate_t pX, ScreenCooordinate_t pY, ScannerContextInfo scannerContext, SensorId p_oSensorID)
    {
        Q_UNUSED(pX)
        Q_UNUSED(pY)
        Q_UNUSED(scannerContext)
        Q_UNUSED(p_oSensorID)
        return {};
    }
    DPoint getTCPPosition(SensorId p_oSensorId, ScannerContextInfo scannerContext)
    {
        Q_UNUSED(p_oSensorId)
        Q_UNUSED(scannerContext)
        return {};
    }
    bool availableOCTCoordinates(OCT_Mode mode)
    {
        Q_UNUSED(mode)
        return false;
    }
    DPoint getNewsonPosition(double xScreen, double yScreen, OCT_Mode mode)
    {
        Q_UNUSED(xScreen)
        Q_UNUSED(yScreen)
        Q_UNUSED(mode)
        return {};
    }
    DPoint getScreenPositionFromNewson(double xNewson, double yNewson, OCT_Mode mode)
    {
        Q_UNUSED(xNewson)
        Q_UNUSED(yNewson)
        Q_UNUSED(mode)
        return {};
    }
    DPoint getScannerPositionFromScanFieldImage(double x_pixel, double y_pixel, Configuration scanfieldImageConfiguration)
    {
        Q_UNUSED(scanfieldImageConfiguration)
        return DPoint{10.0 * x_pixel, 10.0 * y_pixel};
    }
    DPoint getScanFieldImagePositionFromScannerPosition(double x_mm, double y_mm, Configuration scanfieldImageConfiguration)
    {
        Q_UNUSED(scanfieldImageConfiguration)
        return DPoint{0.1 * x_mm, 0.1 * y_mm};
    }
};

}
}

class ScanfieldModuleTest : public QObject
{
    Q_OBJECT
private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testGrabberProxy();
    void testSeries();
    void testImageDir();
    void testConfiguration();
    void testCopyFromOtherSeries();
    void testCalibrationProxy();
    void testImageSize();
    void testCenterValid();
    void testCameraRect();
    void testMirrorRect();
    void testToValidCameraCenter();

private:
    QTemporaryDir m_dir;
};

void ScanfieldModuleTest::initTestCase()
{
    QVERIFY(m_dir.isValid());
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void ScanfieldModuleTest::testCtor()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(module->series().isNull());
    QVERIFY(!module->calibrationCoordinatesRequestProxy());
    QVERIFY(module->sourceImageDir().isEmpty());
    QVERIFY(!module->configurationValid());
    QVERIFY(!module->grabberDeviceProxy());

    QCOMPARE(module->cameraSize(), QSize(1280, 1024));
    QCOMPARE(module->imageSize(), QSize(7168, 4096));
}

void ScanfieldModuleTest::testGrabberProxy()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(!module->grabberDeviceProxy());
    QCOMPARE(module->cameraSize(), QSize(1280, 1024));

    QSignalSpy grabberDeviceProxyChangedSpy{module, &ScanfieldModule::grabberDeviceProxyChanged};
    QVERIFY(grabberDeviceProxyChangedSpy.isValid());

    QSignalSpy cameraSizeChangedSpy{module, &ScanfieldModule::cameraSizeChanged};
    QVERIFY(cameraSizeChangedSpy.isValid());

    module->setGrabberDeviceProxy(nullptr);
    QCOMPARE(grabberDeviceProxyChangedSpy.count(), 0);
    QCOMPARE(cameraSizeChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockGrabberProxy>();
    auto gabberDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("1f50352e-a92a-4521-b184-e16809345026"), this);

    module->setGrabberDeviceProxy(gabberDeviceProxy);

    QCOMPARE(grabberDeviceProxyChangedSpy.count(), 1);
    QTRY_COMPARE(cameraSizeChangedSpy.count(), 1);

    QCOMPARE(module->cameraSize(), QSize(1400, 1600));

    module->setGrabberDeviceProxy(gabberDeviceProxy);
    QCOMPARE(grabberDeviceProxyChangedSpy.count(), 1);
    QCOMPARE(cameraSizeChangedSpy.count(), 1);

    module->setGrabberDeviceProxy(nullptr);

    QCOMPARE(grabberDeviceProxyChangedSpy.count(), 2);
    QTRY_COMPARE(cameraSizeChangedSpy.count(), 2);

    QVERIFY(!module->grabberDeviceProxy());
    QCOMPARE(module->cameraSize(), QSize(1280, 1024));
}

void ScanfieldModuleTest::testSeries()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(module->series().isNull());

    QSignalSpy seriesChangedSpy{module, &ScanfieldModule::seriesChanged};
    QVERIFY(seriesChangedSpy.isValid());

    module->setSeries({});
    QCOMPARE(seriesChangedSpy.count(), 0);

    const auto& uuid = QUuid::createUuid();
    module->setSeries(uuid);
    QCOMPARE(module->series(), uuid);
    QCOMPARE(seriesChangedSpy.count(), 1);

    module->setSeries(uuid);
    QCOMPARE(seriesChangedSpy.count(), 1);
}

void ScanfieldModuleTest::testImageDir()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(module->sourceImageDir().isEmpty());

    QSignalSpy sourceImageDirChangedSpy{module, &ScanfieldModule::sourceImageDirChanged};
    QVERIFY(sourceImageDirChangedSpy.isValid());

    const auto& uuid = QUuid::createUuid();
    module->setSeries(uuid);

    QCOMPARE(sourceImageDirChangedSpy.count(), 1);
    QString baseDir{QString::fromUtf8(qgetenv("WM_BASE_DIR"))};
    if (baseDir.endsWith(QDir::separator()))
    {
        baseDir = baseDir.left(baseDir.size() - 1);
    }
    QCOMPARE(module->sourceImageDir(), baseDir + QStringLiteral("/config/scanfieldimage/%1/ScanFieldImage.jpg").arg(uuid.toString(QUuid::WithoutBraces)));

    module->setSeries(uuid);
    QCOMPARE(sourceImageDirChangedSpy.count(), 1);
}

void ScanfieldModuleTest::testConfiguration()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(!module->configurationValid());

    QSignalSpy configurationValidChangedSpy{module, &ScanfieldModule::configurationValidChanged};
    QVERIFY(configurationValidChangedSpy.isValid());

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");

    module->loadCalibrationFile(testdata);

    QCOMPARE(configurationValidChangedSpy.count(), 1);
    QVERIFY(module->configurationValid());

    const auto config = module->m_calibrationConfig;
    QCOMPARE(config.size(), 9);

    QCOMPARE(config.at(0)->key(), "SM_scanXToPixel");
    QCOMPARE(config.at(1)->key(), "SM_scanYToPixel");
    QCOMPARE(config.at(2)->key(), "SM_slopePixel");
    QCOMPARE(config.at(3)->key(), "SM_mirrorX");
    QCOMPARE(config.at(4)->key(), "SM_mirrorY");
    QCOMPARE(config.at(5)->key(), "xMinLeft_mm");
    QCOMPARE(config.at(6)->key(), "yMinTop_mm");
    QCOMPARE(config.at(7)->key(), "ScanFieldImageWidth");
    QCOMPARE(config.at(8)->key(), "ScanFieldImageHeight");

    QCOMPARE(config.at(0)->value<double>(), -27.0);
    QCOMPARE(config.at(1)->value<double>(), 27.0);
    QCOMPARE(config.at(2)->value<double>(), 0);
    QCOMPARE(config.at(3)->value<bool>(), true);
    QCOMPARE(config.at(4)->value<bool>(), true);
    QCOMPARE(config.at(5)->value<double>(), 62.6666666666667);
    QCOMPARE(config.at(6)->value<double>(), -57.1666666666667);
    QCOMPARE(config.at(7)->value<int>(), 3402);
    QCOMPARE(config.at(8)->value<int>(), 3105);
}

void ScanfieldModuleTest::testCopyFromOtherSeries()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(!module->configurationValid());

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");

    QVERIFY(QDir{}.mkpath(m_dir.filePath("config/scanfieldimage/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778")));
    QDir seriesDir{m_dir.path() + QStringLiteral("/config/scanfieldimage/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778")};
    for (const auto& info : QDir{testdata}.entryInfoList(QDir::Files))
    {
        QVERIFY(QFile::copy(info.absoluteFilePath(), seriesDir.absoluteFilePath(info.fileName())));
    }

    module->setSeries(QUuid::createUuid());
    module->copyFromOtherSeries(QStringLiteral("1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778"));

    QVERIFY(module->configurationValid());

    const auto config = module->m_calibrationConfig;
    QCOMPARE(config.size(), 9);

    QCOMPARE(config.at(0)->key(), "SM_scanXToPixel");
    QCOMPARE(config.at(1)->key(), "SM_scanYToPixel");
    QCOMPARE(config.at(2)->key(), "SM_slopePixel");
    QCOMPARE(config.at(3)->key(), "SM_mirrorX");
    QCOMPARE(config.at(4)->key(), "SM_mirrorY");
    QCOMPARE(config.at(5)->key(), "xMinLeft_mm");
    QCOMPARE(config.at(6)->key(), "yMinTop_mm");
    QCOMPARE(config.at(7)->key(), "ScanFieldImageWidth");
    QCOMPARE(config.at(8)->key(), "ScanFieldImageHeight");

    QCOMPARE(config.at(0)->value<double>(), -27.0);
    QCOMPARE(config.at(1)->value<double>(), 27.0);
    QCOMPARE(config.at(2)->value<double>(), 0);
    QCOMPARE(config.at(3)->value<bool>(), true);
    QCOMPARE(config.at(4)->value<bool>(), true);
    QCOMPARE(config.at(5)->value<double>(), 62.6666666666667);
    QCOMPARE(config.at(6)->value<double>(), -57.1666666666667);
    QCOMPARE(config.at(7)->value<int>(), 3402);
    QCOMPARE(config.at(8)->value<int>(), 3105);
}

void ScanfieldModuleTest::testCalibrationProxy()
{
    auto module = new ScanfieldModule{this};

    QVERIFY(!module->calibrationCoordinatesRequestProxy());
    QVERIFY(!module->configurationValid());
    QCOMPARE(module->imageToScannerCoordinates(100, 200), QPointF(-1, -1));
    QCOMPARE(module->scannerToImageCoordinates(100, 200), QPointF(-1, -1));

    QSignalSpy configurationValidChangedSpy{module, &ScanfieldModule::configurationValidChanged};
    QVERIFY(configurationValidChangedSpy.isValid());

    QSignalSpy calibrationCoordinatesRequestProxyChangedSpy{module, &ScanfieldModule::calibrationCoordinatesRequestProxyChanged};
    QVERIFY(calibrationCoordinatesRequestProxyChangedSpy.isValid());

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");

    module->loadCalibrationFile(testdata);

    QCOMPARE(configurationValidChangedSpy.count(), 1);
    QCOMPARE(calibrationCoordinatesRequestProxyChangedSpy.count(), 0);
    QVERIFY(module->configurationValid());

    QCOMPARE(module->imageToScannerCoordinates(100, 200), QPointF(-1, -1));
    QCOMPARE(module->scannerToImageCoordinates(100, 200), QPointF(-1, -1));

    auto proxy = std::make_shared<precitec::interface::MockCalibrationProxy>();

    module->setCalibrationCoordinatesRequestProxy(proxy);
    QCOMPARE(module->calibrationCoordinatesRequestProxy(), proxy);
    QCOMPARE(configurationValidChangedSpy.count(), 1);
    QCOMPARE(calibrationCoordinatesRequestProxyChangedSpy.count(), 1);

    QCOMPARE(module->imageToScannerCoordinates(100, 200), QPointF(1000, 2000));
    QCOMPARE(module->scannerToImageCoordinates(100, 200), QPointF(10, 20));
}

void ScanfieldModuleTest::testImageSize()
{
    auto module = new ScanfieldModule{this};

    QCOMPARE(module->imageSize(), QSize(7168, 4096));

    QSignalSpy imageSizeChangedSpy{module, &ScanfieldModule::imageSizeChanged};
    QVERIFY(imageSizeChangedSpy.isValid());

    module->setImageSize({7168, 4096});
    QCOMPARE(imageSizeChangedSpy.count(), 0);

    module->setImageSize({4096, 2048});
    QCOMPARE(module->imageSize(), QSize(4096, 2048));
    QCOMPARE(imageSizeChangedSpy.count(), 1);

    module->setImageSize({4096, 2048});
    QCOMPARE(imageSizeChangedSpy.count(), 1);
}

void ScanfieldModuleTest::testCenterValid()
{
    auto module = new ScanfieldModule{this};

    QCOMPARE(module->cameraSize(), QSize(1280, 1024));
    QCOMPARE(module->imageSize(), QSize(7168, 4096));

    QVERIFY(module->centerValid({700, 600}));
    QVERIFY(!module->centerValid({350, 600}));
    QVERIFY(!module->centerValid({6800, 600}));
    QVERIFY(!module->centerValid({700, 500}));
    QVERIFY(!module->centerValid({700, 3700}));
}

void ScanfieldModuleTest::testCameraRect()
{
    auto module = new ScanfieldModule{this};

    QCOMPARE(module->cameraSize(), QSize(1280, 1024));
    QCOMPARE(module->imageSize(), QSize(7168, 4096));

    QCOMPARE(module->cameraRect({700, 600}), QRectF(60, 88, 1280, 1024));
    QCOMPARE(module->cameraRect({350, 600}), QRectF(0, 88, 1280, 1024));
    QCOMPARE(module->cameraRect({6800, 600}), QRectF(5888, 88, 1280, 1024));
    QCOMPARE(module->cameraRect({700, 500}), QRectF(60, 0, 1280, 1024));
    QCOMPARE(module->cameraRect({700, 3700}), QRectF(60, 3072, 1280, 1024));
}

void ScanfieldModuleTest::testMirrorRect()
{
    auto module = new ScanfieldModule{this};

    QCOMPARE(module->cameraSize(), QSize(1280, 1024));
    QCOMPARE(module->imageSize(), QSize(7168, 4096));
    QVERIFY(!module->configurationValid());

    QSignalSpy configurationValidChangedSpy{module, &ScanfieldModule::configurationValidChanged};
    QVERIFY(configurationValidChangedSpy.isValid());

    QCOMPARE(module->mirrorRect({60, 88, 100, 200}), QRectF(60, 88, 100, 200));

    const auto& testdata = QFINDTESTDATA("testdata/scanner_data/1cd8f7cb-e57a-4c21-b05d-1b40cc2b0778/");

    module->loadCalibrationFile(testdata);

    QCOMPARE(configurationValidChangedSpy.count(), 1);
    QVERIFY(module->configurationValid());

    QCOMPARE(module->mirrorRect({60, 88, 100, 200}), QRectF(1280 - 60 - 100, 1024 - 88 - 200, 100, 200));
}

void ScanfieldModuleTest::testToValidCameraCenter()
{
    auto module = new ScanfieldModule{this};

    QCOMPARE(module->cameraSize(), QSize(1280, 1024));
    QCOMPARE(module->imageSize(), QSize(7168, 4096));

    QCOMPARE(module->toValidCameraCenter({700, 600}), QPointF(700, 600));
    QCOMPARE(module->toValidCameraCenter({350, 600}), QPointF(640, 600));
    QCOMPARE(module->toValidCameraCenter({6800, 600}), QPointF(6528, 600));
    QCOMPARE(module->toValidCameraCenter({700, 500}), QPointF(700, 512));
    QCOMPARE(module->toValidCameraCenter({700, 3700}), QPointF(700, 3584));
}

QTEST_GUILESS_MAIN(ScanfieldModuleTest)
#include "scanfieldModuleTest.moc"

