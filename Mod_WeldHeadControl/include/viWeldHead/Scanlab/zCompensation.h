#pragma once

#include <utility>
#include "common/systemConfiguration.h"

using precitec::interface::LensType;

namespace precitec
{
namespace hardware
{
namespace welding
{

/**
 * The class contains the mathematical model to compensate different working planes.
 * Because of the offset from one working plane to the working plane which lies at
 * the reference level, the position which the scanner tries to move to has an offset.
 **/
class ZCompensation
{
public:
    ZCompensation();
    ~ZCompensation();

    std::pair<double, double> focalLength() const
    {
        return m_focalLength;
    }

    double zOffset() const
    {
        return m_zOffset;
    }
    void setZOffset(double newZOffset);

    bool compensationActive() const
    {
        return m_compensationActive;
    }
    void setCompensationActive(bool isCompensationActive);

    LensType lensType() const
    {
        return m_lensType;
    }
    void setLensType(LensType newLensType);

    bool isCompensationFixed() const
    {
        return m_isCompensationFixed;
    }
    void setIsCompensationFixed(bool isCompensationFixed);

    double positionValueCompensated(double positionValue, bool isYAxis = false);

private:
    void setFocalLength();
    double compensateZOffset(double positionValue, double focalLength);
    std::pair<double, double> focalLength(LensType lensType);
    bool checkFocalLengthOffsetIsNull(bool isYAxis);
    bool fuzzyIsNull(double value);

    bool m_compensationActive = false;
    bool m_isCompensationFixed = false;
    std::pair<double, double> m_focalLength = {0.0, 0.0};
    double m_zOffset = 0.0;
    LensType m_lensType = LensType::F_Theta_340;
};

}
}
}

