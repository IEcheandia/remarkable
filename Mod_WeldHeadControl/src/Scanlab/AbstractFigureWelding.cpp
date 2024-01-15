#include "viWeldHead/Scanlab/AbstractFigureWelding.h"

#include "viWeldHead/Scanlab/FigureWeldingFunctions.h"
#include "viWeldHead/Scanlab/RTC6WeldingStrategy.h"

#include "module/moduleLogger.h"

#include <cmath>

using namespace precitec::FigureWeldingFunctions;

AbstractFigureWelding::AbstractFigureWelding(const precitec::hardware::Scanner2DWeldingData& control,
                                             precitec::hardware::welding::ZCompensation& zCompensation,
                                             std::unique_ptr<precitec::hardware::AbstractWeldingStrategy> weldingStrategy)
    : AbstractScanner2DWelding(control, std::move(weldingStrategy))
    , m_zCompensation(zCompensation)
{
}

AbstractFigureWelding::~AbstractFigureWelding() = default;

void AbstractFigureWelding::setLaserPowerDelayCompensation(int powerDelayCompensation)
{
    m_laserPowerDelayCompensation = powerDelayCompensation;
}

void AbstractFigureWelding::setADCValue(int ADCValue)
{
    m_ADCValue = ADCValue;
}

bool AbstractFigureWelding::setNominalPowers()
{
    if (m_freeFigure.figure.empty())
    {
        return false;
    }

    const auto& firstPoint = m_freeFigure.figure.front();
    setNominalPower(firstPoint.relativePower);
    setNominalRingPower(firstPoint.relativeRingPower);
    return true;
}

double AbstractFigureWelding::nominalPower() const
{
    return m_nominalPower;
}

void AbstractFigureWelding::setNominalPower(double newNominalPower)
{
    m_nominalPower = newNominalPower;
}

double AbstractFigureWelding::nominalRingPower() const
{
    return m_nominalRingPower;
}

void AbstractFigureWelding::setNominalRingPower(double newNominalRingPower)
{
    m_nominalRingPower = newNominalRingPower;
}

void AbstractFigureWelding::calculateShiftAndShiftedNominalPower(precitec::WobbleControl wobbleControl)
{
    m_shift = 0;
    if (m_laserPowerDelayCompensation != 0)
    {
        bool coreAndRing = modulateCoreAndRing(wobbleControl);
        const auto& divisor = findDivisor(m_freeFigure.figure.size() - 1, m_freeFigure.microVectorFactor, coreAndRing);
        const auto& resolution = calculateShiftResolution(divisor);
        const auto& shift = calculateShiftToCompensateDelay(calculateLaserPowerCompensationIn10us(m_laserPowerDelayCompensation), resolution);
        m_shift = shift;
        double newNominalPower = 0.0;
        double newNominalRingPower = 0.0;

        const auto& figureWithMicroPoints = interpolateMicroPoints(m_freeFigure.figure, m_freeFigure.microVectorFactor / divisor);
        if (std::abs(m_shift) > figureWithMicroPoints.size() - 1)
        {
            setNominalPower(newNominalPower);
            setNominalRingPower(newNominalRingPower);
            wmLogTr(precitec::eError, "QnxMsg.VI.ShiftTooHigh", "Shift too high for figure! Change laser power compensation value!\n");
            return;
        }
        if (shift < 0)
        {
            newNominalPower = figureWithMicroPoints.at(figureWithMicroPoints.size() - std::abs(m_shift) - 1).relativePower;
            newNominalRingPower = figureWithMicroPoints.at(figureWithMicroPoints.size() - std::abs(m_shift) - 1).relativeRingPower;
        }
        else
        {
            newNominalPower = figureWithMicroPoints.at(m_shift).relativePower;
            newNominalRingPower = figureWithMicroPoints.at(m_shift).relativeRingPower;
        }

        m_freeFigure.microVectorFactor = divisor;
        m_freeFigure.figure = figureWithMicroPoints;

        setNominalPower(newNominalPower);
        setNominalRingPower(newNominalRingPower);
    }
}

