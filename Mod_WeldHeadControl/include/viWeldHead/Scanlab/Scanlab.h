/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Alexander Egger (EA)
 * 	@date		2020
 * 	@brief		Communicates via Scanlab library with the Scanlab Scanner
 */

#ifndef SCANLAB_H_
#define SCANLAB_H_

#include <atomic>

#include "event/deviceNotification.proxy.h"
#include "event/results.h"
#include "event/results.interface.h"

#include "viWeldHead/Scanlab/ScannerControl.h"
#include "viWeldHead/Scanlab/RTC6FigureWelding.h"
#include "viWeldHead/Scanlab/InfiniteWobbleWelding.h"
#include "viWeldHead/Scanlab/ScannerCalibrationWelding.h"
#include "viWeldHead/Scanlab/zCompensation.h"

#include "common/definesScanlab.h"
#include "common/triggerInterval.h"
#include <geo/point.h>
#include <condition_variable>
#include <thread>
#include <mutex>
#include <optional>

#include "common/systemConfiguration.h"

class LoadWobbleFigureTest;
class PositionDifferenceToleranceTest;
class FillVelocityTest;
class CheckListOverflowTest;

using precitec::interface::LensType;
using precitec::interface::CorrectionFileMode;

namespace precitec
{

namespace hardware
{

class AbstractSpotWelding;

enum LaserPowerParameter { eLaserPowerPara1 = 0,  eLaserPowerPara2,  eLaserPowerPara3,  eLaserPowerPara4,  eLaserPowerPara5,  eLaserPowerPara6,  eLaserPowerPara7,  eLaserPowerPara8,
                           eLaserPowerPara9,  eLaserPowerPara10, eLaserPowerPara11, eLaserPowerPara12, eLaserPowerPara13, eLaserPowerPara14, eLaserPowerPara15, eLaserPowerPara16,
                           eLaserPowerPara17, eLaserPowerPara18, eLaserPowerPara19, eLaserPowerPara20, eLaserPowerPara21, eLaserPowerPara22, eLaserPowerPara23, eLaserPowerPara24,
                           eLaserPowerPara25, eLaserPowerPara26, eLaserPowerPara27, eLaserPowerPara28, eLaserPowerPara29, eLaserPowerPara30, eLaserPowerPara31, eLaserPowerPara32 };

class Scanlab
{

public:

    /**
        * @brief CTor.
        * @param
        */
    Scanlab(void);

    /**
    * @brief DTor.
    */
    virtual ~Scanlab();

    bool isScanlabScannerEnabled(void) { return m_oScanlabScannerEnable; }

    interface::ScannerGeneralMode getScannerGeneralMode(void)
    {
        return m_scannerGeneralMode;
    }
    interface::ScannerModel scannerModel(void)
    {
        return m_scannerModel;
    }
    interface::ScannerModel controller() const
    {
        return m_controller;
    }

    bool isSCANMASTER_GeneralApplication(void) { return m_oIsSCANMASTER_GeneralApplication; }
    bool isOCT_with_reference_arms(void) const { return m_oIsOCT_with_reference_arms; }

    void burst(interface::TriggerContext const& context, interface::TriggerInterval const& interval);
    void cancel(void);

    ScannerControlStatus InitScanlabObjects();
    void ScannerDriveToZero(void);
    void ScannerDriveToPosition(void);
    void ScannerDriveWithOCTReference(void);
    bool reloadFiberSwitchCalibration(void);
    void ScannerSetOCTReference(void);
    void ScannerOperatingState(void);
    void ScannerStartWeldingPreview(void);
    void ScannerStopWeldingPreview(void);
    void ScannerTestFunction1(void);
    void ScannerTestFunction2(void);

    void ScannerResetLastError(void);

    void ScanmasterWeldingData(interface::ResultDoubleArray const& p_rWeldingData, bool sendEndOfSeam); // Interface: viWeldHeadSubscribe (event)
    void ScanmasterPrepareList(interface::ResultDoubleArray const& p_rWeldingData);
    void ScanmasterWeldPreparedList(const interface::ResultDoubleArray& contextData, bool sendEndOfSeam);
    void ScanmasterWeldingDataWorker(void);

