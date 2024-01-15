#pragma once

#include <map>
#include <string>
#include "module/moduleLogger.h"

#include "abstractTaskProcess.h"

namespace precitec
{
namespace scheduler
{

class ExportProductTaskProcess : public AbstractTaskProcess
{
public:
    ExportProductTaskProcess() = default;
    ~ExportProductTaskProcess() override = default;

    virtual std::string name() override;
protected:
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) override;
private:
};

}
}

