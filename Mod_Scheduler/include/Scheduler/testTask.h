#pragma once

#include "abstractTask.h"

namespace precitec
{
namespace scheduler
{

class TestTask : public AbstractTask
{
public:
    void run() override;
    const std::string &name() const override;
    AbstractTask *clone() const override;
    bool checkSettings() const override;

    ~TestTask() override{};

    static const std::string mName;
    static unsigned int callNumber;
};

class TestTaskFactory : public AbstractTaskFactory
{
public:
    std::unique_ptr<AbstractTask> make(const std::map<std::string, std::string> &settings) const override;
};

}
}