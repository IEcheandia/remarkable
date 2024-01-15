/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Alexander Egger (EA)
 * 	@date		2020
 * 	@brief		Communicates via Scanlab library with the Scanlab Scanner
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <iomanip>
#include <ios>
#include <string>
#include <sys/prctl.h>
#include <fstream>
#include <regex>

#include "common/definesScanlab.h"

#include "module/moduleLogger.h"
#include "viWeldHead/Scanlab/RTC6Control.h"
#include "viWeldHead/Scanlab/Scanlab.h"
#include "system/realTimeSupport.h"
#include "viWeldHead/Scanlab/calculator.h"
#include "viWeldHead/Scanlab/scannerLogger.h"
#include "viWeldHead/Scanlab/smartMoveControl.h"
#include "viWeldHead/Scanlab/smartMoveFigureWelding.h"
#include "viWeldHead/Scanlab/smartMoveSpotWelding.h"
#include "viWeldHead/Scanlab/RTC6SpotWelding.h"


namespace
{
bool fuzzyIsNull(double value)
{
    return std::abs(value) <= 0.000000000001;
}
}

namespace precitec
{

namespace hardware
{

using namespace interface;
using precitec::hardware::welding::Calculator;
using precitec::hardware::logger::ScannerLogger;

static const double POSITION_DIFFERENCE_TOLERANCE_PERCENT{0.000015};
static const int MAX_SCANFIELD_BITS{1048576};

//***************************************************************************
//* class Scanlab                                                           *
//***************************************************************************

namespace
{
std::unique_ptr<ScannerControl> createControl(ScannerModel model, LensType type)
{
    switch (model)
    {
    case ScannerModel::ScanlabScanner:
        return std::make_unique<RTC6::Control>(type);
    case ScannerModel::SmartMoveScanner:
        return std::make_unique<SmartMoveControl>();
    default:
        __builtin_unreachable();
    }
}

std::unique_ptr<AbstractFigureWelding> createFigureWelding(ScannerModel model, Scanner2DWeldingData& control, welding::ZCompensation& zCompensation)
{
    switch (model)
    {
    case ScannerModel::ScanlabScanner:
        return std::make_unique<RTC6::FigureWelding>(control, zCompensation);
    case ScannerModel::SmartMoveScanner:
        return std::make_unique<smartMove::FigureWelding>(control, zCompensation);
    default:
        __builtin_unreachable();
    }
}

std::unique_ptr<AbstractSpotWelding> createSpotWelding(ScannerModel model, ScannerControl& control)
{
    switch (model)
    {
    case ScannerModel::ScanlabScanner:
        return std::make_unique<RTC6::SpotWelding>(dynamic_cast<RTC6::Control&>(control));
    case ScannerModel::SmartMoveScanner:
        return std::make_unique<smartMove::SpotWelding>(dynamic_cast<SmartMoveControl&>(control));
    default:
        __builtin_unreachable();
    }
}

}

Scanlab::Scanlab(void):
    m_oScanlabScannerIpAddress("192.168.170.105"),
    m_controller(static_cast<ScannerModel>(SystemConfiguration::instance().get(SystemConfiguration::IntKey::Scanner2DController))),
    m_oScanlabLensType(1),
    m_oCalibValueBitsPerMM(4000.0), //Calibration factor of the correction file
    m_oNewScannerXPosition(0.0),
    m_oNewScannerYPosition(0.0),
    m_oActualScannerXPosition(0.0),
    m_oActualScannerYPosition(0.0),
    m_oScannerJumpSpeed(0.5), // m/s , mm/ms
    m_oScannerMarkSpeed(0.1), // m/s , mm/ms
    m_oScannerWobbelFreq(0), // Hz
    m_oLaserPowerValuesShadow{0},
    m_oWobbelXOffset(0.0), // mm
    m_oWobbelYOffset(0.0), // mm
    m_oWobbelAngle(0.0), // degree
    m_figureFilePath(std::string(getenv("WM_BASE_DIR")) + "/config/laser_controls"),
    m_figureFileName("/figureWobble"),
    m_figureFileEnding(".json"),
    m_oLaserOnDelay(0), // us
    m_oLaserOffDelay(0), // us
    m_oLaserPowerStatic(0), // %
    m_oLaserPowerStaticRing(0), //%
    m_oWeldingFigureNumber(-1),
    m_oWobbelFigureNumber(-1),
    m_oADCValue(4095),
    m_oPosDiffTolerance(static_cast<int> (PositionDifferenceTolerance::DefaultTolerance)),
    m_oPosDiffToleranceMillimeter(0.5),
    m_laserPowerDelayCompensation(0),
    m_oZCDrivingHasFinished(true),
    m_oScanlabMovingHasFinished(true),
    m_oWeldSeamRequested(false),
    m_oPrepareSeamRequested(false),
    m_oWeldSeamGenerationIsReady(false),
    m_oStartWeldingThreadIsActive(false),
    m_usePreparedContour(false),
    m_oScannerWeldingFinished(0),
    m_oContourPreparedFinished(0),
    m_contourAlreadyPreparedInAnotherSeam(0),
    m_zCompensationSet(false),
    m_zCompensation(),
    m_scannerControl{createControl(m_controller, static_cast<LensType>(m_oScanlabLensType))},
    m_figureWelding(createFigureWelding(m_controller, *m_scannerControl, m_zCompensation)),
    m_scantracker2DSeam(*m_scannerControl)
{
    m_loggedSignals.fill(LoggerSignals::NoSignal);
    m_fillScanlab.thread = std::thread{&Scanlab::fillScanlabListThread, this};
    m_startWelding.thread = std::thread{&Scanlab::startWeldingThread, this};
    m_scannerMoving.thread = std::thread{&Scanlab::scannerMovingThread , this};

    for (unsigned int i = 0; i < (sizeof(m_pLaserPowerValues) / sizeof(m_pLaserPowerValues[0])); i++)
    {
        m_pLaserPowerValues[i] = 0;
    }
    for (unsigned int i = 0; i < (sizeof(m_oLaserPowerValuesShadow) / sizeof(m_oLaserPowerValuesShadow[0])); i++)
    {
        m_oLaserPowerValuesShadow[i] = 0;
    }

    // SystemConfig Switches for Scanlab Scanner
    m_oScanlabScannerEnable = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::Scanner2DEnable);
    wmLog(eDebug, "m_oScanlabScannerEnable (bool): %d\n", m_oScanlabScannerEnable);
    m_oScanlabScannerIpAddress = SystemConfiguration::instance().get(SystemConfiguration::StringKey::Scanner2DController_IP_Address);
    wmLog(eDebug, "m_oScanlabScannerIpAddress: <%s>\n", m_oScanlabScannerIpAddress.c_str());
    m_scannerModel = static_cast<ScannerModel>(SystemConfiguration::instance().get(SystemConfiguration::IntKey::ScannerModel));
    wmLog(eDebug, "Scanner model is: %d\n", static_cast<int>(m_scannerModel));
    m_scannerGeneralMode = static_cast<ScannerGeneralMode>(SystemConfiguration::instance().get(SystemConfiguration::IntKey::ScannerGeneralMode));
    wmLog(eDebug, "m_scannerGeneralMode (int): %d\n", static_cast<int>(m_scannerGeneralMode));
    m_oScanlabLensType = SystemConfiguration::instance().get(SystemConfiguration::IntKey::ScanlabScanner_Lens_Type);
    m_scanlabCorrectionFile = SystemConfiguration::instance().get(SystemConfiguration::StringKey::ScanlabScanner_Correction_File);
    wmLog(eDebug, "m_oScanlabLensType (int): %d\n", m_oScanlabLensType);
    wmLog(eDebug, "m_scanlabCorrectionFile (string): %s\n", m_scanlabCorrectionFile);
    wmLogTr(eInfo, "QnxMsg.VI.InfoContourPointLimit", "Current contour point limit is %d.\n", numberOfPossiblePointsForListMemory());

