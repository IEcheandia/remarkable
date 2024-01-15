#pragma once

#include "RTC6DataTypes.h"
#include "common/definesScanlab.h"

#include <array>
#include <functional>
#include <memory>

using precitec::LoggerSignals;
using precitec::StatusSignals;

namespace precitec::hardware
{
class Scanner2DWeldingData;
class AbstractWeldingStrategy;
}

namespace RTC6
{
class WeldingStrategy;
}

class AbstractScanner2DWelding
{
public:
    ~AbstractScanner2DWelding();
    void set_CalibrationFactor(const double &calibFactor);
    void add_LaserPower(const unsigned long int &newLaserPower);
    void setLaserPowerStatic(const unsigned long int &newLaserPower);
    void set_Speeds(const double p_oJumpSpeed, const double p_oMarkSpeed, const int p_oWobbelFreq);
    void set_MarkOffset(const long int p_oXOffset, const long int p_oYOffset, double p_oAngle);
    void set_MarkAngle( const double p_oAngle);
    void set_MarkSize(const long int &p_oXSize, const long int &p_oYSize, const long int &p_oRadius);
    std::vector<unsigned long int>* get_LaserPower();
    unsigned long int getLaserPowerStatic(void);
    void get_Speeds(double &p_rJumpSpeed, double &p_rMarkSpeed, int &p_rWobbelFreq);
    void get_MarkOffset(long int &p_rXOffset, long int &p_rYOffset);
    double get_MarkAngle();
    void get_MarkSize(long int &p_rXSize, long int &p_rYSize, long int &p_rRadius);
    void stop_Mark();
    bool done_Mark();

    bool readFigureFromFile(const std::string& filename, precitec::FigureFileType figureFileType);
    RTC6::Figure* getFigure(precitec::FigureFileType figureFileType);

    //Debug mode
    bool get_EnableDebugMode();
    void set_EnableDebugMode(bool oValue);
    int get_MeasurementPeriod();
    void set_MeasurementPeriod(int oValue);
    LoggerSignals getLoggedSignal(std::size_t index) const
    {
        return m_loggedSignals.at(index);
    }
    void setLoggedSignal(std::size_t index, LoggerSignals value);
    enum class Axis : std::uint32_t
    {
        First = 1,
        Second = 2,
    };
    int setStatusSignalHead1Axis(Axis axis, StatusSignals value);
protected:
    explicit AbstractScanner2DWelding(const precitec::hardware::Scanner2DWeldingData& control,
                          std::unique_ptr<precitec::hardware::AbstractWeldingStrategy> weldingStrategy);
    std::uint32_t loggedSignal(std::size_t i) const
    {
        return static_cast<std::uint32_t>(m_loggedSignals.at(i));
    }
    bool isLoggedSignal(std::size_t i) const
    {
        return m_loggedSignals.at(i) != LoggerSignals::NoSignal;
    }

    std::reference_wrapper<const precitec::hardware::Scanner2DWeldingData> m_control;
    std::unique_ptr<precitec::hardware::AbstractWeldingStrategy> m_weldingStrategy;
    double m_calibrationFactor = 0.0;
    double m_oJumpSpeed = 0.0;
    double m_oMarkSpeed = 0.0;
    int m_oWobbelFreq = 0;
    long int m_oXOffset = 0;
    long int m_oYOffset = 0;
    double m_oAngle = 0.0;
    long int m_oXSize = 0;
    long int m_oYSize = 0;
    long int m_oRadius = 0;
    std::vector<unsigned long int> m_pLaserPowerValues;
    unsigned long int m_laserPowerStatic{0};
    RTC6::Figure m_freeFigure;
    RTC6::Figure m_weldingFigure;

    //Debug mode
    bool m_enableDebugMode = false;
    int m_measurementPeriod = 1;                                                //[10us]

private:
    std::array<LoggerSignals, 4> m_loggedSignals;
};
