#include <QTest>
#include <QSignalSpy>
#include <QSettings>

#include "../src/scanLabController.h"
#include "../src/deviceProxyWrapper.h"
#include "../src/permissions.h"
#include "precitec/userManagement.h"
#include "precitec/permission.h"
#include "message/device.interface.h"
#include "message/device.proxy.h"
#include "attributeModel.h"

using precitec::gui::ScanLabController;
using precitec::gui::DeviceProxyWrapper;
using precitec::gui::Permission;
using precitec::storage::AttributeModel;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace interface
{

class MockDeviceProxy : public TDevice<AbstractInterface>
{

public:
    MockDeviceProxy() {}

    KeyHandle set(SmpKeyValue keyValue, int subDevice = 0) override
    {
        Q_UNUSED(subDevice)
        const auto& key = keyValue->key();

        if ( key == "Scanner_New_X_Position")
        {
            m_x = keyValue->value<double>();
        }
        if ( key == "Scanner_New_Y_Position")
        {
            m_y = keyValue->value<double>();
        }
        if ( key == "Scanner_DriveToPosition")
        {
            m_driveToPosition = keyValue->value<bool>();
        }
        if ( key == "Scanner_DriveToZero")
        {
            m_x = 0.0;
            m_y = 0.0;
            m_driveToZero = keyValue->value<bool>();
        }

        return KeyHandle();
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
        return {};
    }

    double m_x = 0.0;
    double m_y = 0.0;
    bool m_driveToPosition = false;
    bool m_driveToZero = false;
};

}
}

class ScanLabControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testAttributeModel();
    void testDeviceProxy();
    void testScannerXPosition();
    void testScannerYPosition();
    void testResetToZero();
    void testIncrementDecrement();

private:
    QTemporaryDir m_dir;
};

void ScanLabControllerTest::initTestCase()
{
    UserManagement::instance()->installPermissions({
        {int(precitec::gui::Permission::ViewWeldHeadDeviceConfig), QLatin1String("")},
        {int(precitec::gui::Permission::EditWeldHeadDeviceConfig), QLatin1String("")}
    });
    QSettings settings(QFINDTESTDATA("testdata/lwm/user.ini"), QSettings::IniFormat);

    UserManagement::instance()->loadUsers(&settings);
    QVERIFY(UserManagement::instance()->authenticate(1, QStringLiteral("1234")));
    QVERIFY(UserManagement::instance()->hasPermission(int(Permission::ViewWeldHeadDeviceConfig)));
    QVERIFY(UserManagement::instance()->hasPermission(int(Permission::EditWeldHeadDeviceConfig)));

    QVERIFY(m_dir.isValid());
    QVERIFY(QDir{}.mkpath(m_dir.filePath(QStringLiteral("config"))));
    qputenv("WM_BASE_DIR", m_dir.path().toUtf8());
}

void ScanLabControllerTest::testCtor()
{
    auto controller = new ScanLabController{this};

    QVERIFY(!controller->weldheadDeviceProxy());
    QVERIFY(!controller->attributeModel());
    QCOMPARE(controller->scannerXPosition(), 0.0);
    QCOMPARE(controller->scannerYPosition(), 0.0);
    QVERIFY(!controller->ready());
    QVERIFY(!controller->updating());
    QCOMPARE(controller->xMinLimit(), -500.0);
    QCOMPARE(controller->xMaxLimit(), 500.0);
    QCOMPARE(controller->yMinLimit(), -500.0);
    QCOMPARE(controller->yMaxLimit(), 500.0);
    QVERIFY(controller->canIncrementX());
    QVERIFY(controller->canDecrementX());
    QVERIFY(controller->canIncrementY());
    QVERIFY(controller->canDecrementY());
}