void AbstractFigureWelding::defineOnePortWobbleFigure(precitec::WobbleControl wobbleControl)
{
    m_wobbelTransVector.clear();
    m_wobbelLongVector.clear();
    m_deltaPower.clear();

    const auto& shiftedPowers = shiftedPowerValues(wobbleControl, m_freeFigure.figure, m_shift);

    std::size_t oNumberOfWobbelVectors = m_freeFigure.figure.size() - 1;

    m_wobbelTransVector.reserve(oNumberOfWobbelVectors);
    m_wobbelLongVector.reserve(oNumberOfWobbelVectors);
    m_deltaPower.reserve(oNumberOfWobbelVectors);

    for (std::size_t i = 1; i < m_freeFigure.figure.size(); i++)
    {
        const auto& targetPoint = m_freeFigure.figure.at(i);
        const auto& startPoint = m_freeFigure.figure.at(i - 1);

        m_wobbelTransVector.emplace_back(-1.0 * (targetPoint.endPosition.second - startPoint.endPosition.second));
        m_wobbelLongVector.emplace_back(targetPoint.endPosition.first - startPoint.endPosition.first);
        if (wobbleControl == precitec::WobbleControl::AnalogOut1Variation)
        {
            m_deltaPower.emplace_back(i == 1 ? calculateRelativeLaserPower(m_nominalPower, shiftedPowers[i], m_nominalPower, m_freeFigure.microVectorFactor) : calculateRelativeLaserPower(m_nominalPower, shiftedPowers[i], shiftedPowers[i - 1], m_freeFigure.microVectorFactor));
        }
        else
        {
            m_deltaPower.emplace_back(i == 1 ? calculateRelativeLaserPower(m_nominalRingPower, shiftedPowers[i], m_nominalRingPower, m_freeFigure.microVectorFactor) : calculateRelativeLaserPower(m_nominalRingPower, shiftedPowers[i], shiftedPowers[i - 1], m_freeFigure.microVectorFactor));
        }
    }

    setOnePortWobbleVector(oNumberOfWobbelVectors);
}

void AbstractFigureWelding::defineTwoPortWobbleFigure(std::size_t countWobbelVectors)
{
    m_wobbelTransVector.clear();
    m_wobbelLongVector.clear();
    m_deltaPower.clear();
    m_deltaRingPower.clear();

    const auto& shiftedPowers = shiftedPowerValues(precitec::WobbleControl::AnalogOut1Variation, m_freeFigure.figure, m_shift);
    const auto& shiftedRingPowers = shiftedPowerValues(precitec::WobbleControl::AnalogOut2Variation, m_freeFigure.figure, m_shift);

    m_wobbelTransVector.reserve(countWobbelVectors);
    m_wobbelLongVector.reserve(countWobbelVectors);
    m_deltaPower.reserve(countWobbelVectors);
    m_deltaRingPower.reserve(countWobbelVectors);

    for (std::size_t i = 1; i < m_freeFigure.figure.size(); i++)
    {
        const auto& targetPoint = m_freeFigure.figure.at(i);
        const auto& startPoint = m_freeFigure.figure.at(i - 1);

        m_wobbelTransVector.emplace_back(-1.0 * (targetPoint.endPosition.second - startPoint.endPosition.second));
        m_wobbelLongVector.emplace_back(targetPoint.endPosition.first - startPoint.endPosition.first);
        m_deltaPower.emplace_back(i == 1 ? calculateRelativeLaserPower(m_nominalPower, shiftedPowers[i], m_nominalPower, m_freeFigure.microVectorFactor - 1) : calculateRelativeLaserPower(m_nominalPower, shiftedPowers[i], shiftedPowers[i - 1], m_freeFigure.microVectorFactor - 1));
        m_deltaRingPower.emplace_back(i == 1 ? calculateRelativeLaserPower(m_nominalRingPower, shiftedRingPowers[i], m_nominalRingPower, m_freeFigure.microVectorFactor) : calculateRelativeLaserPower(m_nominalRingPower, shiftedRingPowers[i], shiftedRingPowers[i - 1], m_freeFigure.microVectorFactor));
    }

    setTwoPortWobbleVector(countWobbelVectors);
}

std::pair<double, double> AbstractFigureWelding::definePrePosition(double laserDelay, std::pair<double, double> firstPoint, std::pair<double, double> secondPoint)
{
    const auto intendedLength = m_oMarkSpeed / m_calibrationFactor * laserDelay;
    return ::definePrePosition(intendedLength, m_calibrationFactor, firstPoint, secondPoint);
}