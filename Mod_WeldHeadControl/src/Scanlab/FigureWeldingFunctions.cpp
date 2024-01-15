#include "viWeldHead/Scanlab/FigureWeldingFunctions.h"

#include <algorithm>
#include <cmath>

namespace precitec::FigureWeldingFunctions
{
namespace
{
bool fuzzyIsNull(double value)
{
    return std::abs(value) <= 0.000000000001;
}

static constexpr double MIN_WOBBLE_DELTA_POWER = -1.0;
static constexpr double MAX_WOBBLE_DELTA_POWER = 1.0;
}

double calculateRelativeLaserPower(double nominalPower, double newPower, double oldPower, unsigned int microVectorFactor)
{
    if (fuzzyIsNull(nominalPower) || fuzzyIsNull(microVectorFactor))
    {
        return 0.0;
    }

    auto result = (newPower - oldPower) / (nominalPower * static_cast<double>(microVectorFactor));

    if (result < MIN_WOBBLE_DELTA_POWER)
    {
        result = MIN_WOBBLE_DELTA_POWER;
    }
    if (result > MAX_WOBBLE_DELTA_POWER)
    {
        result = MAX_WOBBLE_DELTA_POWER;
    }

    return result;
}

std::vector<double> shiftPowerToCompensateDelay(const std::vector<double> powerValues, int shift)
{
    std::vector<double> shiftedPowerValues = powerValues;
    if (shift < 0)
    {
        std::rotate(shiftedPowerValues.begin(), shiftedPowerValues.begin() + shiftedPowerValues.size() - std::abs(shift), shiftedPowerValues.end());
        return shiftedPowerValues;
    }

    std::rotate(shiftedPowerValues.begin(), shiftedPowerValues.begin() + shift, shiftedPowerValues.end());
    return shiftedPowerValues;
}
int findDivisor(std::size_t wobbleVectorsSize, std::size_t microVectorFactor, bool modulateCoreAndRing)
{
    const int limit = modulateCoreAndRing ? 511 : 1023;
    int divisor = modulateCoreAndRing ? 2 : 1;
    int numberOfVectors = wobbleVectorsSize * (microVectorFactor / divisor);

    while (numberOfVectors > limit)
    {
        divisor++;
        if (modulateCoreAndRing)
        {
            if ((microVectorFactor % divisor) == 0 && (divisor % 2) == 0)
            {
                numberOfVectors = wobbleVectorsSize * (microVectorFactor / divisor);
            }
        }
        else
        {
            if ((microVectorFactor % divisor) == 0)
            {
                numberOfVectors = wobbleVectorsSize * (microVectorFactor / divisor);
            }
        }
        if (divisor == 100)
        {
            break;
        }
    }

    return divisor;
}
int calculateShiftToCompensateDelay(int laserPowerDelayCompensation, int resolution)
{
    return std::round(laserPowerDelayCompensation / resolution);
}
std::vector<RTC6::command::Order> interpolateMicroPoints(const std::vector<RTC6::command::Order>& points, unsigned int numberOfInterpolations)
{
    if (points.empty() || numberOfInterpolations <= 1 || (points.size() - 1) * numberOfInterpolations > 1023)
    {
        return points;
    }

    std::vector<RTC6::command::Order> microPoints;
    microPoints.reserve((points.size() - 1) * numberOfInterpolations + 1);
    microPoints.emplace_back(points.front());

    for (std::size_t currentPoint = 1; currentPoint < points.size(); currentPoint++)
    {
        const auto& startPoint = points[currentPoint - 1];
        const auto& endPoint = points[currentPoint];

        for (std::size_t i = 1; i <= numberOfInterpolations; i++)
        {
            RTC6::command::Order microPoint;
            using precitec::math::interpolate;
            microPoint.endPosition.first = interpolate(startPoint.endPosition.first, endPoint.endPosition.first, numberOfInterpolations, i);
            microPoint.endPosition.second = interpolate(startPoint.endPosition.second, endPoint.endPosition.second, numberOfInterpolations, i);
            microPoint.relativePower = interpolate(startPoint.relativePower, endPoint.relativePower, numberOfInterpolations, i);
            microPoint.relativeRingPower = interpolate(startPoint.relativeRingPower, endPoint.relativeRingPower, numberOfInterpolations, i);
            microPoint.velocity = 0; //Velocity isn't used for wobble figures
            microPoints.emplace_back(microPoint);
        }
    }

    return microPoints;
}

std::vector<double> shiftedPowerValues(precitec::WobbleControl wobbleControl, std::vector<RTC6::command::Order> const & figure, int shift)
{
    if (figure.empty())
    {
        return {};
    }

    std::vector<double> powers;
    powers.reserve(figure.size());

    if (wobbleControl == precitec::WobbleControl::AnalogOut1Variation)
    {
        std::transform(figure.begin(), std::prev(figure.end(), 1), std::back_inserter(powers), [](const auto& point)
                       { return point.relativePower; });
    }
    else
    {
        std::transform(figure.begin(), std::prev(figure.end(), 1), std::back_inserter(powers), [](const auto& point)
                       { return point.relativeRingPower; });
    }

    auto shiftedPowerValues = shiftPowerToCompensateDelay(powers, shift);
    shiftedPowerValues.emplace_back(shiftedPowerValues.front());
    return shiftedPowerValues;
}

std::pair<double, double> definePrePosition(double intendedLength, double calibrationFactor, std::pair<double, double> firstPoint, std::pair<double, double> secondPoint)
{
    if (fuzzyIsNull(intendedLength) || fuzzyIsNull(calibrationFactor))
    {
        return {firstPoint.first, firstPoint.second};
    }

    const auto xDiff = secondPoint.first - firstPoint.first;
    const auto yDiff = secondPoint.second - firstPoint.second;

    /*
     *  First segment has a specific angle defined by the figure.
     *  Calculate a factor between the length of acceleration vector and the already existing one.
     *  Then multiply the factor with the components of the existing vector.
     *  The acceleration vector has the same angle and the intended length now.
     */
    const auto lengthFirstSegment = std::sqrt((xDiff * xDiff) + (yDiff * yDiff));
    const auto factor = intendedLength / lengthFirstSegment;

    const auto deltaX = factor * xDiff;
    const auto deltaY = factor * yDiff;

    return {firstPoint.first - deltaX, firstPoint.second - deltaY};
}
}