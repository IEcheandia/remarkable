#pragma once

#include "liveModeController.h"
#include "event/results.interface.h"
#include "image/ipSignal.h"
#include "hardwareParameters.h"

#include <functional>

namespace precitec
{
namespace storage
{

class Seam;
class SensorSettingsModel;
class AttributeModel;

}
namespace gui
{
namespace components
{
namespace plotter
{

class DataSet;

}
}

class DeviceProxyWrapper;

class LwmController : public LiveModeController
{
    Q_OBJECT

    Q_PROPERTY(precitec::gui::DeviceProxyWrapper* weldheadDeviceProxy READ weldheadDeviceProxy WRITE setWeldheadDeviceProxy NOTIFY weldheadDeviceProxyChanged)

    Q_PROPERTY(precitec::storage::SensorSettingsModel* sensorConfigModel READ sensorConfigModel WRITE setSensorConfigModel NOTIFY sensorConfigModelChanged)

    Q_PROPERTY(precitec::storage::AttributeModel* attributeModel READ attributeModel WRITE setAttributeModel NOTIFY attributeModelChanged)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* backReflection READ backReflection CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* laserPower READ laserPower CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* plasma READ plasma CONSTANT)

    Q_PROPERTY(precitec::gui::components::plotter::DataSet* temperature READ temperature CONSTANT)

    Q_PROPERTY(int backReflectionAmplification READ backReflectionAmplification WRITE setBackReflectionAmplification NOTIFY backReflectionAmplificationChanged)

    Q_PROPERTY(int laserPowerAmplification READ laserPowerAmplification WRITE setLaserPowerAmplification NOTIFY laserPowerAmplificationChanged)

    Q_PROPERTY(int plasmaAmplification READ plasmaAmplification WRITE setPlasmaAmplification NOTIFY plasmaAmplificationChanged)

    Q_PROPERTY(int temperatureAmplification READ temperatureAmplification WRITE setTemperatureAmplification NOTIFY temperatureAmplificationChanged)

    Q_PROPERTY(bool ready READ ready NOTIFY readyChanged)

    Q_PROPERTY(bool updating READ updating NOTIFY updatingChanged)

    Q_PROPERTY(QStringList backReflectionAmplificationModel READ backReflectionAmplificationModel NOTIFY attributeModelChanged)

    Q_PROPERTY(QStringList laserPowerAmplificationModel READ laserPowerAmplificationModel NOTIFY attributeModelChanged)

    Q_PROPERTY(QStringList plasmaAmplificationModel READ plasmaAmplificationModel NOTIFY attributeModelChanged)

    Q_PROPERTY(QStringList temperatureAmplificationModel READ temperatureAmplificationModel NOTIFY attributeModelChanged)

public:
    explicit LwmController(QObject* parent = nullptr);
    ~LwmController();

    DeviceProxyWrapper* weldheadDeviceProxy() const
    {
        return m_weldheadDeviceProxy;
    }
    void setWeldheadDeviceProxy(DeviceProxyWrapper* device);

    int backReflectionAmplification() const
    {
        return m_backReflectionAmplification;
    }
    void setBackReflectionAmplification(int amplification);

    int laserPowerAmplification() const
    {
        return m_laserPowerAmplification;
    }
    void setLaserPowerAmplification(int amplification);

    int plasmaAmplification() const
    {
        return m_plasmaAmplification;
    }
    void setPlasmaAmplification(int amplification);

    int temperatureAmplification() const
    {
        return m_temperatureAmplification;
    }
    void setTemperatureAmplification(int amplification);

    bool ready() const
    {
        return m_ready;
    }

    bool updating() const
    {
        return m_updating;
    }

    precitec::gui::components::plotter::DataSet* backReflection() const
    {
        return m_backReflection;
    }

    precitec::gui::components::plotter::DataSet* laserPower() const
    {
        return m_laserPower;
    }

    precitec::gui::components::plotter::DataSet* plasma() const
    {
        return m_plasma;
    }

    precitec::gui::components::plotter::DataSet* temperature() const
    {
        return m_temperature;
    }

    precitec::storage::SensorSettingsModel* sensorConfigModel() const
    {
        return m_sensorConfigModel;
    }
    void setSensorConfigModel(precitec::storage::SensorSettingsModel* model);

    precitec::storage::AttributeModel* attributeModel() const
    {
        return m_attributeModel;
    }
    void setAttributeModel(precitec::storage::AttributeModel* model);

    QStringList backReflectionAmplificationModel() const;

    QStringList laserPowerAmplificationModel() const;

    QStringList plasmaAmplificationModel() const;

    QStringList temperatureAmplificationModel() const;

public Q_SLOTS:
    void addSample(int sensor, const precitec::image::Sample& sample);

Q_SIGNALS:
    void samplesRendered();
    void sensorConfigModelChanged();
    void weldheadDeviceProxyChanged();
    void attributeModelChanged();
    void backReflectionAmplificationChanged();
    void laserPowerAmplificationChanged();
    void plasmaAmplificationChanged();
    void temperatureAmplificationChanged();
    void readyChanged();
    void updatingChanged();

private:
    precitec::storage::Seam* defaultSeam() const;
    void insertSamples(precitec::gui::components::plotter::DataSet* dataSet, const precitec::image::Sample& sample, int triggerDelta);
    void updateSettings();
    void updateSensorData(precitec::gui::components::plotter::DataSet* dataSet, int sensor);
    void setReady(bool set);
    void setUpdating(bool set);
    void update(std::function<void()> updateFunction);
    bool hasPermission();
    QStringList enumValues(HardwareParameters::Key key) const;

    int m_backReflectionAmplification = 0;
    int m_laserPowerAmplification = 0;
    int m_plasmaAmplification = 0;
    int m_temperatureAmplification = 0;
    bool m_ready = false;
    bool m_updating = false;

    precitec::gui::components::plotter::DataSet* m_backReflection;
    precitec::gui::components::plotter::DataSet* m_laserPower;
    precitec::gui::components::plotter::DataSet* m_plasma;
    precitec::gui::components::plotter::DataSet* m_temperature;

    DeviceProxyWrapper* m_weldheadDeviceProxy = nullptr;
    QMetaObject::Connection  m_weldheadDeviceDestroyConnection;

    precitec::storage::SensorSettingsModel* m_sensorConfigModel = nullptr;
    QMetaObject::Connection m_sensorConfigModelDestroyedConnection;

    precitec::storage::AttributeModel* m_attributeModel = nullptr;
    QMetaObject::Connection m_attributeModelDestroyedConnection;
};

}
}

Q_DECLARE_METATYPE(precitec::gui::LwmController*)
