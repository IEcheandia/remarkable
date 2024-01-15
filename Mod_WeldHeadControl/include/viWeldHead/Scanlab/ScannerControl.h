#pragma once
#include "common/definesScanlab.h"
#include "common/systemConfiguration.h"
#include "Scanner2DWeldingData.h"

#include <string>

namespace precitec::hardware
{

enum class ScannerControlStatus
{
    InitializationFailed,
    Disabled,
    Initialized
};

class ScannerControl : public Scanner2DWeldingData
{
public:
    [[nodiscard]] virtual ScannerControlStatus init(std::string ipAddress,
                                                    const std::string& scanlabCorrectionFile,
                                                    precitec::interface::LensType lensType,
                                                    precitec::interface::ScannerModel scannerModel) = 0;

    [[nodiscard]] virtual double calibValueBitsPerMM() = 0;
    [[nodiscard]] virtual unsigned int numberOfPointsFromContour(std::size_t contourSize) = 0;
    [[nodiscard]] virtual unsigned int numberOfPossiblePointsForListMemory() = 0;

    virtual double calculateJumpSpeed(double meterPerSeconds) const = 0;
    virtual double calculateMarkSpeed(double meterPerSeconds) const = 0;

    virtual void setLaserDelays(LaserDelays delays) = 0;
    virtual void setScannerJumpSpeed(double speedInMeterPerSec) = 0;
    virtual void setScannerMarkSpeed(double speedInMeterPerSec) = 0;
    virtual void sendToleranceInBitsToScanner(int toleranceInBits) = 0;

    virtual void scannerDriveToZero() = 0;
    virtual void scannerDriveToPosition(double xPosInMM, double yPosInMM) = 0;
    virtual void scannerDriveWithOCTReference(int32_t xPosInBits, int32_t yPosInBits, uint16_t binaryValue, uint16_t maskValue) = 0;
    virtual void scannerSetOCTReference(uint16_t binaryValue, uint16_t maskValue) = 0;
    virtual void jumpScannerOffset() = 0;

    virtual void checkStatusWord(precitec::Axis axis) = 0;
    virtual void checkTemperature(precitec::Axis axis) = 0;
    virtual void checkServoCardTemperature(precitec::Axis axis) = 0;
    virtual void checkStopEvent(precitec::Axis axis) = 0;

    virtual void scannerCheckLastError() = 0;
    virtual void scannerResetLastError() = 0;

    virtual void scannerTestFunction1() = 0;
    virtual void scannerTestFunction2() = 0;

    virtual double selectCorrectionFileMode(precitec::interface::CorrectionFileMode mode) = 0;
};

}
