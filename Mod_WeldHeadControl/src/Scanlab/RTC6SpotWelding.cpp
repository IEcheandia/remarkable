#include "viWeldHead/Scanlab/RTC6SpotWelding.h"
#include "viWeldHead/Scanlab/RTC6Control.h"

#include <rtc6.h>

namespace RTC6
{

SpotWelding::SpotWelding(RTC6::Control& control)
    : precitec::hardware::AbstractSpotWelding()
    , m_control(control)
{
}

SpotWelding::~SpotWelding()
{
    stop_execution();
}

void SpotWelding::start_Mark() const
{
    UINT oStatus = 0;
    UINT oPos = 0;
    while ((oStatus & 1) == 1)
    {
        get_status(&oStatus, &oPos);
    }
     execute_list(1);
}

void SpotWelding::define_Mark(double weldingDurationInSec, double laserPowerCenterInPercent, double laserPowerRingInPercent)
{
    AbstractSpotWelding::define_Mark(weldingDurationInSec, laserPowerCenterInPercent, laserPowerRingInPercent);

    const auto analogOutput = m_control.getAnalogOutput().value;

    while (load_list(1, 0) == 0);
    set_laser_power(analogOutput -1, transformLaserPowerToBits(laserPowerCenterInPercent));
    set_laser_power(analogOutput, transformLaserPowerToBits(laserPowerRingInPercent));
    const auto weldingDurationIn10us = transformDurationInSecTo10us(m_durationInSec);
    laser_on_list(weldingDurationIn10us);
    set_end_of_list();
}

unsigned int SpotWelding::transformDurationInSecTo10us(double durationInSec) const
{
    return static_cast<unsigned int>(durationInSec * 100000);
}

double SpotWelding::transformLaserPowerToBits(double laserPower) const
{
    return laserPower * 0.01 * m_control.getAnalogOutput().max;
}

}
