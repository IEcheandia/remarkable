/** @defgroup App_Fieldbus App_Fieldbus
 */

/*
 *  AppMain.h for App_Fieldbus
 *
 *  Created on: 25.2.2019
 *      Author: a.egger
 */

#include "AppMain.h"

#include <sys/mman.h>
#include <sys/resource.h>

namespace precitec
{
    using namespace interface;

namespace ethercat
{

AppMain::AppMain()
    :BaseModule(system::module::FieldbusModul)
    ,m_oFieldbus(m_oFieldbusInputsProxy, m_oFieldbusInputsToServiceProxy)
    ,m_oFieldbusOutputsServer( m_oFieldbus )
    ,m_oFieldbusOutputsHandler( &m_oFieldbusOutputsServer )
    ,m_systemStatusServer(m_oFieldbus)
    ,m_systemStatusHandler(&m_systemStatusServer)
{
    {
        ///////////////////////////////////////////////////////
        // Prozess zu einem Realtime Prozess machen
        ///////////////////////////////////////////////////////
        system::raiseRtPrioLimit();
        system::makeThreadRealTime(system::Priority::FieldBus);

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

    m_oFieldbus.StartCyclicTaskThread();
    m_oFieldbus.StartCheckProcessesThread();

    m_oFieldbusOutputsHandler.setRealTimePriority(system::Priority::FieldBus);

    registerPublication(&m_oFieldbusInputsProxy);
    registerPublication(&m_oFieldbusInputsToServiceProxy);
    registerSubscription(&m_oFieldbusOutputsHandler);
    registerSubscription(&m_systemStatusHandler);

    initialize(this);

    ConnectionConfiguration::instance().setInt( pidKeys[FIELDBUS_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
    m_oFieldbus.stopThreads();
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

} // namespace ethercat

} // namespace precitec