    // SystemConfig Switches for SCANMASTER application and functions
    m_oIsSCANMASTER_Application = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::SCANMASTER_Application);
    wmLog(eDebug, "m_oIsSCANMASTER_Application (bool): %d\n", m_oIsSCANMASTER_Application);
    m_oIsSCANMASTER_ThreeStepInterface = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::SCANMASTER_ThreeStepInterface);
    wmLog(eDebug, "m_oIsSCANMASTER_ThreeStepInterface (bool): %d\n", m_oIsSCANMASTER_ThreeStepInterface);
    m_oIsSCANMASTER_GeneralApplication = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::SCANMASTER_GeneralApplication);
    wmLog(eDebug, "m_oIsSCANMASTER_GeneralApplication (bool): %d\n", m_oIsSCANMASTER_GeneralApplication);
    m_oIsOCT_with_reference_arms = SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::OCT_with_reference_arms);
    wmLog(eDebug, "m_oIsOCT_with_reference_arms (bool): %d\n", m_oIsOCT_with_reference_arms);
    wmLog(eDebug, "WobbleFigureFile (std::string): %s\n", GetFigureFilePath());
    wmLog(eDebug, "WobbleFigureFile (std::string): %s\n", GetFigureFileName());
    wmLog(eDebug, "WobbleFigureFile (std::string): %s\n", std::to_string(m_oWobbelFigureNumber));
    wmLog(eDebug, "WobbleFigureFile (std::string): %s\n", GetFigureFileEnding());
    wmLog(eDebug, "WeldingFigureFile (std::string): %s\n", std::to_string(m_oWeldingFigureNumber));

    m_figureWelding->set_Speeds(2000.0, 400.0, 100.0);
    m_figureWelding->set_MarkOffset(0, 0, 0.0);
    m_figureWelding->set_MarkAngle(0.0);
    m_scantracker2DSeam.set_Speeds(2000.0, 400.0, 100.0);
    m_scantracker2DSeam.set_MarkOffset(0, 0, 0.0);
    m_scantracker2DSeam.set_MarkAngle(0.0);
    m_scantracker2DSeam.set_MarkSize(1, 1, 0);
    m_scantracker2DSeam.set_Scale(1.0, 1.0);

    reloadFiberSwitchCalibration();

    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        if (m_scannerGeneralMode == ScannerGeneralMode::eScantracker2DMode)
        {
            wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "ScanTracker2D not available for MarkingEngine.\n");
        }
        if (m_oIsOCT_with_reference_arms)
        {
            wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "OCT with reference arms not available for MarkingEngine.\n");
        }
        if (SystemConfiguration::instance().get(SystemConfiguration::BooleanKey::LaserControlTwoChannel))
        {
            wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "LaserControlTwoChannel not available for MarkingEngine.\n");
        }
    }
}

Scanlab::~Scanlab()
{
    if (isScanlabScannerEnabled())
    {
        m_scannerControl->jumpScannerOffset();
    }
    {
        std::lock_guard lock{m_fillScanlab.mutex};
        std::lock_guard lock2{m_startWelding.mutex};
        m_tearDown.store(true);
    }
    m_fillScanlab.condition.notify_one();
    m_startWelding.condition.notify_one();
    m_scannerMoving.condition.notify_one();
    m_startWelding.thread.join();
    m_fillScanlab.thread.join();
    m_scannerMoving.thread.join();
}

ScannerControlStatus Scanlab::InitScanlabObjects()
{
    if (!isScanlabScannerEnabled())
    {
        return ScannerControlStatus::Disabled;
    }

    auto lensType = static_cast<LensType>(m_oScanlabLensType);
    if (const auto status = m_scannerControl->init(m_oScanlabScannerIpAddress, m_scanlabCorrectionFile, lensType, m_scannerModel);
        status != ScannerControlStatus::Initialized)
    {
        return status;
    }

    m_oCalibValueBitsPerMM = m_scannerControl->calibValueBitsPerMM();
    wmLog(eDebug, "m_oCalibValueBitsPerMM: %f\n", m_oCalibValueBitsPerMM);

    m_oPosDiffToleranceMillimeter = POSITION_DIFFERENCE_TOLERANCE_PERCENT * static_cast<int> (PositionDifferenceTolerance::DefaultTolerance) * MAX_SCANFIELD_BITS / m_oCalibValueBitsPerMM;
    m_zCompensation.setLensType(lensType);

    return ScannerControlStatus::Initialized;
}

void Scanlab::burst(interface::TriggerContext const& context, interface::TriggerInterval const& interval)
{
    if (getScannerGeneralMode() == ScannerGeneralMode::eScantracker2DMode && m_controller == ScannerModel::ScanlabScanner)
    {
        m_scantracker2DSeam.start_Mark();
    }
}

void Scanlab::cancel(void)
{
    if (getScannerGeneralMode() == ScannerGeneralMode::eScantracker2DMode && m_controller == ScannerModel::ScanlabScanner)
    {
        m_scantracker2DSeam.stop_Mark();
        ScannerDriveToZero();
    }
}

void Scanlab::SetEnableDebugMode(bool oValue)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Debug Mode not implemented for MarkingEngine!\n");
        return;
    }
    m_enableDebugMode = oValue;
    m_figureWelding->set_EnableDebugMode(oValue);
    m_scantracker2DSeam.set_EnableDebugMode(oValue);
}

void Scanlab::SetMeasurementPeriod(int oValue)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Measurement Period not implemented for MarkingEngine!\n");
        return;
    }
    m_measurementPeriod = oValue;
    m_figureWelding->set_MeasurementPeriod(m_measurementPeriod);
    m_scantracker2DSeam.set_MeasurementPeriod(m_measurementPeriod);
}

void Scanlab::setLoggedSignal(std::size_t index, int value)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Logged signal not implemented for MarkingEngine!\n");
        return;
    }
    if (value > static_cast<int>(LoggerSignals::ScaledEncoderZ))
    {
        return;
    }
    const auto signal{static_cast<LoggerSignals>(value)};
    m_loggedSignals.at(index) = signal;
    m_figureWelding->setLoggedSignal(index, signal);
    m_scantracker2DSeam.setLoggedSignal(index, signal);
}

void Scanlab::setStatusSignalHead1Axis(std::size_t index, int value)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "StatusSignalHead1 not implemented for MarkingEngine!\n");
        return;
    }
    if (value > static_cast<int>(StatusSignals::TemperatureServocard))
    {
        return;
    }
    m_statusSignals.at(index) = static_cast<StatusSignals>(value);
    m_figureWelding->setStatusSignalHead1Axis(index == 0u ? AbstractScanner2DWelding::Axis::First : AbstractScanner2DWelding::Axis::Second, m_statusSignals.at(index));
    m_scantracker2DSeam.setStatusSignalHead1Axis(index == 0u ? AbstractScanner2DWelding::Axis::First : AbstractScanner2DWelding::Axis::Second, m_statusSignals.at(index));
}

void Scanlab::SaveLoggedScannerData()
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Save logged scanner data not implemented for MarkingEngine!\n");
        return;
    }
    m_figureWelding->saveLoggedScannerData();
}

unsigned int Scanlab::numberOfPointsFromContour(std::size_t contourSize)
{
    return m_scannerControl->numberOfPointsFromContour(contourSize);
}

unsigned int Scanlab::numberOfPossiblePointsForListMemory()
{
    return m_scannerControl->numberOfPossiblePointsForListMemory();
}

void Scanlab::setPositionDifferenceTolerance(int toleranceInBits)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Set position difference tolerance not implemented for MarkingEngine!\n");
        return;
    }
    m_oPosDiffTolerance = toleranceInBits;
    m_oPosDiffToleranceMillimeter = positionDifferenceToleranceFromBitToMM(toleranceInBits);

    sendToleranceInBitsToScanner(m_oPosDiffTolerance);
}

void Scanlab::setPositionDifferenceTolerance(double toleranceInMM)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Set position difference tolerance not implemented for MarkingEngine!\n");
        return;
    }
    m_oPosDiffToleranceMillimeter = toleranceInMM;
    m_oPosDiffTolerance = positionDifferenceToleranceFromMMToBits(toleranceInMM);

    sendToleranceInBitsToScanner(m_oPosDiffTolerance);
}

double Scanlab::minPositionDifferenceToleranceInMM()
{
    return POSITION_DIFFERENCE_TOLERANCE_PERCENT * static_cast<int> (PositionDifferenceTolerance::MinTolerance) * MAX_SCANFIELD_BITS / m_oCalibValueBitsPerMM;
}

double Scanlab::maxPositionDifferenceToleranceInMM()
{
    return POSITION_DIFFERENCE_TOLERANCE_PERCENT * static_cast<int> (PositionDifferenceTolerance::MaxTolerance) * MAX_SCANFIELD_BITS / m_oCalibValueBitsPerMM;
}

double Scanlab::defaultPositionDifferenceToleranceInMM()
{
    return POSITION_DIFFERENCE_TOLERANCE_PERCENT * static_cast<int> (PositionDifferenceTolerance::DefaultTolerance) * MAX_SCANFIELD_BITS / m_oCalibValueBitsPerMM;
}

double Scanlab::positionDifferenceToleranceFromBitToMM(int positionDifferenceToleranceInBits)
{
    return transformToleranceInScanlabBitToMillimeter(transformToleranceInPercentToScanlabBit(transformToleranceInBitToPercent(positionDifferenceToleranceInBits)));
}

int Scanlab::positionDifferenceToleranceFromMMToBits(double positionDifferenceToleranceInMM)
{
    return transformToleranceInPercentToBit(transformToleranceInScanlabBitToPercent(transformToleranceInMillimeterToScanlabBit(positionDifferenceToleranceInMM)));
}

int Scanlab::laserPowerDelayCompensation()
{
    return m_laserPowerDelayCompensation;
}

