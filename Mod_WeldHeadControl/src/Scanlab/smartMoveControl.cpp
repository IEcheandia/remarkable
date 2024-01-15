#include "viWeldHead/Scanlab/smartMoveControl.h"

#include "viWeldHead/Scanlab/lensModel.h"

#include "module/moduleLogger.h"

#include "common/definesSmartMove.h"

using precitec::smartMove::SystemOperationModes;
using precitec::smartMove::InitializationError;
using precitec::smartMove::FileDescriptorError;
using precitec::smartMove::FileUploadError;
using precitec::smartMove::ScanfieldBits;
using precitec::smartMove::TCPCommand;
using precitec::smartMove::InputModes;

using precitec::weldingFigure::SEAMWELDING_RESULT_FIELDS_PER_POINT;
using precitec::weldingFigure::SeamWeldingResultSpecialCases;
using precitec::weldingFigure::SeamWeldingResultFields;
using precitec::weldingFigure::SpecialValueInformation;
using precitec::weldingFigure::Point;

namespace precitec
{
namespace hardware
{

static const int SCANNER_INITIALIZE_LIMIT = 500000;

SmartMoveControl::SmartMoveControl()
{
}

ScannerControlStatus SmartMoveControl::init(std::string ipAddress,
                                                const std::string& scanlabCorrectionFile,
                                                precitec::interface::LensType lensType,
                                                precitec::interface::ScannerModel scannerModel)
{
    //TODO Check if scannerController is important!
    InitData data;
    data.ipAddress = ipAddress;
    data.correctionFile = scanlabCorrectionFile;
    data.lens = lensType;
    data.scannerModel = scannerModel;
    switch (init(data))
    {
    case static_cast<int>(smartMove::InitializationError::Success):
        return ScannerControlStatus::Initialized;
    case static_cast<int>(InitializationError::Failure):
        return ScannerControlStatus::InitializationFailed;
    default:
        __builtin_unreachable();
    }
}

double SmartMoveControl::calibValueBitsPerMM()
{
    // TODO: implement
    return 0.0;
}

unsigned int SmartMoveControl::numberOfPointsFromContour(std::size_t contourSize)
{
    // TODO: implement
    return 0;
}

unsigned int SmartMoveControl::numberOfPossiblePointsForListMemory()
{
    // TODO: implement
    return 0;
}

void SmartMoveControl::setLaserDelays(LaserDelays delays){}
void SmartMoveControl::sendToleranceInBitsToScanner(int toleranceInBits){}

void SmartMoveControl::scannerDriveToPosition(double xPosInMM, double yPosInMM)
{
    scannerDriveToPosition(std::make_pair(xPosInMM, yPosInMM));
}

void SmartMoveControl::scannerDriveWithOCTReference(int32_t xPosInBits, int32_t yPosInBits, uint16_t binaryValue, uint16_t maskValue){}
void SmartMoveControl::scannerSetOCTReference(uint16_t binaryValue, uint16_t maskValue){}
void SmartMoveControl::jumpScannerOffset(){}

void SmartMoveControl::checkStatusWord(precitec::Axis axis){}
void SmartMoveControl::checkTemperature(precitec::Axis axis){}
void SmartMoveControl::checkServoCardTemperature(precitec::Axis axis){}
void SmartMoveControl::checkStopEvent(precitec::Axis axis){}

void SmartMoveControl::scannerCheckLastError(){}
void SmartMoveControl::scannerResetLastError(){}

void SmartMoveControl::scannerTestFunction1(){}
void SmartMoveControl::scannerTestFunction2(){}

double SmartMoveControl::selectCorrectionFileMode(precitec::interface::CorrectionFileMode mode)
{
    // TODO: implement
    return 0;
}

LaserDelays SmartMoveControl::getLaserDelays() const
{
    // TODO: implement
    return {};
}

AnalogOutput SmartMoveControl::getAnalogOutput() const
{
    // TODO: implement
    return {};
}

ScannerGeometricalTransformationData SmartMoveControl::getScannerGeometricalTransformationData() const
{
    // TODO: implement
    return {};
}

int SmartMoveControl::init(const InitData& dataForInitialization)
{
    LensModel lensModel;
    lensModel.setScannerController(dataForInitialization.scannerModel);              //Lens model needs the scanner controller but scanner controller isn't passed by the init function yet. The marking engine also only controls the SmartMove scanner thus it should be ok to misuse the scanner model.
    lensModel.setType(dataForInitialization.lens);

    const auto& lensData = lensModel.currentLensInformation();

    m_calculator.setScanfieldSize(lensData.scanFieldSquare);

    m_globalGenerator.reset();
    m_globalGenerator.addCalibrationFilename();
    m_globalGenerator.addScanfieldSize(m_calculator.calculateMeterFromMillimeter(lensData.scanFieldSquare), TCPCommand::Set);
    m_globalGenerator.addFocalLength(m_calculator.calculateMeterFromMillimeter(lensData.focalLength), TCPCommand::Set);
    m_globalGenerator.addForceEnabled();
    m_globalGenerator.addSysTs();

    m_globalInterpreter.reset();
    m_globalInterpreter.translateGlobalCommands(m_globalGenerator.generatedGlobalCommands());

    const auto& globalCommands = m_globalInterpreter.currentCommandSeries();

    if (globalCommands.size() != 5)
    {
        wmLog(eError, "Number of commands is wrong\n");
        return static_cast<int>(InitializationError::Failure);
    }

    if (m_networkInterface.openTcpConnection() != static_cast<int>(InitializationError::Success))
    {
        return static_cast<int>(InitializationError::Failure);
    }

    m_networkInterface.askForVersion();

    const auto& currentCorrectionFile = correctionFile(lensData, dataForInitialization.correctionFile);

    const auto& markingEngineCorrectionFile = m_networkInterface.sendCalibrationFilename(globalCommands.front());
    wmLog(eDebug, "Corr. file: %s\n", markingEngineCorrectionFile);

    if (!sendCorrectionFile(currentCorrectionFile, markingEngineCorrectionFile))
    {
        wmLog(eError, "Failed to update correction file\n");
        wmLog(eDebug, "Update focal length instead\n");
        if (!m_networkInterface.sendFocalLength(globalCommands.at(2)))
        {
            wmLog(eError, "Send focal length failed\n");
            return static_cast<int>(InitializationError::Failure);
        }
    }

    const auto& updatedMarkingEngineCorrectionFile = m_networkInterface.sendCalibrationFilename(globalCommands.front());
    if (updatedMarkingEngineCorrectionFile != markingEngineCorrectionFile)
    {
        wmLog(eDebug, "Corr. file: %s\n", updatedMarkingEngineCorrectionFile);
    }

    if (!m_networkInterface.sendScanfieldSize(globalCommands.at(1)))
    {
        wmLog(eError, "Send scanfield size failed\n");
        return static_cast<int>(InitializationError::Failure);
    }
    if (!m_networkInterface.sendForceEnable(globalCommands.at(3)))
    {
        wmLog(eError, "Send force enable size failed\n");
        return static_cast<int>(InitializationError::Failure);
    }
    if (const auto& value = m_networkInterface.sendGetSysTs(globalCommands.at(4)); value.has_value())
    {
        m_sysTs = value.value();
    }

    int counter = 0;
    while(!m_networkInterface.scannerReady() && counter <= SCANNER_INITIALIZE_LIMIT)
    {
        counter++;
    }
    if (counter == SCANNER_INITIALIZE_LIMIT)
    {
        wmLog(eError, "Scanner has a failure!\n");
    }

    return static_cast<int>(InitializationError::Success);
}

void SmartMoveControl::startJob(JobRepeats repeatMode)
{
    m_globalGenerator.reset();
    m_globalGenerator.addSystemOperationMode(SystemOperationModes::Marking, TCPCommand::Set);
    m_globalGenerator.addProcessJob(repeatMode);

    m_globalInterpreter.reset();
    m_globalInterpreter.translateGlobalCommands(m_globalGenerator.generatedGlobalCommands());

    const auto& globalCommands = m_globalInterpreter.currentCommandSeries();

    if (globalCommands.size() != 2)
    {
        wmLog(eError, "Number of commands is wrong\n");
        return;
    }

    if (!m_networkInterface.isTCPConnectionOpen())
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication\n");
        return;
    }

