#include "lwmController.h"
#include "productModel.h"
#include "product.h"
#include "seamSeries.h"
#include "seam.h"
#include "sensorSettingsModel.h"
#include "deviceProxyWrapper.h"
#include "attributeModel.h"
#include "attribute.h"

#include "precitec/dataSet.h"
#include "precitec/userManagement.h"
#include "event/sensor.h"

#include <QFutureWatcher>
#include <QtConcurrentRun>

using precitec::image::Sample;
using precitec::storage::Seam;
using precitec::storage::SensorSettingsModel;
using precitec::storage::ResultSetting;
using precitec::storage::AttributeModel;
using precitec::interface::Configuration;
using precitec::interface::ImageContext;
using precitec::interface::SmpKeyValue;
using precitec::interface::TKeyValue;
using precitec::gui::components::plotter::DataSet;
using precitec::gui::components::user::UserManagement;

namespace precitec
{
namespace gui
{

namespace
{

template <typename T>
T getValue(const Configuration &configuration, const std::string &key, T defaultValue)
{
    auto it = std::find_if(configuration.begin(), configuration.end(), [key] (auto kv) { return kv->key() == key; });
    if (it == configuration.end())
    {
        return defaultValue;
    }
    return (*it)->template value<T>();
}

}

LwmController::LwmController(QObject* parent)
    : LiveModeController(parent)
    , m_backReflection(new DataSet{this})
    , m_laserPower(new DataSet{this})
    , m_plasma(new DataSet{this})
    , m_temperature(new DataSet{this})
{
    connect(this, &LwmController::sensorConfigModelChanged, this, &LwmController::updateSettings);
    connect(this, &LwmController::weldheadDeviceProxyChanged, this,
        [this]
        {
            if (!m_weldheadDeviceProxy)
            {
                setReady(false);
                return;
            }
            auto weldheadDeviceProxy = m_weldheadDeviceProxy;
            auto watcher = new QFutureWatcher<Configuration>{this};
            connect(watcher, &QFutureWatcher<Configuration>::finished, this,
                [this, watcher]
                {
                    watcher->deleteLater();
                    auto configuration = watcher->result();

                    m_backReflectionAmplification = getValue(configuration, std::string("LWM40_No1_AmpBackReflection"), 0);
                    emit backReflectionAmplificationChanged();
                    m_laserPowerAmplification = getValue(configuration, std::string("LWM40_No1_AmpAnalogInput"), 0);
                    emit laserPowerAmplificationChanged();
                    m_plasmaAmplification = getValue(configuration, std::string("LWM40_No1_AmpPlasma"), 0);
                    emit plasmaAmplificationChanged();
                    m_temperatureAmplification = getValue(configuration, std::string("LWM40_No1_AmpTemperature"), 0);
                    emit temperatureAmplificationChanged();

                    setReady(true);
                }
            );
            watcher->setFuture(QtConcurrent::run(
                [weldheadDeviceProxy]
                {
                    return weldheadDeviceProxy->deviceProxy()->get();
                }));
        }
    );

    updateSettings();
}

LwmController::~LwmController() = default;

void LwmController::addSample(int sensor, const Sample& sample)
{
    auto seam = defaultSeam();
    if (!seam)
    {
        emit samplesRendered();
        return;
    }

    switch (sensor)
    {
        case precitec::interface::eLWM40_1_Plasma:
            insertSamples(m_plasma, sample, seam->triggerDelta());
            break;
        case precitec::interface::eLWM40_1_Temperature:
            insertSamples(m_temperature, sample, seam->triggerDelta());
            break;
        case precitec::interface::eLWM40_1_BackReflection:
            insertSamples(m_backReflection, sample, seam->triggerDelta());
            break;
        case precitec::interface::eLWM40_1_AnalogInput:
            insertSamples(m_laserPower, sample, seam->triggerDelta());
            break;
    };
    emit samplesRendered();
}

void LwmController::insertSamples(DataSet* dataSet, const Sample& sample, int triggerDelta)
{
    if (!dataSet)
    {
        return;
    }

    const auto& oversamplingRate = sample.getSize();

    const auto& sampleDistance = (float) triggerDelta / (float) oversamplingRate;

    std::vector<QVector2D> spectrumVector;
    spectrumVector.reserve(oversamplingRate);
    for (std::size_t i = 0u; i < oversamplingRate; i++)
    {
        const auto& position = (i * sampleDistance) / 1000.0f;
        spectrumVector.emplace_back(position, float(sample[i]));
    }

    dataSet->clear();
    dataSet->addSamples(spectrumVector);
}

Seam* LwmController::defaultSeam() const
{
    if (!productModel())
    {
        return nullptr;
    }
    auto defaultProduct = productModel()->defaultProduct();
    if (!defaultProduct)
    {
        return nullptr;
    }
    if (defaultProduct->seamSeries().empty())
    {
        return nullptr;
    }
    auto seamSeries = defaultProduct->seamSeries().front();
    if (seamSeries->seams().empty())
    {
        return nullptr;
    }
    return seamSeries->seams().front();
}

void LwmController::setSensorConfigModel(SensorSettingsModel* model)
{
    if (m_sensorConfigModel == model)
    {
        return;
    }

    if (m_sensorConfigModel)
    {
        disconnect(m_sensorConfigModelDestroyedConnection);
        disconnect(m_sensorConfigModel, &QAbstractItemModel::modelReset, this, &LwmController::updateSettings);
        disconnect(m_sensorConfigModel, &QAbstractItemModel::dataChanged, this, &LwmController::updateSettings);
    }

    m_sensorConfigModel = model;

    if (m_sensorConfigModel)
    {
        m_sensorConfigModelDestroyedConnection = connect(m_sensorConfigModel, &QObject::destroyed, this, std::bind(&LwmController::setSensorConfigModel, this, nullptr));
        connect(m_sensorConfigModel, &QAbstractItemModel::modelReset, this, &LwmController::updateSettings);
        connect(m_sensorConfigModel, &QAbstractItemModel::dataChanged, this, &LwmController::updateSettings);
    } else
    {
        m_sensorConfigModelDestroyedConnection = {};
    }

    emit sensorConfigModelChanged();
}

void LwmController::setWeldheadDeviceProxy(DeviceProxyWrapper* device)
{
    if (m_weldheadDeviceProxy == device)
    {
        return;
    }
    disconnect(m_weldheadDeviceDestroyConnection);

    m_weldheadDeviceProxy = device;

    if (m_weldheadDeviceProxy)
    {
        m_weldheadDeviceDestroyConnection = connect(m_weldheadDeviceProxy, &QObject::destroyed, this, std::bind(&LwmController::setWeldheadDeviceProxy, this, nullptr));
    } else
    {
        m_weldheadDeviceDestroyConnection = {};
    }

    emit weldheadDeviceProxyChanged();
}

void LwmController::updateSettings()
{
    updateSensorData(m_backReflection, precitec::interface::eLWM40_1_BackReflection);
    updateSensorData(m_laserPower, precitec::interface::eLWM40_1_AnalogInput);
    updateSensorData(m_plasma, precitec::interface::eLWM40_1_Plasma);
    updateSensorData(m_temperature, precitec::interface::eLWM40_1_Temperature);
}

void LwmController::updateSensorData(DataSet* dataSet, int sensor)
{
    ResultSetting* resultConfig = sensorConfigModel() ? sensorConfigModel()->getItem(sensor) : nullptr;

    if (resultConfig)
    {
        dataSet->setColor(QColor(resultConfig->lineColor()));
        dataSet->setName(resultConfig->name());
    } else
    {
        dataSet->setName(SensorSettingsModel::sensorName(sensor));
        dataSet->setColor(SensorSettingsModel::sensorColor(sensor));
    }

    dataSet->setDrawingMode(DataSet::DrawingMode::LineWithPoints);
    dataSet->setMaxElements(100000);
}

void LwmController::setReady(bool set)
{
    if (m_ready == set)
    {
        return;
    }
    m_ready = set;
    emit readyChanged();
}

void LwmController::setUpdating(bool set)
{
    if (m_updating == set)
    {
        return;
    }
    m_updating = set;
    emit updatingChanged();
}

void LwmController::setBackReflectionAmplification(int amplification)
{
    if (m_backReflectionAmplification == amplification || amplification < 0 || amplification > 6)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }

