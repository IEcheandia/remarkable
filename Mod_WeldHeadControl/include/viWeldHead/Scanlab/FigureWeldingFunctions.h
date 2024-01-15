#pragma once

#include "common/definesScanlab.h"
#include "viWeldHead/Scanlab/RTC6DataTypes.h"

#include <vector>

namespace precitec::math
{
inline double interpolate(double startValue, double endValue, int numberOfSteps, int step)
{
    return ((endValue - startValue) / numberOfSteps) * step + startValue;
}
}

namespace precitec::FigureWeldingFunctions
{

double calculateRelativeLaserPower(double nominalPower, double newPower, double oldPower, unsigned int microVectorFactor);

std::vector<double> shiftPowerToCompensateDelay(const std::vector<double> powerValues, int shift);

int findDivisor(std::size_t wobbleVectorsSize, std::size_t microVectorFactor, bool modulateCoreAndRing);

inline bool modulateCoreAndRing(precitec::WobbleControl wobbleControl)
{
    return wobbleControl == precitec::WobbleControl::AnalogOut1And2Variation;
}

inline int calculateLaserPowerCompensationIn10us(int laserPowerDelayCompensation)
{
    return 10 * laserPowerDelayCompensation;
}

inline int calculateShiftResolution(int divisor)
{
    return 10 * divisor; //[us]
}

int calculateShiftToCompensateDelay(int laserPowerDelayCompensation, int resolution);

std::vector<RTC6::command::Order> interpolateMicroPoints(const std::vector<RTC6::command::Order>& points, unsigned int numberOfInterpolations);

std::vector<double> shiftedPowerValues(precitec::WobbleControl wobbleControl, std::vector<RTC6::command::Order> const & figure, int shift);

std::pair<double, double> definePrePosition(double intendedLength, double calibrationFactor, std::pair<double, double> firstPoint, std::pair<double, double> secondPoint);
}