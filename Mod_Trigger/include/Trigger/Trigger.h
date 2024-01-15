/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       2019
 *  @brief      Generates image trigger out of encoder signals
 */

#ifndef TRIGGER_H_
#define TRIGGER_H_

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

#include <vector>

#include "Poco/Mutex.h"

#include "common/triggerContext.h"
#include "common/triggerInterval.h"

#include "event/trigger.proxy.h"

#include "Trigger/encoderAccess.h"

#define MAX_SAFE_STACK  (256 * 1024)

///////////////////////////////////////////////////////////
//
///////////////////////////////////////////////////////////

namespace precitec
{

namespace trigger
{

using namespace interface;

///////////////////////////////////////////////////////////
// typedefs
///////////////////////////////////////////////////////////

class Trigger
{

public:
    Trigger(TTrigger<EventProxy>& p_oTriggerInterfaceProxy);
    virtual ~Trigger(void);

    void StartEncoderAccess(void);
    uint32_t ReadEncoderCounter(void);

    /**
    * Passt das Triggern der Bildaufnahme an den Bildabstand an.
    * @param ids Sensor IDs
    * @param p_rContext TriggerContext
    * @param p_rInterval Interval
    */
    void burst(const std::vector<int>& ids, TriggerContext const& p_rContext, TriggerInterval const& p_rInterval);
    void cancel(int id);

private:
    /**
    * Disables cstates to improve latency. For more information see https://access.redhat.com/articles/65410
    *
    * "Modern CPUs are quite aggressive in their desire to transition into power-saving states (called C-states).
    * Unfortunately, transitioning from power saving states back to fully-powered-up-running state takes time and
    * can introduce undesired application delays when powering on components, refilling caches, etc.
    *
    * Real-time applications can avoid these delays by preventing the system from making C-state transitions.
    * [snip]
    * If more fine-grained control of power saving states is desired, a latency sensitive application may use the
    * Power management Quality of Service (PM QOS) interface, /dev/cpu_dma_latency, to prevent the processor from
    * entering deep sleep states and causing unexpected latencies when exiting deep sleep states.
    * Opening /dev/cpu_dma_latency and writing a zero to it will prevent transitions to deep sleep states while the
    * file descriptor is held open. Additionally, writing a zero to it emulates the idle=poll behavior."
    **/
    void disableCStates();

    uint32_t IRQfromEncoder(uint32_t p_oEncoderCounter);

    /****************************************************************************/

    /****************************************************************************/

    /****************************************************************************/

    /****************************************************************************/

    bool m_oIsImageTriggerViaEncoderSignals;

    int m_cStateFd;

    TTrigger<EventProxy>& m_oTriggerInterfaceProxy;

    Poco::FastMutex m_oMutex;
    TriggerContext m_oContext;
    TriggerInterval m_oInterval;
    uint32_t m_oTriggerDeltaUM;
    uint32_t m_oTriggerDeltaMM;
    uint32_t m_oEncoderDividerValue;
    int m_oImageNoForContext;
    bool m_oBurstIsOn;

    uint32_t m_oCounterOfIRQHangs;
    uint32_t m_oOldEncoderCounter;

    EncoderAccess *m_pEncoderAccess;
};

} // namespace trigger

} // namespace precitec

#endif /* TRIGGER_H_ */

