#pragma once

#include <map>
#include <string>

#include "abstractTaskProcess.h"

namespace precitec
{

namespace scheduler
{

class ResultExcelFileFromProductInstanceTaskProcess : public AbstractTaskProcess
{
public:
    ResultExcelFileFromProductInstanceTaskProcess() = default;
    ~ResultExcelFileFromProductInstanceTaskProcess() override = default;

    virtual std::string name() override;
protected:
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) override;
private:
};

}
}
