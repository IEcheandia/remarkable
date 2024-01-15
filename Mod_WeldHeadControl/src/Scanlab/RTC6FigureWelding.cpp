#define _USE_MATH_DEFINES
#include "viWeldHead/Scanlab/RTC6FigureWelding.h"

#include "event/results.h"
#include "viWeldHead/Scanlab/calculator.h"
#include "viWeldHead/Scanlab/FigureWeldingFunctions.h"
#include "viWeldHead/Scanlab/RTC6WeldingStrategy.h"
#include "viWeldHead/Scanlab/Scanner2DWeldingData.h"
#include "viWeldHead/Scanlab/zCompensation.h"

#include <iostream>
#include <cmath>
#include <fstream>
#include <iomanip>

#include "rtc6.h"

#include "module/moduleLogger.h"

static const int MAX_WOBBLE_SEGMENTS = 1023;
static const int MIN_WOBBLE_FIGURE_POINTS = 2;

static const int MIN_ADC_12BIT_VALUE = 0;
static const int MAX_ADC_12BIT_VALUE = 4095;
static const int MAX_ADC_12BIT_VALUE_32BIT = 0x0FFF0FFF; //32 Bit value: highest 16 bits define max adc 12bit value for analog out 1 and the lowest 16 bits define max adc 12 bit value for analog out 2
static const int USE_VALUE_ON_ANALOG_PORT = 0xFFFFFFFF;

static const int INVALID__WOBBEL_MODE_VALUE = 0;
static const int VALID__WOBBEL_MODE_VALUE = 1;

using namespace precitec;