void Scanlab::setLaserPowerDelayCompensation(int delayCompensation)
{
    m_laserPowerDelayCompensation = delayCompensation;
}

void Scanlab::setScannerCompensateHeight(bool isHeightCompensated)
{
    m_zCompensation.setCompensationActive(isHeightCompensated);
}

void Scanlab::setCompensationHeight(double heightForCompensation)
{
    m_zCompensation.setZOffset(heightForCompensation);
}

void Scanlab::setIsCompensateHeightFixed(bool isCompensateHeightFixed)
{
    m_zCompensation.setIsCompensationFixed(isCompensateHeightFixed);
}

void Scanlab::setCorrectionFileMode(int newMode)
{
    if (!isScanlabScannerEnabled())
    {
        return;
    }
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmLog(eDebug, "Switching correction file not available for MarkingEngine!\n");
        return;
    }
    ScannerLogger logger;
    m_correctionFileMode = static_cast<CorrectionFileMode> (newMode);
    auto newCalibrationValue = m_scannerControl->selectCorrectionFileMode(m_correctionFileMode);
    if (fuzzyIsNull(newCalibrationValue))
    {
        logger.logCorrectionFileModeSelectionFailed(m_correctionFileMode);
        m_correctionFileMode = CorrectionFileMode::Welding;
        newCalibrationValue = m_scannerControl->selectCorrectionFileMode(m_correctionFileMode);
        logger.logCorrectionFileChangeToDefault();
    }
    m_oCalibValueBitsPerMM = newCalibrationValue;
    logger.logCorrectionFileMode(m_correctionFileMode);
}
//Bit to mm
double Scanlab::transformToleranceInBitToPercent(int toleranceInBits)
{
    return static_cast<double> (toleranceInBits * POSITION_DIFFERENCE_TOLERANCE_PERCENT);
}

int Scanlab::transformToleranceInPercentToScanlabBit(double toleranceInPercentage)
{
    return static_cast<int> (std::round(toleranceInPercentage * MAX_SCANFIELD_BITS));
}

double Scanlab::transformToleranceInScanlabBitToMillimeter(int toleranceInScanlabBits)
{
    return static_cast<double> (toleranceInScanlabBits / m_oCalibValueBitsPerMM);
}

int Scanlab::transformToleranceInPercentToBit(double toleranceInPercentage)
{
    return static_cast<int> (std::round(toleranceInPercentage / POSITION_DIFFERENCE_TOLERANCE_PERCENT));
}

double Scanlab::transformToleranceInScanlabBitToPercent(int toleranceInScanlabBits)
{
    return static_cast<double> (toleranceInScanlabBits) / MAX_SCANFIELD_BITS;
}

int Scanlab::transformToleranceInMillimeterToScanlabBit(double toleranceInMillimeter)
{
    return static_cast<int> (std::round(toleranceInMillimeter * m_oCalibValueBitsPerMM));
}

void Scanlab::sendToleranceInBitsToScanner(int toleranceInBits)
{
    m_scannerControl->sendToleranceInBitsToScanner(toleranceInBits);
}

double Scanlab::transformFromMMPerSToBitPerMS(double velocityInMillimeterPerSecond)
{
    return static_cast<double>(velocityInMillimeterPerSecond / 1000.0 * m_oCalibValueBitsPerMM);
}

void Scanlab::fillVelocities(double productMarkSpeed, double productJumpSpeed)
{
    m_velocities.clear();

    m_velocities.reserve(m_oWeldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT);

    for (std::size_t i = 0; i < m_oWeldingData.value().size() / SEAMWELDING_RESULT_FIELDS_PER_POINT; i++)
    {
        auto velocity = m_oWeldingData.value()[(i * SEAMWELDING_RESULT_FIELDS_PER_POINT) + 4];
        if (fuzzyIsNull(m_oWeldingData.value()[(i * SEAMWELDING_RESULT_FIELDS_PER_POINT) + 2]))               //Check if part is a jump (power = 0) or a mark (power != 0) because a jump can be done faster. (Different speeds)
        {
            fillJumpVelocity(velocity, productJumpSpeed);
        }
        else
        {
            fillMarkVelocity(velocity, productMarkSpeed);
        }
    }
}

void Scanlab::fillMarkVelocity(double velocity, double productMarkSpeed)
{
    switch(static_cast<int> (std::round(velocity)))
    {
        case SCANMASTERWELDINGDATA_UNDEFINEDVALUE:                                                  //Use last valid mark speed or if there is no last valid speed then use speed from product or scanmaster
        {
            m_velocities.push_back(m_lastValidMarkSpeedInBitsPerMs);
            break;
        }
        case SCANMASTERWELDINGDATA_USESTATICVALUE:                                                  //Use mark speed from product or scanmaster
        {
            m_velocities.push_back(productMarkSpeed);
            m_lastValidMarkSpeedInBitsPerMs = productMarkSpeed;
            break;
        }
        default:
        {
            // it is assumed that the velocity is transferred in the unit [mm/s].
            auto velocityInMPerS = transformFromMMPerSToBitPerMS(velocity);
            m_velocities.push_back(velocityInMPerS);
            m_lastValidMarkSpeedInBitsPerMs = velocityInMPerS;
        }
    }
}

void Scanlab::fillJumpVelocity(double velocity, double productJumpSpeed)
{
    switch(static_cast<int> (std::round(velocity)))
    {
        case SCANMASTERWELDINGDATA_UNDEFINEDVALUE:                                                  //Use last valid jump speed or if there is no last valid speed then use speed from product or scanmaster
        {
            m_velocities.push_back(m_lastValidJumpSpeedInBitsPerMs);
            break;
        }
        case SCANMASTERWELDINGDATA_USESTATICVALUE:                                                  //Use jump speed from product or scanmaster
        {
            m_velocities.push_back(productJumpSpeed);
            m_lastValidJumpSpeedInBitsPerMs = productJumpSpeed;
            break;
        }
        default:
        {
            // it is assumed that the velocity is transferred in the unit [mm/s].
            auto velocityInMPerS = transformFromMMPerSToBitPerMS(velocity);
            m_velocities.push_back(velocityInMPerS);
            m_lastValidJumpSpeedInBitsPerMs = velocityInMPerS;
        }
    }
}

void Scanlab::SetZCDrivingV2IsActive(bool p_oState) // info from ZCollimator function
{
    // p_oState is true after a driving request has started
    // p_oState is false after a driving request has ended and also if there is no driving request
    if (p_oState == true) // driving has started
    {
        m_oZCDrivingHasFinished.store(false);
    }
    else // driving has ended or there is no driving
    {
        std::unique_lock<std::mutex> lock{m_startWeldingMutex};
        m_oZCDrivingHasFinished.store(true);
        notifyStartWelding();
    }
}

void Scanlab::startAutomaticmode(void) // gives start of inspection cycle to Scanlab class
{
    ScannerStopWeldingPreview();
}

void Scanlab::SeamStart(int seamNumber)
{
    m_currentSeam = seamNumber;
    m_oScannerWeldingFinished.store(0);
    m_oContourPreparedFinished.store(0);
    m_oZCDrivingHasFinished.store(true);
    m_oScanlabMovingHasFinished.store(true);
    m_oWeldSeamGenerationIsReady.store(false);
    m_oWeldSeamRequested.store(false);
    m_oPrepareSeamRequested.store(false);
    m_usePreparedContour.store(false);
    m_zCompensationSet.store(false);
}

void Scanlab::SeamEnd(void)
{
}

void Scanlab::ScannerDriveToZero(void)
{
    SetScannerJumpSpeed(m_oScannerJumpSpeed);
    SetScannerMarkSpeed(m_oScannerMarkSpeed);

    if (isScanlabScannerEnabled())
    {
        m_scannerControl->scannerDriveToZero();
    }

    m_oActualScannerXPosition.store(0.0);
    m_oActualScannerYPosition.store(0.0);
    setActualScannerPositionToDeviceNotification();
}

void Scanlab::ScannerDriveToPosition(void)
{
    if (m_zCompensation.compensationActive() && (!m_zCompensationSet.load() && !m_zCompensation.isCompensationFixed()))
    {
        wmFatal(eDataConsistency, "QnxMsg.VI.NoZCompensationSet", "Height compensation value never set!\n");
        return;
    }

    SetScannerJumpSpeed(m_oScannerJumpSpeed);
    SetScannerMarkSpeed(m_oScannerMarkSpeed);

    if (isScanlabScannerEnabled())
    {
        m_scannerControl->scannerDriveToPosition(m_oNewScannerXPosition, m_oNewScannerYPosition);
    }

    m_oActualScannerXPosition.store(m_oNewScannerXPosition);
    m_oActualScannerYPosition.store(m_oNewScannerYPosition);
    setActualScannerPositionToDeviceNotification();
}

