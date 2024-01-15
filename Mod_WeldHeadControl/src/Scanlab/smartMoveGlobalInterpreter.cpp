#include "viWeldHead/Scanlab/smartMoveGlobalInterpreter.h"
#include <sstream>
#include <functional>

using precitec::smartMove::TCPCommand;

namespace precitec
{
namespace hardware
{

static const int MAX_MCI_LINE_LENGTH{80};

SmartMoveGlobalInterpreter::SmartMoveGlobalInterpreter() = default;

SmartMoveGlobalInterpreter::~SmartMoveGlobalInterpreter() = default;

void SmartMoveGlobalInterpreter::reset()
{
    m_currentGlobalCommandSeries.clear();
}

namespace
{

std::string translatePrefix(bool set)
{
    if (set)
    {
        return {"S "};
    }
    return {"G "};
}

template <typename T>
std::string translate(const T& job)
{
    const auto& command = translatePrefix(job.set) + std::string{T::command};
    if (!job.set)
    {
        return command;
    }
    return command + std::to_string(job.value());
}

template <typename T>
std::string translate(const std::shared_ptr<T>& job)
{
    return translate(*(job.get()));
}

}

void SmartMoveGlobalInterpreter::translateGlobalCommands(const std::vector<std::shared_ptr<GlobalCommand>>& globalCommands)
{
    m_currentGlobalCommandSeries.reserve(globalCommands.size());

    for (const auto& element : globalCommands)
    {
        translateGlobalCommand(element, m_currentGlobalCommandSeries);
    }
}

std::vector<std::string> SmartMoveGlobalInterpreter::translateGlobalCommand(const std::shared_ptr<GlobalCommand>& globalCommand) const
{
    std::vector<std::string> ret;
    translateGlobalCommand(globalCommand, ret);
    return ret;
}

void SmartMoveGlobalInterpreter::translateGlobalCommand(const std::shared_ptr<GlobalCommand>& element, std::vector<std::string>& series) const
{
    if (const auto processJob = std::dynamic_pointer_cast<ProcessJob>(element))
    {
        series.push_back(translateProcessJob(processJob));
    }
    if (const auto jobSelect = std::dynamic_pointer_cast<JobSelect>(element))
    {
        series.push_back(translateJobSelect(jobSelect));
    }
    if (const auto systemOperationMode = std::dynamic_pointer_cast<SystemOperationMode>(element))
    {
        series.push_back(translate(systemOperationMode));
    }
    if (const auto inputMode = std::dynamic_pointer_cast<InputMode>(element))
    {
        series.push_back(translate(inputMode));
    }
    if (const auto align = std::dynamic_pointer_cast<Align>(element))
    {
        series.push_back(translate(AlignX{align}));
        series.push_back(translate(AlignY{align}));
    }
    if (const auto laserOnForAlignment = std::dynamic_pointer_cast<LaserOnForAlignment>(element))
    {
        series.push_back(translate(LaserOn{laserOnForAlignment}));
        series.push_back(translate(LaserOnMax{laserOnForAlignment}));
    }
    if (const auto calibrationFilename = std::dynamic_pointer_cast<CalibrationFilename>(element))
    {
        series.push_back(translateCalibrationFileName());
    }
    if (const auto focalLength = std::dynamic_pointer_cast<FocalLength>(element))
    {
        series.push_back(translate(focalLength));
    }
    if (const auto scanfieldSize = std::dynamic_pointer_cast<ScanfieldSize>(element))
    {
        series.push_back(translate(scanfieldSize));
    }
    if (const auto jumpSpeed = std::dynamic_pointer_cast<JumpSpeed>(element))
    {
        series.push_back(translate(jumpSpeed));
    }
    if (const auto markSpeed = std::dynamic_pointer_cast<MarkSpeed>(element))
    {
        series.push_back(translate(markSpeed));
    }
    if (const auto positionFeedback = std::dynamic_pointer_cast<PositionFeedbackBits>(element))
    {
        series.push_back(translatePositionFeedbackX());
        series.push_back(translatePositionFeedbackY());
    }
    if (const auto positionCommand = std::dynamic_pointer_cast<PositionCommandBits>(element))
    {
        series.push_back(translatePositionCommandX());
        series.push_back(translatePositionCommandY());
    }
    if (const auto& forceEnable = std::dynamic_pointer_cast<ForceEnable>(element))
    {
        series.emplace_back(translateForceEnable());
    }
    if (const auto& sysTs = std::dynamic_pointer_cast<SysTs>(element))
    {
        series.emplace_back(translateSysTs());
    }
    if (const auto& power = std::dynamic_pointer_cast<LaserPower>(element))
    {
        series.emplace_back(translate(power));
        // TODO: add ring
    }
}

std::string SmartMoveGlobalInterpreter::translateProcessJob(const std::shared_ptr<ProcessJob>& processJob) const
{
    return std::string{"P "} + std::to_string(processJob->repeats);
}

std::string SmartMoveGlobalInterpreter::translateJobSelect(const std::shared_ptr<JobSelect>& jobSelect) const
{
    return std::string{"J id "} + std::to_string(jobSelect->jobID);
}

std::string SmartMoveGlobalInterpreter::translateCalibrationFileName() const
{
    return translatePrefixSetOrGet(TCPCommand::Get) + std::string{"SFC_NAME"};
}

std::string SmartMoveGlobalInterpreter::translatePositionFeedbackX() const
{
    return translatePrefixSetOrGet(TCPCommand::Get) + std::string{"RT_PFB_X"};
}

std::string SmartMoveGlobalInterpreter::translatePositionFeedbackY() const
{
    return translatePrefixSetOrGet(TCPCommand::Get) + std::string{"RT_PFB_Y"};
}

std::string SmartMoveGlobalInterpreter::translatePositionCommandX() const
{
    return translatePrefixSetOrGet(TCPCommand::Get) + std::string{"RT_CMD_X"};
}
std::string SmartMoveGlobalInterpreter::translatePositionCommandY() const
{
    return translatePrefixSetOrGet(TCPCommand::Get) + std::string{"RT_CMD_Y"};
}

std::string SmartMoveGlobalInterpreter::translateForceEnable() const
{
    return translatePrefixSetOrGet(TCPCommand::Set) + std::string("FLOW_FORCEEN 1");
}

std::string SmartMoveGlobalInterpreter::translateSysTs() const
{
    return translatePrefixSetOrGet(TCPCommand::Get) + std::string("SYS_TS");
}

std::string SmartMoveGlobalInterpreter::translatePrefixSetOrGet(bool set) const
{
    return translatePrefix(set);
}

std::string SmartMoveGlobalInterpreter::mergeMultipleSetsToOneMultiSet(const std::vector<std::string>& multipleSets)
{
    if (!checkIfOnlySetCommands(multipleSets))
    {
        return {};
    }

    m_sizeForMultiSetMerge -= ((multipleSets.size() - 1) + multipleSets.front().size());            //Remove S or G from the size because they will be erased but the first one. The first set is used in the strint ctor.

    const auto& multiSetCommand = createMultiSetCommand(multipleSets);

    return multiSetCommand;
}

bool SmartMoveGlobalInterpreter::checkIfOnlySetCommands(const std::vector<std::string>& multipleSets)
{
    m_sizeForMultiSetMerge = 0;

    for (const auto& setCommand : multipleSets)
    {
        if (setCommand.front() == 'G')
        {
            return false;
        }
        m_sizeForMultiSetMerge += setCommand.size();
    }

    return true;
}

std::string SmartMoveGlobalInterpreter::createMultiSetCommand(const std::vector<std::string>& multipleSets)
{
    auto multiSetCommand{multipleSets.front()};
    multiSetCommand.reserve(m_sizeForMultiSetMerge);

    for (std::size_t i = 1; i < multipleSets.size(); i++)
    {
        auto setCommand = multipleSets.at(i);
        setCommand.erase(setCommand.begin());
        multiSetCommand += setCommand;
    }

    if (multiSetCommand.size() >= MAX_MCI_LINE_LENGTH)
    {
        return {};
    }

    return multiSetCommand;
}

}
}
