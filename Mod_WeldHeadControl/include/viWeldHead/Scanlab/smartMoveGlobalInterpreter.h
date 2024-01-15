#pragma once

#include <string>
#include <vector>
#include <memory>

#include "viWeldHead/Scanlab/globalMetaLanguage.h"

class SmartMoveGlobalInterpreterTest;

namespace precitec
{
namespace hardware
{

/**
* Smart move global interpreter generates from an array filled with the global meta language the specific smart move commands to execute the task.
**/
class SmartMoveGlobalInterpreter
{
public:
    SmartMoveGlobalInterpreter();
    ~SmartMoveGlobalInterpreter();

    const std::vector<std::string>& currentCommandSeries() const
    {
        return m_currentGlobalCommandSeries;
    }
    bool isCurrentCommandSeriesEmpty() const
    {
        return m_currentGlobalCommandSeries.empty();
    }
    void reset();
    void translateGlobalCommands(const std::vector<std::shared_ptr<GlobalCommand>>& globalCommands);
    std::vector<std::string> translateGlobalCommand(const std::shared_ptr<GlobalCommand>& globalCommand) const;

private:
    //Job
    void translateGlobalCommand(const std::shared_ptr<GlobalCommand>& globalCommand, std::vector<std::string>& series) const;
    std::string translateProcessJob(const std::shared_ptr<ProcessJob>& processJob) const;
    std::string translateJobSelect(const std::shared_ptr<JobSelect>& jobSelect) const;
    //DriveToPosition
    std::string translateCalibrationFileName() const;
    std::string translatePositionFeedbackX() const;
    std::string translatePositionFeedbackY() const;
    std::string translatePositionCommandX() const;
    std::string translatePositionCommandY() const;
    std::string translateForceEnable() const;
    std::string translateSysTs() const;

    std::string translatePrefixSetOrGet(bool set) const;
    std::string mergeMultipleSetsToOneMultiSet(const std::vector<std::string>& multipleSets);
    bool checkIfOnlySetCommands(const std::vector<std::string>& multipleSets);
    std::string createMultiSetCommand(const std::vector<std::string>& multipleSets);

    std::vector<std::string> m_currentGlobalCommandSeries;
    std::size_t m_sizeForMultiSetMerge{0};

    friend SmartMoveGlobalInterpreterTest;
};

}
}

