#include "viWeldHead/Scanlab/RTC6Laser.h"

#include "rtc6.h"
#include "common/definesScanlab.h"

namespace RTC6
{
Laser::Laser():
    m_oLaserMode(0),
    m_oAnalogOutput(1),
    m_oMinAnalogOutput(0),
    m_oMaxAnalogOutput(0),
    m_oLaserOnDelay(0),
    m_oLaserOffDelay(0)
{
}

Laser::Laser(unsigned int p_oLaserMode, unsigned int p_oAnalogOutput, unsigned int p_oMinAnalogOutput, unsigned int p_oMaxAnalogOutput, long int p_oLaserOnDelay, long int p_oLaserOffDelay):
    m_oLaserMode(p_oLaserMode),
    m_oAnalogOutput(p_oAnalogOutput),
    m_oMinAnalogOutput(p_oMinAnalogOutput),
    m_oMaxAnalogOutput(p_oMaxAnalogOutput),
    m_oLaserOnDelay(p_oLaserOnDelay),
    m_oLaserOffDelay(p_oLaserOffDelay)
{
}

Laser::~Laser()
{
    disable_laser();
}

void Laser::set_LaserMode(unsigned int p_oLaserMode)
{
    m_oLaserMode = p_oLaserMode;
}

void Laser::set_AnalogOutput(unsigned int p_oAnalogOutput, unsigned int p_oMinAnalogOutput, unsigned int p_oMaxAnalogOutput)
{
    m_oAnalogOutput = p_oAnalogOutput;
    m_oMinAnalogOutput = p_oMinAnalogOutput;
    m_oMaxAnalogOutput = p_oMaxAnalogOutput;
}

void Laser::set_LaserDelays(long int p_oLaserOnDelay, long int p_oLaserOffDelay)
{
    m_oLaserOnDelay = p_oLaserOnDelay;
    m_oLaserOffDelay = p_oLaserOffDelay;
}

unsigned int Laser::get_LaserMode()
{
    return(m_oLaserMode);
}

void Laser::get_AnalogOutput(unsigned int& p_rAnalogOutput, unsigned int& p_rMinAnalogOutput, unsigned int& p_rMaxAnalogOutput) const
{
    p_rAnalogOutput = m_oAnalogOutput;
    p_rMinAnalogOutput = m_oMinAnalogOutput;
    p_rMaxAnalogOutput = m_oMaxAnalogOutput;
}

void Laser::get_LaserDelays(long int& p_rLaserOnDelay, long int& p_rLaserOffDelay) const
{
    p_rLaserOnDelay = m_oLaserOnDelay;
    p_rLaserOffDelay = m_oLaserOffDelay;
}

void Laser::init()
{
    set_laser_mode(m_oLaserMode);

    setLaserControl();
}

void Laser::disable_LaserUnit()
{
    disable_laser();
}

void Laser::setScannerModel(precitec::interface::ScannerModel newScannerModel)
{
    m_scannerModel = newScannerModel;
}

/**
 * Laser control bit explanation (see set_laser_control in rtc6 manual):
 * Bit #3 LaserOn: 1 = high-active, 0 = low-active
 * Bit #16 PowerOk: 1 = PowerOK head-1 x-axis is used for automatic power off
 * Bit #19 PowerOk: 1 = PowerOK head-1 y-axis is used for automatic power off
 * Bit #28 Stop event: 1 = Stop event occurs list is going to be stopped and laser control signals is going to be turned off
 * Bit #29 Stop event for all: Stop event is send to all master and slaves, so all members stop immediately
 **/

void Laser::setLaserControl()
{
    switch (m_scannerModel)
    {
        case ScannerModel::ScanlabScanner:
            set_laser_control(0x30090000);
            break;
        case ScannerModel::SmartMoveScanner:
            set_laser_control(0x00090000);
            break;
    }
}
}