void Scanlab::SetOCTReferenceArm(int oValue)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Set OCT Reference Arm not implemented for MarkingEngine!\n");
        return;
    }
    if (isOCT_with_reference_arms())
    {
        m_oOCTReferenceArmTarget.store(oValue);
    }
    else
    {
        m_oOCTReferenceArmTarget.store(1);
    }
}

int Scanlab::GetOCTReferenceArm(void)
{
    return m_oOCTReferenceArmTarget.load();
}

int Scanlab::GetOCTReferenceArmActual(void)
{
    return m_oOCTReferenceArmActual.load();
}

void Scanlab::DetermineReferenceArmBits(uint16_t& p_oBinaryValue, uint16_t& p_oBinaryMask)
{
    // The following assignment of case x to port y is determined by the hardware.
    // The fiber length within the fiber switch determines the assignment.

    switch(m_oOCTReferenceArmTarget.load())
    {
        case 1:
            p_oBinaryValue = 0x00; // Port A
            break;
        case 2:
            p_oBinaryValue = 0x02; // Port D
            break;
        case 3:
            p_oBinaryValue = 0x05; // Port B
            break;
        case 4:
            p_oBinaryValue = 0x01; // Port C
            break;
        default:
            p_oBinaryValue = 0x00; // Port A
            wmLog(eError, "Wrong number for OCT reference arm\n");
            break;
    }
    p_oBinaryMask = 0x07;

    // outputs DIGITAL OUT8 to DIGITAL OUT10 on RTC6 card are to be used
    p_oBinaryValue <<= 8; // shift 8 bits left
    p_oBinaryMask <<= 8; // shift 8 bits left
}

void Scanlab::ScannerDriveWithOCTReference(void)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Drive with OCT Reference not implemented for MarkingEngine!\n");
        return;
    }
    if (!isOCT_with_reference_arms())
    {
        return;
    }

    int32_t oPosXinBits = static_cast<int32_t>(m_oNewScannerXPosition * m_oCalibValueBitsPerMM);
    int32_t oPosYinBits = static_cast<int32_t>(m_oNewScannerYPosition * m_oCalibValueBitsPerMM);

    SetScannerJumpSpeed(m_oScannerJumpSpeed);
    SetScannerMarkSpeed(m_oScannerMarkSpeed);

    if (isScanlabScannerEnabled())
    {
        m_oOCTReferenceArmTarget.store(fiberSwitchSelect(m_oReferenceArmCalibrationData, 5000, m_oNewScannerXPosition, m_oNewScannerYPosition) + 1);
        wmLog(eDebug, "fiberSwitchSelect: %d (x: %d, y: %d)\n", (m_oOCTReferenceArmTarget.load() - 1), m_oNewScannerXPosition, m_oNewScannerYPosition);
        uint16_t oBinaryValue{0x00};
        uint16_t oMaskValue{0x07};
        DetermineReferenceArmBits(oBinaryValue, oMaskValue);

        wmLog(eDebug, "Scanlab::ScannerDriveWithOCTReference (%d,%d,%d)\n", oPosXinBits, oPosYinBits, m_oOCTReferenceArmTarget.load());
        m_scannerControl->scannerDriveWithOCTReference(oPosXinBits, oPosYinBits, oBinaryValue, oMaskValue);
        m_oOCTReferenceArmActual.store(m_oOCTReferenceArmTarget.load());
    }

    m_oActualScannerXPosition.store(m_oNewScannerXPosition);
    m_oActualScannerYPosition.store(m_oNewScannerYPosition);
    setActualScannerPositionToDeviceNotification();
}

void Scanlab::ScannerSetOCTReference(void)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Set OCT Reference not implemented for MarkingEngine!\n");
        return;
    }
    if (!isOCT_with_reference_arms())
    {
        return;
    }

    wmLog(eDebug, "Scanlab::ScannerSetOCTReference (%d)\n", m_oOCTReferenceArmTarget.load());

    if (isScanlabScannerEnabled())
    {
        uint16_t oBinaryValue{0x00};
        uint16_t oMaskValue{0x07};
        DetermineReferenceArmBits(oBinaryValue, oMaskValue);

        m_scannerControl->scannerSetOCTReference(oBinaryValue, oMaskValue);
        m_oOCTReferenceArmActual.store(m_oOCTReferenceArmTarget.load());
    }
}

bool Scanlab::reloadFiberSwitchCalibration(void)
{
    wmLog(eDebug, "Scanlab::reloadFiberSwitchCalibration\n");

    m_oReferenceArmCalibrationData = readIdmCalibrationValuesXml();

    {
        std::stringstream oLoggerText{"First: "};
        for (auto& oDoubleValue: m_oReferenceArmCalibrationData.first)
        {
            oLoggerText << oDoubleValue << ", ";
        }
        wmLog(eDebug, "%s\n", oLoggerText.str().c_str());
    }
    {
        std::stringstream oLoggerText{"Second: "};
        for (auto& oDoubleValue: m_oReferenceArmCalibrationData.second)
        {
            oLoggerText << oDoubleValue << ", ";
        }
        wmLog(eDebug, "%s\n", oLoggerText.str().c_str());
    }

    return true;
}

int Scanlab::fiberSwitchSelect(const std::pair<std::vector<double>, std::vector<double>>& model, double idmHalfRange, double sx, double sy)
{
    const auto& l0 = model.first;
    const auto& k = model.second;

    double min = std::numeric_limits<double>::infinity();
    int n = 0;

    for (std::size_t i = 0; i < l0.size(); ++i)
    {
        const double distance = std::abs(std::abs(l0[i] + k[0] * sx * sx + k[1] * sy * sy) - idmHalfRange);
        if (distance < min)
        {
            min = distance;
            n = i;
        }
    }

    return n;
}

double Scanlab::stringToDouble(const std::string& input)
{
    std::stringstream ss;
    double result;
    static std::locale uselocale("C");
    ss.imbue(uselocale);
    ss << input;
    ss >> result;
    return result;
}

std::pair<std::vector<double>, std::vector<double>> Scanlab::readIdmCalibrationValuesXml(std::string fileName)
{
    std::pair<std::vector<double>, std::vector<double>> model;
    model.second = {0.0, 0.0, 0.0, 0.0};
    model.first = {0.0, 0.0, 0.0, 0.0};

    if (fileName == "")
    {
        fileName = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") + "/config/calibrationData0.xml";
    }

    std::ifstream file(fileName);
    std::stringstream buffer;
    buffer << file.rdbuf();

    const std::vector<std::string> regex1 =
    {
        "<IDM_k1>(.*)<\\/IDM_k1>",
        "<IDM_k2>(.*)<\\/IDM_k2>",
        "<IDM_k3>(.*)<\\/IDM_k3>",
        "<IDM_k4>(.*)<\\/IDM_k4>"
    };
    const std::vector<std::string> regex2 =
    {
        "<IDM_l01>(.*)<\\/IDM_l01>",
        "<IDM_l02>(.*)<\\/IDM_l02>",
        "<IDM_l03>(.*)<\\/IDM_l03>",
        "<IDM_l04>(.*)<\\/IDM_l04>"
    };

    std::smatch match;
    std::string fileText = buffer.str();

    for (int i = 0; i < 4; ++i)
    {
        std::regex_search(fileText, match, std::regex(regex1[i]));
        if (match.size() == 2)
        {
            model.second[i] = stringToDouble(match[1].str());
        }
    }

    for (int i = 0; i < 4; ++i)
    {
        std::regex_search(fileText, match, std::regex(regex2[i]));
        if (match.size() == 2)
        {
            model.first[i] = stringToDouble(match[1].str());
        }
    }

    return model;
}

void Scanlab::checkStopEvent(Axis axis)
{
    m_scannerControl->checkStopEvent(axis);
}

void Scanlab::checkStopEvents()
{
    checkStopEvent(Axis::Axis1);
    ScannerCheckLastError();
    checkStopEvent(Axis::Axis2);
    ScannerCheckLastError();
}

void Scanlab::checkStatusWord(Axis axis)
{
    m_scannerControl->checkStatusWord(axis);
}

void Scanlab::checkStatusWords()
{
    checkStatusWord(Axis::Axis1);
    ScannerCheckLastError();
    checkStatusWord(Axis::Axis2);
    ScannerCheckLastError();
}

void Scanlab::checkTemperature(Axis axis)
{
    m_scannerControl->checkTemperature(axis);
}

void Scanlab::checkTemperatures()
{
    checkTemperature(Axis::Axis1);
    ScannerCheckLastError();
    checkTemperature(Axis::Axis2);
    ScannerCheckLastError();
}

void Scanlab::checkServoCardTemperature(Axis axis)
{
    m_scannerControl->checkServoCardTemperature(axis);
}

void Scanlab::checkServoCardTemperatures()
{
    checkServoCardTemperature(Axis::Axis1);
    ScannerCheckLastError();
    checkServoCardTemperature(Axis::Axis2);
    ScannerCheckLastError();
}

