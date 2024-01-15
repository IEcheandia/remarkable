/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Generates image trigger out of encoder signals
 */

#include <time.h>
#include <unistd.h>
#include <errno.h>
#include <sched.h>
#include <sys/time.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#include "event/sensor.h"
#include "Trigger/Trigger.h"

#include "module/moduleLogger.h"

#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"

namespace precitec
{

namespace trigger
{

using namespace interface;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

Trigger::Trigger(TTrigger<EventProxy>& p_oTriggerInterfaceProxy):
        m_oIsImageTriggerViaEncoderSignals(false),
        m_cStateFd(-1),
        m_oTriggerInterfaceProxy(p_oTriggerInterfaceProxy),
        m_oTriggerDeltaUM(5000),
        m_oTriggerDeltaMM(5),
        m_oEncoderDividerValue(100),
        m_oImageNoForContext(0),
        m_oBurstIsOn(false),
        m_oCounterOfIRQHangs(0),
        m_oOldEncoderCounter(100),
        m_pEncoderAccess(nullptr)
{
    // SystemConfig Switch for ImageTriggerViaEncoderSignals
    m_oIsImageTriggerViaEncoderSignals = SystemConfiguration::instance().getBool("ImageTriggerViaEncoderSignals", false);
    wmLog(eDebug, "ImageTriggerViaEncoderSignals (bool): %d\n", m_oIsImageTriggerViaEncoderSignals);

    ///////////////////////////////////////////////////////
    // Inits
    ///////////////////////////////////////////////////////

    disableCStates();
}

///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

Trigger::~Trigger(void)
{
    if(m_pEncoderAccess != nullptr)
    {
        delete m_pEncoderAccess;
    }

    // enable cstates
    if (m_cStateFd != -1)
    {
        close(m_cStateFd);
    }
}

///////////////////////////////////////////////////////////
// private members
///////////////////////////////////////////////////////////

void Trigger::disableCStates()
{
    struct stat s;
    if (stat("/dev/cpu_dma_latency", &s) == -1) {
        wmLog(eDebug, "ERROR: Could not stat /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(001)");
        return;
    }

    m_cStateFd = open("/dev/cpu_dma_latency", O_RDWR);
    if (m_cStateFd == -1) {
        wmLog(eDebug, "ERROR: Failed to open /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(002)");
        return;
    }

    const int32_t target = 0;
    if (write(m_cStateFd, &target, sizeof(target)) < 1) {
        wmLog(eDebug, "ERROR: Failed writing to /dev/cpu_dma_latency: %s\n", strerror(errno));
        wmLogTr(eError, "QnxMsg.VI.ChangeLatFailed", "Change of latency failed ! %s\n", "(003)");
        close(m_cStateFd);
        return;
    }
    wmLog(eDebug, "Adjusted /dev/cpu_dma_latency to have lower latency\n");
}

void Trigger::StartEncoderAccess(void)
{
    m_pEncoderAccess = new EncoderAccess();
    if (m_pEncoderAccess != nullptr)
    {
        usleep(100*1000);
        m_pEncoderAccess->test();

        usleep(100*1000);
        m_pEncoderAccess->initEncoder();

        m_pEncoderAccess->connectCallback([&](uint32_t i) { return IRQfromEncoder(i); });
    }
    else
    {
        wmLog(eDebug, "was not able to create EncoderAccess !\n");
        wmFatal(eAxis, "QnxMsg.VI.InitEncoderFault", "Problem while initializing encoder access ! %s\n", "(001)");
    }
}

uint32_t Trigger::ReadEncoderCounter(void)
{
    if (m_pEncoderAccess != nullptr)
    {
        return m_pEncoderAccess->readEncoder();
    }
    return 0;
}

uint32_t Trigger::IRQfromEncoder(uint32_t p_oEncoderCounter)
{
    if (m_oBurstIsOn)
    {
        m_oContext.setImageNumber(m_oImageNoForContext++);
        m_oTriggerInterfaceProxy.trigger(m_oContext);
wmLog(eDebug, "Sent ImageTrigger ! (count: %u) (image: %d)\n", p_oEncoderCounter, m_oContext.imageNumber());
    }

    uint32_t oDiff = p_oEncoderCounter - m_oOldEncoderCounter;
    if ((oDiff < (m_oEncoderDividerValue - 10)) || (oDiff > (m_oEncoderDividerValue + 10)))
    {
        m_oCounterOfIRQHangs++;
        wmLog(eDebug, "IRQ hanger ! (now: %u, last: %u, hangs: %u)\n", p_oEncoderCounter, m_oOldEncoderCounter, m_oCounterOfIRQHangs);
        wmLogTr(eWarning, "QnxMsg.VI.TrigIRQHang", "Problem with encoder trigger: trigger is lait\n");
    }
    m_oOldEncoderCounter = p_oEncoderCounter;

    return 0;
}

//*****************************************************************************
//* triggerCmd Interface (Event)                                              *
//*****************************************************************************

/**
 * Passt das Triggern der Bildaufnahme an den Bildabstand an.
 * @param ids Sensor IDs
 * @param p_rContext TriggerContext
 * @param p_rInterval Interval
 */
void Trigger::burst(const std::vector<int>& ids, TriggerContext const& p_rContext, TriggerInterval const& p_rInterval)
{
    m_oBurstIsOn = true;

    if (m_oIsImageTriggerViaEncoderSignals)
    {
        Poco::FastMutex::ScopedLock lock(m_oMutex);
        m_oContext = p_rContext;
        m_oInterval = p_rInterval;

        wmLog(eDebug, "Trigger::burst: Distns: %d, nbTrig: %d, Delta: %d, state: %d\n", m_oInterval.triggerDistance(), m_oInterval.nbTriggers(), m_oInterval.triggerDelta(), m_oInterval.state());

        // start image trigger, dependent from encoder position, only if it is a burst out of automatic mode
        if (m_oInterval.state() != workflow::eAutomaticMode)
        {
            wmLog(eDebug, "Trigger::burst : kein AutomaticMode -> return\n");
            return;
        }

        m_oTriggerDeltaUM = m_oInterval.triggerDelta();
        m_oTriggerDeltaMM = m_oTriggerDeltaUM / 1000;
        if ((m_oTriggerDeltaUM % 1000) != 0)
        {
            wmLogTr(eError, "QnxMsg.VI.TrigNotMM", "Trigger Distance is not on a millimeter boundary !\n");
        }
        m_oImageNoForContext = 0;

        m_oEncoderDividerValue = m_oTriggerDeltaMM * 100; // this is valid for 100.000 pulses per meter, after quadrature decoding
        m_oOldEncoderCounter = 0;
        m_pEncoderAccess->writeEncDivider (m_oEncoderDividerValue);
        m_pEncoderAccess->start_encoder_interrupt();

        m_oContext.setImageNumber(m_oImageNoForContext++);
        m_oTriggerInterfaceProxy.trigger(m_oContext);
wmLog(eDebug, "Sent ImageTrigger ! (count: %u) (image: %d)\n", 0, m_oContext.imageNumber());
    }
} //release lock

void Trigger::cancel(int id)
{
    if (m_oIsImageTriggerViaEncoderSignals)
    {
        Poco::FastMutex::ScopedLock lock(m_oMutex);
        m_oContext = TriggerContext(0,0,0);
        m_oInterval = TriggerInterval(0,0);

        m_pEncoderAccess->stop_encoder_interrupt();
        m_oImageNoForContext = 0;

        wmLog(eDebug, "m_oCounterOfIRQHangs: %u\n", m_oCounterOfIRQHangs);
    }
    m_oBurstIsOn = false;
}

} // namespace trigger

} // namespace precitec

