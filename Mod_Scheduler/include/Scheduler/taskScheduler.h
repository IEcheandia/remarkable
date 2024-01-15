#pragma once
#include <memory>
#include <mutex>

#include <Poco/Timer.h>
#include <Poco/ThreadPool.h>

#include "signalManager.h"
#include "event/schedulerEvents.handler.h"

#include <set>

namespace precitec
{

using namespace interface;

namespace scheduler
{

class TaskScheduler
{
public:
    explicit TaskScheduler();
    ~TaskScheduler();

    void setUpdateTime(uint32_t ms);
    void start();
    void stop();

    void addSignalManager(const SignalManager &signalManager);
    void addSignalManagers(const std::set<SignalManager> &signalManagers);
    void rewriteSignalManagers(const std::set<SignalManager> &signalManagers);
    void removeSignalManager(const SignalManager &signalManager);

    void schedulerEventFunction(SchedulerEvents event, const std::map<std::string, std::string> &info);

    const std::set<SignalManager> &signalManagers();
    const Poco::Timer *timer();
    Poco::ThreadPool *threadPool();

private:
    void execute(Poco::Timer &timer);
    std::vector<JobDescription> getJobs();
    void executeJobs(std::vector<JobDescription> &jobs);

    uint32_t m_updateTime = 0;
    std::mutex m_mutex;
    std::unique_ptr<Poco::ThreadPool> m_threadPool = nullptr;
    std::set<SignalManager> m_signalManagers;
    std::unique_ptr<Poco::Timer> m_timer = nullptr;
};
}
}