namespace RTC6
{
namespace
{
bool fuzzyIsNull(double value)
{
    return std::abs(value) <= 0.000000000001;
}

std::vector<long unsigned int> laserPowerFromWeldingData(const interface::ResultDoubleArray& weldingData, int dataIndex, int staticValue, double scalingFactor)
{
    std::vector<long unsigned int> laserPowerValues;
    laserPowerValues.reserve(weldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT);
    for (unsigned int i = 0; i < weldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT; i++)
    {
        const double laserPower = weldingData.value()[(i * SEAMWELDING_RESULT_FIELDS_PER_POINT) + dataIndex];
        if (laserPower == SCANMASTERWELDINGDATA_UNDEFINEDVALUE)
        {
            if (laserPowerValues.size() == 0)
            {
                laserPowerValues.push_back(staticValue / 100.0 * scalingFactor);
            }
            else
            {
                laserPowerValues.push_back(laserPowerValues.back());
            }
        }
        else if (laserPower == SCANMASTERWELDINGDATA_USESTATICVALUE)
        {
            laserPowerValues.push_back(staticValue / 100.0 * scalingFactor);
        }
        else
        {
            laserPowerValues.push_back(static_cast<long unsigned int>(laserPower * scalingFactor));
        }
    }
    return laserPowerValues;
}

}

FigureWelding::FigureWelding(const precitec::hardware::Scanner2DWeldingData& control,
                                     precitec::hardware::welding::ZCompensation& zCompensation)
    : AbstractFigureWelding(control, zCompensation, std::make_unique<RTC6::WeldingStrategy>())
{
}

FigureWelding::~FigureWelding()
{
    stop_execution();
}

void FigureWelding::buildPreviewList(double newScannerXPosition,
                                         double newScannerYPosition,
                                         bool oFigureWobbleFileIsReady,
                                         double oJumpSpeedInBitsPerMs)
{
    const long oOffsetX = static_cast<long>(newScannerXPosition * m_calibrationFactor);
    const long oOffsetY = static_cast<long>(newScannerYPosition * m_calibrationFactor);

    RTC6::Figure* weldingSeam = getFigure(FigureFileType::WeldingFigureType);
    int oNumberOfPositions = weldingSeam->figure.size();
    int oActualPosition = 0;
    long oActualX = static_cast<long>(weldingSeam->figure[oActualPosition].endPosition.first * m_calibrationFactor) + oOffsetX;
    long oActualY = static_cast<long>(weldingSeam->figure[oActualPosition].endPosition.second * m_calibrationFactor) + oOffsetY;

    define_MiniSeamInit(ScanlabOperationMode::ScanlabPreview);

    set_MarkSize(m_oWobbelXSize * m_calibrationFactor, m_oWobbelYSize * m_calibrationFactor, m_oWobbelRadius * m_calibrationFactor);
    if (oFigureWobbleFileIsReady)
    {
        setNominalPower(0.0);
        setNominalRingPower(0.0);
        define_WobbleFigure(m_oWobbleMode, ScanlabOperationMode::ScanlabPreview);
    }
    else
    {
        setNominalPower(0.0);
        setNominalRingPower(0.0);
        define_WobbleFigure(WobbleMode::NoWobbling, ScanlabOperationMode::ScanlabPreview);
    }

    define_MiniSeamStart(oActualX, oActualY);
    ++oActualPosition;

    while (oActualPosition < oNumberOfPositions)
    {
        oActualX = static_cast<long>(weldingSeam->figure[oActualPosition].endPosition.first * m_calibrationFactor) + oOffsetX;
        oActualY = static_cast<long>(weldingSeam->figure[oActualPosition].endPosition.second * m_calibrationFactor) + oOffsetY;

        if (m_oWobbleMode == WobbleMode::Free)
        {
            define_MiniSeamLineDual(oActualX, oActualY, 0, oJumpSpeedInBitsPerMs, false, ScanlabOperationMode::ScanlabPreview);
        }
        else
        {
            define_MiniSeamLine(oActualX, oActualY, 0, 0, oJumpSpeedInBitsPerMs, ScanlabOperationMode::ScanlabPreview);
        }
        ++oActualPosition;
    }

    define_MiniSeamEnd(ScanlabOperationMode::ScanlabPreview);
}

void FigureWelding::prepareWeldingList(const precitec::interface::ResultDoubleArray& m_oWeldingData,
                                           std::size_t velocitiesSize,
                                           int m_oLaserPowerStatic,
                                           int m_oLaserPowerStaticRing,
                                           double m_oADCValue)
{
    if (!m_hasDigitalLaserPower)
    {
        laserPowerValues = laserPowerFromWeldingData(m_oWeldingData, 2, m_oLaserPowerStatic, m_oADCValue);
        laserPowerRingValues = laserPowerFromWeldingData(m_oWeldingData, 3, m_oLaserPowerStaticRing, m_oADCValue);
        laserPowerRingValues.reserve(m_oWeldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT);
        if (laserPowerValues.size() != velocitiesSize)
        {
            wmLog(eWarning, "Laser power size isn't equal to velocity size!");
        }
    }
    else
    {
        laserPowerValues = laserPowerFromWeldingData(m_oWeldingData, 2, 0, 1.0);
        if (laserPowerValues.size() != velocitiesSize)
        {
            wmLog(eWarning, "Seam program size isn't equal to velocity size!");
        }
    }
}

void FigureWelding::loadWobbleFigureFile(const std::string& figureFile, int m_laserPowerDelayCompensation, double m_oADCValue)
{
    readFigureFromFile(figureFile, FigureFileType::WobbleFigureType);
    setADCValue(m_oADCValue);
    setLaserPowerDelayCompensation(m_laserPowerDelayCompensation);
    if (!setNominalPowers())
    {
        wmLog(eWarning, "Wobble file is empty! No Wobbling is performed!");
    }

    if (laserPowerValues.front() == 0)
    {
        laserPowerValues.front() = 1;
    }
}

void FigureWelding::buildWeldingList(precitec::hardware::welding::Calculator& calculator,
                                         const precitec::interface::ResultDoubleArray& m_oWeldingData,
                                         const std::vector<double>& m_velocities)
{
    m_actualX.reset();
    m_actualY.reset();
    if (hasDigitalLaserPower())
    {
        buildDigitalList(calculator, m_oWeldingData, m_velocities);
        wmLog(eDebug, "ScanmasterSeamWelding, digital finished!\n");
    }
    else
    {
        buildAnalogList(calculator, m_oWeldingData, m_velocities);
        wmLog(eDebug, "ScanmasterSeamWelding, analog finished!\n");
    }
}

void FigureWelding::buildDigitalList(hardware::welding::Calculator& calculator,
                                         const interface::ResultDoubleArray& m_oWeldingData,
                                         const std::vector<double>& m_velocities)
{
    int oNumberOfPositions = m_oWeldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT; //Points are stored in x,y,p,v,r tuple (x = 0,2; y = 1,3; p = 100; r (Ring laser power) = 25; v = 50)
    int oActualPosition = 0;
    double oActualX = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT)]);
    double oActualY = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT) + 1]);

    bool firstTime = true;
    const long unsigned int seamProgramValue = laserPowerValues.at(0);
    define_MiniSeamInitDigital(seamProgramValue);

    if (m_oLaserDelay != 0.0) //First part of the prefix
    {
        auto prePositionForAccelerationInBits = definePrePosition(m_oLaserDelay, {m_oWeldingData.value()[0], m_oWeldingData.value()[1]}, {m_oWeldingData.value()[SEAMWELDING_RESULT_FIELDS_PER_POINT], m_oWeldingData.value()[SEAMWELDING_RESULT_FIELDS_PER_POINT + 1]});
        define_MiniSeamStart(prePositionForAccelerationInBits.first * m_calibrationFactor, prePositionForAccelerationInBits.second * m_calibrationFactor);
        define_MiniSeamJump(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY));
        ++oActualPosition;
    }
    else
    {
        define_MiniSeamStart(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY));
        ++oActualPosition;
    }

    while (oActualPosition < oNumberOfPositions)
    {
        oActualX = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT)]);
        oActualY = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT) + 1]);

        define_MiniSeamLineDigital(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY), m_velocities.at(oActualPosition - 1), firstTime);
        ++oActualPosition;
        if (firstTime == true)
        {
            firstTime = false;
        }
        m_actualX = oActualX;
        m_actualY = oActualY;
    }
    define_MiniSeamEndDigital();

}

