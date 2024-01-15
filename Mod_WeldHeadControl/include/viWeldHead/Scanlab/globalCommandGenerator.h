#pragma once

#include <vector>
#include <string>
#include <memory>

#include "globalMetaLanguage.h"

namespace precitec
{
namespace hardware
{

/**
* Global command generator generates an array filled with the global meta language.
* The input can be a function or global settings which are set at startup or before a contour.
* The global meta language is constructed with different data structures which represents commands which are needed to achieve the task. (Apply function or set property)
**/
class GlobalCommandGenerator
{
public:
    GlobalCommandGenerator();
    ~GlobalCommandGenerator();
    const std::vector<std::shared_ptr<GlobalCommand>>& generatedGlobalCommands() const
    {
        return m_generatedGlobalSettings;
    }
    bool empty() const
    {
        return m_generatedGlobalSettings.empty();
    }
    void reset();
    void addProcessJob(JobRepeats repeats);
    void addJobSelect(int jobID);
    void addSystemOperationMode(SystemOperationModes mode, bool setCommand);
    void addInputMode(InputModes mode, bool setCommand);
    void addAlign(const std::pair<int, int>& positionInBits, bool setCommand);
    void addLaserOnForAlignment(const std::pair<int, int>& laserOnSettings, bool setCommand);
    void addCalibrationFilename();
    void addFocalLength(double focalLengthInMeter, bool setCommand);
    void addScanfieldSize(double scanfieldSize, bool setCommand);
    void addJumpSpeed(double speed, bool setCommand);
    void addMarkSpeed(double speed, bool setCommand);
    void addPositionFeedback();
    void addPositionCommand();
    void addForceEnabled();
    void addSysTs();
    void addLaserPower(double center, double ring = 0.0, bool setCommand = true);
private:
    std::vector<std::shared_ptr<GlobalCommand>> m_generatedGlobalSettings;
};

}
}
