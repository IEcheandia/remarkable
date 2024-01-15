#include "viWeldHead/Scanlab/globalCommandGenerator.h"

namespace precitec
{
namespace hardware
{

GlobalCommandGenerator::GlobalCommandGenerator() = default;

GlobalCommandGenerator::~GlobalCommandGenerator() = default;

void GlobalCommandGenerator::reset()
{
    m_generatedGlobalSettings.clear();
}

void GlobalCommandGenerator::addProcessJob(JobRepeats repeats)
{
    std::shared_ptr<ProcessJob> processJob(new ProcessJob);
    processJob->repeats = static_cast<int>(repeats);
    m_generatedGlobalSettings.push_back(std::move(processJob));
}

void GlobalCommandGenerator::addJobSelect(int jobID)
{
    std::shared_ptr<JobSelect> jobSelect(new JobSelect);
    jobSelect->jobID = jobID;
    m_generatedGlobalSettings.push_back(std::move(jobSelect));
}

void GlobalCommandGenerator::addSystemOperationMode(SystemOperationModes mode, bool setCommand)
{
    std::shared_ptr<SystemOperationMode> systemOperationMode(new SystemOperationMode);
    systemOperationMode->sysOpMode = mode;
    systemOperationMode->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(systemOperationMode));
}

void GlobalCommandGenerator::addInputMode(InputModes mode, bool setCommand)
{
    std::shared_ptr<InputMode> inputMode(new InputMode);
    inputMode->inputMode = mode;
    inputMode->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(inputMode));
}

void GlobalCommandGenerator::addAlign(const std::pair<int, int>& positionInBits, bool setCommand)
{
    std::shared_ptr<Align> align(new Align);
    align->x = positionInBits.first;
    align->y = positionInBits.second;
    align->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(align));
}

void GlobalCommandGenerator::addLaserOnForAlignment(const std::pair<int, int>& laserOnSettings, bool setCommand)
{
    std::shared_ptr<LaserOnForAlignment> laserOnForAlignment(new LaserOnForAlignment);
    laserOnForAlignment->laserOn = laserOnSettings.first;
    laserOnForAlignment->laserOnMax = laserOnSettings.second;
    laserOnForAlignment->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(laserOnForAlignment));
}

void GlobalCommandGenerator::addCalibrationFilename()
{
    std::shared_ptr<CalibrationFilename> calibrationFile(new CalibrationFilename);
    calibrationFile->set = false;
    m_generatedGlobalSettings.push_back(std::move(calibrationFile));
}

void GlobalCommandGenerator::addFocalLength(double focalLengthInMeter, bool setCommand)
{
    std::shared_ptr<FocalLength> focalLength(new FocalLength);
    focalLength->focalLength = focalLengthInMeter;
    focalLength->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(focalLength));
}

void GlobalCommandGenerator::addScanfieldSize(double scanfieldSize, bool setCommand)
{
    std::shared_ptr<ScanfieldSize> scanfield(new ScanfieldSize);
    scanfield->scanfieldSize = scanfieldSize;
    scanfield->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(scanfield));
}

void GlobalCommandGenerator::addJumpSpeed(double speed, bool setCommand)
{
    std::shared_ptr<JumpSpeed> jumpSpeed(new JumpSpeed);
    jumpSpeed->speed = speed;
    jumpSpeed->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(jumpSpeed));
}

void GlobalCommandGenerator::addMarkSpeed(double speed, bool setCommand)
{
    std::shared_ptr<MarkSpeed> markSpeed(new MarkSpeed);
    markSpeed->speed = speed;
    markSpeed->set = setCommand;
    m_generatedGlobalSettings.push_back(std::move(markSpeed));
}

void GlobalCommandGenerator::addPositionFeedback()
{
    std::shared_ptr<PositionFeedbackBits> positionFeedback(new PositionFeedbackBits);
    m_generatedGlobalSettings.push_back(std::move(positionFeedback));
}

void GlobalCommandGenerator::addPositionCommand()
{
    std::shared_ptr<PositionCommandBits> positionCommand(new PositionCommandBits);
    m_generatedGlobalSettings.push_back(std::move(positionCommand));
}

void GlobalCommandGenerator::addForceEnabled()
{
    m_generatedGlobalSettings.emplace_back(std::make_shared<ForceEnable>());
}

void GlobalCommandGenerator::addSysTs()
{
    m_generatedGlobalSettings.emplace_back(std::make_shared<SysTs>());
}

void GlobalCommandGenerator::addLaserPower(double center, double ring, bool setCommand)
{
    auto power = std::make_shared<LaserPower>();
    power->center = center;
    power->ring = ring;
    power->set = setCommand;
    m_generatedGlobalSettings.emplace_back(std::move(power));
}

}
}
