#include "Scheduler/SchedulerEventsServer.h"

namespace precitec
{

namespace scheduler
{

SchedulerEventsServer::SchedulerEventsServer(TaskScheduler& taskScheduler):
    m_taskScheduler(taskScheduler)
{
}

SchedulerEventsServer::~SchedulerEventsServer()
{
}

void SchedulerEventsServer::schedulerEventFunction(SchedulerEvents event, const std::map<std::string, std::string> &info)
{
    m_taskScheduler.schedulerEventFunction(event, info);
}

}
}

