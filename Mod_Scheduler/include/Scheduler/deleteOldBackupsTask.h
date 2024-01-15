#pragma once

#include "abstractTask.h"

namespace precitec
{
namespace scheduler
{

class DeleteOldBackupsTask : public AbstractTask
{
public:
    void run() override;
    virtual const std::string &name() const override;
    virtual AbstractTask *clone() const override;
    virtual bool checkSettings() const override;

    ~DeleteOldBackupsTask() override{};
    static const std::string mName;
};

class DeleteOldBackupsTaskFactory : public AbstractTaskFactory
{
public:
    std::unique_ptr<AbstractTask> make(const std::map<std::string, std::string> &settings) const override;
};

}
}