    if (!static_cast<bool>(m_networkInterface.sendSystemOperationMode(globalCommands.front())))
    {
        wmLog(eError, "Start job failed, switching to Marking System operation failed!\n");
    }
    if (!static_cast<bool>(m_networkInterface.processJob(globalCommands.back())))
    {
        wmLog(eError, "Start job failed!\n");
    }
}

void SmartMoveControl::scannerDriveToPosition(std::pair<double, double> position)
{
    scannerDriveToPositionBits(std::make_pair(m_calculator.calculateDriveToBits(position.first), m_calculator.calculateDriveToBits(position.second)));
}

void SmartMoveControl::scannerDriveToPositionBits(std::pair<unsigned int, unsigned int> position)
{
    m_globalGenerator.reset();
    m_globalGenerator.addSystemOperationMode(SystemOperationModes::Service, TCPCommand::Set);
    m_globalGenerator.addInputMode(InputModes::Alignment, TCPCommand::Set);
    m_globalGenerator.addAlign(position, TCPCommand::Set);

    wmLog(eDebug, "SmartMoveControl::ScannerDriveToPositionBits (%d,%d)\n", position.first, position.second);

    m_globalInterpreter.reset();
    m_globalInterpreter.translateGlobalCommands(m_globalGenerator.generatedGlobalCommands());

    const auto& globalCommands = m_globalInterpreter.currentCommandSeries();

    if (globalCommands.size() != 4)
    {
        wmLog(eError, "Number of commands is wrong\n");
        return;
    }

    if (!m_networkInterface.isTCPConnectionOpen())
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication\n");
        return;
    }

