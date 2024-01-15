/*
 *  AppMain.h for App_Scheduler
 *
 *  Created on: 25.2.2022
 *      Author: a.egger
 */

#pragma once

#include "message/module.h"
#include "module/baseModule.h"

#include "event/schedulerEvents.handler.h"
#include "Scheduler/SchedulerEventsServer.h"

#include "common/connectionConfiguration.h"

#include "Scheduler/eventTrigger.h"
#include "Scheduler/configurationMonitor.h"

namespace precitec
{

namespace scheduler
{

class AppMain : public framework::module::BaseModule
{
public:
    AppMain();
    virtual ~AppMain();

    void runClientCode() override;
    int init(int argc, char* argv[]);

private:
    static std::string mBaseDir;
    static std::string mSettingFolder;
    static std::string mSettingFileName;

    TaskScheduler m_scheduler{};

    SchedulerEventsServer m_schedulerEventsServer;
    TSchedulerEvents<EventHandler> m_schedulerEventsHandler;
    ConfigurationMonitor m_configurationMonitor;
};

} // namespace scheduler

} // namespace precitec

