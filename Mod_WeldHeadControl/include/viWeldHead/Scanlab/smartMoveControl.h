#pragma once

#include <string>
#include "viWeldHead/Scanlab/scannerInterface.h"
#include "viWeldHead/Scanlab/smartMoveLowLevel.h"
#include "viWeldHead/Scanlab/globalCommandGenerator.h"
#include "viWeldHead/Scanlab/smartMoveGlobalInterpreter.h"
#include "viWeldHead/Scanlab/smartMoveCalculator.h"
#include "viWeldHead/Scanlab/contourGenerator.h"
#include "viWeldHead/Scanlab/smartMoveInterpreter.h"
#include "common/definesWeldingFigure.h"
#include "event/results.h"
#include "ScannerControl.h"

class SmartMoveControlTest;

namespace precitec
{
namespace hardware
{

class LensData;

class SmartMoveControl : public ScannerControl
{
public:
    explicit SmartMoveControl();
    [[nodiscard]] ScannerControlStatus init(std::string ipAddress,
                                                    const std::string& scanlabCorrectionFile,
                                                    precitec::interface::LensType lensType,
                                                    precitec::interface::ScannerModel scannerModel) override;

    [[nodiscard]] double calibValueBitsPerMM() override;
    [[nodiscard]] unsigned int numberOfPointsFromContour(std::size_t contourSize) override;
    [[nodiscard]] unsigned int numberOfPossiblePointsForListMemory() override;

    void setLaserDelays(LaserDelays delays) override;
    void setScannerJumpSpeed(double speedInMeterPerSec) override;
    void setScannerMarkSpeed(double speedInMeterPerSec) override;
    void sendToleranceInBitsToScanner(int toleranceInBits) override;

    double calculateJumpSpeed(double meterPerSeconds) const override;
    double calculateMarkSpeed(double meterPerSeconds) const override;

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

    double selectCorrectionFileMode(precitec::interface::CorrectionFileMode mode) override;

    [[nodiscard]] LaserDelays getLaserDelays() const override;
    [[nodiscard]] AnalogOutput getAnalogOutput() const override;
    [[nodiscard]] ScannerGeometricalTransformationData getScannerGeometricalTransformationData() const override;

    int init(const InitData& dataForInitialization);

    void startJob(JobRepeats repeatMode);

    void scannerDriveToPosition(std::pair<double, double> position);
    void scannerDriveToZero() override;

    void prepareWeldingList(const std::vector<double>& weldingData, precitec::weldingFigure::ProductValues defaultValuesProduct);       //NOTE precitec::interface::ResultDoubleArray.value() returns a const std::vector<double>& NOTE The function can be tested now.
    void buildPreviewList();
    void buildWeldingList();

    void startMark();
    void selectList();

    precitec::hardware::SmartMoveLowLevel& networkInterface()
    {
        return m_networkInterface;
    }

    double sysTs() const
    {
        return m_sysTs;
    }

private:
    double translateSpecialValue(const precitec::weldingFigure::SpecialValueInformation& currentState);
    void scannerDriveToPositionBits(std::pair<unsigned int, unsigned int> position);
    void checkHpglUpload(const std::string& answer) const;

    std::string correctionFile(const LensData& currentLensData, const std::string& customCorrectionFile);
    bool sendCorrectionFile(const std::string& correctionFile, const std::string& correctionFileOnME);
    bool uploadCorrectionFile(const std::string& correctionFile);
    //Holds all points of a contour after using the function prepareWeldingList
    //PrepareWeldingList creates the points and handles the special values (-1, -2)
    std::vector<precitec::weldingFigure::Point> m_currentPoints;

    //Class for calculating from millimeter to bits (depends on scan field size)
    precitec::hardware::SmartMoveCalculator m_calculator;

    //Hardware connector class is used to transfer the commands to the marking engine
    precitec::hardware::SmartMoveLowLevel m_networkInterface;

    //Classes for using the meta language for global commands
    //Global commands manipulate the state or parameters of the marking engine that aren't changed while marking
    precitec::hardware::GlobalCommandGenerator m_globalGenerator;
    precitec::hardware::SmartMoveGlobalInterpreter m_globalInterpreter;

    //Classes for using the contour meta language for welding figures
    //Defines the route and the settings on every part of the marking
    precitec::hardware::ContourGenerator m_generator;
    precitec::hardware::SmartMoveInterpreter m_interpreter;

    double m_sysTs{0.0};

    friend SmartMoveControlTest;
};

}
}