    void ScanmasterScannerMoving(interface::ResultDoubleArray const& p_rWeldingData); // Interface: viWeldHeadSubscribe (event)
    void ScanmasterScannerSpotWelding(interface::ResultDoubleArray const& weldingData);
    bool weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserDurationInMs, double laserPowerInPct, double jumpSpeedInMmPerSec);

    void ScannerHeightCompensation(double compensationHeight);

    void generateScantracker2DList(void);

    void SetScannerNewXPosition(double oValue) { m_oNewScannerXPosition = oValue; }
    void SetScannerNewYPosition(double oValue) { m_oNewScannerYPosition = oValue; }
    double GetScannerNewXPosition(void) { return m_oNewScannerXPosition; }
    double GetScannerNewYPosition(void) { return m_oNewScannerYPosition; }

    void SetScannerActXPosition(double oValue) { m_oActualScannerXPosition.store(oValue); }
    void SetScannerActYPosition(double oValue) { m_oActualScannerYPosition.store(oValue); }
    double GetScannerActXPosition(void) { return m_oActualScannerXPosition.load(); }
    double GetScannerActYPosition(void) { return m_oActualScannerYPosition.load(); }

    void SetOCTReferenceArm(int oValue);
    int GetOCTReferenceArm(void);
    int GetOCTReferenceArmActual(void);

    void SetScannerJumpSpeed(double oValue);
    double GetScannerJumpSpeed(void) { return m_oScannerJumpSpeed; };
    void SetScannerMarkSpeed(double oValue);
    double GetScannerMarkSpeed(void) { return m_oScannerMarkSpeed; };
    void SetScannerWobbleFrequency(int oValue) { m_oScannerWobbelFreq = oValue; };
    int GetScannerWobbleFrequency(void) { return m_oScannerWobbelFreq; };
    void SetScannerWobbleXSize(double oValue) { m_figureWelding->SetScannerWobbleXSize(oValue); };
    double GetScannerWobbleXSize(void) { return m_figureWelding->GetScannerWobbleXSize(); };
    void SetScannerWobbleYSize(double oValue) { m_figureWelding->SetScannerWobbleYSize(oValue); };
    double GetScannerWobbleYSize(void) { return m_figureWelding->GetScannerWobbleYSize(); };
    void SetScannerWobbleRadius(double oValue) { m_figureWelding->SetScannerWobbleRadius(oValue); };
    double GetScannerWobbleRadius(void) { return m_figureWelding->GetScannerWobbleRadius(); };
    void SetWobbleMode(int oValue) { m_figureWelding->SetWobbleMode(static_cast<precitec::WobbleMode> (oValue)); };
    int GetWobbleMode(void) { return static_cast<int> (m_figureWelding->GetWobbleMode()); };
    std::string GetFigureFilePath() { return m_figureFilePath; };
    std::string GetFigureFileName() { return m_figureFileName; };
    std::string GetFigureFileEnding() { return m_figureFileEnding; };

    void SetScannerLaserOnDelay(int oValue) { m_oLaserOnDelay = oValue; };
    int GetScannerLaserOnDelay(void) { return m_oLaserOnDelay; };
    void SetScannerLaserOffDelay(int oValue) { m_oLaserOffDelay = oValue; };
    int GetScannerLaserOffDelay(void) { return m_oLaserOffDelay; };
    void SetLaserPowerStatic(int oValue) { m_oLaserPowerStatic = oValue; };
    int GetLaserPowerStatic(void) { return m_oLaserPowerStatic; };
    void SetLaserPowerStaticRing(int oValue) { m_oLaserPowerStaticRing = oValue; };
    int GetLaserPowerStaticRing(void) { return m_oLaserPowerStaticRing; };

    void SetLaserPowerParameter(LaserPowerParameter oParaNo, int oValue);
    int GetLaserPowerParameter(LaserPowerParameter oParaNo);

    void SetWeldingFigureNumber(int oValue) {m_oWeldingFigureNumber = oValue;};
    int GetWeldingFigureNumber(void) {return m_oWeldingFigureNumber;};
    void SetWobbelFigureNumber(int oValue) {m_oWobbelFigureNumber = oValue;};
    int GetWobbelFigureNumber(void) {return m_oWobbelFigureNumber;};
    void SetLaserPowerIsDigital(bool oValue) {m_figureWelding->setHasDigitalLaserPower(oValue);};
    bool GetLaserPowerIsDigital(void) {return m_figureWelding->hasDigitalLaserPower(); };
    void SetLaserDelay(double oValue) {m_figureWelding->SetLaserDelay(oValue);};
    double GetLaserDelay(void) {return m_figureWelding->GetLaserDelay();};