void Scanlab::ScannerOperatingState(void)
{
    if (!isScanlabScannerEnabled() || m_enableDebugMode)
    {
        return;
    }

    switch (m_scannerModel)
    {
        case ScannerModel::ScanlabScanner:
        {
            checkStopEvents();
            checkTemperatures();
            checkServoCardTemperatures();
            break;
        }
        case ScannerModel::SmartMoveScanner:
            break;
    }
    checkStatusWords();
}

void Scanlab::ScannerStartWeldingPreview(void)
{
    wmLog(eDebug, "ScannerStartWeldingPreview\n");

    // to avoid problems if a preview is already running
    ScannerStopWeldingPreview();

    if (m_oWeldingFigureNumber == -1)
    {
        // function makes no sense, if there is no weldingSeam-file selected
        wmLogTr(eError, "QnxMsg.VI.SMPreFile1", "Cannot load welding figure file with number %d\n", m_oWeldingFigureNumber);
        return;
    }

    std::string weldingSeamFile = precitec::system::wmBaseDir()
        + "/config/weld_figure/weldingSeam"
        + std::to_string(m_oWeldingFigureNumber)
        + ".json";

    wmLog(eDebug, "Load weldingSeamFile " + weldingSeamFile + "\n");
    if (m_figureWelding->readFigureFromFile(weldingSeamFile, FigureFileType::WeldingFigureType))
    {
        wmLog(eDebug, "Loading weldingSeamFile successful\n");
    }
    else
    {
        // function makes no sense, if the weldingSeam-file is not present
        wmLogTr(eError, "QnxMsg.VI.SMPreFile2", "Error while loading welding figure file with number %d\n", m_oWeldingFigureNumber);
        return;
    }

    RTC6::Figure* weldingSeam = m_figureWelding->getFigure(FigureFileType::WeldingFigureType);
    wmLog(eDebug, "weldingSeamFile contains %d points\n", weldingSeam->figure.size());

    m_figureWelding->set_CalibrationFactor(m_oCalibValueBitsPerMM);

    /* Set attributes of FigureWelding class:
     * For RTC6:
     * Set jump speed, which is used for scanner jumps.
     * Default value is 10.000 Bits/ms --> 2,5 mm/ms ([Bits/ms] / [Bits/mm] = [mm/ms])
     * Set mark speed, which is used for scanner jumps.
     * Default value is 1.000 Bits/ms --> 0,25 mm/ms ([Bits/ms] / [Bits/mm] = [mm/ms])
     */
    const auto jumpSpeedScannerSpecific = m_scannerControl->calculateJumpSpeed(m_oScannerJumpSpeed);
    m_figureWelding->set_Speeds(jumpSpeedScannerSpecific, m_scannerControl->calculateMarkSpeed(m_oScannerMarkSpeed), m_oScannerWobbelFreq);

    //Loading of the wobbelFile
    bool oFigureWobbleFileIsReady{true};
    if (m_oWobbelFigureNumber == -1)
    {
        wmLog(eDebug, "m_oWobbelFigureNumber is not set\n");
        oFigureWobbleFileIsReady = false;
    }
    else
    {
        std::string figureWobbleFile = GetFigureFilePath() + GetFigureFileName() + std::to_string(m_oWobbelFigureNumber) + GetFigureFileEnding();

        wmLog(eDebug, "Load figureWobbleFile " + figureWobbleFile + "\n");
        if (m_figureWelding->readFigureFromFile(figureWobbleFile, FigureFileType::WobbleFigureType))
        {
            wmLog(eDebug, "Loading figureWobbleFile successful\n");
        }
        else
        {
            wmLogTr(eError, "QnxMsg.VI.SMPreFile3", "Error while loading figure wobble file with number %d\n", m_oWobbelFigureNumber);
            oFigureWobbleFileIsReady = false;
        }

        RTC6::Figure* figureWobble = m_figureWelding->getFigure(FigureFileType::WobbleFigureType);
        wmLog(eDebug, "figureWobbleFile contains %d points\n", figureWobble->figure.size());
    }

    long oWobbelXOffsetInBits = static_cast<long>(m_oWobbelXOffset * m_oCalibValueBitsPerMM);
    long oWobbelYOffsetInBits = static_cast<long>(m_oWobbelYOffset * m_oCalibValueBitsPerMM);

    m_figureWelding->set_MarkOffset(oWobbelXOffsetInBits, oWobbelYOffsetInBits, m_oWobbelAngle);

    //Start building list for Scanlab scanner
    m_figureWelding->buildPreviewList(m_oNewScannerXPosition,
                                      m_oNewScannerYPosition,
                                      oFigureWobbleFileIsReady,
                                      jumpSpeedScannerSpecific);

    // start endless figure draw
    m_figureWelding->start_Mark(ScanlabOperationMode::ScanlabPreview);
}

void Scanlab::ScannerStopWeldingPreview(void)
{
    m_figureWelding->stop_Mark();
}

void Scanlab::ScannerTestFunction1(void)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "TestFunction1 not implemented for MarkingEngine!\n");
        return;
    }
    if (!isScanlabScannerEnabled())
    {
        return;
    }

    m_scannerControl->scannerTestFunction1();
}

void Scanlab::ScannerTestFunction2(void)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "TestFunction2 not implemented for MarkingEngine!\n");
        return;
    }
    if (!isScanlabScannerEnabled())
    {
        return;
    }

    m_scannerControl->scannerTestFunction2();
}

void Scanlab::SetScannerJumpSpeed(double oValue)
{
    if (isScanlabScannerEnabled())
    {
        m_scannerControl->setScannerJumpSpeed(oValue);
    }

    m_oScannerJumpSpeed = oValue;
}

void Scanlab::SetScannerMarkSpeed(double oValue)
{
    if (isScanlabScannerEnabled())
    {
        m_scannerControl->setScannerMarkSpeed(oValue);
    }

    m_oScannerMarkSpeed = oValue;
}

void Scanlab::ScannerCheckLastError(void)
{
    if (isScanlabScannerEnabled())
    {
        m_scannerControl->scannerCheckLastError();
    }
}

void Scanlab::ScannerResetLastError(void)
{
    if (isScanlabScannerEnabled())
    {
        m_scannerControl->scannerResetLastError(); // clear the error register
    }
}

void Scanlab::SetLaserPowerParameter(LaserPowerParameter oParaNo, int oValue)
{
    int oIndex = static_cast<int>(oParaNo);
    m_oLaserPowerValuesShadow[oIndex] = oValue;
}

int Scanlab::GetLaserPowerParameter(LaserPowerParameter oParaNo)
{
    int oIndex = static_cast<int>(oParaNo);
    return m_oLaserPowerValuesShadow[oIndex];
}

void Scanlab::ScanmasterWeldingData(interface::ResultDoubleArray const& p_rWeldingData, bool sendEndOfSeam) // Interface: viWeldHeadSubscribe (event)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Not implemented for MarkingEngine!\n");
        return;
    }
    if (m_oWeldSeamRequested.load() == true)
    {
        wmLog(eWarning, "ScanmasterWeldingResult already received in the current seam, ignoring request at imageNumber %d, position %d\n",
            p_rWeldingData.context().imageNumber(), p_rWeldingData.context().position());
        return;
    }
    if (m_currentSeam != p_rWeldingData.context().taskContext().measureTask()->seam())
    {
        wmLog(eWarning, "ScanmasterWeldingResult not for current seam. Expected %d, got %d.\n",
            m_currentSeam, p_rWeldingData.context().taskContext().measureTask()->seam());
        return;
    }

    {
        std::lock_guard m_fillScanlabLock{m_fillScanlab.mutex};
        std::lock_guard m_startWeldingLock{m_startWelding.mutex};

        m_oWeldSeamRequested.store(true);
        m_oWeldingData = p_rWeldingData;
        m_sendEndOfSeam = sendEndOfSeam;

        m_fillScanlab.run.store(true);
        m_startWelding.run.store(true);
    }
    m_fillScanlab.condition.notify_one();
    m_startWelding.condition.notify_one();
}

void Scanlab::ScanmasterPrepareList(const interface::ResultDoubleArray& p_rWeldingData)
{
    if (m_oPrepareSeamRequested.load() == true)
    {
        wmLog(eWarning, "ScanmasterWeldingResult already received preparation, ignoring request at imageNumber %d, position %d\n",
            p_rWeldingData.context().imageNumber(), p_rWeldingData.context().position());
        return;
    }
    if (m_currentSeam != p_rWeldingData.context().taskContext().measureTask()->seam())
    {
        wmLog(eWarning, "ScanmasterPrepareList not for current seam. Expected %d, got %d.\n",
            m_currentSeam, p_rWeldingData.context().taskContext().measureTask()->seam());
        return;
    }

    {
        std::lock_guard m_fillScanlabLock{m_fillScanlab.mutex};

        m_oPrepareSeamRequested.store(true);
        m_oWeldingData = p_rWeldingData;

        m_fillScanlab.run.store(true);
    }
    m_fillScanlab.condition.notify_one();
}