void FigureWelding::buildAnalogList(precitec::hardware::welding::Calculator& calculator,
                                        const precitec::interface::ResultDoubleArray& m_oWeldingData,
                                        const std::vector<double>& m_velocities)
{
    int oNumberOfPositions = m_oWeldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT; //Points are stored in x,y,p,v,r tuple (x = 0,2; y = 1,3; p = 100; r (Ring laser power) = 25; v = 50)
    int oActualPosition = 0;
    double oActualX = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT)]);
    double oActualY = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT) + 1]);
    double oActualXBefore = 0.0;
    double oActualYBefore = 0.0;

    define_MiniSeamInit(ScanlabOperationMode::ScanlabWelding);

    set_MarkSize(m_oWobbelXSize * m_calibrationFactor, m_oWobbelYSize * m_calibrationFactor, m_oWobbelRadius * m_calibrationFactor);
    define_WobbleFigure(m_oWobbleMode, ScanlabOperationMode::ScanlabWelding);

    if (!calculator.wobbleStartPositionIsNull() && m_oWobbleMode == WobbleMode::Free)
    {
        oActualXBefore = oActualX;
        oActualYBefore = oActualY;
        oActualX = calculator.addOffset(oActualX, calculator.wobbleStartPosition().first);
        oActualY = calculator.addOffset(oActualY, calculator.wobbleStartPosition().second);
    }

    if (m_oLaserDelay != 0.0)
    {
        auto prePositionForAccelerationInBits = definePrePosition(m_oLaserDelay, {m_oWeldingData.value()[0], m_oWeldingData.value()[1]}, {m_oWeldingData.value()[SEAMWELDING_RESULT_FIELDS_PER_POINT], m_oWeldingData.value()[SEAMWELDING_RESULT_FIELDS_PER_POINT + 1]});
        define_MiniSeamStart(prePositionForAccelerationInBits.first * m_calibrationFactor, prePositionForAccelerationInBits.second * m_calibrationFactor);
        define_MiniSeamJump(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY));
        ++oActualPosition;
    }
    else
    {
        define_MiniSeamStart(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY));
        ++oActualPosition;
    }

    while (oActualPosition < oNumberOfPositions)
    {
        oActualX = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT)]);
        oActualY = m_zCompensation.get().positionValueCompensated(m_oWeldingData.value()[(oActualPosition * SEAMWELDING_RESULT_FIELDS_PER_POINT) + 1]);

        if (!calculator.wobbleStartPositionIsNull() && m_oWobbleMode == WobbleMode::Free)
        {
            auto angle = calculator.angleToXAxis(calculator.calculateVector(std::make_pair(oActualXBefore, oActualYBefore), std::make_pair(oActualX, oActualY)));
            oActualXBefore = oActualX;
            oActualYBefore = oActualY;
            const auto& rotatedOffset = calculator.rotateVector(calculator.wobbleStartPosition(), angle);
            oActualX = calculator.addOffset(oActualX, rotatedOffset.first);
            oActualY = calculator.addOffset(oActualY, rotatedOffset.second);
        }

        if (m_oWobbleMode == WobbleMode::Free)
        {
            define_MiniSeamLineDual(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY), laserPowerValues.at(oActualPosition - 1), m_velocities.at(oActualPosition - 1), true, ScanlabOperationMode::ScanlabWelding);
        }
        else
        {
            define_MiniSeamLine(calculator.calculateBitsFromMM(oActualX), calculator.calculateBitsFromMM(oActualY), laserPowerValues.at(oActualPosition - 1), laserPowerRingValues.at(oActualPosition - 1), m_velocities.at(oActualPosition - 1), ScanlabOperationMode::ScanlabWelding);
        }
        ++oActualPosition;
        m_actualX = oActualX;
        m_actualY = oActualY;
    }

    define_MiniSeamEnd(ScanlabOperationMode::ScanlabWelding);
}

