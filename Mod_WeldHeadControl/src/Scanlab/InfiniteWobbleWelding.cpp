#define _USE_MATH_DEFINES
#include "viWeldHead/Scanlab/InfiniteWobbleWelding.h"

#include "module/moduleLogger.h"

#include "viWeldHead/Scanlab/RTC6Scanner.h"
#include "viWeldHead/Scanlab/RTC6Laser.h"
#include "viWeldHead/Scanlab/RTC6WeldingStrategy.h"
#include "viWeldHead/Scanlab/Scanner2DWeldingData.h"

#include <cmath>
#include "rtc6.h"

#include <iostream>
#include <fstream>

static const int NO_LASER_OFF = -1;
static const int LOWER_LIMIT_MICROVECTOR_FACTOR = 1;
static const int LIST_START = 2;
static const int LIST_START_WITH_DEBUG = 3;

using namespace precitec;

InfiniteWobbleWelding::InfiniteWobbleWelding(const precitec::hardware::Scanner2DWeldingData& control)
    : AbstractScanner2DWelding(control, std::make_unique<RTC6::WeldingStrategy>())
{
}

InfiniteWobbleWelding::~InfiniteWobbleWelding()
{
    stop_execution();
}

void InfiniteWobbleWelding::set_Scale(const double& p_oXScale, const double& p_oYScale)
{
    m_oXScale = p_oXScale;
    m_oYScale = p_oYScale;
}

void InfiniteWobbleWelding::get_Scale(double& p_oXScale, double& p_oYScale)
{
    p_oXScale = m_oXScale;
    p_oYScale = m_oYScale;
}

void InfiniteWobbleWelding::start_Mark()
{
    UINT oStatus = 0;
    UINT oPos = 0;
    while ((oStatus & 1) == 1)
    {
        get_status(&oStatus, &oPos);
    }
    enable_laser();
    execute_list(1);
}

void InfiniteWobbleWelding::jump_MarkOrigin()
{
    stop_execution();
    const auto geometricalTransformationData = m_control.get().getScannerGeometricalTransformationData();
    set_offset(1, (m_oXOffset + geometricalTransformationData.x), (m_oYOffset + geometricalTransformationData.y), 3);
    set_angle(1, (m_oAngle + geometricalTransformationData.angle), 3);
    goto_xy(0, 0);
}

void InfiniteWobbleWelding::defineFigure()
{
    reset();

    m_freeFigure.microVectorFactor = calculateMicroVectorFactor(static_cast<double>(m_oWobbelFreq), m_freeFigure.figure.size() - 1);
    if (m_freeFigure.microVectorFactor < LOWER_LIMIT_MICROVECTOR_FACTOR)
    {
        wmLog(eError, "Frequency isn't valid!\n");
        return;
    }

    calculateVectors();
    writeGlobalRTC6Properties();
    writeRTC6List();
}

void InfiniteWobbleWelding::refreshOffsetFromCamera(double offsetX, double offsetY)
{
    set_start_list_pos(m_currentList, m_enableDebugMode ? LIST_START_WITH_DEBUG : LIST_START);
    micro_vector_abs(offsetX + m_XFigureOffset, offsetY + m_YFigureOffset, NO_LASER_OFF, NO_LASER_OFF);
    m_processXOffset += offsetX;
    m_processYOffset += offsetY;
}

void InfiniteWobbleWelding::refreshSizeFromCameraByScale(const std::pair<double,double>& scale)
{
    m_currentList = (m_currentList == precitec::ScanlabList::ListOne) ? precitec::ScanlabList::ListTwo : precitec::ScanlabList::ListOne;
    writeRTC6List(scale);
}

void InfiniteWobbleWelding::reset()
{
    m_relativeXMovement.clear();
    m_relativeYMovement.clear();
    m_absolutePower.clear();
    m_processXOffset = 0.0;
    m_processYOffset = 0.0;
    m_XFigureOffset = 0.0;
    m_YFigureOffset = 0.0;
    m_currentList = precitec::ScanlabList::ListOne;
    m_laserPowerStaticActive = true;
}

unsigned int InfiniteWobbleWelding::calculateMicroVectorFactor(double frequency, unsigned int vectorCount)
{
    return std::round(1 / (frequency * vectorCount * 0.00001));
}

void InfiniteWobbleWelding::calculateVectors()
{
    if (m_freeFigure.figure.empty())
    {
        wmLog(eError, "Figure is empty!\n");
        return;
    }

    std::vector<double> positionX;
    std::vector<double> positionY;
    std::vector<double> power;

    positionX.reserve(m_freeFigure.figure.size());
    positionY.reserve(m_freeFigure.figure.size());
    power.reserve(m_freeFigure.figure.size());

    for (const auto& point : m_freeFigure.figure)
    {
        positionX.push_back(point.endPosition.first);
        positionY.push_back(point.endPosition.second);
        power.push_back(point.relativePower);
    }

    m_XFigureOffset = positionX.front() * m_calibrationFactor;
    m_YFigureOffset = positionY.front() * m_calibrationFactor;

    if (power.front() == SCANMASTERWELDINGDATA_UNDEFINEDVALUE || power.front() == SCANMASTERWELDINGDATA_USESTATICVALUE)
    {
        m_laserPowerStaticActive = true;
        fillPowerVectorWithStaticPower(power.size(), m_freeFigure.microVectorFactor);
    }
    else
    {
        m_laserPowerStaticActive = false;
        m_absolutePower = calculateAbsoluteMicroPower(power, m_freeFigure.microVectorFactor);
    }

    const auto& vectorsX = createVectors(positionX);
    const auto& vectorsY = createVectors(positionY);

    if (vectorsX.empty() || vectorsY.empty())
    {
        wmLog(eError, "Calculation of vectors failed!\n");
    }
    m_relativeXMovement = createMicroVectors(vectorsX, m_freeFigure.microVectorFactor);
    m_relativeYMovement = createMicroVectors(vectorsY, m_freeFigure.microVectorFactor);
    if (m_laserPowerStaticActive)
    {
        return;
    }
    m_absolutePower = calculateAbsoluteMicroPower(power, m_freeFigure.microVectorFactor);
}

