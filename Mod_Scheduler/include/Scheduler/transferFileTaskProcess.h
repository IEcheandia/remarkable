#pragma once

#include <map>
#include <string>

#include "abstractTaskProcess.h"

namespace precitec
{

namespace scheduler
{

// Transmission of a file to a remote ServerApplication
class TransferFileTaskProcess : public AbstractTaskProcess
{
public:
    TransferFileTaskProcess() = default;
    ~TransferFileTaskProcess() override = default;

    virtual std::string name() override;
protected:
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) override;
private:
};

}
}