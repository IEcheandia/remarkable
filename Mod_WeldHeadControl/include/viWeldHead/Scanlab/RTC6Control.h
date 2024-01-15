#pragma once

#include "RTC6Scanner.h"
#include "RTC6Laser.h"
#include "RTC6Board.h"
#include "ScannerControl.h"

namespace precitec::hardware
{
enum class ScannerControlStatus;
}

namespace RTC6
{

class Control : public precitec::hardware::ScannerControl
{
public:
    explicit Control(precitec::interface::LensType lensType);

    [[nodiscard]] precitec::hardware::ScannerControlStatus init(std::string ipAddress,
                                                                const std::string& scanlabCorrectionFile,
                                                                precitec::interface::LensType lensType,
                                                                precitec::interface::ScannerModel scannerModel) override;

    [[nodiscard]] double calibValueBitsPerMM() override;
    [[nodiscard]] unsigned int numberOfPointsFromContour(std::size_t contourSize) override;
    [[nodiscard]] unsigned int numberOfPossiblePointsForListMemory() override;

    void setLaserDelays(precitec::hardware::LaserDelays delays) override;
    void setScannerJumpSpeed(double speedInMeterPerSec) override;
    void setScannerMarkSpeed(double speedInMeterPerSec) override;
    void sendToleranceInBitsToScanner(int toleranceInBits) override;

    double calculateJumpSpeed(double meterPerSeconds) const override;
    double calculateMarkSpeed(double meterPerSeconds) const override;

    void scannerDriveToZero() override;
    void scannerDriveToPosition(double xPosInMM, double yPosInMM) override;
    void scannerDriveWithOCTReference(int32_t xPosInBits, int32_t yPosInBits, uint16_t binaryValue, uint16_t maskValue) override;
    void scannerSetOCTReference(uint16_t binaryValue, uint16_t maskValue) override;
    void jumpScannerOffset() override;

    void checkStatusWord(precitec::Axis axis) override;
    void checkTemperature(precitec::Axis axis) override;
    void checkServoCardTemperature(precitec::Axis axis) override;
    void checkStopEvent(precitec::Axis axis) override;

    void scannerCheckLastError() override;
    void scannerResetLastError() override;

    void scannerTestFunction1() override;
    void scannerTestFunction2() override;

    [[nodiscard]] precitec::hardware::LaserDelays getLaserDelays() const override;
    [[nodiscard]] precitec::hardware::AnalogOutput getAnalogOutput() const override;
    [[nodiscard]] precitec::hardware::ScannerGeometricalTransformationData getScannerGeometricalTransformationData() const override;

    [[nodiscard]] double selectCorrectionFileMode(CorrectionFileMode mode) override;

private:
    Board m_board;
    Scanner m_scanner;
    Laser m_laser;
};

}
