/**
 *  @file
 *  @copyright   Precitec Vision GmbH & Co. KG
 *  @author      EA
 *  @date        2019
 *  @brief       Trigger command server.
 */

#ifndef TRIGGERCMDSERVER_H_
#define TRIGGERCMDSERVER_H_

#include "event/triggerCmd.handler.h"
#include "event/sensor.h"
#include "Trigger.h"

namespace precitec
{

namespace trigger
{

    using namespace interface;

    /**
     * TriggerCmdServer
     **/
    class TriggerCmdServer : public TTriggerCmd<AbstractInterface>
    {
public:
        /**
        * Ctor.
        * @param _service Service
        * @return void
        **/
        TriggerCmdServer(Trigger& rTrigger);
        virtual ~TriggerCmdServer();
public:

        /**
        * Einzelner Trigger kann sofort abgesetzt werden
        * @param ids
        * @param p_rContext
        */
        virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& p_rContext)
        {
            const auto oSensorIdIsAnImageId = [](int p_oSensorId)
            {
                return p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax;
            };

            if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnImageId) == true)
            {
                return; // this is not the appropriate receiver - abort
            }

            TriggerInterval oInterval( TriggerInterval::mSec * 10, 1 );
            m_rTrigger.burst(p_rSensorIds, p_rContext, oInterval);
        }

        /**
        * Initalisiert und startet die Triggerquelle ueber mehrere Abschnitte
        * @param ids
        * @param p_rContext
        * @param source
        * @param p_rInterval
        **/
        virtual void burst(const std::vector<int>& p_rSensorIds, TriggerContext const& p_rContext, int p_oSource, TriggerInterval const& p_rInterval)
        {
            const auto oSensorIdIsAnImageId = [](int p_oSensorId)
            {
                return p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax;
            };

            if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnImageId) == true)
            {
                return; // this is not the appropriate receiver - abort
            }

            m_rTrigger.burst(p_rSensorIds, p_rContext, p_rInterval);
        }

        /**
        * Unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
        * @param ids
        **/
        virtual void cancel(const std::vector<int>& p_rSensorIds)
        {
            m_rTrigger.cancel(p_rSensorIds.front());
        }

private:
        Trigger &m_rTrigger;
    };

} // namespace trigger

} // namespace precitec

#endif /* TRIGGERCMDSERVER_H_*/

