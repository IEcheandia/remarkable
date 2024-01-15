#pragma once

#include <vector>
#include <geo/point.h>

class ScannerCalibrationWelding
{
public:
    ScannerCalibrationWelding(
        const std::vector<precitec::geo2d::DPoint>& points,
        double weldingDurationInMs,
        double laserPowerCenterInBits,
        double jumpSpeedInMmPerSec,
        unsigned int analogOutput,
        double calibValueBitsPerMM);

    ~ScannerCalibrationWelding();
    void start_Mark() const;
    void define_Mark() const;
    bool done_Mark() const;
    void stop_Mark() const;

private:
    static unsigned int transformDurationInMsTo10us(double durationInMs);
    precitec::geo2d::DPoint transformPointInMMToBit(const precitec::geo2d::DPoint& point) const;
    double transformSpeedInMsPerSecToBitsPerMs(double jumpSpeedInMmPerSec) const;
    std::vector<precitec::geo2d::DPoint> m_points;
    double m_laserPowerCenterInBits;
    double m_weldingDurationInMs;
    double m_jumpSpeedInMmPerSec;
    unsigned int m_analogOutput;
    double m_calibValueBitsPerMM;
};
