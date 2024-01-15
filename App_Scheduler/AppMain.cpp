/** @defgroup App_Scheduler App_Scheduler
 */

/*
 *  AppMain.cpp for App_Scheduler
 *
 *  Created on: 25.2.2022
 *      Author: a.egger
 */

#include "AppMain.h"
#include "Scheduler/signalManagerLoader.h"
#include <fstream>

namespace precitec
{

namespace scheduler
{

using namespace interface;

std::string AppMain::mBaseDir = getenv("WM_BASE_DIR");
std::string AppMain::mSettingFolder = "config";
std::string AppMain::mSettingFileName = "scheduler.json";

AppMain::AppMain() : BaseModule(system::module::SchedulerModul),    // we always need moduleId!!
    m_scheduler(),
    m_schedulerEventsServer(m_scheduler),
    m_schedulerEventsHandler(&m_schedulerEventsServer),
    m_configurationMonitor(m_scheduler, mBaseDir + "/" + mSettingFolder, mSettingFileName)
{
   registerSubscription(&m_schedulerEventsHandler);

    initialize(this);

    ConnectionConfiguration::instance().setInt(pidKeys[SCHEDULER_KEY_INDEX], getpid()); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
}

int AppMain::init(int argc, char * argv[])
{
    processCommandLineArguments(argc, argv);
    notifyStartupFinished();
    return 0;
}

void AppMain::runClientCode()
{
    std::cout << "runClientCode" << std::endl;

    std::ifstream jsonSettingFile(mBaseDir + "/" + mSettingFolder + "/" + mSettingFileName);
    if (!jsonSettingFile.bad())
    {
        m_scheduler.rewriteSignalManagers(SignalManagerLoader::load(jsonSettingFile));
        m_scheduler.start();
        m_configurationMonitor.monitor();
    }
    char wait = 0;
    std::cin >> wait;
}

} // namespace scheduler
} // namespace precitec

