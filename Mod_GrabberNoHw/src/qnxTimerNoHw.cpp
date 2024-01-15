#include <system/timer.h>
#include <sys/prctl.h>
#include <sys/timerfd.h>
#include <cstring>
#include <module/moduleLogger.h>
#include "../include/trigger/qnxTimerNoHw.h"

namespace precitec {
namespace trigger {


int QnxTimerNoHw::m_timerFd = -1;
pthread_t QnxTimerNoHw::m_our_thread_id = 0;
itimerspec QnxTimerNoHw::m_itime{};

QnxTimerNoHw::QnxTimerNoHw()
{

}

bool QnxTimerNoHw::initTimer()
{
#ifdef __linux__
    m_timerFd = timerfd_create(CLOCK_MONOTONIC, TFD_CLOEXEC);
    if (m_timerFd == -1)
    {
        std::cout << "Creating timerfd failed with: " << strerror(errno) << std::endl;
    }
#endif
    return true;
}


/// Start or stop the timer
void QnxTimerNoHw::setTimer(int bRun, long liveTimens)
{
    long sec  = liveTimens/1000000000;
    long nsec = liveTimens%1000000000;
    std::cout << "softtrigger timer sec: " << sec << " , " << nsec << std::endl;
    m_itime.it_value.tv_sec = 0;
    m_itime.it_value.tv_nsec = bRun * 100000; //liveTimens;
    m_itime.it_interval.tv_sec  = sec;
    m_itime.it_interval.tv_nsec = bRun * nsec; //liveTimens;
#ifdef __QNX__
    timer_settime(timer_id, 0, &m_itime, NULL);
#endif
    timerfd_settime(m_timerFd, 0, &m_itime, NULL);
}


/// starte thread mit einem message receive
void QnxTimerNoHw::startThread(void* arg,  void *(*func) (void *))
{
    // Init the timer ...
    initTimer();
    if(m_our_thread_id == 0)
    {
        std::cout << "timer thread ID ist NULL..." << std::endl;
    }
    else
    {
        std::cout << "thread ID ist: " << m_our_thread_id << std::endl;
    }

    std::cout << "Creating thread now..." << std::endl;
    pthread_create(&m_our_thread_id, NULL, func , (void*)arg);
    std::cout << "Thread was created..." << std::endl;
}


int QnxTimerNoHw::getTimerFd()
{
     return m_timerFd;
}

pthread_t QnxTimerNoHw::getOurThreadID()
{
    return m_our_thread_id;
}

}
}