void Scanlab::ScanmasterWeldPreparedList(const interface::ResultDoubleArray& contextData, bool sendEndOfSeam)
{
    if (m_oWeldSeamRequested.load() == true)
    {
        wmLog(eWarning, "ScanmasterWeldingResult already received welding prepared list, ignoring request\n");
        return;
    }
    if (m_currentSeam != contextData.context().taskContext().measureTask()->seam())
    {
        wmLog(eWarning, "ScanmasterWeldPreparedList not for current seam. Expected %d, got %d.\n",
            m_currentSeam, contextData.context().taskContext().measureTask()->seam());
        return;
    }

    {
        std::lock_guard m_startWeldingLock{m_startWelding.mutex};

        m_oWeldSeamRequested.store(true);
        m_usePreparedContour.store(true);
        m_oWeldingData = contextData;
        m_sendEndOfSeam = sendEndOfSeam;

        m_startWelding.run.store(true);
    }
    m_startWelding.condition.notify_one();
}

void Scanlab::ScanmasterWeldingDataWorker(void) // worker function of FillScanlabListThread
{
    /* Unit of values aren't in ScanlabBits (sBit) to get ScanlabBits for RTC6 card multiply
     * with m_oCalibValueBitsPerMM (see definition in ctor of Scanlab class). The value is 4000
     * and can be checked by the correction file of the scanner. The name of the value is
     * calibration factor.
     */

    wmLog(eDebug, "Scanlab::ScanmasterWeldingDataWorker\n");

    wmLog(eDebug, "ScanmasterSeamWelding, imageNumber: %d, position: %d, size: %d\n",
          m_oWeldingData.context().imageNumber(), m_oWeldingData.context().position(), m_oWeldingData.value().size());

    if ((m_oWeldingData.value().size() % 5) != 0)
    {
        wmLogTr(eError, "QnxMsg.VI.SMResNotEven", "The number of values ​​in the value array of the Scanmaster result is not a multiple of five\n");
    }
    if (m_oWeldingData.value().size() < 10)
    {
        wmLogTr(eError, "QnxMsg.VI.SMResTooSmall", "The number of values ​​in the value array of the Scanmaster result is too small\n");
    }

    m_contourAlreadyPreparedInAnotherSeam.store(0);

    if (numberOfPointsFromContour(m_oWeldingData.value().size()) > numberOfPossiblePointsForListMemory())     //Info X and Y are used in one command
    {
        wmLogTr(eInfo, "QnxMsg.VI.PointLimit", "Current contour point limit is %d and provided point count is %d.\n", numberOfPossiblePointsForListMemory(), numberOfPointsFromContour(m_oWeldingData.value().size()));
        wmFatal(eDataConsistency, "QnxMsg.VI.TooManyPoints", "Too many contour points for RTC6 list memory!\n");
        return;
    }

    if (m_zCompensation.compensationActive() && (!m_zCompensationSet.load() && !m_zCompensation.isCompensationFixed()))
    {
        wmFatal(eDataConsistency, "QnxMsg.VI.NoZCompensationSet", "Height compensation value never set!\n");
        return;
    }

    if (!isScanlabScannerEnabled())
    {
        return;
    }
    if (m_controller == ScannerModel::SmartMoveScanner && GetLaserPowerIsDigital())
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Laser power digital not available for MarkingEngine.\n");
        return;
    }

    Calculator calculator;
    calculator.setCalibrationFactor(m_oCalibValueBitsPerMM);
    m_figureWelding->set_CalibrationFactor(m_oCalibValueBitsPerMM);
    m_scannerControl->setLaserDelays({m_oLaserOnDelay*64, m_oLaserOffDelay*64});

    // the velocity is transferred in the unit [um/s].
    // to get [m/s] divide by 10^6 -> 1000000
    const double markSpeep = static_cast<double>(m_oWeldingData.context().taskContext().measureTask()->velocity()) / 1000000.0;

    m_figureWelding->set_Speeds(m_scannerControl->calculateJumpSpeed(m_oScannerJumpSpeed), m_scannerControl->calculateMarkSpeed(markSpeep), m_oScannerWobbelFreq);

    long oWobbelXOffsetInBits = static_cast<long>(m_oWobbelXOffset * m_oCalibValueBitsPerMM);
    long oWobbelYOffsetInBits = static_cast<long>(m_oWobbelYOffset * m_oCalibValueBitsPerMM);

    m_figureWelding->set_MarkOffset(oWobbelXOffsetInBits, oWobbelYOffsetInBits, m_oWobbelAngle);

    /* Set attribute for laser power:
     * Laser power can be changed by an analog value or with a digital output.
     * In the seam file there has to be an attribute which tells the scanlab class if digital or analog laser power is used.
     */
    int dummy;
    m_figureWelding->get_Speeds(m_lastValidJumpSpeedInBitsPerMs, m_lastValidMarkSpeedInBitsPerMs, dummy);
    fillVelocities(m_lastValidMarkSpeedInBitsPerMs, m_lastValidJumpSpeedInBitsPerMs);

#if 0
    for (auto const &element : m_velocities)       //Debug
    {
        wmLog(eWarning, "ScanmasterSeamWelding, Velocity: %d\n", element);
    }
#endif

#if 0
    wmLog(eDebug, "ScanmasterSeamWelding isLaserPowerDigital? %d\n", m_oIsLaserPowerDigital);
#endif

    m_figureWelding->prepareWeldingList(m_oWeldingData, m_velocities.size(), m_oLaserPowerStatic, m_oLaserPowerStaticRing, m_oADCValue);
    int wobbelFileNumber = m_oWobbelFigureNumber;
    if (wobbelFileNumber != -1)
    {
        std::string figureFile = GetFigureFilePath() + GetFigureFileName() + std::to_string(wobbelFileNumber) + GetFigureFileEnding();
        m_figureWelding->loadWobbleFigureFile(figureFile, m_laserPowerDelayCompensation, m_oADCValue);
        calculator.setWobbleStartPosition(m_figureWelding->getFigure(FigureFileType::WobbleFigureType)->figure.at(0).endPosition);
    }
    m_figureWelding->buildWeldingList(calculator, m_oWeldingData, m_velocities);
    m_oContourPreparedFinished.store(1);
    m_contourAlreadyPreparedInAnotherSeam.store(1);

    if (GetLaserPowerIsDigital())
    {
        m_oWeldSeamGenerationIsReady.store(true);
    }
    else
    {
        std::unique_lock<std::mutex> lock{m_startWeldingMutex};
        m_oWeldSeamGenerationIsReady.store(true);
        notifyStartWelding();
    }
}

void Scanlab::ScanmasterScannerMoving(interface::ResultDoubleArray const& p_rWeldingData)
{
    if (m_currentSeam != p_rWeldingData.context().taskContext().measureTask()->seam())
    {
        wmLog(eWarning, "ScanmasterScannerMoving not for current seam. Expected %d, got %d.\n",
            m_currentSeam, p_rWeldingData.context().taskContext().measureTask()->seam());
        return;
    }
    wmLog(eDebug, "ScanmasterScannerMoving, imageNumber: %d, position: %d, size: %d, xPos: %f, yPos: %f\n",
          p_rWeldingData.context().imageNumber(), p_rWeldingData.context().position(), p_rWeldingData.value().size(), p_rWeldingData.value()[0], p_rWeldingData.value()[1]);

    if (!isScanlabScannerEnabled())
    {
        return;
    }

    {
        std::lock_guard ScannerMoving_lock{m_scannerMoving.mutex};

        m_oScanlabMovingHasFinished.store(false);
        m_oWeldingData = p_rWeldingData;

        m_scannerMoving.run.store(true);

    }
    m_scannerMoving.condition.notify_one();


}

void Scanlab::ScannerMoving()  //Worker fucntion of ScannerMoving Thread
{
    SetScannerNewXPosition(m_oWeldingData.value()[0]);
    SetScannerNewYPosition(m_oWeldingData.value()[1]);
    ScannerDriveToPosition();

    std::unique_lock<std::mutex> lock{m_startWeldingMutex};
    m_oScanlabMovingHasFinished.store(true);
    notifyStartWelding();
}

double Scanlab::transformLaserPowerToBits(double laserPower) const
{
    return laserPower * 0.01 * m_oADCValue;
}