void ScanLabControllerTest::testAttributeModel()
{
    auto controller = new ScanLabController{this};

    QVERIFY(!controller->attributeModel());
    QCOMPARE(controller->xMinLimit(), -500.0);
    QCOMPARE(controller->xMaxLimit(), 500.0);
    QCOMPARE(controller->yMinLimit(), -500.0);
    QCOMPARE(controller->yMaxLimit(), 500.0);

    // set values different from the template attribute
    controller->m_xMin = -1;
    controller->m_xMax = 1;
    controller->m_yMin = -1;
    controller->m_yMax = 1;

    QCOMPARE(controller->xMinLimit(), -1.0);
    QCOMPARE(controller->xMaxLimit(), 1.0);
    QCOMPARE(controller->yMinLimit(), -1.0);
    QCOMPARE(controller->yMaxLimit(), 1.0);

    QSignalSpy attributeModelChangedSpy{controller, &ScanLabController::attributeModelChanged};
    QVERIFY(attributeModelChangedSpy.isValid());

    QSignalSpy xLimitChangedSpy{controller, &ScanLabController::xLimitChanged};
    QVERIFY(xLimitChangedSpy.isValid());

    QSignalSpy yLimitChangedSpy{controller, &ScanLabController::yLimitChanged};
    QVERIFY(yLimitChangedSpy.isValid());

    QSignalSpy canReachXLimitChangedSpy{controller, &ScanLabController::canReachXLimitChanged};
    QVERIFY(canReachXLimitChangedSpy.isValid());

    QSignalSpy canReachYLimitChangedSpy{controller, &ScanLabController::canReachYLimitChanged};
    QVERIFY(canReachYLimitChangedSpy.isValid());

    controller->setAttributeModel(nullptr);
    QCOMPARE(attributeModelChangedSpy.count(), 0);

    auto attributeModel = new AttributeModel{this};
    QSignalSpy attributeModelResetSpy{attributeModel, &AttributeModel::modelReset};
    QVERIFY(attributeModelResetSpy.isValid());

    attributeModel->load(QFINDTESTDATA("../../wm_inst/system_graphs/keyValueAttributes.json"));
    QVERIFY(attributeModelResetSpy.wait());

    controller->setAttributeModel(attributeModel);
    QCOMPARE(controller->attributeModel(), attributeModel);
    QCOMPARE(attributeModelChangedSpy.count(), 1);

    QCOMPARE(xLimitChangedSpy.count(), 1);
    QCOMPARE(yLimitChangedSpy.count(), 1);

    QCOMPARE(canReachXLimitChangedSpy.count(), 1);
    QCOMPARE(canReachYLimitChangedSpy.count(), 1);

    QCOMPARE(controller->xMinLimit(), -500.0);
    QCOMPARE(controller->xMaxLimit(), 500.0);
    QCOMPARE(controller->yMinLimit(), -500.0);
    QCOMPARE(controller->yMaxLimit(), 500.0);

    controller->setAttributeModel(attributeModel);
    QCOMPARE(attributeModelChangedSpy.count(), 1);
}

