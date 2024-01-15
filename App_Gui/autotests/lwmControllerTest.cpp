#include <QTest>
#include <QSignalSpy>
#include <QSettings>

#include "../src/lwmController.h"
#include "../src/deviceProxyWrapper.h"
#include "../src/permissions.h"
#include "precitec/userManagement.h"
#include "precitec/dataSet.h"
#include "precitec/permission.h"
#include "event/sensor.h"
#include "message/device.interface.h"
#include "message/device.proxy.h"
#include "resultSetting.h"
#include "sensorSettingsModel.h"
#include "attributeModel.h"

using precitec::gui::LwmController;
using precitec::gui::DeviceProxyWrapper;
using precitec::gui::Permission;
using precitec::storage::ResultSetting;
using precitec::storage::SensorSettingsModel;
using precitec::storage::AttributeModel;
using precitec::gui::components::user::UserManagement;

static const int NUMBER_OF_SENSORS = 55;

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
        m_value = keyValue->value<int>();
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

    int m_value = -1;
};

}
}

class LwmControllerTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void initTestCase();
    void testCtor();
    void testSensorConfig();
    void testAttributeModel();
    void testDeviceProxy();
    void testBackReflectionAmplification();
    void testLaserPowerAmplification();
    void testPlasmaAmplification();
    void testTemperatureAmplification();

private:
    QTemporaryDir m_dir;
};

void LwmControllerTest::initTestCase()
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

void LwmControllerTest::testCtor()
{
    auto controller = new LwmController{this};

    QVERIFY(!controller->weldheadDeviceProxy());
    QVERIFY(!controller->sensorConfigModel());
    QVERIFY(!controller->attributeModel());
    QVERIFY(controller->backReflection());
    QCOMPARE(controller->backReflection()->name(), QStringLiteral("LWM Back Reflection"));
    QCOMPARE(controller->backReflection()->color(), QColor("mediumseagreen"));
    QVERIFY(controller->laserPower());
    QCOMPARE(controller->laserPower()->name(), QStringLiteral("LWM Laser Power Monitor"));
    QCOMPARE(controller->laserPower()->color(), QColor("mediumturquoise"));
    QVERIFY(controller->plasma());
    QCOMPARE(controller->plasma()->name(), QStringLiteral("LWM Plasma"));
    QCOMPARE(controller->plasma()->color(), QColor("mediumblue"));
    QVERIFY(controller->temperature());
    QCOMPARE(controller->temperature()->name(), QStringLiteral("LWM Temperature"));
    QCOMPARE(controller->temperature()->color(), QColor("mediumorchid"));
    QCOMPARE(controller->backReflectionAmplification(), 0);
    QVERIFY(controller->backReflectionAmplificationModel().empty());
    QCOMPARE(controller->laserPowerAmplification(), 0);
    QVERIFY(controller->laserPowerAmplificationModel().empty());
    QCOMPARE(controller->plasmaAmplification(), 0);
    QVERIFY(controller->plasmaAmplificationModel().empty());
    QCOMPARE(controller->temperatureAmplification(), 0);
    QVERIFY(controller->temperatureAmplificationModel().empty());
    QVERIFY(!controller->ready());
    QVERIFY(!controller->updating());
}