void Scanlab::ScanmasterScannerSpotWelding(interface::ResultDoubleArray const& weldingData)
{
    wmLog(eDebug, "Scanlab::ScanmasterScannerSpotWelding\n");
    if (m_currentSeam != weldingData.context().taskContext().measureTask()->seam())
    {
        wmLog(eWarning, "ScanmasterScannerSpotWelding not for current seam. Expected %d, got %d.\n",
            m_currentSeam, weldingData.context().taskContext().measureTask()->seam());
        return;
    }

    if(!isScanlabScannerEnabled())
    {
        return;
    }

    if(weldingData.value().size() < 3)
    {
        wmLogTr(eError, "QnxMsg.VI.SMResTooSmall", "The number of values ​​in the value array of the Scanmaster result is too small\n");
        return;
    }
    else if(weldingData.value().size() > 3)
    {
        wmLogTr(eError, "QnxMsg.VI.SMResTooHigh", "The number of values ​​in the value array of the Scanmaster result is too high\n");
        return;
    }

    double laserPowerCenterInPercent = 0.0;
    double laserPowerRingInPercent = 0.0;

    if (weldingData.value()[1] == SCANMASTERWELDINGDATA_UNDEFINEDVALUE)
    {
        laserPowerCenterInPercent = m_oLaserPowerStatic;
    }
    else
    {
        laserPowerCenterInPercent = weldingData.value()[1];
    }
    if (weldingData.value()[2] == SCANMASTERWELDINGDATA_UNDEFINEDVALUE)
    {
        laserPowerRingInPercent = m_oLaserPowerStaticRing;
    }
    else
    {
        laserPowerRingInPercent = weldingData.value()[2];
    }

    wmLog(eDebug, "laser core power is %f percent and laser ring power is %f percent\n", laserPowerCenterInPercent, laserPowerRingInPercent);

    const auto durationInSec = weldingData.value()[0];
    m_spotWelding = createSpotWelding(m_controller, *m_scannerControl);
    wmLog(eDebug, "ScanmasterSpotWelding, welding started!\n");
    m_spotWelding->define_Mark(durationInSec, laserPowerCenterInPercent, laserPowerRingInPercent);
    m_spotWelding->start_Mark();
    wmLog(eDebug, "ScanmasterSpotWelding, welding finished!\n");
    m_oWeldSeamGenerationIsReady.store(true);
}

bool Scanlab::weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserPowerInPct, double laserDurationInMs, double jumpSpeedInMmPerSec)
{
    wmLog(eDebug, "Scanlab::weldForScannerCalibration\n");

    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Not implemented for MarkingEngine!\n");
        return false;
    }

    if(isScanlabScannerEnabled())
   {
        ScannerCheckLastError();
        double laserPowerCenterInBits = transformLaserPowerToBits(laserPowerInPct);
        const auto analogOutput = m_scannerControl->getAnalogOutput();
        const auto calibrationWelding = ScannerCalibrationWelding(points,
                                                                  laserPowerCenterInBits,
                                                                  laserDurationInMs,
                                                                  jumpSpeedInMmPerSec,
                                                                  analogOutput.value,
                                                                  m_oCalibValueBitsPerMM);
        calibrationWelding.define_Mark();
        calibrationWelding.start_Mark();

        m_oScannerWeldingFinished.store(0);

        auto timeout = 0;
        const auto offsetInMs = 5000;
        const auto numOfPoints = points.size();
        const auto jumpSpeedInMmPerMs = jumpSpeedInMmPerSec * 0.001;
        const auto seamLengthInMm = numOfPoints * 10.;
        const auto jumpDurationInMs = seamLengthInMm / jumpSpeedInMmPerMs;
        const auto timeoutLimit = (numOfPoints * laserDurationInMs + jumpDurationInMs) / 2. + offsetInMs;
        wmLog(eDebug, "timeout limit: %d \n", timeoutLimit);
        while ((calibrationWelding.done_Mark() != true) && (timeout < timeoutLimit))
        {
            usleep(2 * 1000); // 2 ms
            timeout++;
        }
        if (timeout >= timeoutLimit)
        {
            wmLogTr(eError, "QnxMsg.VI.SMTimeoutWeld", "Timeout while waiting for calibration welding has finished\n");
            calibrationWelding.stop_Mark();
            return false;
        }
        else
        {
            wmLog(eDebug, "calibration welding has finished: timeout:%d !\n", timeout);
            m_oScannerWeldingFinished.store(1);
        }
    }
    return true;
}

void Scanlab::ScannerHeightCompensation(double compensationHeight)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "Not implemented for MarkingEngine!\n");
        return;
    }
    if (!m_zCompensation.compensationActive())
    {
        m_zCompensationSet.store(0);
        return;
    }
    m_zCompensation.setZOffset(compensationHeight);
    m_zCompensationSet.store(1);
}

void Scanlab::notifyStartWelding()
{
    if (isStartWeldingReady())
    {
        m_startWeldingCondition.notify_all();
    }
}

bool Scanlab::isStartWeldingReady() const
{
    if (!m_oZCDrivingHasFinished.load())
    {
        return false;
    }
    if ((!m_usePreparedContour.load() && !m_oWeldSeamGenerationIsReady.load()) || (m_usePreparedContour.load() && !m_contourAlreadyPreparedInAnotherSeam.load()))
    {
        return false;
    }
    if (!m_oScanlabMovingHasFinished.load())
    {
        return false;
    }
    return true;
}

void Scanlab::StartWelding_ScanMaster(void) // worker function of StartWeldingThread
{
    m_oStartWeldingThreadIsActive.store(true);
    wmLog(eDebug, "Scanlab::StartWelding_ScanMaster\n");
    if (m_oScanlabMovingHasFinished.load() == false)
    {
        wmLog(eWarning, "Scanmaster scanner moving has not finished yet \n");
    }

    // waiting for ZCollimator is ready and preparation of weld seam is ready
    const std::chrono::milliseconds waitFor{400};
    const auto startTime{std::chrono::steady_clock::now()};
    std::unique_lock<std::mutex> lock{m_startWeldingMutex};
    while (true)
    {
        const auto waited = std::chrono::steady_clock::now() - startTime;
        if (isStartWeldingReady())
        {
            wmLog(eDebug, "waiting for welding start: oTimeout:%d ! %d,%d,%d\n", waited.count(), m_oZCDrivingHasFinished.load(), m_oWeldSeamGenerationIsReady.load(), m_oScanlabMovingHasFinished.load());
            break;
        }
        const auto remainingTime = std::chrono::duration_cast<std::chrono::nanoseconds>(waitFor) - waited;
        if (remainingTime.count() < 0)
        {
            wmLogTr(eError, "QnxMsg.VI.SMWeldTimeout", "Timeout while waiting for start of welding (%d,%d,%d)\n", m_oZCDrivingHasFinished.load(), m_oWeldSeamGenerationIsReady.load(), m_oScanlabMovingHasFinished.load());
            m_oStartWeldingThreadIsActive.store(false);
            return;
        }
        if (m_startWeldingCondition.wait_for(lock, remainingTime) == std::cv_status::timeout)
        {
            wmLogTr(eError, "QnxMsg.VI.SMWeldTimeout", "Timeout while waiting for start of welding (%d,%d, %d)\n", m_oZCDrivingHasFinished.load(), m_oWeldSeamGenerationIsReady.load(), m_oScanlabMovingHasFinished.load());
            m_oStartWeldingThreadIsActive.store(false);
            return;
        }
    }
    lock.unlock();

    // start welding procedure on Scanlab scanner
    wmLog(eDebug, "Start welding procedure\n");

    m_oScannerWeldingFinished.store(0);
    m_figureWelding->start_Mark(ScanlabOperationMode::ScanlabWelding);

    wmLog(eDebug, "End welding procedure\n");

    if( isSCANMASTER_GeneralApplication() )
    {
        // waiting for welding has finished
        auto oTimeout = 0;
        while ((m_figureWelding->done_Mark() != true) && (oTimeout < 15000)) // timeout at 30 seconds
        {
            usleep(2 * 1000); // 2 ms
            oTimeout++;
        }
        if (oTimeout >= 15000)
        {
            wmLogTr(eError, "QnxMsg.VI.SMTimeoutWeld", "Timeout while waiting for welding has finished\n");
        }
        else
        {
            wmLog(eDebug, "welding has finished: oTimeout:%d !\n", oTimeout);
            if (m_figureWelding->getActualXPosition().has_value() && m_figureWelding->getActualYPosition().has_value())
            {
                m_oActualScannerXPosition.store(m_figureWelding->getActualXPosition().value());
                m_oActualScannerYPosition.store(m_figureWelding->getActualYPosition().value());
                setActualScannerPositionToDeviceNotification();
            }

            m_oScannerWeldingFinished.store(1);
            if (m_resultsProxy && m_sendEndOfSeam)
            {
                GeoDoublearray geoArray{};
                geoArray.ref().getData().push_back(1.0);
                geoArray.ref().getRank().push_back(filter::ValueRankType::eRankMax);
                auto result = interface::ResultDoubleArray{{},
                    EndOfSeamMarker,
                    EndOfSeamMarker,
                    m_oWeldingData.context(),
                    geoArray,
                    {-100000.0, 100000.0},
                    false };
                m_resultsProxy->result(result);
                wmLog(eDebug, "EndOfSeamMarker sent from welding finished\n");
            }
        }
    }

    m_oStartWeldingThreadIsActive.store(false);
}