    void setScanTracker2DAngle(double value) {m_oWobbelAngle = value;}
    double getScanTracker2DAngle(void) {return m_oWobbelAngle;}
    void setScanTracker2DLaserDelay(int value) {m_scanTracker2DLaserDelay = value;}
    int getScanTracker2DLaserDelay(void) {return m_scanTracker2DLaserDelay;}
    void setScanTracker2DCustomFigure(bool value) {m_scanTracker2DCustomFigure = value;}
    bool getScanTracker2DCustomFigure(void) {return m_scanTracker2DCustomFigure;}

    void setScanTracker2DScanWidthFixedX(double value);
    double getScanTracker2DScanWidthFixedX(void);
    void setScanTracker2DScanWidthFixedY(double value);
    double getScanTracker2DScanWidthFixedY(void);
    void setScanTracker2DScanPosFixedX(double value);
    double getScanTracker2DScanPosFixedX(void);
    void setScanTracker2DScanPosFixedY(double value);
    double getScanTracker2DScanPosFixedY(void);

    void SetTrackerScanWidthControlled(int value); // Interface: viWeldHeadSubscribe (event)
    void SetTrackerScanPosControlled(int value); // Interface: viWeldHeadSubscribe (event)
    void SetScanWidthOutOfGapWidth(bool onOff);
    void SetScanPosOutOfGapPos(bool onOff);

    void SetZCDrivingV2IsActive(bool p_oState);
    void startAutomaticmode(void); // gives start of inspection cycle to Scanlab class
    void SeamStart(int seamNumber);
    void SeamEnd(void);

    void StartWelding_ScanMaster(void); // thread function
    int GetScannerWeldingFinished(void) { return m_oScannerWeldingFinished.load(); };
    void ScannerMoving(void);
    int GetContourPreparedFinished(void) { return m_oContourPreparedFinished.load(); };

    void setResultsProxy(const std::shared_ptr<interface::TResults<interface::AbstractInterface>> &proxy)
    {
        m_resultsProxy = proxy;
    }

    void setDeviceNotificationProxy(const std::shared_ptr<interface::TDeviceNotification<interface::EventProxy>> &proxy)
    {
        m_deviceNotificationProxy = proxy;
    }

    int getPositionDifferenceToleranceInBits()
    {
        return m_oPosDiffTolerance;
    }
    void setPositionDifferenceTolerance(int toleranceInBits);
    double getPositionDifferenceTolerance()
    {
        return m_oPosDiffToleranceMillimeter;
    }
    void setPositionDifferenceTolerance(double toleranceInMM);

    double minPositionDifferenceToleranceInMM();
    double maxPositionDifferenceToleranceInMM();
    double defaultPositionDifferenceToleranceInMM();
    double positionDifferenceToleranceFromBitToMM(int positionDifferenceToleranceInBits);
    int positionDifferenceToleranceFromMMToBits(double positionDifferenceToleranceInMM);

    int laserPowerDelayCompensation();
    void setLaserPowerDelayCompensation(int delayCompensation);

    bool scannerCompensateHeight() const
    {
        return m_zCompensation.compensationActive();
    }
    void setScannerCompensateHeight(bool isHeightCompensated);
    double compensationHeight() const
    {
        return m_zCompensation.zOffset();
    }
    void setCompensationHeight(double heightForCompensation);
    bool isCompensateHeightFixed() const
    {
        return m_zCompensation.isCompensationFixed();
    }
    void setIsCompensateHeightFixed(bool isCompensateHeightFixed);

    int correctionFileMode()
    {
        return static_cast<int> (m_correctionFileMode);
    }
    void setCorrectionFileMode(int newMode);

