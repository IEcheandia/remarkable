#pragma once

#include "abstractTask.h"

namespace precitec
{
namespace scheduler
{

class TransferFileTask : public AbstractTask
{
public:
    void run() override;
    const std::string &name() const override;
    AbstractTask *clone() const override;
    ~TransferFileTask() override{};

    bool checkSettings() const override;
    std::set<std::pair<std::string, std::optional<precitec::interface::SchedulerEvents>>> onlySupportedTriggers() const override;

    static const std::string mName;
};

class TransferFileTaskFactory : public AbstractTaskFactory
{
public:
    std::unique_ptr<AbstractTask> make(const std::map<std::string, std::string> &settings) const override;
};

}
}
