/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AL, EA, HS
 * 	@date		2010
 * 	@brief		Trigger command server.
 */

#ifndef TRIGGERCMDSERVER_H_
#define TRIGGERCMDSERVER_H_

#include "event/triggerCmd.handler.h"
#include "event/sensor.h"
#include "WeldingHeadControl.h"

namespace precitec
{
namespace ethercat
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
		TriggerCmdServer(WeldingHeadControl& rWeldingHeadControl);
		virtual ~TriggerCmdServer();
public:

		/**
		 * Einzelner Trigger kann sofort abgesetzt werden
		 * @param ids
		 * @param context
		 */
		virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
		{
			const auto oSensorIdIsAnExternSensorId	=	[](int p_oSensorId) {
				return p_oSensorId >= interface::eExternSensorMin && p_oSensorId <= interface::eExternSensorMax; };

			if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnExternSensorId) == true) {
				return; // this is not the appropriate receiver - abort
			}

			TriggerInterval interval( TriggerInterval::mSec * 10, 1 );
			m_rWeldingHeadControl.burst(p_rSensorIds, context,interval);
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
			const auto oSensorIdIsAnExternSensorId	=	[](int p_oSensorId) {
				return p_oSensorId >= interface::eExternSensorMin && p_oSensorId <= interface::eExternSensorMax; };

			if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnExternSensorId) == true) {
				return; // this is not the appropriate receiver - abort
			}

			m_rWeldingHeadControl.burst(p_rSensorIds, p_rContext, p_rInterval);
		}

		/**
		 * Unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
		 * @param ids
		 **/
		virtual void cancel(const std::vector<int>& p_rSensorIds)
		{
			m_rWeldingHeadControl.cancel(p_rSensorIds.front());
		}

private:
		WeldingHeadControl &m_rWeldingHeadControl;
	};

} // namespace ethercat
} // namespace precitec

#endif /* TRIGGERCMDSERVER_H_*/
