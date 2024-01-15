#include "viWeldHead/Scanlab/smartMoveCalculator.h"

#include <math.h>

#include "common/definesSmartMove.h"

using precitec::smartMove::DriveToLimits;
using precitec::smartMove::ScanfieldBits;

namespace precitec
{
namespace hardware
{

SmartMoveCalculator::SmartMoveCalculator() = default;

SmartMoveCalculator::~SmartMoveCalculator() = default;

void SmartMoveCalculator::setScanfieldSize(int quadraticScanfieldSizeMillimeter)
{
    m_scanfieldSize = quadraticScanfieldSizeMillimeter;
}

int SmartMoveCalculator::calculateDriveToBits(double millimeter)
{
    if (millimeter == 0 || m_scanfieldSize == 0)
    {
        return DriveToLimits::OriginScanfield;
    }
    if (millimeter > 0)
    {
        return static_cast<int>((millimeter / m_scanfieldSize) * DriveToLimits::PositiveScanfield);
    }
    return static_cast<int>((std::abs(millimeter) / m_scanfieldSize) * DriveToLimits::NegativeScanfield) ;
}

int SmartMoveCalculator::calculateDriveToMillimeter(int bits)
{
    if (bits == 0 || m_scanfieldSize == 0)
    {
        return DriveToLimits::OriginScanfield;
    }
    if (bits > 0)
    {
        return static_cast<int>((static_cast<double>(bits) / DriveToLimits::PositiveScanfield) * m_scanfieldSize);
    }
    return static_cast<int>((static_cast<double>(std::abs(bits)) / DriveToLimits::NegativeScanfield) * m_scanfieldSize);
}

int SmartMoveCalculator::calculateMillimeterToBits(double millimeter)
{
    return millimeter * conversionFactor();
}

double SmartMoveCalculator::calculateBitsToMillimeter(int bits)
{
    auto calculationFactor = conversionFactor();
    if (calculationFactor <= 0.0)
    {
        return 0.0;
    }
    return bits / calculationFactor;
}

double SmartMoveCalculator::calculateBitsPerMillisecondsFromMeterPerSecond(double speedInMeterPerSecond) const
{
    return speedInMeterPerSecond / calculateMillimeterFromMicrometer(calculateMicrometerFromNanometer(calculateResolutionInNanometer()));
}

double SmartMoveCalculator::calculateMeterPerSecondFromBitsPerMilliseconds(int speedInBitsPerMilliseconds)
{
    return speedInBitsPerMilliseconds * calculateMillimeterFromMicrometer(calculateMicrometerFromNanometer(calculateResolutionInNanometer()));
}

double SmartMoveCalculator::calculateMillimeterFromMeter(double meter) const
{
    return meter * 1000;
}

double SmartMoveCalculator::calculateMeterFromMillimeter(double millimeter) const
{
    return millimeter / 1000.;
}

double SmartMoveCalculator::calculateMicrometerFromMillimeter(double millimeter) const
{
    return millimeter * 1000;
}

double SmartMoveCalculator::calculateMillimeterFromMicrometer(double micrometer) const
{
    return micrometer / 1000.;
}

double SmartMoveCalculator::calculateNanometerFromMicrometer(double micrometer) const
{
    return micrometer * 1000;
}

double SmartMoveCalculator::calculateMicrometerFromNanometer(double nanometer) const
{
    return nanometer / 1000.0;
}

int SmartMoveCalculator::calculateResolutionInNanometer() const
{
    return calculateNanometerFromMicrometer(calculateMicrometerFromMillimeter(m_scanfieldSize)) / ScanfieldBits::Max;
}

double SmartMoveCalculator::conversionFactor() const
{
    //TODO Maybe remove check because a scanfield of size zero doesn't make any sense.
    //NOTE If check is removed check unit tests and check calculateBitsToMillimeter.
    if (m_scanfieldSize <= 0.0)
    {
        return 0.0;
    }
    return ScanfieldBits::Max / static_cast<double> (m_scanfieldSize);
}

}
}