    //TODO Maybe use "Multi-Set" for faster sending --> Is implemented in smartMoveGlobalInterpreter but isn't used --> The check if sending the command is successful should behave as already.
    if (!m_networkInterface.sendSystemOperationMode(globalCommands.front()))
    {
        wmLog(eError, "Send system operation mode failed\n");
    }
    if (!m_networkInterface.sendInputMode(globalCommands.at(1)))
    {
        wmLog(eError, "Send input mode failed\n");
    }
    if (!m_networkInterface.sendAlignX(globalCommands.at(2)))
    {
        wmLog(eError, "Send align X failed\n");
    }
    if (!m_networkInterface.sendAlignY(globalCommands.back()))
    {
        wmLog(eError, "Send align Y failed\n");
    }
}

void SmartMoveControl::scannerDriveToZero()
{
    scannerDriveToPosition(std::make_pair(ScanfieldBits::Origin, ScanfieldBits::Origin));
}

void SmartMoveControl::setScannerJumpSpeed(double speed)
{
    /**
     * Speed from user is [m/s] but SmartMove needs [counts/s]
     * Counts refer to the HPGL Plotter (scanfield size)
     * Transforming from [m/s] to [counts/s] is done with the formula speed_SM = speed * resolution; [counts/ms] = [mm/ms] * [counts/mm] --> [mm/ms] = [m/s]
     **/

    m_globalGenerator.reset();
    m_globalGenerator.addJumpSpeed(m_calculator.calculateBitsPerMillisecondsFromMeterPerSecond(speed), TCPCommand::Set);

    m_globalInterpreter.reset();
    m_globalInterpreter.translateGlobalCommands(m_globalGenerator.generatedGlobalCommands());

    const auto& globalCommands = m_globalInterpreter.currentCommandSeries();

    if (!m_networkInterface.isTCPConnectionOpen())
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication\n");
        return;
    }

    if (globalCommands.size() != 1)
    {
        wmLog(eError, "Number of commands is wrong\n");
        return;
    }

    if (!m_networkInterface.sendJumpSpeed(globalCommands.front()))
    {
        wmLog(eError, "Send jump speed failed\n");
    }
}

double SmartMoveControl::calculateJumpSpeed(double meterPerSeconds) const
{
    return m_calculator.calculateBitsPerMillisecondsFromMeterPerSecond(meterPerSeconds);
}

double SmartMoveControl::calculateMarkSpeed(double meterPerSeconds) const
{
    return m_calculator.calculateBitsPerMillisecondsFromMeterPerSecond(meterPerSeconds);
}

