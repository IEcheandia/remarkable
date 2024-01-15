/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, AL, AB, HS
 * 	@date		2009
 * 	@brief		Trigger command interface. Signals sensors to take a single measurement or several measurements over time (burst).
 */

#ifndef TRIGGERCMD_SERVER_H_
#define TRIGGERCMD_SERVER_H_


#include "triggerCmd.interface.h"

namespace precitec {
namespace interface {

	template <>
	class TTriggerCmd<EventServer> : public TTriggerCmd<AbstractInterface>
	{
	public:
		TTriggerCmd(){}
		virtual ~TTriggerCmd() {}
	public:
		virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context) {}
		// initalisirt und startet die Triggerquelle ueber mehrere Abschnitte
		virtual void burst(const std::vector<int>& p_rSensorIds, TriggerContext const& context, int source, TriggerInterval const& interval) {}
		// unterbricht die laufende Triggerquelle, wenn noch Triggersignale versendet werden
		virtual void cancel(const std::vector<int>& p_rSensorIds) {}
	};

} // interface
} // precitec


#endif /*TRIGGERCMD_SERVER_H_*/
