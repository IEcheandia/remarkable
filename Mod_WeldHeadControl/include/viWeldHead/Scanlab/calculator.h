#pragma once

#include <utility>

namespace precitec
{
namespace hardware
{
namespace welding
{

/**
 * The class contains various functions to perform calculations.
 * All calculations are important for the scanner. (ScanMaster)
 **/
class Calculator
{
public:
    Calculator();
    ~Calculator();

    double calibrationFactor() const
    {
        return m_calibrationFactor;
    }
    void setCalibrationFactor(double newCalibrationFactor);

    std::pair<double, double> wobbleStartPosition() const
    {
        return m_wobbleStartPosition;
    }
    void setWobbleStartPosition(const std::pair<double, double>& newWobbleStartPosition);
    bool wobbleStartPositionIsNull();

    long calculateBitsFromMM(double millimeter);
    double calculateMMFromBits(long bits);

    /* Vector calculus */
    std::pair<double, double> calculateVector(const std::pair<double, double>& startPoint, const std::pair<double, double>& endPoint);
    double calculateVectorLength(const std::pair<double, double>& vector);
    double angleScalarProduct(const std::pair<double, double>& vector1, const std::pair<double, double>& vector2);
    double angleToXAxis(const std::pair<double, double>& vector);
    std::pair<double, double> rotateVector(const std::pair<double, double>& vector, double angle);

    double addOffset(double value, double offset);

    double limitTo3Digits(double value);
    std::pair<double, double> limitTo3Digits(const std::pair<double, double>& pair);

    bool fuzzyIsNull(double value);

private:
    double scalarProduct(const std::pair<double, double>& vector1, const std::pair<double, double>& vector2);
    double rotateXComponent(const std::pair<double, double>& vector, double angle);
    double rotateYComponent(const std::pair<double, double>& vector, double angle);
    std::pair<double, double> xAxisVector();

    double m_calibrationFactor = 4000.0;
    std::pair<double, double> m_wobbleStartPosition{0.0, 0.0};
};

}
}
}
