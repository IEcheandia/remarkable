#include "viWeldHead/Scanlab/zCompensation.h"
#include "viWeldHead/Scanlab/calculator.h"

#include <cmath>

namespace precitec
{
namespace hardware
{
namespace welding
{

ZCompensation::ZCompensation()
{
}

ZCompensation::~ZCompensation() = default;

void ZCompensation::setZOffset(double newZOffset)
{
    m_zOffset = -1.0 * newZOffset;                      //Multiply by -1.0 to have the same sign like when setting focus /z-collimator
}

void ZCompensation::setCompensationActive(bool isCompensationActive)
{
    m_compensationActive = isCompensationActive;
}

void ZCompensation::setLensType(LensType newLensType)
{
    m_lensType = newLensType;
    setFocalLength();
}

void ZCompensation::setIsCompensationFixed(bool isCompensationFixed)
{
    m_isCompensationFixed = isCompensationFixed;
}

double ZCompensation::positionValueCompensated(double positionValue, bool isYAxis)
{
    if (!compensationActive() || checkFocalLengthOffsetIsNull(isYAxis) || fuzzyIsNull(positionValue))
    {
        return positionValue;
    }
    return isYAxis ? compensateZOffset(positionValue, std::get<1>(m_focalLength)) : compensateZOffset(positionValue, std::get<0>(m_focalLength));
}

void ZCompensation::setFocalLength()
{
    m_focalLength = focalLength(m_lensType);
}

double ZCompensation::compensateZOffset(double positionValue, double focalLength)
{
    return focalLength * positionValue / (focalLength + m_zOffset);
}

std::pair<double, double> ZCompensation::focalLength(LensType lensType)
{
    std::pair<double, double> focalLengthInXAndY = {0.0, 0.0};
    switch(lensType)
    {
        case LensType::F_Theta_340:
            focalLengthInXAndY = std::make_pair(808.1, 650.0);                  //647.4
            break;
        case LensType::F_Theta_460:
            focalLengthInXAndY = std::make_pair(693.852, 622.01);
            break;
        case LensType::F_Theta_255:
            focalLengthInXAndY = std::make_pair(762.148, 542.06);
            break;
    }

    return focalLengthInXAndY;
}

bool ZCompensation::fuzzyIsNull(double value)
{
    Calculator calculator;
    return calculator.fuzzyIsNull(value);
}

bool ZCompensation::checkFocalLengthOffsetIsNull(bool isYAxis)
{
    return isYAxis ? fuzzyIsNull(std::get<1>(m_focalLength) + m_zOffset) : fuzzyIsNull(std::get<0>(m_focalLength) + m_zOffset);
}

}
}
}