void SmartMoveControl::setScannerMarkSpeed(double speed)
{
    /**
     * Speed from user is [m/s] but SmartMove needs [counts/s]
     * Counts refer to the HPGL Plotter (scanfield size)
     * Transforming from [m/s] to [counts/s] is done with the formula speed_SM = speed * resolution; [counts/ms] = [mm/ms] * [counts/mm] --> [mm/ms] = [m/s]
     **/

    m_globalGenerator.reset();
    m_globalGenerator.addMarkSpeed(m_calculator.calculateBitsPerMillisecondsFromMeterPerSecond(speed), TCPCommand::Set);

    m_globalInterpreter.reset();
    m_globalInterpreter.translateGlobalCommands(m_globalGenerator.generatedGlobalCommands());

    const auto& globalCommands = m_globalInterpreter.currentCommandSeries();

    if (!m_networkInterface.isTCPConnectionOpen())
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication\n");
        return;
    }

    if (globalCommands.size() != 1)
    {
        wmLog(eError, "Number of commands is wrong\n");
        return;
    }

    if (!m_networkInterface.sendMarkSpeed(globalCommands.front()))
    {
        wmLog(eError, "Send mark speed failed\n");
    }
}

void SmartMoveControl::prepareWeldingList(const std::vector<double>& weldingData, precitec::weldingFigure::ProductValues defaultValuesProduct)
{
    //TODO Add calculations and check units from weldingData to m_currentPoints;
    //Power should be ok, but speed is m/s (?) and should be counts/ms
    m_currentPoints.clear();

    int pointCount = weldingData.size() / SEAMWELDING_RESULT_FIELDS_PER_POINT;
    int actualPoint = 0;

    m_currentPoints.reserve(pointCount);

    while (actualPoint < pointCount)
    {
        Point currentPoint;
        currentPoint.X = weldingData[(actualPoint * SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(SeamWeldingResultFields::X)];
        currentPoint.Y = weldingData[(actualPoint * SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(SeamWeldingResultFields::Y)];
        SpecialValueInformation informationForSpecialValue;
        informationForSpecialValue.currentPointValue = weldingData[(actualPoint * SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(SeamWeldingResultFields::Power)];
        informationForSpecialValue.pointValueBefore = actualPoint == 0 ? -1.0 : m_currentPoints.back().laserPower;
        informationForSpecialValue.productValue = defaultValuesProduct.laserPower;
        currentPoint.laserPower = translateSpecialValue(informationForSpecialValue);
        informationForSpecialValue.currentPointValue = weldingData[(actualPoint * SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(SeamWeldingResultFields::RingPower)];
        informationForSpecialValue.pointValueBefore = actualPoint == 0 ? -1.0 : m_currentPoints.back().laserPowerRing;
        informationForSpecialValue.productValue = defaultValuesProduct.laserPowerRing;
        currentPoint.laserPowerRing = translateSpecialValue(informationForSpecialValue);
        informationForSpecialValue.currentPointValue = weldingData[(actualPoint * SEAMWELDING_RESULT_FIELDS_PER_POINT) + static_cast<int>(SeamWeldingResultFields::Velocity)];
        informationForSpecialValue.pointValueBefore = actualPoint == 0 ? -1.0 : m_currentPoints.back().speed;
        informationForSpecialValue.productValue = defaultValuesProduct.velocity;
        currentPoint.speed = translateSpecialValue(informationForSpecialValue);
        m_currentPoints.push_back(currentPoint);
        actualPoint++;
    }
}

void SmartMoveControl::buildPreviewList()
{
    //NOTE It's quite the same code like in buildWeldingList but should be ok! Otherwise split out some components which can be used in both functions.
    m_generator.resetContour();
    m_generator.addInitialize();
    //Jump to first position        TODO Feature to jump before the current first position and accelerate to first point isn't implemented --> Ask if this feature is still worth --> Calculation should be done by a calculator class
    m_generator.addJump(m_calculator.calculateMillimeterToBits(m_currentPoints.front().X), m_calculator.calculateMillimeterToBits(m_currentPoints.front().Y));
    m_generator.addLaserPower(precitec::weldingFigure::JUMP_POWER_VALUE);
    m_generator.addMarkSpeed(m_currentPoints.front().speed);

    for (std::size_t i = 1; i < m_currentPoints.size(); i++)
    {
        //There are some improvements for special cases like for example only one laser power is used. (File could be shorter)
        //TODO Ask if improvements are worth and then implement.
        const auto& currentPoint = m_currentPoints[i];
        if (currentPoint.laserPower == precitec::weldingFigure::JUMP_POWER_VALUE)   //TODO Use fuzzy compare --> fuzzy compare should be in SmartMove calculator after adding calculator to SmartMove calculator and add a new calcualtorRTC6.
        {
            m_generator.addJump(m_calculator.calculateMillimeterToBits(currentPoint.X), m_calculator.calculateMillimeterToBits(currentPoint.Y));
        }
        else
        {
            //TODO Ask if this order results in drive to position and change values after reaching the position before
            m_generator.addMark(m_calculator.calculateMillimeterToBits(currentPoint.X), m_calculator.calculateMillimeterToBits(currentPoint.Y));
            m_generator.addLaserPower(precitec::weldingFigure::JUMP_POWER_VALUE);
            m_generator.addMarkSpeed(currentPoint.speed);
        }
    }

    if (m_generator.empty())
    {
        precitec::wmLog(eError, "Generator doesn't work for building a preview list\n");    //TODO Just for testing! It might be better to use a wmFatal
        return;
    }

    m_interpreter.newContour();

    /*const auto& filenameWithPath = std::string(getenv("WM_BASE_DIR")) + "/logfiles/hpglFile.txt";
    precitec::wmLog(eWarning, "%s\n", filenameWithPath);    //TODO Just for testing!
    m_interpreter.setFilename(filenameWithPath);
    m_interpreter.setDebug(true);*/

    m_interpreter.createContourFile(m_generator.generatedContour());

    auto figureFileDescriptor = m_interpreter.fileDescriptor();
    if (figureFileDescriptor == -1)
    {
        precitec::wmLog(eError, "Creating file descriptor for the preview figure failed\n");    //TODO Just for testing! It might be better to use a wmFatal
        return;
    }

    if (m_networkInterface.openTcpConnection() == -1)
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication\n");
        return;
    }

    std::string answer{};
    m_networkInterface.transmitMarkingFileBTX(figureFileDescriptor, answer);
    //m_networkInterface.transmitMarkingFileXModem(figureFileDescriptor, answer);
    checkHpglUpload(answer);

    //TODO Check if vector count from marking engine is the same like expected to check if the upload was successful? --> Vector counts should be size of m_currentPoints but keep in mind if the pre position is calculated for accelerating the scanner the vector counts increases by one.
}

void SmartMoveControl::buildWeldingList()
{
    m_generator.resetContour();
    m_generator.addInitialize();
    //Jump to first position        TODO Feature to jump before the current first position and accelerate to first point isn't implemented --> Ask if this feature is still worth --> Calculation should be done by a calculator class
    m_generator.addJump(m_currentPoints.front().X, m_currentPoints.front().Y);
    m_generator.addLaserPower(m_currentPoints.front().laserPower);
    m_generator.addMarkSpeed(m_currentPoints.front().speed);

    for (std::size_t i = 1; i < m_currentPoints.size(); i++)
    {
        //There are some improvements for special cases like for example only one laser power is used. (File could be shorter)
        //TODO Ask if improvements are worth and then implement.
        const auto& currentPoint = m_currentPoints[i];
        if (currentPoint.laserPower == precitec::weldingFigure::JUMP_POWER_VALUE)   //TODO Use fuzzy compare --> fuzzy compare should be in SmartMove calculator after adding calculator to SmartMove calculator and add a new calcualtorRTC6.
        {
            m_generator.addJump(currentPoint.X, currentPoint.Y);
        }
        else
        {
            //TODO Ask if this order results in drive to position and change values after reaching the position before
            m_generator.addMark(currentPoint.X, currentPoint.Y);
            m_generator.addLaserPower(currentPoint.laserPower);
            m_generator.addMarkSpeed(currentPoint.speed);           //TODO Ask for new Firmware and check command for SmartMove engine.
        }
    }

    if (m_generator.empty())
    {
        precitec::wmLog(eError, "Generator doesn't work for building a list\n");    //TODO Just for testing! It might be better to use a wmFatal
        return;
    }

    m_interpreter.newContour();
    m_interpreter.createContourFile(m_generator.generatedContour());

    auto figureFileDescriptor = m_interpreter.fileDescriptor();
    if (figureFileDescriptor == -1)
    {
        precitec::wmLog(eError, "Creating file descriptor for the figure failed\n");    //TODO Just for testing! It might be better to use a wmFatal
        return;
    }

    if (m_networkInterface.openTcpConnection() == -1)
    {
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication\n");
        return;
    }

    std::string answer{};
    m_networkInterface.transmitMarkingFileBTX(figureFileDescriptor, answer);
    checkHpglUpload(answer);

    //TODO Check if vector count from marking engine is the same like expected to check if the upload was successful? --> Vector counts should be size of m_currentPoints but keep in mind if the pre position is calculated for accelerating the scanner the vector counts increases by one.
}

void SmartMoveControl::checkHpglUpload(const std::string& answer) const
{
    try
    {
        if (const auto status = std::stoi(answer); status != 0)
        {
            wmLog(eError, "Error transmitting HPGL to MarkingEngine. Status code: %d\n", status);
        }
    } catch (...)
    {
        wmLog(eError, "Answer for transmitting HPGL is not numeric!\n");
    }
}

void SmartMoveControl::startMark()
{
    //TODO Implement like other function --> Branch "WEL-6380 Implement job select and start" contains all changes which are important to implement this function. (SmartMoveControl changes, smartMoveGlobalInterpreter, definesSmartMove and globalCommandGenerator
}

void SmartMoveControl::selectList()
{
    //TODO This might be important default value is job 1 which is filled and started by default!
}

double SmartMoveControl::translateSpecialValue(const SpecialValueInformation& currentState)
{
    auto currentValue = static_cast<int>(currentState.currentPointValue);
    if (currentValue == static_cast<int>(SeamWeldingResultSpecialCases::TakeLastValid) && static_cast<int>(currentState.pointValueBefore) == static_cast<int>(SeamWeldingResultSpecialCases::TakeLastValid))
    {
        return currentState.productValue;
    }
    if (currentValue == static_cast<int>(SeamWeldingResultSpecialCases::TakeLastValid))
    {
        return currentState.pointValueBefore;
    }
    if (currentValue == static_cast<int>(SeamWeldingResultSpecialCases::TakeProduct))
    {
        return currentState.productValue;
    }

    return currentState.currentPointValue;
}

std::string SmartMoveControl::correctionFile(const LensData& currentLensData, const std::string& customCorrectionFile)
{
    if (customCorrectionFile.empty())
    {
        return std::string(getenv("WM_BASE_DIR")) + "/calib/" + currentLensData.calibrationFile;
    }

    return std::string(getenv("WM_BASE_DIR")) + "/config/calib/" + customCorrectionFile;
}

bool SmartMoveControl::sendCorrectionFile(const std::string& correctionFile, const std::string& correctionFileOnME)
{
    if (correctionFile.empty())
    {
        wmLog(eError, "Correction file is empty\n");
        return false;
    }

    if (correctionFile.find(correctionFileOnME) != std::string::npos)
    {
        //NOTE No update is needed
        return true;
    }

    if (!m_networkInterface.uploadFilePermanentlyToFlash(correctionFile))
    {
        wmLog(eFatal, "Uploading correction file failed\n");
        return false;
    }

    return true;
}

bool SmartMoveControl::uploadCorrectionFile(const std::string& correctionFile)
{
    if (correctionFile.empty())
    {
        wmLog(eError, "Correction file is empty\n");
        return false;
    }

    auto fileDescriptor = m_networkInterface.openFile(correctionFile);
    if (fileDescriptor == static_cast<int>(FileDescriptorError::Failure))
    {
        wmLog(eError, "Calibration file doesn't exist or cannot be open\n");
        m_networkInterface.closeFile(fileDescriptor);
        return false;
    }

    std::string answer{};
    auto uploadValue = m_networkInterface.transmitMarkingFileXModem(fileDescriptor, answer);    //Maybe XModem?
    wmLog(eError, "Upload return value %d\n", uploadValue);

    if (uploadValue == static_cast<int>(FileUploadError::Failure))
    {
        wmLog(eError, "Upload file %s failed\n", correctionFile);
        m_networkInterface.closeFile(fileDescriptor);
        return false;
    }

    auto closeValue = m_networkInterface.closeFile(fileDescriptor);
    if (closeValue == static_cast<int>(FileDescriptorError::Failure))
    {
        wmLog(eError, "Closing file descriptor failed\n");
        return false;
    }

    return true;
}

}
}