    m_backReflectionAmplification = amplification;

    update([this, amplification]
    {
        if (m_weldheadDeviceProxy)
        {
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LWM40_No1_AmpBackReflection"), amplification)));
        }
    });

    emit backReflectionAmplificationChanged();
}

void LwmController::setLaserPowerAmplification(int amplification)
{
    if (m_laserPowerAmplification == amplification || amplification < 0 || amplification > 6)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }

    m_laserPowerAmplification = amplification;

    update([this, amplification]
    {
        if (m_weldheadDeviceProxy)
        {
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LWM40_No1_AmpAnalogInput"), amplification)));
        }
    });

    emit laserPowerAmplificationChanged();
}

void LwmController::setPlasmaAmplification(int amplification)
{
    if (m_plasmaAmplification == amplification || amplification < 0 || amplification > 6)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }

    m_plasmaAmplification = amplification;

    update([this, amplification]
    {
        if (m_weldheadDeviceProxy)
        {
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LWM40_No1_AmpPlasma"), amplification)));
        }
    });

    emit plasmaAmplificationChanged();
}

void LwmController::setTemperatureAmplification(int amplification)

{
    if (m_temperatureAmplification == amplification || amplification < 0 || amplification > 6)
    {
        return;
    }
    if (!hasPermission())
    {
        return;
    }

    m_temperatureAmplification = amplification;

    update([this, amplification]
    {
        if (m_weldheadDeviceProxy)
        {
            m_weldheadDeviceProxy->setKeyValue(SmpKeyValue(new TKeyValue<int>(std::string("LWM40_No1_AmpTemperature"), amplification)));
        }
    });

    emit temperatureAmplificationChanged();
}

