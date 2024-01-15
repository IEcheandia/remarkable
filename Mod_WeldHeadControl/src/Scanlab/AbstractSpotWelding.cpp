#include "viWeldHead/Scanlab/AbstractSpotWelding.h"

namespace precitec
{
namespace hardware
{

AbstractSpotWelding::AbstractSpotWelding()
{
}

AbstractSpotWelding::~AbstractSpotWelding() = default;

void AbstractSpotWelding::define_Mark(double weldingDurationInSec, double laserPowerCenterInPercent, double laserPowerRingInPercent)
{
    m_durationInSec = weldingDurationInSec;
    m_laserPowerCenterInPercent = laserPowerCenterInPercent;
    m_laserPowerRingInPercent = laserPowerRingInPercent;
}

}
}
