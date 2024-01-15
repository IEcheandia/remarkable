#pragma once

namespace precitec
{
namespace hardware
{

class AbstractSpotWelding
{
public:
    AbstractSpotWelding();
    virtual ~AbstractSpotWelding();

    virtual void start_Mark() const = 0;
    /**
     * Default implementation caches the value. If needed in start_Mark call default impl
     **/
    virtual void define_Mark(double weldingDurationInSec, double laserPowerCenterInPercent, double laserPowerRingInPercent);

    double getDurationInSec() const
    {
        return m_durationInSec;
    }

protected:
    double m_durationInSec;
    double m_laserPowerCenterInPercent;
    double m_laserPowerRingInPercent;
};

}
}