void ScanLabControllerTest::testDeviceProxy()
{
    auto controller = new ScanLabController{this};

    QVERIFY(!controller->weldheadDeviceProxy());
    QVERIFY(!controller->ready());

    QSignalSpy weldheadDeviceProxyChangedSpy{controller, &ScanLabController::weldheadDeviceProxyChanged};
    QVERIFY(weldheadDeviceProxyChangedSpy.isValid());

    QSignalSpy readyChangedSpy{controller, &ScanLabController::readyChanged};
    QVERIFY(readyChangedSpy.isValid());

    QSignalSpy scannerXPositionChangedSpy{controller, &ScanLabController::scannerXPositionChanged};
    QVERIFY(scannerXPositionChangedSpy.isValid());

    QSignalSpy scannerYPositionChangedSpy{controller, &ScanLabController::scannerYPositionChanged};
    QVERIFY(scannerYPositionChangedSpy.isValid());

    QSignalSpy canReachXLimitChangedSpy{controller, &ScanLabController::canReachXLimitChanged};
    QVERIFY(canReachXLimitChangedSpy.isValid());

    QSignalSpy canReachYLimitChangedSpy{controller, &ScanLabController::canReachYLimitChanged};
    QVERIFY(canReachYLimitChangedSpy.isValid());

    controller->setWeldheadDeviceProxy(nullptr);
    QCOMPARE(weldheadDeviceProxyChangedSpy.count(), 0);
    QCOMPARE(readyChangedSpy.count(), 0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 0);
    QCOMPARE(canReachXLimitChangedSpy.count(), 0);
    QCOMPARE(canReachYLimitChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QCOMPARE(weldheadDeviceProxyChangedSpy.count(), 1);
    QCOMPARE(controller->weldheadDeviceProxy(), weldheadDeviceProxy);
    QTRY_COMPARE(readyChangedSpy.count(), 1);
    QVERIFY(controller->ready());
    QCOMPARE(scannerXPositionChangedSpy.count(), 1);
    QCOMPARE(scannerYPositionChangedSpy.count(), 1);
    QCOMPARE(canReachXLimitChangedSpy.count(), 1);
    QCOMPARE(canReachYLimitChangedSpy.count(), 1);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QCOMPARE(weldheadDeviceProxyChangedSpy.count(), 1);
}

void ScanLabControllerTest::testScannerXPosition()
{
    auto controller = new ScanLabController{this};

    QCOMPARE(controller->scannerXPosition(), 0.0);
    QVERIFY(controller->canIncrementX());
    QVERIFY(controller->canDecrementX());

    QSignalSpy readyChangedSpy{controller, &ScanLabController::readyChanged};
    QVERIFY(readyChangedSpy.isValid());

    QSignalSpy scannerXPositionChangedSpy{controller, &ScanLabController::scannerXPositionChanged};
    QVERIFY(scannerXPositionChangedSpy.isValid());

    QSignalSpy canReachXLimitChangedSpy{controller, &ScanLabController::canReachXLimitChanged};
    QVERIFY(canReachXLimitChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &ScanLabController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    controller->setScannerXPosition(0.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(readyChangedSpy.count(), 1);
    QCOMPARE(scannerXPositionChangedSpy.count(), 1);
    QCOMPARE(canReachXLimitChangedSpy.count(), 1);

    controller->setScannerXPosition(499.5);
    QCOMPARE(controller->scannerXPosition(), 499.5);
    QVERIFY(controller->canIncrementX());
    QVERIFY(controller->canDecrementX());
    QCOMPARE(scannerXPositionChangedSpy.count(), 2);
    QCOMPARE(canReachXLimitChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_x, 499.5);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerXPosition(502.0);
    QCOMPARE(controller->scannerXPosition(), 500.0);
    QVERIFY(!controller->canIncrementX());
    QVERIFY(controller->canDecrementX());
    QCOMPARE(scannerXPositionChangedSpy.count(), 3);
    QCOMPARE(canReachXLimitChangedSpy.count(), 3);
    QCOMPARE(updatingChangedSpy.count(), 3);

    QTRY_COMPARE(updatingChangedSpy.count(), 4);
    QCOMPARE(deviceServer->m_x, 500.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerXPosition(507.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 3);
    QCOMPARE(canReachXLimitChangedSpy.count(), 3);
    QCOMPARE(updatingChangedSpy.count(), 4);

    controller->setScannerXPosition(-502.0);
    QCOMPARE(controller->scannerXPosition(), -500.0);
    QVERIFY(controller->canIncrementX());
    QVERIFY(!controller->canDecrementX());
    QCOMPARE(scannerXPositionChangedSpy.count(), 4);
    QCOMPARE(canReachXLimitChangedSpy.count(), 4);
    QCOMPARE(updatingChangedSpy.count(), 5);

    QTRY_COMPARE(updatingChangedSpy.count(), 6);
    QCOMPARE(deviceServer->m_x, -500.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerXPosition(-507.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 4);
    QCOMPARE(canReachXLimitChangedSpy.count(), 4);
    QCOMPARE(updatingChangedSpy.count(), 6);
}

void ScanLabControllerTest::testScannerYPosition()
{
    auto controller = new ScanLabController{this};

    QCOMPARE(controller->scannerYPosition(), 0.0);
    QVERIFY(controller->canIncrementY());
    QVERIFY(controller->canDecrementY());

    QSignalSpy readyChangedSpy{controller, &ScanLabController::readyChanged};
    QVERIFY(readyChangedSpy.isValid());

    QSignalSpy scannerYPositionChangedSpy{controller, &ScanLabController::scannerYPositionChanged};
    QVERIFY(scannerYPositionChangedSpy.isValid());

    QSignalSpy canReachYLimitChangedSpy{controller, &ScanLabController::canReachYLimitChanged};
    QVERIFY(canReachYLimitChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &ScanLabController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    controller->setScannerYPosition(0.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 0);
    QCOMPARE(canReachYLimitChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(readyChangedSpy.count(), 1);
    QCOMPARE(scannerYPositionChangedSpy.count(), 1);
    QCOMPARE(canReachYLimitChangedSpy.count(), 1);

    controller->setScannerYPosition(499.5);
    QCOMPARE(controller->scannerYPosition(), 499.5);
    QVERIFY(controller->canIncrementY());
    QVERIFY(controller->canDecrementY());
    QCOMPARE(scannerYPositionChangedSpy.count(), 2);
    QCOMPARE(canReachYLimitChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_y, 499.5);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerYPosition(502.0);
    QCOMPARE(controller->scannerYPosition(), 500.0);
    QVERIFY(!controller->canIncrementY());
    QVERIFY(controller->canDecrementY());
    QCOMPARE(scannerYPositionChangedSpy.count(), 3);
    QCOMPARE(canReachYLimitChangedSpy.count(), 3);
    QCOMPARE(updatingChangedSpy.count(), 3);

    QTRY_COMPARE(updatingChangedSpy.count(), 4);
    QCOMPARE(deviceServer->m_y, 500.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerYPosition(507.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 3);
    QCOMPARE(canReachYLimitChangedSpy.count(), 3);
    QCOMPARE(updatingChangedSpy.count(), 4);

    controller->setScannerYPosition(-502.0);
    QCOMPARE(controller->scannerYPosition(), -500.0);
    QVERIFY(controller->canIncrementY());
    QVERIFY(!controller->canDecrementY());
    QCOMPARE(scannerYPositionChangedSpy.count(), 4);
    QCOMPARE(canReachYLimitChangedSpy.count(), 4);
    QCOMPARE(updatingChangedSpy.count(), 5);

    QTRY_COMPARE(updatingChangedSpy.count(), 6);
    QCOMPARE(deviceServer->m_y, -500.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerYPosition(-507.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 4);
    QCOMPARE(canReachYLimitChangedSpy.count(), 4);
    QCOMPARE(updatingChangedSpy.count(), 6);
}

void ScanLabControllerTest::testResetToZero()
{
    auto controller = new ScanLabController{this};

    QCOMPARE(controller->scannerXPosition(), 0.0);
    QCOMPARE(controller->scannerYPosition(), 0.0);

    QSignalSpy readyChangedSpy{controller, &ScanLabController::readyChanged};
    QVERIFY(readyChangedSpy.isValid());

    QSignalSpy scannerXPositionChangedSpy{controller, &ScanLabController::scannerXPositionChanged};
    QVERIFY(scannerXPositionChangedSpy.isValid());

    QSignalSpy scannerYPositionChangedSpy{controller, &ScanLabController::scannerYPositionChanged};
    QVERIFY(scannerYPositionChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &ScanLabController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(readyChangedSpy.count(), 1);
    QCOMPARE(scannerXPositionChangedSpy.count(), 1);
    QCOMPARE(scannerYPositionChangedSpy.count(), 1);

    controller->setScannerXPosition(20.0);
    QCOMPARE(controller->scannerXPosition(), 20.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_x, 20.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->setScannerYPosition(30.0);
    QCOMPARE(controller->scannerYPosition(), 30.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 3);

    QTRY_COMPARE(updatingChangedSpy.count(), 4);
    QCOMPARE(deviceServer->m_y, 30.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->resetToZero();
    QCOMPARE(updatingChangedSpy.count(), 5);

    QTRY_COMPARE(updatingChangedSpy.count(), 6);
    QCOMPARE(controller->scannerXPosition(), 0.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 3);
    QCOMPARE(controller->scannerYPosition(), 0.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 3);
    QVERIFY(deviceServer->m_driveToZero);
}

void ScanLabControllerTest::testIncrementDecrement()
{
    auto controller = new ScanLabController{this};

    QCOMPARE(controller->scannerXPosition(), 0.0);
    QCOMPARE(controller->scannerYPosition(), 0.0);

    QSignalSpy readyChangedSpy{controller, &ScanLabController::readyChanged};
    QVERIFY(readyChangedSpy.isValid());

    QSignalSpy scannerXPositionChangedSpy{controller, &ScanLabController::scannerXPositionChanged};
    QVERIFY(scannerXPositionChangedSpy.isValid());

    QSignalSpy scannerYPositionChangedSpy{controller, &ScanLabController::scannerYPositionChanged};
    QVERIFY(scannerYPositionChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &ScanLabController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(readyChangedSpy.count(), 1);
    QCOMPARE(scannerXPositionChangedSpy.count(), 1);
    QCOMPARE(scannerYPositionChangedSpy.count(), 1);

    controller->incrementXPosition();;
    QCOMPARE(controller->scannerXPosition(), 1.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_x, 1.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->decrementXPosition();
    QCOMPARE(controller->scannerXPosition(), 0.0);
    QCOMPARE(scannerXPositionChangedSpy.count(), 3);
    QCOMPARE(updatingChangedSpy.count(), 3);

    QTRY_COMPARE(updatingChangedSpy.count(), 4);
    QCOMPARE(deviceServer->m_x, 0.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->incrementYPosition();
    QCOMPARE(controller->scannerYPosition(), 1.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 5);

    QTRY_COMPARE(updatingChangedSpy.count(), 6);
    QCOMPARE(deviceServer->m_y, 1.0);
    QVERIFY(deviceServer->m_driveToPosition);

    deviceServer->m_driveToPosition = false;
    QVERIFY(!deviceServer->m_driveToPosition);

    controller->decrementYPosition();
    QCOMPARE(controller->scannerYPosition(), 0.0);
    QCOMPARE(scannerYPositionChangedSpy.count(), 3);
    QCOMPARE(updatingChangedSpy.count(), 7);

    QTRY_COMPARE(updatingChangedSpy.count(), 8);
    QCOMPARE(deviceServer->m_y, 0.0);
    QVERIFY(deviceServer->m_driveToPosition);
}

QTEST_GUILESS_MAIN(ScanLabControllerTest)
#include "scanLabControllerTest.moc"

