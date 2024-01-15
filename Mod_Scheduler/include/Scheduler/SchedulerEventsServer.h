#pragma once

#include "event/schedulerEvents.handler.h"
#include "taskScheduler.h"

namespace precitec
{

using namespace interface;

namespace scheduler
{

/**
 * SchedulerEventsServer
 **/
class SchedulerEventsServer : public TSchedulerEvents<AbstractInterface>
{
public:
    SchedulerEventsServer(TaskScheduler &eventTrigger);
    virtual ~SchedulerEventsServer();

    void schedulerEventFunction(SchedulerEvents event,
                                const std::map<std::string, std::string> &info) override;

private:
    TaskScheduler &m_taskScheduler;
};

} // namespace scheduler

} // namespace precitec