    //Debug mode
    void SaveLoggedScannerData(void);
    bool GetEnableDebugMode(void) {return m_enableDebugMode;};
    void SetEnableDebugMode(bool oValue);
    int GetMeasurementPeriod(void) {return m_measurementPeriod;};
    void SetMeasurementPeriod(int oValue);
    int getLoggedSignal(std::size_t index) const
    {
        return static_cast<int>(m_loggedSignals.at(index));
    }
    void setLoggedSignal(std::size_t index, int value);
    int GetStatusSignalHead1Axis1(void) {return static_cast<int>(m_statusSignals.at(0));};
    int GetStatusSignalHead1Axis2(void) {return static_cast<int>(m_statusSignals.at(1));};
    void setStatusSignalHead1Axis(std::size_t index, int value);
    RTC6::FigureWelding& rtc6FigureWelding() { return *static_cast<RTC6::FigureWelding*>(m_figureWelding.get()); }; //Just for unit tests

private:
    void checkStopEvent(Axis axis);
    void checkStopEvents();
    void checkStatusWord(Axis axis);
    void checkStatusWords();
    void checkTemperature(Axis axis);
    void checkTemperatures();
    void checkServoCardTemperature(Axis axis);
    void checkServoCardTemperatures();
    void ScannerCheckLastError(void);
    bool isStartWeldingReady() const;
    void notifyStartWelding();

    unsigned int numberOfPointsFromContour(std::size_t contourSize);
    unsigned int numberOfPossiblePointsForListMemory();

    //Bit to mm
    double transformToleranceInBitToPercent(int toleranceInBits);
    int transformToleranceInPercentToScanlabBit(double toleranceInPercentage);
    double transformToleranceInScanlabBitToMillimeter(int toleranceInScanlabBits);
    //Mm to bit
    int transformToleranceInPercentToBit(double toleranceInPercentage);
    double transformToleranceInScanlabBitToPercent(int toleranceInScanlabBits);
    int transformToleranceInMillimeterToScanlabBit(double toleranceInMillimeter);

    double transformLaserPowerToBits(double laserPower) const;

    void sendToleranceInBitsToScanner(int toleranceInBits);

    double transformFromMMPerSToBitPerMS(double velocityInMillimeterPerSecond);
    void fillVelocities(double productMarkSpeed, double productJumpSpeed);
    void fillMarkVelocity(double velocity, double productMarkSpeed);
    void fillJumpVelocity(double velocity, double productJumpSpeed);

    bool timeoutInSpotWelding();
    bool timeoutInSeamWobbel();

    void SetScanTracker2DScanWidth(void);
    void SetScanTracker2DScanPos(void);

    std::vector<double> m_velocities;
    double m_lastValidMarkSpeedInBitsPerMs{0.0};
    double m_lastValidJumpSpeedInBitsPerMs{0.0};

    struct WorkerThread {
        std::atomic<bool> run{false};
        std::mutex mutex;
        std::condition_variable condition;
        std::thread thread;
    };
    void fillScanlabListThread();
    void startWeldingThread();
    void scannerMovingThread();

    void threadFunction(WorkerThread &worker, std::function<void(void)> runMethod);

    void DetermineReferenceArmBits(uint16_t& p_oBinaryValue, uint16_t& p_oBinaryMask);

    int fiberSwitchSelect(const std::pair<std::vector<double>, std::vector<double>>& model, double idmHalfRange, double sx, double sy);
    static double stringToDouble(const std::string& input);
    std::pair<std::vector<double>, std::vector<double>> readIdmCalibrationValuesXml(std::string fileName = "");
    void setKeyValueToDeviceNotification(const std::string& keyValue, double value) const;
    void setActualScannerPositionToDeviceNotification() const;

    bool m_oScanlabScannerEnable;
    std::string m_oScanlabScannerIpAddress;
    interface::ScannerModel m_scannerModel;
    interface::ScannerModel m_controller;
    interface::ScannerGeneralMode m_scannerGeneralMode;
    int m_oScanlabLensType;
    std::string m_scanlabCorrectionFile;
    bool m_oIsSCANMASTER_Application;
    bool m_oIsSCANMASTER_ThreeStepInterface;
    bool m_oIsSCANMASTER_GeneralApplication;
    bool m_oIsOCT_with_reference_arms;

    double m_oCalibValueBitsPerMM;

    double m_oNewScannerXPosition;
    double m_oNewScannerYPosition;
    std::atomic<double> m_oActualScannerXPosition;
    std::atomic<double> m_oActualScannerYPosition;
    std::atomic<int> m_oOCTReferenceArmTarget{1};
    std::atomic<int> m_oOCTReferenceArmActual{1};
    std::pair<std::vector<double>, std::vector<double>> m_oReferenceArmCalibrationData{};

