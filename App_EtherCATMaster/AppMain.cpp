/** @defgroup App_EtherCATMaster App_EtherCATMaster
 */

/*
 *  AppMain.h for App_EtherCATMaster
 *
 *  Created on: 20.11.2016
 *      Author: a.egger
 */

#include "AppMain.h"

#include <sys/mman.h>
#include <sys/resource.h>

using precitec::ethercat::EtherCATMaster;

namespace precitec
{
	using namespace interface;

namespace ethercat
{

#define DEBUG_INPUT 0

AppMain::AppMain()
    :BaseModule(system::module::EtherCATMasterModul)
    ,m_oEtherCATMaster(m_oEthercatInputsProxy, m_oEthercatInputsToServiceProxy)
    ,m_oEthercatOutputsServer(m_oEtherCATMaster)
    ,m_oEthercatOutputsHandler(&m_oEthercatOutputsServer)
    ,m_systemStatusServer(m_oEtherCATMaster)
    ,m_systemStatusHandler(&m_systemStatusServer)
{
    {
        system::raiseRtPrioLimit();
        system::makeThreadRealTime(system::Priority::FieldBus);

        // adjust limit for memlock
        struct rlimit memLock;
        if (getrlimit(RLIMIT_MEMLOCK, &memLock) == 0)
        {
            memLock.rlim_cur = memLock.rlim_max;
            if (setrlimit(RLIMIT_MEMLOCK, &memLock) != 0)
            {
                printf("ERROR: setrlimit for MEMLOCK failed: %s\n", strerror(errno));
            }
        } else
        {
            printf("ERROR: getrlimit for MEMLOCK failed: %s\n", strerror(errno));
        }

        if (mlockall(MCL_CURRENT | MCL_FUTURE) == -1)
        {
            printf("ERROR: mlockall failed: %s\n", strerror(errno));
        }
        stack_prefault();
    }

    m_oEtherCATMaster.StartDebugDataThread();
    m_oEtherCATMaster.StartCycleTaskThread();
    m_oEtherCATMaster.StartCheckProcessesThread();

    m_oEthercatOutputsHandler.setRealTimePriority(system::Priority::FieldBus);

	registerPublication(&m_oEthercatInputsProxy);
    registerPublication(&m_oEthercatInputsToServiceProxy);
	registerSubscription(&m_oEthercatOutputsHandler);
    registerSubscription(&m_systemStatusHandler);

	initialize(this);

#if DEBUG_INPUT
    SetTerminalMode();
#endif

ConnectionConfiguration::instance().setInt( pidKeys[ECATMASTER_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
#if DEBUG_INPUT
    ResetTerminalMode();
#endif

    m_oEtherCATMaster.stopThreads();
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

void AppMain::ResetTerminalMode(void)
{
    tcsetattr(0, TCSANOW, &m_oTermiosBackup);
}

void AppMain::SetTerminalMode(void)
{
    struct termios oTermiosNew;
    
    tcgetattr(0, &m_oTermiosBackup);
    memcpy(&oTermiosNew, &m_oTermiosBackup, sizeof(oTermiosNew));

//    atexit(ResetTerminalMode);
    cfmakeraw(&oTermiosNew);
    oTermiosNew.c_oflag |= OPOST;
    tcsetattr(0, TCSANOW, &oTermiosNew);
}

int AppMain::kbhit(void)
{
    struct timeval tv = {0L, 0L};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int AppMain::getch(void)
{
    int retVal;
    unsigned char inputChar;
    retVal = read(0, &inputChar, sizeof(inputChar));
    if (retVal < 0)
    {
        return retVal;
    }
    else
    {
        return inputChar;
    }
}

void AppMain::readKeyboard(void)
{
    int inputChar;

    if (kbhit())
    {
        inputChar = getch();
        switch(inputChar)
        {
            case 0x0D:
                printf("\n");
                break;
            case '1':
                m_oEtherCATMaster.resetDebugFileVars();
                break;
            default:
                break;
        }
    }
}

void AppMain::runClientCode()
{
    std::cout << "runClientCode" << std::endl;

#if DEBUG_INPUT
    while (true)
    {
        readKeyboard();
        usleep(100 * 1000);
    }
#else
    char wait = 0;
    std::cin >> wait;
#endif
}

} // namespace ethercat

} // namespace precitec

