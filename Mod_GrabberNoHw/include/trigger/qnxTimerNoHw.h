#pragma once
#include <iostream>


namespace precitec {
namespace trigger {



class QnxTimerNoHw
{
public:

    static bool initTimer();

    static void setTimer(int bRun, long liveTimens);

    static void startThread(void* arg, void *(*func) (void *));

    static int getTimerFd();

    static pthread_t getOurThreadID();

private:

    QnxTimerNoHw();

    static int m_timerFd;

    static pthread_t m_our_thread_id;

    static itimerspec m_itime;
};

}
}