    double m_oScannerJumpSpeed; // m/s , mm/ms
    double m_oScannerMarkSpeed; // m/s , mm/ms

    int m_oScannerWobbelFreq; // Hz
    unsigned long m_pLaserPowerValues[30];
    unsigned long m_oLaserPowerValuesShadow[32];
    double m_oWobbelXOffset; // mm
    double m_oWobbelYOffset; // mm
    double m_oWobbelAngle; // degree
    std::string m_figureFilePath;
    std::string m_figureFileName;
    std::string m_figureFileEnding;

    int m_oLaserOnDelay; // us
    int m_oLaserOffDelay; // us
    int m_oLaserPowerStatic; // %       center
    int m_oLaserPowerStaticRing;
    int m_scanTracker2DLaserDelay{0};
    bool m_scanTracker2DCustomFigure{false};

    int m_oWeldingFigureNumber;
    int m_oWobbelFigureNumber;
    double m_oADCValue;
    int m_oPosDiffTolerance;
    double m_oPosDiffToleranceMillimeter;
    int m_laserPowerDelayCompensation;

    bool m_scanWidthOutOfGapWidth{false};
    bool m_scanPosOutOfGapPos{true};
    double m_scantracker2DScanWidthFixedX{1.0};
    double m_scantracker2DScanWidthFixedY{1.0};
    double m_scantracker2DScanPosFixedX{0.0}; // [mm]
    double m_scantracker2DScanPosFixedY{0.0}; // [mm]
    double m_scantracker2DScanWidthControlledX{1.0};
    double m_scantracker2DScanWidthControlledY{1.0};
    double m_scantracker2DScanPosControlledX{0.0}; // [mm]
    double m_scantracker2DScanPosControlledY{0.0}; // [mm]
    CorrectionFileMode m_correctionFileMode {CorrectionFileMode::Welding};

    //Debug mode
    bool m_enableDebugMode = false;
    int m_measurementPeriod = 1;                                                //[10us]
    std::array<LoggerSignals, 4> m_loggedSignals;
    std::array<StatusSignals, 2> m_statusSignals{StatusSignals::NoSignal, StatusSignals::NoSignal};

    std::atomic<bool> m_oZCDrivingHasFinished;
    std::atomic<bool> m_oScanlabMovingHasFinished;
    std::atomic<bool> m_oWeldSeamRequested;
    std::atomic<bool> m_oPrepareSeamRequested;
    std::atomic<bool> m_oWeldSeamGenerationIsReady;
    std::atomic<bool> m_oStartWeldingThreadIsActive;
    std::atomic<bool> m_usePreparedContour;
    interface::ResultDoubleArray m_oWeldingData;
    std::atomic<int> m_oScannerWeldingFinished;
    std::atomic<int> m_oContourPreparedFinished;
    std::atomic<int> m_contourAlreadyPreparedInAnotherSeam;
    std::atomic<bool> m_zCompensationSet;
    int m_currentSeam{0};

    welding::ZCompensation m_zCompensation;
    std::unique_ptr<ScannerControl> m_scannerControl;
    std::unique_ptr<AbstractFigureWelding> m_figureWelding;
    InfiniteWobbleWelding m_scantracker2DSeam;
    std::unique_ptr<precitec::hardware::AbstractSpotWelding> m_spotWelding;


    std::mutex m_startWeldingMutex;
    std::condition_variable m_startWeldingCondition;

    std::shared_ptr<interface::TResults<interface::AbstractInterface>> m_resultsProxy;
    std::shared_ptr<interface::TDeviceNotification<interface::EventProxy>> m_deviceNotificationProxy;
    bool m_sendEndOfSeam = false;

    WorkerThread m_fillScanlab;
    WorkerThread m_startWelding;
    WorkerThread m_scannerMoving;

    std::atomic<bool> m_tearDown{false};

    friend LoadWobbleFigureTest;
    friend PositionDifferenceToleranceTest;
    friend FillVelocityTest;
    friend CheckListOverflowTest;
}; // class Scanlab

} // namespace hardware

} // namespace precitec

#endif /* SCANLAB_H_ */

