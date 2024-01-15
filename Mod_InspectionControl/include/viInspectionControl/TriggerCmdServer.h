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
#include "VI_InspectionControl.h"

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
		TriggerCmdServer(VI_InspectionControl& p_rVI_InspectionControl);
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

            auto oContinuouslyModeActive = m_rVI_InspectionControl.isContinuouslyModeActive();
            if (!oContinuouslyModeActive)
            {
                if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnExternSensorId) == true) {
                    return; // this is not the appropriate receiver - abort
                }
            }

			TriggerInterval interval( TriggerInterval::mSec * 10, 1 );
			m_rVI_InspectionControl.burst(p_rSensorIds, context,interval);
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
            m_rVI_InspectionControl.setTriggerDistNanoSecs( p_rInterval.triggerDistance()) ; // used in AutoRunner
            
            const auto oSensorIdIsAnExternSensorId	=	[](int p_oSensorId) {
				return p_oSensorId >= interface::eExternSensorMin && p_oSensorId <= interface::eExternSensorMax; };

            auto oContinuouslyModeActive = m_rVI_InspectionControl.isContinuouslyModeActive();
            auto oIsSOUVIS6000_Application = m_rVI_InspectionControl.isSOUVIS6000_Application();
            auto oIsSCANMASTER_Application = m_rVI_InspectionControl.isSCANMASTER_Application();
            if ((!oContinuouslyModeActive) && (!oIsSOUVIS6000_Application) && (!oIsSCANMASTER_Application))
            {
                if (std::none_of(std::begin(p_rSensorIds), std::end(p_rSensorIds), oSensorIdIsAnExternSensorId) == true) {
                    return; // this is not the appropriate receiver - abort
                }
            }

            m_rVI_InspectionControl.burst(p_rSensorIds, p_rContext, p_rInterval);
		}

		/**
		 * Unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
		 * @param ids
		 **/
		virtual void cancel(const std::vector<int>& p_rSensorIds)
		{
			m_rVI_InspectionControl.cancel(p_rSensorIds.front());
		}

private:
		VI_InspectionControl &m_rVI_InspectionControl;
	};

} // namespace ethercat
} // namespace precitec

#endif /* TRIGGERCMDSERVER_H_*/
