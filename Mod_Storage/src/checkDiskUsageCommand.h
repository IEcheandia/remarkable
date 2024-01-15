#pragma once
#include "videoRecorder/baseCommand.h"

#include <functional>
#include <string>

namespace precitec
{
namespace storage
{

class CheckDiskUsageCommand : public vdr::BaseCommand
{
public:
    CheckDiskUsageCommand(const std::string &fileSystemPath, double relativeMaxDiskUsage, const std::function<void(double)> &callbackForDiskUsageReached);
    void execute() override;

private:
    std::string m_fileSystemPath;
    double m_relativeMaxDiskUsage;
    std::function<void(double)> m_callbackForDiskUsageReached;
};

}
}
