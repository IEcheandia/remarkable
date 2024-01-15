/**
 *     @file
 *     @copyright    Precitec Vision GmbH & Co. KG
 *     @author       EA
 *     @date         2017
 *     @brief        Trigger command server.
 */

#ifndef TRIGGERCMDSERVER_H_
#define TRIGGERCMDSERVER_H_

#include "event/triggerCmd.handler.h"
#include "event/sensor.h"
#include "CHRCommunication/CHRCommunication.h"

namespace precitec
{

namespace grabber
{

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
    TriggerCmdServer(CHRCommunication& p_rCHRCommunication);
    virtual ~TriggerCmdServer();

public:
    /**
		 * Einzelner Trigger kann sofort abgesetzt werden
		 * @param ids
		 * @param context
		 */
    virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
    {
        const auto oSensorIdIsAnExternSensorId = [](int p_oSensorId)
        {
            return (p_oSensorId >= interface::eExternSensorMin && p_oSensorId <= interface::eExternSensorMax) ||
                   (p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax);
        };

        if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnExternSensorId) == true)
        {
            return; // this is not the appropriate receiver - abort
        }

        TriggerInterval interval(TriggerInterval::mSec * 10, 1);
        m_rCHRCommunication.burst(p_rSensorIds, context, interval);
    }

    /**
		 * Initalisiert und startet die Triggerquelle ueber mehrere Abschnitte
		 * @param ids
		 * @param context
		 * @param source
		 * @param interval
		 **/
    virtual void burst(const std::vector<int>& p_rSensorIds, TriggerContext const& p_rContext, int p_oSource, TriggerInterval const& p_rInterval)
    {
        const auto oSensorIdIsAnExternSensorId = [](int p_oSensorId)
        {
            return (p_oSensorId >= interface::eExternSensorMin && p_oSensorId <= interface::eExternSensorMax) ||
                   (p_oSensorId >= interface::eImageSensorMin && p_oSensorId <= interface::eImageSensorMax);
        };

        if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnExternSensorId) == true)
        {
            return; // this is not the appropriate receiver - abort
        }

        m_rCHRCommunication.burst(p_rSensorIds, p_rContext, p_rInterval);
    }

    /**
		 * Unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
		 * @param ids
		 **/
    virtual void cancel(const std::vector<int>& p_rSensorIds)
    {
        m_rCHRCommunication.cancel(p_rSensorIds.front());
    }

private:
    CHRCommunication& m_rCHRCommunication;
};

} // namespace grabber
} // namespace precitec

#endif /* TRIGGERCMDSERVER_H_*/
