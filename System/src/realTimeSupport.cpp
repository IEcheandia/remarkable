#include "system/realTimeSupport.h"
#include "module/moduleLogger.h"

#include <sys/resource.h>
#include <string.h>
#include <errno.h>
#include <iostream>

namespace precitec
{
namespace system
{

void raiseRtPrioLimit()
{
    struct rlimit rtPrio;
    rtPrio.rlim_cur = uint32_t(Priority::MaxPriority);
    rtPrio.rlim_max = rtPrio.rlim_cur;
    if (setrlimit(RLIMIT_RTPRIO, &rtPrio) != 0)
    {
        std::cout << "ERROR: setrlimit for RTPRIO failed: " << strerror(errno) << std::endl;
        wmLog(eError, "ERROR: setrlimit for RTPRIO failed: %s\n", strerror(errno));
    }
}

void makeThreadRealTime(Priority priority)
{
    makeThreadRealTime(uint32_t(priority));
}

void makeThreadRealTime(Priority priority, pthread_attr_t* pthreadAttr)
{
    // first of all the scheduling policy must be set
    if (pthread_attr_setschedpolicy(pthreadAttr, SCHED_FIFO) != 0)
    {
        char errorBuffer[256];
        wmLog(eDebug, "pthread_attr_setschedpolicy failed (%s)(%s)\n", "001", strerror_r(errno, errorBuffer, sizeof(errorBuffer)));
        wmLogTr(eError, "QnxMsg.VI.SetThreadAttrFail", "Cannot set thread attributes (%s)\n", "002");
    }
    struct sched_param schedParam{};
    schedParam.sched_priority = uint32_t(priority);
    if (pthread_attr_setschedparam(pthreadAttr, &schedParam) != 0)
    {
        char errorBuffer[256];
        wmLog(eDebug, "pthread_attr_setschedparam failed (%s)(%s)\n", "001", strerror_r(errno, errorBuffer, sizeof(errorBuffer)));
        wmLogTr(eError, "QnxMsg.VI.SetThreadAttrFail", "Cannot set thread attributes (%s)\n", "003");
    }
    if (pthread_attr_setinheritsched(pthreadAttr, PTHREAD_EXPLICIT_SCHED) != 0)
    {
        char errorBuffer[256];
        wmLog(eDebug, "pthread_attr_setinheritsched failed (%s)(%s)\n", "001", strerror_r(errno, errorBuffer, sizeof(errorBuffer)));
        wmLogTr(eError, "QnxMsg.VI.SetThreadAttrFail", "Cannot set thread attributes (%s)\n", "004");
    }
}

void makeThreadRealTime(uint32_t priority)
{
    struct sched_param parameters;
    parameters.sched_priority = priority;
    if (sched_setscheduler(0, SCHED_FIFO | SCHED_RESET_ON_FORK, &parameters) == -1)
    {
        std::cout << "ERROR: sched_scheduler failed: " << strerror(errno) << std::endl;
        wmLog(eError, "ERROR: sched_scheduler failed: %s\n", strerror(errno));
    }
}

}
}
