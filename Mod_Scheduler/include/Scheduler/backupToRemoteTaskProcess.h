#pragma once

#include <map>
#include <string>

#include "abstractTaskProcess.h"

namespace precitec
{

namespace scheduler
{

class BackupToRemoteTaskProcess : public AbstractTaskProcess
{
public:
    BackupToRemoteTaskProcess() = default;
    ~BackupToRemoteTaskProcess() override = default;

    virtual std::string name() override;
protected:
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) override;
private:
};

}
}

