#include "Scheduler/taskScheduler.h"

#include "Scheduler/eventTrigger.h"
#include "Scheduler/manualTrigger.h"

#include <Poco/Timer.h>

namespace precitec
{
namespace scheduler
{

TaskScheduler::TaskScheduler() : m_updateTime(1000), m_threadPool(new Poco::ThreadPool), m_timer(new Poco::Timer)
{
}

TaskScheduler::~TaskScheduler()
{
    stop();
}

void TaskScheduler::setUpdateTime(uint32_t ms)
{
    m_updateTime = ms;
}

void TaskScheduler::start()
{
    m_timer->setStartInterval(m_updateTime);
    m_timer->setPeriodicInterval(m_updateTime);
    Poco::TimerCallback<TaskScheduler> callback(*this, &TaskScheduler::execute);
    m_timer->start(callback, *m_threadPool.get());
}

void TaskScheduler::stop()
{
    m_timer->stop();
    m_threadPool->stopAll();
}

void TaskScheduler::addSignalManager(const SignalManager &signalManager)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_signalManagers.insert(signalManager);
}

void TaskScheduler::addSignalManagers(const std::set<SignalManager> &signalManagers)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_signalManagers.insert(signalManagers.begin(), signalManagers.end());
}

void TaskScheduler::rewriteSignalManagers(const std::set<SignalManager> &signalManagers)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_signalManagers.clear();
    m_signalManagers.insert(signalManagers.begin(), signalManagers.end());
}


void TaskScheduler::removeSignalManager(const SignalManager &signalManager)
{
    const std::lock_guard<std::mutex> lock(m_mutex);
    m_signalManagers.erase(signalManager);
}

void TaskScheduler::execute(Poco::Timer &timer)
{
    auto jobs = getJobs();
    executeJobs(jobs);
}

const std::set<SignalManager> &TaskScheduler::signalManagers()
{
    return m_signalManagers;
}

const Poco::Timer *TaskScheduler::timer()
{
    return m_timer.get();
}

Poco::ThreadPool *TaskScheduler::threadPool()
{
    return m_threadPool.get();
}

std::vector<JobDescription> TaskScheduler::getJobs()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    std::vector<JobDescription> jobs;
    for (auto &manager : m_signalManagers)
    {
        auto job = manager.generate();
        if (job.signalInfo.signalNumberDuringObservationalPeriod)
        {
            job.task->setSignalInfo(job.signalInfo);
            jobs.emplace_back(std::move(job));
        }
    }
    return jobs;
}

void TaskScheduler::executeJobs(std::vector<JobDescription> &jobs)
{
    for (auto &job : jobs)
    {
        m_threadPool->start(*std::move(job.task).release());
    }
}

void TaskScheduler::schedulerEventFunction(SchedulerEvents event, const std::map<std::string, std::string> &info)
{
    std::lock_guard<std::mutex> lock(m_mutex);
    for (auto &manager : m_signalManagers)
    {
        if (auto eventTrigger = dynamic_cast<EventTrigger*>(manager.trigger()))
        {
            eventTrigger->schedulerEventFunction(event, info);
        }
        else if (auto manualTrigger = dynamic_cast<ManualTrigger*>(manager.trigger()))
        {
            manualTrigger->schedulerEventFunction(event, info);
        }
    }
}

}
}