#include "viWeldHead/Scanlab/calculator.h"

#include <cmath>

namespace precitec
{
namespace hardware
{
namespace welding
{

Calculator::Calculator()
{
}

Calculator::~Calculator() = default;

void Calculator::setCalibrationFactor(double newCalibrationFactor)
{
    m_calibrationFactor = newCalibrationFactor;
}

void Calculator::setWobbleStartPosition(const std::pair<double, double>& newWobbleStartPosition)
{
    m_wobbleStartPosition = newWobbleStartPosition;
}

bool Calculator::wobbleStartPositionIsNull()
{
    return fuzzyIsNull(m_wobbleStartPosition.first) && fuzzyIsNull(m_wobbleStartPosition.second);
}

long Calculator::calculateBitsFromMM(double millimeter)
{
    return static_cast<long> (millimeter * m_calibrationFactor);
}

double Calculator::calculateMMFromBits(long bits)
{
    return static_cast<double> (bits / m_calibrationFactor);
}

std::pair<double, double> Calculator::calculateVector(const std::pair<double, double>& startPoint, const std::pair<double, double>& endPoint)
{
    return std::make_pair(endPoint.first - startPoint.first, endPoint.second - startPoint.second);
}

double Calculator::calculateVectorLength(const std::pair<double, double>& vector)
{
    return std::sqrt(std::pow(vector.first, 2) + std::pow(vector.second, 2));
}

double Calculator::angleScalarProduct(const std::pair<double, double>& vector1, const std::pair<double, double>& vector2)
{
    return std::acos( scalarProduct(vector1, vector2) / (calculateVectorLength(vector1) * calculateVectorLength(vector2)));
}

double Calculator::angleToXAxis(const std::pair<double, double>& vector)
{
    if (vector.second >= 0)
    {
        return angleScalarProduct(vector, xAxisVector());
    }
    return (2 * M_PI) - angleScalarProduct(vector, xAxisVector());
}

std::pair<double, double> Calculator::rotateVector(const std::pair<double, double>& vector, double angle)
{
    return std::make_pair(rotateXComponent(vector, angle), rotateYComponent(vector, angle));
}

double Calculator::addOffset(double value, double offset)
{
    return value + offset;
}

double Calculator::limitTo3Digits(double value)
{
    return static_cast<double> (static_cast<int> (value * 1000.0) / 1000.0);
}

std::pair<double, double> Calculator::limitTo3Digits(const std::pair<double, double>& pair)
{
    return std::make_pair(limitTo3Digits(pair.first), limitTo3Digits(pair.second));
}

bool Calculator::fuzzyIsNull(double value)
{
    return std::abs(value) <= 0.000000000001;
}

double Calculator::scalarProduct(const std::pair<double, double>& vector1, const std::pair<double, double>& vector2)
{
    return (vector1.first * vector2.first) + (vector1.second * vector2.second);
}

double Calculator::rotateXComponent(const std::pair<double, double>& vector, double angle)
{
    return vector.first * std::cos(angle) - vector.second * std::sin(angle);
}

double Calculator::rotateYComponent(const std::pair<double, double>& vector, double angle)
{
    return vector.first * std::sin(angle) + vector.second * std::cos(angle);
}

std::pair<double, double> Calculator::xAxisVector()
{
    return std::make_pair(1.0, 0.0);
}

}
}
}
