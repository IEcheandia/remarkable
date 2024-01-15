#pragma once

#include <map>
#include <string>

#include "abstractTaskProcess.h"

namespace precitec
{

namespace scheduler
{

class TestTaskProcess : public AbstractTaskProcess
{
public:
    TestTaskProcess() = default;
    ~TestTaskProcess() override = default;
    virtual std::string name() override;
protected:
    virtual std::vector<std::string> processArguments(const int loggerPipeFds[2]) override;
private:
};

}
}