void FigureWelding::start_Mark(ScanlabOperationMode p_oOperationMode)
{
    UINT oStatus = 0;
    UINT oPos = 0;
    while ((oStatus & 1) == 1)
    {
        get_status(&oStatus, &oPos);
    }
    if (p_oOperationMode == ScanlabOperationMode::ScanlabWelding)
    {
        enable_laser();
    }
    else
    {
        disable_laser();
    }
    execute_list(1);
}

void FigureWelding::define_MiniSeamInit(ScanlabOperationMode p_oOperationMode)
{
    const auto laserDelays = m_control.get().getLaserDelays();
    const auto geometricalTransformationData = m_control.get().getScannerGeometricalTransformationData();
    set_angle(1, (m_oAngle + geometricalTransformationData.angle), 0);
    set_offset(1, (m_oXOffset + geometricalTransformationData.x), (m_oYOffset + geometricalTransformationData.y), 0);

    resetLastPointProperties();

    while (load_list(1, 0) == 0);
    set_jump_speed(m_oJumpSpeed);
    set_mark_speed(m_oMarkSpeed);
    if (p_oOperationMode == ScanlabOperationMode::ScanlabWelding)
    {
        set_laser_delays(laserDelays.on, laserDelays.off);
    }
    set_scanner_delays(0, 0, 0);
    if (m_enableDebugMode)
    {
        set_trigger4(m_measurementPeriod, loggedSignal(0), loggedSignal(1), loggedSignal(2), loggedSignal(3));
    }
}

void FigureWelding::define_MiniSeamInitDigital(long unsigned int program)
{
    const auto laserDelays = m_control.get().getLaserDelays();
    const auto geometricalTransformationData = m_control.get().getScannerGeometricalTransformationData();
    set_angle(1, (m_oAngle + geometricalTransformationData.angle), 0);
    set_offset(1, (m_oXOffset + geometricalTransformationData.x), (m_oYOffset + geometricalTransformationData.y), 0);
    write_io_port(0); //Pro-start Low!

    while (load_list(1, 0) == 0);
    write_io_port_list(program << 1);
    set_jump_speed(m_oJumpSpeed);
    set_mark_speed(m_oMarkSpeed);
    set_laser_delays(laserDelays.on, laserDelays.off);
    set_scanner_delays(0, 0, 0);
}

