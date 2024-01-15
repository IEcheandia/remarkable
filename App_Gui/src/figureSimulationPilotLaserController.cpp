#include "figureSimulationPilotLaserController.h"
#include "deviceProxyWrapper.h"
#include "common/systemConfiguration.h"

#include <QDebug>

using precitec::interface::Configuration;
using precitec::interface::SmpKeyValue;
using precitec::interface::CorrectionFileMode;

namespace precitec
{
namespace gui
{

FigureSimulationPilotLaserController::FigureSimulationPilotLaserController(QObject* parent)
    : QObject(parent)
{
    connect(this, &FigureSimulationPilotLaserController::wobbleChanged, this, &FigureSimulationPilotLaserController::updateValid);
    connect(this, &FigureSimulationPilotLaserController::seamIdChanged, this, &FigureSimulationPilotLaserController::updateValid);
    connect(this, &FigureSimulationPilotLaserController::wobbleIdChanged, this, &FigureSimulationPilotLaserController::updateValid);
    connect(this, &FigureSimulationPilotLaserController::velocityChanged, this, &FigureSimulationPilotLaserController::updateValid);
}

FigureSimulationPilotLaserController::~FigureSimulationPilotLaserController() = default;

void FigureSimulationPilotLaserController::start()
{
    if (m_running || !m_valid || !m_weldheadDeviceProxy)
    {
        return;
    }
    Configuration config{
        SmpKeyValue{new interface::TKeyValue<int>{std::string{"Scanner_WeldingFigureNumber"}, m_seamId}},
        SmpKeyValue{new interface::TKeyValue<double>{std::string{"Scanner_Jump_Speed"}, m_velocity}},
        SmpKeyValue{new interface::TKeyValue<double>{std::string{"Scanner_Mark_Speed"}, m_velocity}},
        SmpKeyValue{new interface::TKeyValue<int>{std::string{"Scanner_Wobble_Mode"}, m_wobble ? 2 : -2}},
        SmpKeyValue{new interface::TKeyValue<int>{std::string{"Scanner_FileNumber"}, m_wobble ? m_wobbleId : -1}},
        SmpKeyValue{new interface::TKeyValue<int>{std::string{"CorrectionFileMode"}, static_cast<int> (CorrectionFileMode::Pilot)}},
        SmpKeyValue{new interface::TKeyValue<bool>{std::string{"Scanner_StartWeldingPreview"}, true}},
    };
    m_weldheadDeviceProxy->setKeyValues(config);
    m_running = true;
    emit runningChanged();
}

void FigureSimulationPilotLaserController::stop()
{
    if (!m_running || !m_weldheadDeviceProxy)
    {
        return;
    }
    m_weldheadDeviceProxy->setKeyValue(SmpKeyValue{new interface::TKeyValue<bool>{std::string{"Scanner_StopWeldingPreview"}, true}});
    m_weldheadDeviceProxy->setKeyValue(SmpKeyValue{new interface::TKeyValue<int>{std::string{"CorrectionFileMode"}, static_cast<int> (CorrectionFileMode::Welding)}});
    m_running = false;
    emit runningChanged();
}

void FigureSimulationPilotLaserController::setWobble(bool wobble)
{
    if (m_wobble == wobble)
    {
        return;
    }
    m_wobble = wobble;
    emit wobbleChanged();
}

void FigureSimulationPilotLaserController::setSeamId(int seamId)
{
    if (m_seamId == seamId)
    {
        return;
    }
    m_seamId = seamId;
    emit seamIdChanged();
}

void FigureSimulationPilotLaserController::setWobbleId(int wobbleId)
{
    if (m_wobbleId == wobbleId)
    {
        return;
    }
    m_wobbleId = wobbleId;
    emit wobbleIdChanged();
}

void FigureSimulationPilotLaserController::setVelocity(double velocity)
{
    if (qFuzzyCompare(m_velocity, velocity))
    {
        return;
    }
    m_velocity = velocity;
    emit velocityChanged();
}

void FigureSimulationPilotLaserController::setWeldheadDeviceProxy(precitec::gui::DeviceProxyWrapper* device)
{
    if (m_weldheadDeviceProxy == device)
    {
        return;
    }
    disconnect(m_weldheadDeviceDestroyConnection);

    m_weldheadDeviceProxy = device;

    if (m_weldheadDeviceProxy)
    {
        m_weldheadDeviceDestroyConnection = connect(m_weldheadDeviceProxy, &QObject::destroyed, this, std::bind(&FigureSimulationPilotLaserController::setWeldheadDeviceProxy, this, nullptr));
    }
    else
    {
        m_weldheadDeviceDestroyConnection = {};
    }

    emit weldheadDeviceProxyChanged();
}

void FigureSimulationPilotLaserController::updateValid()
{
    bool valid = true;
    if (m_seamId == -1)
    {
        valid = false;
    }
    if (m_wobble && m_wobbleId == -1)
    {
        valid = false;
    }
    if (qFuzzyIsNull(m_velocity))
    {
        valid = false;
    }
    if (valid == m_valid)
    {
        return;
    }
    m_valid = valid;
    emit validChanged();
}

}
}