void Scanlab::threadFunction(WorkerThread &worker, std::function<void(void)> runMethod)
{
    std::unique_lock lock{worker.mutex};
    while (!m_tearDown.load())
    {
        if (worker.run.load())
        {
            worker.run.store(false);
            runMethod();
            continue;
        }
        worker.condition.wait(lock);
    }
}

void Scanlab::fillScanlabListThread()
{
    prctl(PR_SET_NAME, "FillScanlab");
    system::makeThreadRealTime(system::Priority::Sensors);
    threadFunction(m_fillScanlab, std::bind(&Scanlab::ScanmasterWeldingDataWorker, this));
}

void Scanlab::startWeldingThread()
{
    prctl(PR_SET_NAME, "StartWelding");
    system::makeThreadRealTime(system::Priority::Sensors);
    threadFunction(m_startWelding, std::bind(&Scanlab::StartWelding_ScanMaster, this));
}

void Scanlab::scannerMovingThread()
{
    prctl(PR_SET_NAME, "ScannerMoving");
    system::makeThreadRealTime(system::Priority::Sensors);
    threadFunction(m_scannerMoving, std::bind(&Scanlab::ScannerMoving, this));
}

void Scanlab::generateScantracker2DList(void)
{
    if (m_controller == ScannerModel::SmartMoveScanner)
    {
        wmFatal(eAxis, "QnxMsg.VI.MarkingEngineNotImplemented", "ScanTracker2D not implemented for MarkingEngine!\n");
        return;
    }
    if (getScannerGeneralMode() != ScannerGeneralMode::eScantracker2DMode)
    {
        return;
    }

    m_scantracker2DSeam.set_CalibrationFactor(m_oCalibValueBitsPerMM);
    m_scantracker2DSeam.set_Speeds(2000.0, 1.0, m_oScannerWobbelFreq);

    if (!m_scanPosOutOfGapPos) // scan position is a fixed value
    {
        m_scantracker2DSeam.set_MarkOffset(static_cast<long int>(m_scantracker2DScanPosFixedX * m_oCalibValueBitsPerMM),
                                           static_cast<long int>(m_scantracker2DScanPosFixedY * m_oCalibValueBitsPerMM),
                                           m_oWobbelAngle);
    }
    else // scan position is controlled via results
    {
        m_scantracker2DSeam.set_MarkOffset(0, 0, m_oWobbelAngle);
    }
    if (!m_scanWidthOutOfGapWidth) // scan width is a fixed value
    {
        m_scantracker2DSeam.set_Scale(m_scantracker2DScanWidthFixedX, m_scantracker2DScanWidthFixedY);
    }
    else // scan width is controlled via results
    {
        m_scantracker2DSeam.set_Scale(1.0, 1.0);
    }

    m_scantracker2DSeam.set_MarkAngle(m_oWobbelAngle);
    m_scantracker2DSeam.setLaserPowerStatic(m_oLaserPowerStatic);
    m_scannerControl->setLaserDelays({m_oLaserOnDelay * 64, m_oLaserOffDelay * 64});
    m_scantracker2DSeam.setScanTracker2DLaserDelay(m_scanTracker2DLaserDelay);

    if (m_oWobbelFigureNumber == -1)
    {
        wmLogTr(eError, "QnxMsg.VI.SMPreFile1", "Cannot load welding figure file with number %d\n", m_oWobbelFigureNumber);
        return;
    }

    std::string figureWobbleFile{};
    if (m_scanTracker2DCustomFigure) // a custom figure file is used
    {
        figureWobbleFile = GetFigureFilePath() + GetFigureFileName() + std::to_string(m_oWobbelFigureNumber) + GetFigureFileEnding();
    }
    else
    {
        figureWobbleFile = std::string(getenv("WM_BASE_DIR")) + "/system_graphs/basic_figure/basic" + std::to_string(m_oWobbelFigureNumber) + GetFigureFileEnding();
    }

    wmLog(eDebug, "Load figureWobbleFile " + figureWobbleFile + "\n");

    m_scantracker2DSeam.readFigureFromFile(figureWobbleFile, precitec::FigureFileType::WobbleFigureType);
    m_scantracker2DSeam.setADCValue(m_oADCValue);
    m_scantracker2DSeam.defineFigure();

    ScannerDriveToZero();
}

void Scanlab::SetScanWidthOutOfGapWidth(bool onOff)
{
    m_scanWidthOutOfGapWidth = onOff;
}

void Scanlab::SetScanPosOutOfGapPos(bool onOff)
{
    m_scanPosOutOfGapPos = onOff;
}

void Scanlab::setScanTracker2DScanWidthFixedX(double value)
{
    m_scantracker2DScanWidthFixedX = value;

//    SetScanTracker2DScanWidth();
}

double Scanlab::getScanTracker2DScanWidthFixedX(void)
{
    return m_scantracker2DScanWidthFixedX;
}

void Scanlab::setScanTracker2DScanWidthFixedY(double value)
{
    m_scantracker2DScanWidthFixedY = value;

//    SetScanTracker2DScanWidth();
}

double Scanlab::getScanTracker2DScanWidthFixedY(void)
{
    return m_scantracker2DScanWidthFixedY;
}

void Scanlab::setScanTracker2DScanPosFixedX(double value)
{
    // parameter value has unit [mm]
    m_scantracker2DScanPosFixedX = value;

//    SetScanTracker2DScanPos();
}

double Scanlab::getScanTracker2DScanPosFixedX(void)
{
    return m_scantracker2DScanPosFixedX;
}

void Scanlab::setScanTracker2DScanPosFixedY(double value)
{
    // parameter value has unit [mm]
    m_scantracker2DScanPosFixedY = value;

//    SetScanTracker2DScanPos();
}

double Scanlab::getScanTracker2DScanPosFixedY(void)
{
    return m_scantracker2DScanPosFixedY;
}

void Scanlab::SetTrackerScanWidthControlled(int value) // Interface: viWeldHeadSubscribe (event)
{
    // parameter value has unit [um]
    wmLog(eDebug, "Scanlab::SetTrackerScanWidthControlled %d\n", value);
    if (value == 0)
    {
        value = 1; // width of 0 is not useful
    }
    m_scantracker2DScanWidthControlledX = static_cast<double>(value) / 1000.0; // same value X and Y until results are separated into X and Y
    m_scantracker2DScanWidthControlledY = static_cast<double>(value) / 1000.0; // same value X and Y until results are separated into X and Y

    SetScanTracker2DScanWidth();
}

void Scanlab::SetTrackerScanPosControlled(int value) // Interface: viWeldHeadSubscribe (event)
{
    // parameter value has unit [um]
    wmLog(eDebug, "Scanlab::SetTrackerScanPosControlled %d\n", value);
    m_scantracker2DScanPosControlledX = static_cast<double>(0.0); // fixed value until results are separated into X and Y
    m_scantracker2DScanPosControlledY = static_cast<double>(value) / 1000.0;

    SetScanTracker2DScanPos();
}

void Scanlab::SetScanTracker2DScanWidth(void)
{
    if (m_scanWidthOutOfGapWidth)
    {
        double scaleX = static_cast<double>(m_scantracker2DScanWidthControlledX);
        double scaleY = static_cast<double>(m_scantracker2DScanWidthControlledY);
        wmLog(eDebug, "Scanlab::SetScanTracker2DScanWidth %f, %f\n", scaleX, scaleY);
        m_scantracker2DSeam.refreshSizeFromCameraByScale(std::make_pair(scaleX, scaleY));
    }
}

void Scanlab::SetScanTracker2DScanPos(void)
{
    if (m_scanPosOutOfGapPos)
    {
        double offsetX = m_scantracker2DScanPosControlledX * m_oCalibValueBitsPerMM;
        double offsetY = m_scantracker2DScanPosControlledY * m_oCalibValueBitsPerMM;
        wmLog(eDebug, "Scanlab::SetScanTracker2DScanPos %f, %f\n", offsetX, offsetY);
        m_scantracker2DSeam.refreshOffsetFromCamera(offsetX, offsetY);
    }
}

void Scanlab::setKeyValueToDeviceNotification(const std::string& keyValue, double value) const
{
    m_deviceNotificationProxy.get()->keyValueChanged(Poco::UUID(), SmpKeyValue(new interface::TKeyValue<double>(keyValue, value)));
}

void Scanlab::setActualScannerPositionToDeviceNotification() const
{
    setKeyValueToDeviceNotification("Scanner_Actual_X_Position", m_oActualScannerXPosition.load());
    setKeyValueToDeviceNotification("Scanner_Actual_Y_Position", m_oActualScannerYPosition.load());
}

} // namespace hardware

} // namespace precitec