FigureWelding::WobbleReturn FigureWelding::define_WobbleFigure(precitec::WobbleMode mode, precitec::ScanlabOperationMode operationMode)
{
    if (mode == precitec::WobbleMode::NoWobbling)
    {
        deactivateWobbleMode();
        return WobbleReturn::NoWobbling;
    }

    if (mode != precitec::WobbleMode::Free)
    {
        set_wobbel_mode(m_oXSize, m_oYSize, m_oWobbelFreq, static_cast<int>(mode));
        return WobbleReturn::BasicWobbling;
    }

    if (m_freeFigure.figure.size() <= MIN_WOBBLE_FIGURE_POINTS)
    {
        deactivateWobbleMode();
        return WobbleReturn::InvalidWobbleFigure;
    }

    const auto wobbleControl = static_cast<precitec::WobbleControl>(m_freeFigure.powerModulationMode);
    calculateShiftAndShiftedNominalPower(wobbleControl);

    if (operationMode == ScanlabOperationMode::ScanlabWelding && fuzzyIsNull(nominalPower()) && (wobbleControl == precitec::WobbleControl::AnalogOut1Variation || wobbleControl == precitec::WobbleControl::AnalogOut1And2Variation))
    {
        wmFatal(eAxis, "QnxMsg.VI.NominalPowerWrong", "No wobbling is performed because the nominal power (start power) is 0.0!\n");
        return WobbleReturn::NoWobbling;
    }
    if (operationMode == ScanlabOperationMode::ScanlabWelding && fuzzyIsNull(nominalRingPower()) && (wobbleControl == precitec::WobbleControl::AnalogOut2Variation || wobbleControl == precitec::WobbleControl::AnalogOut1And2Variation))
    {
        wmFatal(eAxis, "QnxMsg.VI.NominalRingPowerWrong", "No wobbling is performed because the nominal ring power (start ring power) is 0.0!\n");
        return WobbleReturn::NoWobbling;
    }

    setPowerModulationSettings(wobbleControl);

    if (m_freeFigure.microVectorFactor == 1) //1 is default value
    {
        wmLogTr(eWarning, "QnxMsg.VI.NoFrequencySet", "Wobble frequency wasn't set in the selected wobble figure\n");
    }

    /*
     * Units: Points [mm] in x- and y-direction.
     *        Vector length [mm]
     *        Vector time [10us]
     */

    if (wobbleControl != precitec::WobbleControl::AnalogOut1And2Variation)
    {
        defineOnePortWobbleFigure(wobbleControl);
        return WobbleReturn::OnePortWobbling;
    }

    const std::size_t countWobbelVectors = m_freeFigure.figure.size() - 1;

    if (countWobbelVectors > MAX_WOBBLE_SEGMENTS)
    {
        //RTC6 cards only allow 1023 set_wobbel_vectors (wobble segments) otherwise there is an overflow and the first commands are overwritten.
        wmLog(eWarning, "Too many wobble segments!\n");
        deactivateWobbleMode();
        return WobbleReturn::TooManySegments;
    }

    if (m_freeFigure.microVectorFactor % 2 == 1)
    {
        //If micro vector factor is odd then modulation doesn't work anymore. Ring power looses the last power change.
        wmLog(eWarning, "Micro vector factor has to be even!\n");
        return WobbleReturn::InvalidMicroVectorFactor;
    }

    defineTwoPortWobbleFigure(countWobbelVectors);
    return WobbleReturn::TwoPortWobbling;
}

void FigureWelding::define_MiniSeamStart(long p_oXStart, long p_oYStart)
{
    jump_abs(p_oXStart, p_oYStart);
}

void FigureWelding::define_MiniSeamJump(long p_oXJump, long p_oYJump)
{
    set_jump_speed(m_oMarkSpeed);
    jump_abs(p_oXJump, p_oYJump);
    set_jump_speed(m_oJumpSpeed);
}

void FigureWelding::define_MiniSeamLineDigital(long p_oXEnd, long p_oYEnd, double velocity, bool firstTime)
{
    if (firstTime)
    {
        write_io_port_mask_list(129, 129); //Pro_start && Bit 7 HIGH --> Laser starts to weld! Bit 7 is reference for SPS.
    }
    set_mark_speed(velocity);
    mark_abs(p_oXEnd, p_oYEnd);
    list_nop();
}

void FigureWelding::define_MiniSeamLineDual(long p_oXEnd, long p_oYEnd, unsigned long laserPowerCenter, double velocity, bool wobbleMode, ScanlabOperationMode p_oOperationMode)
{
    if (laserPowerCenter == 0 && !wobbleMode)
    {
        if (p_oOperationMode == ScanlabOperationMode::ScanlabWelding)
        {
            set_jump_speed(velocity);
            jump_abs(p_oXEnd, p_oYEnd);
        }
        else
        {
            set_mark_speed(velocity);
            mark_abs(p_oXEnd, p_oYEnd);
        }
    }
    else
    {
        set_mark_speed(velocity);
        mark_abs(p_oXEnd, p_oYEnd);
    }
}