void LwmController::update(std::function<void()> updateFunction)
{
    if (!m_weldheadDeviceProxy || updating())
    {
        return;
    }

    setUpdating(true);

    if (liveMode())
    {
        stopLiveMode();
    }

    QFutureWatcher<void> *watcher = new QFutureWatcher<void>(this);
    connect(watcher, &QFutureWatcher<void>::finished, this,
        [this, watcher]
        {
            watcher->deleteLater();
            if (liveMode())
            {
                startLiveMode();
            }
            setUpdating(false);
        }
    );
    watcher->setFuture(QtConcurrent::run(updateFunction));
}

bool LwmController::hasPermission()
{
    return m_weldheadDeviceProxy && UserManagement::instance()->hasPermission(int(m_weldheadDeviceProxy->writePermission()));
}

void LwmController::setAttributeModel(AttributeModel *model)
{
    if (m_attributeModel == model)
    {
        return;
    }

    m_attributeModel = model;
    disconnect(m_attributeModelDestroyedConnection);

    if (m_attributeModel)
    {
        m_attributeModelDestroyedConnection = connect(m_attributeModel, &AttributeModel::destroyed, this, std::bind(&LwmController::setAttributeModel, this, nullptr));
    } else
    {
        m_attributeModelDestroyedConnection = {};
    }

    emit attributeModelChanged();
}

QStringList LwmController::backReflectionAmplificationModel() const
{
    return enumValues(HardwareParameters::Key::LWM40No1AmpBackReflection);
}

QStringList LwmController::laserPowerAmplificationModel() const
{
    return enumValues(HardwareParameters::Key::LWM40No1AmpAnalogInput);
}

QStringList LwmController::plasmaAmplificationModel() const
{
    return enumValues(HardwareParameters::Key::LWM40No1AmpPlasma);
}

QStringList LwmController::temperatureAmplificationModel() const
{
    return enumValues(HardwareParameters::Key::LWM40No1AmpTemperature);
}

QStringList LwmController::enumValues(HardwareParameters::Key key) const
{
    if (!m_attributeModel)
    {
        return {};
    }

    if (auto attribute = m_attributeModel->findAttribute(HardwareParameters::instance()->properties(key).uuid))
    {
        return attribute->fields();
    }

    return {};
}

}
}
