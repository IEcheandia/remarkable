#pragma once
#include "AbstractSpotWelding.h"

namespace precitec
{
namespace hardware
{
class SmartMoveControl;
namespace smartMove
{

class SpotWelding : public precitec::hardware::AbstractSpotWelding
{
public:
    SpotWelding(precitec::hardware::SmartMoveControl& control);
    ~SpotWelding();
    void start_Mark() const override;
    void define_Mark(double weldingDurationInSec, double laserPowerCenterInPercent, double laserPowerRingInPercent) override;

private:
    int sysTsFromSec(double sec) const;
    precitec::hardware::SmartMoveControl& m_control;
};

}
}
}