void FigureWelding::define_MiniSeamLine(long p_oXEnd, long p_oYEnd, unsigned long laserPowerCenter, unsigned long laserPowerRing, double velocity, ScanlabOperationMode p_oOperationMode)
{
    const auto analogOutput = m_control.get().getAnalogOutput();

    if (laserPowerCenter == 0)
    {
        if (p_oOperationMode == ScanlabOperationMode::ScanlabWelding)
        {
            set_jump_speed(velocity);
            jump_abs(p_oXEnd, p_oYEnd);
        }
        else
        {
            set_mark_speed(velocity);
            mark_abs(p_oXEnd, p_oYEnd);
        }
    }
    else
    {
        if (m_lastProperties.firstTime || m_lastProperties.power != laserPowerCenter)
        {
            set_laser_power(analogOutput.value - 1, laserPowerCenter);
        }
        if (m_lastProperties.firstTime || m_lastProperties.ringPower != laserPowerRing)
        {
            set_laser_power(analogOutput.value, laserPowerRing);
        }
        if (!fuzzyIsNull(m_lastProperties.velocity - velocity))
        {
            set_mark_speed(velocity);
        }
        mark_abs(p_oXEnd, p_oYEnd);
    }
    m_lastProperties.firstTime = false;
    m_lastProperties.power = laserPowerCenter;
    m_lastProperties.ringPower = laserPowerRing;
    m_lastProperties.velocity = velocity;
}

void FigureWelding::define_MiniSeamEnd(ScanlabOperationMode p_oOperationMode)
{
    set_wobbel_vector(1.0, 1.0, 0, 1.0);
    if (m_enableDebugMode)
    {
        set_trigger4(0, 0, 0, 0, 0); //End measurement otherwise function loggedScannerData has a infinite loop!
    }
    if (p_oOperationMode == ScanlabOperationMode::ScanlabWelding)
    {
        set_end_of_list();
    }
    else
    {
        list_jump_pos(0);
    }
}

void FigureWelding::define_MiniSeamEndDigital()
{
    const auto laserDelays = m_control.get().getLaserDelays();
    set_wobbel_vector(1.0, 1.0, 0, 1.0);
    long_delay(laserDelays.off / 640);
    write_io_port_list(0);
    if (m_enableDebugMode)
    {
        set_trigger4(0, 0, 0, 0, 0); //End measurement otherwise function loggedScannerData has a infinite loop!
    }
    set_end_of_list();
}

void FigureWelding::saveLoggedScannerData()
{
    UINT status = 1;
    UINT counter = 0;
    measurement_status(&status, &counter);
    int whileCounter = 0;

    while (status > 0 && whileCounter < 500)
    {
        measurement_status(&status, &counter);
        whileCounter++;
    }

    if (counter == 0)
    {
        wmLog(eDebug, "No data, buffer is empty!\n");
        return;
    }

    if (!isLoggedSignal(0))
    {
        wmLog(eDebug, "Signal number 1 has to be valid!\n");
        return;
    }

    for (auto it = m_measurementSignals.begin(); it != m_measurementSignals.end(); it++)
    {
        it->clear();
        it->resize(counter);
    }

    for (std::size_t i = 0; i < m_measurementSignals.size(); i++)
    {
        if (!isLoggedSignal(i))
        {
            continue;
        }
        get_waveform(i + 1, counter, reinterpret_cast<ULONG_PTR>(m_measurementSignals.at(i).data()));
    }

    const auto lastError = get_last_error();
    if (lastError != 0)
    {
        wmLog(eError, "Error while transmitting measurement values\n");
    }

    const auto filePath = std::string(getenv("WM_BASE_DIR")) + std::string("/config/weld_figure/");
    saveDataToFile(filePath);
}

