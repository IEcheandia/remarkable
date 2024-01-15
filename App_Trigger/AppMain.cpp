/** @defgroup App_Trigger App_Trigger
 */

/*
 *  AppMain.h for App_Trigger
 *
 *  Created on: 26.2.2019
 *      Author: a.egger
 */

#include "AppMain.h"

#include <sys/mman.h>
#include <sys/resource.h>

#include "common/systemConfiguration.h"

namespace precitec
{

namespace trigger
{

using namespace interface;

AppMain::AppMain() : BaseModule(system::module::TriggerModul),    // wir brauchen immer die moduleId!!
    m_oTriggerCmdServer( m_oTrigger ),
    m_oTriggerCmdHandler( &m_oTriggerCmdServer ),
    m_oTriggerInterfaceProxy(),
    m_oTrigger(m_oTriggerInterfaceProxy)
{
    {
        ///////////////////////////////////////////////////////
        // Prozess zu einem Realtime Prozess machen
        ///////////////////////////////////////////////////////

        system::raiseRtPrioLimit();

        // adjust limit for memlock
        struct rlimit memLock;
        if (getrlimit(RLIMIT_MEMLOCK, &memLock) == 0)
        {
            memLock.rlim_cur = memLock.rlim_max;
            if (setrlimit(RLIMIT_MEMLOCK, &memLock) != 0)
            {
                wmLog(eDebug, "ERROR: setrlimit for MEMLOCK failed: %s\n", strerror(errno));
                wmLogTr(eError, "QnxMsg.VI.ChangeToRTFailed", "Change to realtime mode failed ! %s\n", "(003)");
            }
        } else
        {
            wmLog(eDebug, "ERROR: getrlimit for MEMLOCK failed: %s\n", strerror(errno));
            wmLogTr(eError, "QnxMsg.VI.ChangeToRTFailed", "Change to realtime mode failed ! %s\n", "(004)");
        }

        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
            wmLog(eDebug, "ERROR: mlockall failed: %s\n", strerror(errno));
            wmLogTr(eError, "QnxMsg.VI.ChangeToRTFailed", "Change to realtime mode failed ! %s\n", "(005)");
        }
        stack_prefault();
    }

    if(SystemConfiguration::instance().getBool("ImageTriggerViaEncoderSignals", false))
    {
        m_oTrigger.StartEncoderAccess();

        registerPublication(&m_oTriggerInterfaceProxy);

        m_oTriggerCmdHandler.setRealTimePriority(system::Priority::FieldBus);

        registerSubscription(&m_oTriggerCmdHandler);
    }

    initialize(this);

    ConnectionConfiguration::instance().setInt( pidKeys[TRIGGER_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
}

int AppMain::init(int argc, char * argv[])
{
    processCommandLineArguments(argc, argv);
    notifyStartupFinished();
    for (int i = 0; i < argc; ++i)
    {
    }
    return 0;
}

void AppMain::stack_prefault(void)
{
    unsigned char dummy[MAX_SAFE_STACK];

    memset(dummy, 0, MAX_SAFE_STACK);
    return;
}

void AppMain::runClientCode()
{
    std::cout << "runClientCode" << std::endl;

    char wait = 0;
    std::cin >> wait;
}

} // namespace trigger

} // namespace precitec

