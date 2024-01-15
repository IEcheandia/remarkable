#include "viWeldHead/Scanlab/AbstractScanner2DWelding.h"
#include "viWeldHead/Scanlab/AbstractWeldingStrategy.h"

#include "viWeldHead/Scanlab/RTC6jsonSupport.h"
#include "viWeldHead/Scanlab/RTC6WeldingStrategy.h"

#include "module/moduleLogger.h"

using precitec::eError;
using precitec::eWarning;
using precitec::hardware::Scanner2DWeldingData;
using precitec::LoggerSignals;
using precitec::StatusSignals;

AbstractScanner2DWelding::AbstractScanner2DWelding(const Scanner2DWeldingData& control,
                           std::unique_ptr<precitec::hardware::AbstractWeldingStrategy> weldingStrategy)
    : m_control(control)
    , m_weldingStrategy(std::move(weldingStrategy))
{
    m_loggedSignals.fill(LoggerSignals::NoSignal);
}

AbstractScanner2DWelding::~AbstractScanner2DWelding()
{}

void AbstractScanner2DWelding::set_CalibrationFactor(const double& calibFactor)
{
    m_calibrationFactor = calibFactor;
}

void AbstractScanner2DWelding::add_LaserPower(const unsigned long& newLaserPower)
{
    m_pLaserPowerValues.push_back(newLaserPower);
}

void AbstractScanner2DWelding::setLaserPowerStatic(const unsigned long& newLaserPower)
{
    m_laserPowerStatic = newLaserPower;
}

void AbstractScanner2DWelding::set_Speeds(const double p_oJumpSpeed, const double p_oMarkSpeed, const int p_oWobbelFreq)
{
    m_oJumpSpeed = p_oJumpSpeed;
    m_oMarkSpeed = p_oMarkSpeed;
    m_oWobbelFreq = p_oWobbelFreq;
}

void AbstractScanner2DWelding::set_MarkOffset(const long p_oXOffset, const long p_oYOffset, double p_oAngle)
{
    m_oXOffset = p_oXOffset;
    m_oYOffset = p_oYOffset;
    m_oAngle = p_oAngle;
}

void AbstractScanner2DWelding::set_MarkAngle(const double p_oAngle)
{
    m_oAngle = p_oAngle;
}

void AbstractScanner2DWelding::set_MarkSize(const long& p_oXSize, const long& p_oYSize, const long& p_oRadius)
{
    m_oXSize = p_oXSize;
    m_oYSize = p_oYSize;
    m_oRadius = p_oRadius;
}

std::vector<unsigned long> * AbstractScanner2DWelding::get_LaserPower()
{
    return &m_pLaserPowerValues;
}

unsigned long int AbstractScanner2DWelding::getLaserPowerStatic(void)
{
    return m_laserPowerStatic;
}

void AbstractScanner2DWelding::get_Speeds(double& p_rJumpSpeed, double& p_rMarkSpeed, int& p_rWobbelFreq)
{
    p_rJumpSpeed = m_oJumpSpeed;
    p_rMarkSpeed = m_oMarkSpeed;
    p_rWobbelFreq = m_oWobbelFreq;
}

void AbstractScanner2DWelding::get_MarkOffset(long& p_rXOffset, long& p_rYOffset)
{
    p_rXOffset = m_oXOffset;
    p_rYOffset = m_oYOffset;
}

double AbstractScanner2DWelding::get_MarkAngle()
{
    return m_oAngle;
}

void AbstractScanner2DWelding::get_MarkSize(long& p_rXSize, long& p_rYSize, long& p_rRadius)
{
    p_rXSize = m_oXSize;
    p_rYSize = m_oYSize;
    p_rRadius = m_oRadius;
}

void AbstractScanner2DWelding::stop_Mark()
{
    m_weldingStrategy->stop_Mark();
}

bool AbstractScanner2DWelding::done_Mark()
{
    return m_weldingStrategy->done_Mark();
}

bool AbstractScanner2DWelding::readFigureFromFile(const std::string& filename, precitec::FigureFileType figureFileType)
{
    try
    {
        nlohmann::ordered_json j;
        RTC6::readFromFile(j, filename);
        if (figureFileType == precitec::FigureFileType::WobbleFigureType)
        {
            m_freeFigure = j;
        }
        else
        {
            m_weldingFigure = j;
        }
    }
    catch (nlohmann::json::parse_error& e)
    {
        wmLog(eError, "%s is corrupted!\n", filename.c_str());
        wmLog(eError, "%s\n", e.what());
        return false;
    }
    catch (...)
    {
        wmLog(eWarning, "No wobble figure file found ...\n");
        wmLog(eWarning, "%s is missing!\n", filename.c_str());
        return false;
    }

    return true;
}

RTC6::Figure* AbstractScanner2DWelding::getFigure(precitec::FigureFileType figureFileType)
{
    if (figureFileType == precitec::FigureFileType::WobbleFigureType)
    {
        return &m_freeFigure;
    }
    else
    {
        return &m_weldingFigure;
    }
}

bool AbstractScanner2DWelding::get_EnableDebugMode()
{
    return m_enableDebugMode;
}

void AbstractScanner2DWelding::set_EnableDebugMode(bool oValue)
{
    m_enableDebugMode = oValue;
}

int AbstractScanner2DWelding::get_MeasurementPeriod()
{
    return m_measurementPeriod;
}

void AbstractScanner2DWelding::set_MeasurementPeriod(int oValue)
{
    m_measurementPeriod = oValue;
}

void AbstractScanner2DWelding::setLoggedSignal(std::size_t index, LoggerSignals value)
{
    m_loggedSignals.at(index) = value;
}

int AbstractScanner2DWelding::setStatusSignalHead1Axis(Axis axis, StatusSignals value)
{
    return m_weldingStrategy->setStatusSignalHead1Axis(static_cast<precitec::Axis>(axis), value);
}