bool FigureWelding::saveDataToFile(const std::string& dir)
{
    if (m_measurementSignals.at(0).empty() || !isLoggedSignal(0))
    {
        return false;
    }

    const auto filePath = dir + "/loggedScannerData.txt";
    std::fstream dataFile(filePath, std::ios::out);
    if (!dataFile.is_open())
    {
        return false;
    }

    dataFile << std::setprecision(3) << std::left << "Data1";

    if (isLoggedSignal(1))
    {
        dataFile << ",Data2";
    }

    if (isLoggedSignal(2))
    {
        dataFile << ",Data3";
    }

    if (isLoggedSignal(3))
    {
        dataFile << ",Data4";
    }
    dataFile << std::endl;

    auto iterator2 = m_measurementSignals.at(1).begin();
    auto iterator3 = m_measurementSignals.at(2).begin();
    auto iterator4 = m_measurementSignals.at(3).begin();

    for (auto iterator = m_measurementSignals.at(0).begin(); iterator != m_measurementSignals.at(0).end(); ++iterator)
    {
        dataFile << *iterator;
        if (isLoggedSignal(1))
        {
            dataFile << "," << *iterator2;
            ++iterator2;
        }

        if (isLoggedSignal(2))
        {
            dataFile << "," << *iterator3;
            ++iterator3;
        }

        if (isLoggedSignal(3))
        {
            dataFile << "," << *iterator4;
            ++iterator4;
        }
        dataFile << std::endl;
    }

    dataFile.close();
    return true;
}

void FigureWelding::deactivateWobbleMode()
{
    set_wobbel_mode(VALID__WOBBEL_MODE_VALUE, VALID__WOBBEL_MODE_VALUE, INVALID__WOBBEL_MODE_VALUE, VALID__WOBBEL_MODE_VALUE);
}

void FigureWelding::setPowerModulationSettings(precitec::WobbleControl wobbleControl)
{
    const uint32_t oMaxAnalogOutputValue = wobbleControl == WobbleControl::AnalogOut1And2Variation ? MAX_ADC_12BIT_VALUE_32BIT : MAX_ADC_12BIT_VALUE;

    write_da_x_list(static_cast<int>(precitec::WobbleControl::AnalogOut1Variation), convertPowerToBits(m_nominalPower));
    write_da_x_list(static_cast<int>(precitec::WobbleControl::AnalogOut2Variation), convertPowerToBits(m_nominalRingPower));
    set_wobbel_control(static_cast<int>(wobbleControl), USE_VALUE_ON_ANALOG_PORT, MIN_ADC_12BIT_VALUE, oMaxAnalogOutputValue);
}

void FigureWelding::setOnePortWobbleVector(std::size_t countWobbelVectors)
{
    for (std::size_t i = 0; i < countWobbelVectors - 1; i++)
    {
        set_wobbel_vector(m_wobbelTransVector.at(i) * m_calibrationFactor / m_freeFigure.microVectorFactor, m_wobbelLongVector.at(i) * m_calibrationFactor / m_freeFigure.microVectorFactor, m_freeFigure.microVectorFactor, m_deltaPower.at(i));
    }

    set_wobbel_mode(VALID__WOBBEL_MODE_VALUE, VALID__WOBBEL_MODE_VALUE, VALID__WOBBEL_MODE_VALUE, static_cast<int>(precitec::WobbleMode::Free));
}

void FigureWelding::setTwoPortWobbleVector(std::size_t countWobbelVectors)
{
    for (std::size_t i = 0; i < countWobbelVectors; i++)
    {
        set_wobbel_vector_2(m_wobbelTransVector.at(i) * m_calibrationFactor / m_freeFigure.microVectorFactor, m_wobbelLongVector.at(i) * m_calibrationFactor / m_freeFigure.microVectorFactor, m_freeFigure.microVectorFactor - 1, m_deltaPower.at(i), m_deltaRingPower.at(i), 0);
        set_wobbel_vector_2(m_wobbelTransVector.at(i) * m_calibrationFactor / m_freeFigure.microVectorFactor, m_wobbelLongVector.at(i) * m_calibrationFactor / m_freeFigure.microVectorFactor, 1, 0.0, m_deltaRingPower.at(i), INVALID__WOBBEL_MODE_VALUE);
    }

    set_wobbel_mode(VALID__WOBBEL_MODE_VALUE, VALID__WOBBEL_MODE_VALUE, VALID__WOBBEL_MODE_VALUE, static_cast<int>(precitec::WobbleMode::Free));
}

void FigureWelding::resetLastPointProperties()
{
    m_lastProperties.firstTime = true;
    m_lastProperties.power = 0;
    m_lastProperties.ringPower = 0;
    m_lastProperties.velocity = 0.0;
}

unsigned long int FigureWelding::convertPowerToBits(double power)
{
    return static_cast<unsigned long int>(power * m_ADCValue);
}
}