void LwmControllerTest::testSensorConfig()
{
    auto controller = new LwmController{this};

    QVERIFY(!controller->sensorConfigModel());
    QCOMPARE(controller->backReflection()->name(), QStringLiteral("LWM Back Reflection"));
    QCOMPARE(controller->backReflection()->color(), QColor("mediumseagreen"));
    QCOMPARE(controller->laserPower()->name(), QStringLiteral("LWM Laser Power Monitor"));
    QCOMPARE(controller->laserPower()->color(), QColor("mediumturquoise"));
    QCOMPARE(controller->plasma()->name(), QStringLiteral("LWM Plasma"));
    QCOMPARE(controller->plasma()->color(), QColor("mediumblue"));
    QCOMPARE(controller->temperature()->name(), QStringLiteral("LWM Temperature"));
    QCOMPARE(controller->temperature()->color(), QColor("mediumorchid"));

    QSignalSpy sensorConfigModelChangedSpy{controller, &LwmController::sensorConfigModelChanged};
    QVERIFY(sensorConfigModelChangedSpy.isValid());

    controller->setSensorConfigModel(nullptr);
    QCOMPARE(sensorConfigModelChangedSpy.count(), 0);

    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    QVERIFY(QFile::copy(QFINDTESTDATA("testdata/lwm/lwmSensorConfig.json"), dir.filePath(QStringLiteral("sensorConfig.json"))));

    auto model = new SensorSettingsModel{this};
    QSignalSpy dataChangedSpy{model, &SensorSettingsModel::dataChanged};
    QVERIFY(dataChangedSpy.isValid());

    model->setConfigurationDirectory(dir.path());

    controller->setSensorConfigModel(model);
    QCOMPARE(controller->sensorConfigModel(), model);
    QCOMPARE(sensorConfigModelChangedSpy.count(), 1);

    QCOMPARE(controller->backReflection()->name(), QStringLiteral("LWM Back Reflection Label"));
    QCOMPARE(controller->backReflection()->color(), QColor("#00ced1"));
    QCOMPARE(controller->laserPower()->name(), QStringLiteral("LWM Laser Power Monitor Label"));
    QCOMPARE(controller->laserPower()->color(), QColor("#f8f8ff"));
    QCOMPARE(controller->plasma()->name(), QStringLiteral("LWM Plasma Label"));
    QCOMPARE(controller->plasma()->color(), QColor("#6495ed"));
    QCOMPARE(controller->temperature()->name(), QStringLiteral("LWM Temperature Label"));
    QCOMPARE(controller->temperature()->color(), QColor("#a9a9a9"));

    controller->setSensorConfigModel(model);
    QCOMPARE(sensorConfigModelChangedSpy.count(), 1);

    QCOMPARE(model->rowCount(), NUMBER_OF_SENSORS);
    const auto& plasmaIndex = model->indexForResultType(precitec::interface::eLWM40_1_Plasma);
    model->updateValue(plasmaIndex, QStringLiteral("Another Plasma Label"), ResultSetting::Type::Name);
    QCOMPARE(dataChangedSpy.count(), 1);

    QCOMPARE(controller->backReflection()->name(), QStringLiteral("LWM Back Reflection Label"));
    QCOMPARE(controller->backReflection()->color(), QColor("#00ced1"));
    QCOMPARE(controller->laserPower()->name(), QStringLiteral("LWM Laser Power Monitor Label"));
    QCOMPARE(controller->laserPower()->color(), QColor("#f8f8ff"));
    QCOMPARE(controller->plasma()->name(), QStringLiteral("Another Plasma Label"));
    QCOMPARE(controller->plasma()->color(), QColor("#6495ed"));
    QCOMPARE(controller->temperature()->name(), QStringLiteral("LWM Temperature Label"));
    QCOMPARE(controller->temperature()->color(), QColor("#a9a9a9"));
}

void LwmControllerTest::testAttributeModel()
{
    auto controller = new LwmController{this};

    QVERIFY(!controller->attributeModel());
    QVERIFY(controller->backReflectionAmplificationModel().empty());
    QVERIFY(controller->laserPowerAmplificationModel().empty());
    QVERIFY(controller->plasmaAmplificationModel().empty());
    QVERIFY(controller->temperatureAmplificationModel().empty());

    QSignalSpy attributeModelChangedSpy{controller, &LwmController::attributeModelChanged};
    QVERIFY(attributeModelChangedSpy.isValid());

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

    QCOMPARE(controller->backReflectionAmplificationModel(), QStringList({"10.000", "100.000", "1.000.000", "10.000.000", "10", "100", "1.000"}));
    QCOMPARE(controller->laserPowerAmplificationModel(), QStringList({"10.000", "100.000", "1.000.000", "10.000.000", "10", "100", "1.000"}));
    QCOMPARE(controller->plasmaAmplificationModel(), QStringList({"10.000", "100.000", "1.000.000", "10.000.000", "10", "100", "1.000"}));
    QCOMPARE(controller->temperatureAmplificationModel(), QStringList({"10.000", "100.000", "1.000.000", "10.000.000", "10", "100", "1.000"}));

    controller->setAttributeModel(attributeModel);
    QCOMPARE(attributeModelChangedSpy.count(), 1);
}

void LwmControllerTest::testDeviceProxy()
{
    auto controller = new LwmController{this};

    QVERIFY(!controller->weldheadDeviceProxy());
    QVERIFY(!controller->ready());

    QSignalSpy weldheadDeviceProxyChangedSpy{controller, &LwmController::weldheadDeviceProxyChanged};
    QVERIFY(weldheadDeviceProxyChangedSpy.isValid());

    QSignalSpy readyChangedSpy{controller, &LwmController::readyChanged};
    QVERIFY(readyChangedSpy.isValid());

    QSignalSpy backReflectionAmplificationChangedSpy{controller, &LwmController::backReflectionAmplificationChanged};
    QVERIFY(backReflectionAmplificationChangedSpy.isValid());

    QSignalSpy laserPowerAmplificationChangedSpy{controller, &LwmController::laserPowerAmplificationChanged};
    QVERIFY(laserPowerAmplificationChangedSpy.isValid());

    QSignalSpy plasmaAmplificationChangedSpy{controller, &LwmController::plasmaAmplificationChanged};
    QVERIFY(plasmaAmplificationChangedSpy.isValid());

    QSignalSpy temperatureAmplificationChangedSpy{controller, &LwmController::temperatureAmplificationChanged};
    QVERIFY(temperatureAmplificationChangedSpy.isValid());

    controller->setWeldheadDeviceProxy(nullptr);
    QCOMPARE(weldheadDeviceProxyChangedSpy.count(), 0);
    QCOMPARE(readyChangedSpy.count(), 0);
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 0);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 0);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 0);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QCOMPARE(weldheadDeviceProxyChangedSpy.count(), 1);
    QCOMPARE(controller->weldheadDeviceProxy(), weldheadDeviceProxy);
    QTRY_COMPARE(readyChangedSpy.count(), 1);
    QVERIFY(controller->ready());
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 1);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 1);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 1);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 1);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QCOMPARE(weldheadDeviceProxyChangedSpy.count(), 1);
}

