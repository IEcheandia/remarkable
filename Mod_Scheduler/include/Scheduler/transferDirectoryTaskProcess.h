#pragma once

#include <map>
#include <string>

#include "abstractTaskProcess.h"

namespace precitec
{

namespace scheduler
{

// Transmission of a whole directory to a remote ServerApplication
class TransferDirectoryTaskProcess : public AbstractTaskProcess
{
public:
    TransferDirectoryTaskProcess() = default;
    ~TransferDirectoryTaskProcess() override = default;
    virtual std::string name() override;
protected:
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) override;
private:
};

}
}