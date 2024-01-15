
#include "viWeldHead/Scanlab/ScannerCalibrationWelding.h"
#include "module/moduleLogger.h"
#include <rtc6.h>

ScannerCalibrationWelding::ScannerCalibrationWelding(
    const std::vector<precitec::geo2d::DPoint>& points,
    double laserPowerCenterInBits,
    double weldingDurationInMs,
    double jumpSpeedInMmPerSec,
    unsigned int analogOutput,
    double m_calibValueBitsPerMM):
    m_points(points)
    , m_laserPowerCenterInBits(laserPowerCenterInBits)
    , m_weldingDurationInMs(weldingDurationInMs)
    , m_jumpSpeedInMmPerSec(jumpSpeedInMmPerSec)
    , m_analogOutput(analogOutput)
    , m_calibValueBitsPerMM(m_calibValueBitsPerMM)
{

}

ScannerCalibrationWelding::~ScannerCalibrationWelding()
{
}

void ScannerCalibrationWelding::start_Mark() const
{
    UINT status = 0;
    UINT pos = 0;
    while ((status & 1) == 1)
    {
        get_status(&status, &pos);
    }
    enable_laser();
    execute_list(1);
}


bool ScannerCalibrationWelding::done_Mark() const
{
    UINT status = 0;
    UINT pos = 0;
    get_status(&status, &pos);
    if (!(status & 1))
    {
        return true;
    }
    return false;
}

void ScannerCalibrationWelding::define_Mark() const
{
    const auto weldingDurationIn10us = transformDurationInMsTo10us(m_weldingDurationInMs);
    const auto jumpSpeedInBitsPerMs = transformSpeedInMsPerSecToBitsPerMs(m_jumpSpeedInMmPerSec);
    while (load_list(1, 0) == 0);
    if (m_laserPowerCenterInBits != 0)
    {
        set_laser_power(m_analogOutput -1, m_laserPowerCenterInBits);
    }
    set_jump_speed(jumpSpeedInBitsPerMs);
    for (const auto& point: m_points)
    {
        const auto pointInBits = transformPointInMMToBit(point);
        jump_abs(pointInBits.x, pointInBits.y);
        laser_on_list(weldingDurationIn10us);
    }
    set_end_of_list();
}

void ScannerCalibrationWelding::stop_Mark() const
{
    stop_execution();
    reset_error(0x20); // clear RTC6_BUSY
    write_da_1(0);
}

unsigned int ScannerCalibrationWelding::transformDurationInMsTo10us(double durationInMs)
{
    return static_cast<unsigned int>(durationInMs * 100.);
}

precitec::geo2d::DPoint ScannerCalibrationWelding::transformPointInMMToBit(const precitec::geo2d::DPoint& point) const
{
    return precitec::geo2d::DPoint(point.x * m_calibValueBitsPerMM, point.y * m_calibValueBitsPerMM);
}


double ScannerCalibrationWelding::transformSpeedInMsPerSecToBitsPerMs(double jumpSpeedInMmPerSec) const
{
    auto jumpSpeedInBitsPerMs = jumpSpeedInMmPerSec * 0.001 * m_calibValueBitsPerMM;
    if (jumpSpeedInBitsPerMs < 1.6)
    {
        jumpSpeedInBitsPerMs = 1.6;
    }
    if (jumpSpeedInBitsPerMs > 800000.0)
    {
        jumpSpeedInBitsPerMs = 800000.0;
    }
    return jumpSpeedInBitsPerMs;
}
