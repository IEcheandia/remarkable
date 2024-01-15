
#include <Poco/Timer.h>
#include <system/timer.h>
#include <system/realTimeSupport.h>

#include <common/product.h>
#include <message/device.h>
#include <message/device.server.h>
#include <message/device.proxy.h>
#include <module/moduleLogger.h>

#include <sys/prctl.h>
#include <sys/timerfd.h>
#include <pthread.h>
#include <sched.h>
#include <unistd.h>
#include <semaphore.h>

#include "../include/trigger/triggerControllerNoHw.h"


#define __logTimer 0


namespace precitec {
namespace trigger {
    
using Poco::SharedPtr;
using Poco::Timer;
using Poco::TimerCallback;
using Poco::Thread;

static void* messageRcvThread ( void* triggerController);

typedef union {
#ifdef __QNX__
    struct _pulse   pulse;
#else
    int pulse;
#endif
} my_message_t;


TriggerControllerNoHw::TriggerControllerNoHw(grabber::TriggerServerNoHw* triggerServer) :
      m_triggerServer(triggerServer)
    , m_endless(false)
    , m_triggerPerInterval(0)
    , m_triggerDistance(0)
    , m_imagePerInterval(0)
    , m_triggerStarted(false)

{
}


void TriggerControllerNoHw::singleshot(const std::vector<int>& sensorIds, TriggerContext const& context)
{
    if (!m_triggerServer->isSimulationStation())
    {
        setEvent(context);
    }
    else
    {
        m_triggerServer->trigger(sensorIds, context);
    }
}

void TriggerControllerNoHw::setTriggerSource(precitec::interface::TriggerSource source)
{
    if(QnxTimerNoHw::getOurThreadID() == 0)
    {
        QnxTimerNoHw::startThread(this, &messageRcvThread);
    }
}

void TriggerControllerNoHw::burst(TriggerContext const& context, TriggerInterval const& interval)
{
    m_context  = context;
    m_interval = interval;
    m_endless  = interval.nbTriggers() <= 0;

    m_triggerPerInterval = m_interval.nbTriggers();
    m_triggerDistance = m_interval.triggerDistance();
    m_imagePerInterval = 0;

    setupTrigger(m_triggerDistance, m_triggerPerInterval);
}

void TriggerControllerNoHw::burstAgain(unsigned int triggerDistanceSave, unsigned int nbTriggersSave)
{
    setupTrigger(triggerDistanceSave, nbTriggersSave );
}

int TriggerControllerNoHw::stop()
{
    std::cout << "trigger Controller - stop trigger" << std::endl;
    int status = 0;

#if defined __QNX__ || defined __linux__
    if(m_triggerStarted)
    {
        std::cout << "trigger controller timer laeuft: stoppen " << std::endl;
        QnxTimerNoHw::setTimer(0,0);
        m_triggerStarted = false;
        status = 1;
    }
    else
    {
        std::cout<<"trigger controller timer steht: nicht mehr stoppen "<<std::endl;
    }
#else
    if (m_pTimer.get() != NULL)
    {
        m_pTimer->restart(0);
        std::cout<<"timer stop"<<std::endl;
    }
#endif
    return(status);
}


void TriggerControllerNoHw::onTrigger(void)
{
    static system::Timer timer("TriggerTimer");
    timer.restart();
    ++m_imagePerInterval;

#if __logTimer
    std::cout <<"onTrigger: "<<timer<<" TriggerController: " << m_context.imageNumber()  << std::endl; //Debug Ausgabe
    std::cout <<"m_imagePerInterval: "<<m_imagePerInterval <<" m_oTriggerPerInterval: " << m_triggerPerInterval  << std::endl;
#endif

    setEvent(m_context);
    m_context.setImageNumber(m_context.imageNumber() + 1);

    if (!m_endless && m_imagePerInterval >= m_triggerPerInterval)
    {
        stop();
    }

}

#if defined __QNX__ || defined __linux__
void TriggerControllerNoHw::onTimerQnx(void)
{
    m_triggerTimer.restart();
#if __logTimer
    std::cout<<"onTimerQnx "<< m_triggerTimer<<std::endl;
    std::cout<<std::endl;
#endif
    onTrigger();
}
#endif


grabber::ImageFromDiskParametersNoHw TriggerControllerNoHw::getImageFromDiskHWROI() const
{
    return m_triggerServer->getImageFromDiskHWROI(m_context);
}


void TriggerControllerNoHw::setupTrigger(unsigned int triggerDistance, unsigned int nbTriggers )
{
    m_triggerTimer.start();
    if(!m_triggerStarted)
    {
        std::cout<<"trigger controller timer steht: timer starten "<<std::endl;
         QnxTimerNoHw::setTimer(1, triggerDistance);
        m_triggerStarted = true;
    }
    else
    {
        std::cout<<"trigger controller timer laeuft schon: timer nicht mehr starten "<<std::endl;
    }

    if(m_triggerStarted == false)
    {
        std::cout<<"trigger controller timer steht: timer starten "<<std::endl;
        //m_qnxSoftTimer.setTimer(1, triggerDistance ); // qnx timer setzen
        m_triggerStarted = true;                      // flag - timer gestartet
    }
    else
        std::cout<<"trigger controller timer laeuft schon: timer nicht mehr starten "<<std::endl;

}


void TriggerControllerNoHw::setEvent(TriggerContext const& context)
{
    poco_assert(m_triggerServer);
    m_triggerServer->trigger(context);
}


void TriggerControllerNoHw::setEvent(TriggerContext const& context, TriggerInterval const& interval)
{
    poco_assert(m_triggerServer);
    m_triggerServer->trigger(context, interval);
}


#if defined __QNX__ || defined __linux__

/// QNX timer thread ...
static void* messageRcvThread ( void* triggerController)
{
    prctl(PR_SET_NAME, "trigger");
    system::makeThreadRealTime(system::Priority::Sensors);
    TriggerControllerNoHw *trigger = ((TriggerControllerNoHw*) triggerController);
    bool loop = true;

    while(loop)
    {
#ifdef __QNX__
        rcvid = MsgReceive(chid, &msg, sizeof(msg), NULL);
        if (rcvid == 0)
        {
            if (msg.pulse.code == MY_PULSE_CODE)
            {
                trigger->onTimerQnx();
            }
            else
            {
                loop = false;
            }
        }
#endif
#ifdef __linux__
        uint64_t expired = 0;
        const std::size_t size = read(QnxTimerNoHw::getTimerFd(), &expired, sizeof(uint64_t));
        if (size != sizeof(uint64_t))
        {
            std::cout << "Failed to read timerFd" << std::endl;
            continue;
        }
        if (expired != 1)
        {
            std::cout << "Timer expired " << expired << " times" << std::endl;
        }
        trigger->onTimerQnx();
#endif
        }
    return nullptr;;
}
#endif

void TriggerControllerNoHw::resetImageNumber()
{
    m_context.setImageNumber(0);
}


void TriggerControllerNoHw::resetImageFromDiskNumber()
{
    m_triggerServer->resetImageFromDiskNumber();
}

void TriggerControllerNoHw::setTestImagesPath(std::string const& testImagesPath)
{
    m_triggerServer->setTestImagesPath(testImagesPath);
}

void TriggerControllerNoHw::setTestProductInstance(const Poco::UUID &productInstance)
{
    m_triggerServer->setTestProductInstance(productInstance);
}

void TriggerControllerNoHw::setSimulation(bool onoff)
{
    m_triggerServer->setSimulation(onoff);
}

void TriggerControllerNoHw::setTestImagesProductInstanceMode(bool set)
{
    m_triggerServer->setTestImagesProductInstanceMode(set);
}

}
}