void LwmControllerTest::testBackReflectionAmplification()
{
    auto controller = new LwmController{this};

    QCOMPARE(controller->backReflectionAmplification(), 0);

    QSignalSpy backReflectionAmplificationChangedSpy{controller, &LwmController::backReflectionAmplificationChanged};
    QVERIFY(backReflectionAmplificationChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &LwmController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    controller->setBackReflectionAmplification(0);
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setBackReflectionAmplification(-1);
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setBackReflectionAmplification(7);
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(backReflectionAmplificationChangedSpy.count(), 1);

    controller->setBackReflectionAmplification(5);
    QCOMPARE(controller->backReflectionAmplification(), 5);
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_value, 5);

    controller->setBackReflectionAmplification(5);
    QCOMPARE(backReflectionAmplificationChangedSpy.count(), 2);
}

void LwmControllerTest::testLaserPowerAmplification()
{
    auto controller = new LwmController{this};

    QCOMPARE(controller->laserPowerAmplification(), 0);

    QSignalSpy laserPowerAmplificationChangedSpy{controller, &LwmController::laserPowerAmplificationChanged};
    QVERIFY(laserPowerAmplificationChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &LwmController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    controller->setLaserPowerAmplification(0);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setLaserPowerAmplification(-1);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setLaserPowerAmplification(7);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(laserPowerAmplificationChangedSpy.count(), 1);

    controller->setLaserPowerAmplification(5);
    QCOMPARE(controller->laserPowerAmplification(), 5);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_value, 5);

    controller->setLaserPowerAmplification(5);
    QCOMPARE(laserPowerAmplificationChangedSpy.count(), 2);
}

void LwmControllerTest::testPlasmaAmplification()
{
    auto controller = new LwmController{this};

    QCOMPARE(controller->plasmaAmplification(), 0);

    QSignalSpy plasmaAmplificationChangedSpy{controller, &LwmController::plasmaAmplificationChanged};
    QVERIFY(plasmaAmplificationChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &LwmController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    controller->setPlasmaAmplification(0);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setPlasmaAmplification(-1);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setPlasmaAmplification(7);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(plasmaAmplificationChangedSpy.count(), 1);

    controller->setPlasmaAmplification(5);
    QCOMPARE(controller->plasmaAmplification(), 5);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_value, 5);

    controller->setPlasmaAmplification(5);
    QCOMPARE(plasmaAmplificationChangedSpy.count(), 2);
}

void LwmControllerTest::testTemperatureAmplification()
{
    auto controller = new LwmController{this};

    QCOMPARE(controller->temperatureAmplification(), 0);

    QSignalSpy temperatureAmplificationChangedSpy{controller, &LwmController::temperatureAmplificationChanged};
    QVERIFY(temperatureAmplificationChangedSpy.isValid());

    QSignalSpy updatingChangedSpy{controller, &LwmController::updatingChanged};
    QVERIFY(updatingChangedSpy.isValid());

    controller->setTemperatureAmplification(0);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setTemperatureAmplification(-1);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    controller->setTemperatureAmplification(7);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 0);
    QCOMPARE(updatingChangedSpy.count(), 0);

    auto deviceServer = std::make_shared<precitec::interface::MockDeviceProxy>();
    auto weldheadDeviceProxy =  new DeviceProxyWrapper(deviceServer, Permission::ViewWeldHeadDeviceConfig, Permission::EditWeldHeadDeviceConfig, QByteArrayLiteral("3c57acde-707e-4c7d-a6b5-0e9352568095"), this);

    controller->setWeldheadDeviceProxy(weldheadDeviceProxy);
    QTRY_COMPARE(temperatureAmplificationChangedSpy.count(), 1);

    controller->setTemperatureAmplification(5);
    QCOMPARE(controller->temperatureAmplification(), 5);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 2);
    QCOMPARE(updatingChangedSpy.count(), 1);

    QTRY_COMPARE(updatingChangedSpy.count(), 2);
    QCOMPARE(deviceServer->m_value, 5);

    controller->setTemperatureAmplification(5);
    QCOMPARE(temperatureAmplificationChangedSpy.count(), 2);
}

QTEST_GUILESS_MAIN(LwmControllerTest)
#include "lwmControllerTest.moc"
