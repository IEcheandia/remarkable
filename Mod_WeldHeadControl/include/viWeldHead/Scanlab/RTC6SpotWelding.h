#pragma once
#include "AbstractSpotWelding.h"

namespace RTC6
{

class Control;

class SpotWelding : public precitec::hardware::AbstractSpotWelding
{
public:
    SpotWelding(Control& control);
    ~SpotWelding();
    void start_Mark() const override;
    void define_Mark(double weldingDurationInSec, double laserPowerCenterInPercent, double laserPowerRingInPercent) override;

private:
    unsigned int transformDurationInSecTo10us(double durationInSec) const;
    double transformLaserPowerToBits(double laserPower) const;

    Control& m_control;
};

}
