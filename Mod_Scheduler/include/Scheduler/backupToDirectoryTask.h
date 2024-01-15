#pragma once

#include "abstractTask.h"

namespace precitec
{
namespace scheduler
{

class BackupToDirectoryTask : public AbstractTask
{
public:
    void run() override;
    const std::string &name() const override;
    AbstractTask *clone() const override;
    bool checkSettings() const override;
    virtual ~BackupToDirectoryTask() override{};

    static const std::string mName;
};

class BackupToDirectoryTaskFactory : public AbstractTaskFactory
{
public:
    std::unique_ptr<AbstractTask> make(const std::map<std::string, std::string> &settings) const override;
};

}
}