std::vector<double> InfiniteWobbleWelding::createVectors(const std::vector<double>& positions)
{
    if (positions.empty() || positions.size() < 2)
    {
        return {};
    }

    std::vector<double> vectors;
    vectors.reserve(positions.size() - 1);

    for (std::size_t i = 1; i < positions.size(); i++)
    {
        const auto& lastPosition = positions.at(i - 1);
        const auto& currentPosition = positions.at(i);
        vectors.push_back(currentPosition - lastPosition);
    }
    return vectors;
}

std::vector<double> InfiniteWobbleWelding::createMicroVectors(const std::vector<double>& values, unsigned int microVectorFactor)
{
    if (values.empty() || microVectorFactor < LOWER_LIMIT_MICROVECTOR_FACTOR)
    {
        return {};
    }

    if (microVectorFactor == LOWER_LIMIT_MICROVECTOR_FACTOR)
    {
        return values;
    }
    std::vector<double> microVectors;
    microVectors.reserve(values.size() * microVectorFactor);

    for (const auto& value : values)
    {
        const auto& microValue = value / microVectorFactor;
        for (std::size_t i = 0; i < microVectorFactor; i++)
        {
            microVectors.push_back(microValue);
        }
    }
    return microVectors;
}

void InfiniteWobbleWelding::fillPowerVectorWithStaticPower(std::size_t figureSize, unsigned int microVectorFactor)
{
    m_startPower = static_cast<double>(m_laserPowerStaticActive) / 100.0;
    m_absolutePower.reserve(figureSize * microVectorFactor);
    std::fill(m_absolutePower.begin(), m_absolutePower.end(), m_startPower);
}

std::vector<double> InfiniteWobbleWelding::calculateAbsoluteMicroPower(const std::vector<double>& power, unsigned int microVectorFactor)
{
    if (power.empty() || microVectorFactor < LOWER_LIMIT_MICROVECTOR_FACTOR)
    {
        return {};
    }

    if (microVectorFactor == LOWER_LIMIT_MICROVECTOR_FACTOR)
    {
        return power;
    }

    m_startPower = power.front();

    std::vector<double> absoluteMicroPowers;
    absoluteMicroPowers.reserve(power.size() * microVectorFactor);
    double absolutePower = m_startPower;

    for (std::size_t i = 1; i < power.size(); i++)
    {
        const auto& relativeMicroPower = (power.at(i) - power.at(i - 1)) / microVectorFactor;
        for (std::size_t i = 0; i < microVectorFactor; i++)
        {
            absoluteMicroPowers.push_back(absolutePower + relativeMicroPower);
            absolutePower += relativeMicroPower;
        }
    }
    return absoluteMicroPowers;
}

void InfiniteWobbleWelding::writeGlobalRTC6Properties()
{
    set_offset(precitec::ScannerHead::ScannerHead1, 0, 0, 3);
    set_angle(precitec::ScannerHead::ScannerHead1, 0.0, 3);
    set_scale(precitec::ScannerHead::ScannerHead1, 1.0, 3);
    set_jump_speed_ctrl(m_oJumpSpeed);
}

void InfiniteWobbleWelding::writeRTC6List(const std::pair<double,double>& scale)
{
    if (m_relativeXMovement.empty() || m_relativeYMovement.empty() || m_absolutePower.empty())
    {
        wmLog(eError, "Micro vectors are missing!\n");
        return;
    }

    if (m_relativeXMovement.size() != m_relativeYMovement.size() ||
        m_relativeXMovement.size() != m_absolutePower.size() ||
        m_relativeYMovement.size() != m_absolutePower.size())
    {
        wmLog(eError, "Properties doesn't have the same size!\n");
        return;
    }

    if (scale.first < 0.01 || scale.second < 0.01)
    {
        wmLog(eError, "Scale is too small!\n");
        return;
    }

    int listSize = 0;

    set_start_list_pos(m_currentList, 0);

    if (m_enableDebugMode)
    {
        set_trigger4(m_measurementPeriod, loggedSignal(0), loggedSignal(1), loggedSignal(2), loggedSignal(3));
        listSize++;
    }

    set_vector_control(precitec::ScanlabOutputPorts::AnalogOne, m_startPower * m_ADCValue);               //Corepower
    listSize++;

    micro_vector_abs(scale.first * (m_XFigureOffset + m_processXOffset), scale.second * (m_YFigureOffset + m_processYOffset), NO_LASER_OFF, NO_LASER_OFF);          //Changes frequency!
    listSize++;

    for (std::size_t i = 0; i < m_relativeXMovement.size(); i++)
    {
        timed_para_mark_rel(scale.first * m_relativeXMovement.at(i) * m_calibrationFactor, scale.second * m_relativeYMovement.at(i) * m_calibrationFactor, m_absolutePower.at(i) * m_ADCValue, 10);
        listSize++;
    }

    if (m_currentList == 1)
    {
        list_jump_pos(m_enableDebugMode ? LIST_START_WITH_DEBUG : LIST_START);
        listSize++;
    }
    else
    {
        list_jump_pos(listSize + 1);
    }

    set_end_of_list();

    wmLog(eDebug, "List finished!\n");
}
