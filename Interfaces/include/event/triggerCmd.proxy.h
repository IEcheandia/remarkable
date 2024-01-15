/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, AL, AB, HS
 * 	@date		2009
 * 	@brief		Trigger command interface. Signals sensors to take a single measurement or several measurements over time (burst).
 */

#ifndef TRIGGERCMD_PROXY_H_
#define TRIGGERCMD_PROXY_H_


#include "server/eventProxy.h"
#include "triggerCmd.interface.h"

namespace precitec {
namespace interface {

	template <>
	class TTriggerCmd<EventProxy> : public Server<EventProxy>, public TTriggerCmd<AbstractInterface>, public TTriggerCmdMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TTriggerCmd() : EVENT_PROXY_CTOR(TTriggerCmd), TTriggerCmd<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TTriggerCmd() {}

	public:
		virtual void single(const std::vector<int>& p_rSensorIds, TriggerContext const& context)
		{
			INIT_EVENT(Single);
			signaler().marshal(p_rSensorIds);
			signaler().marshal(context);
			signaler().send();
		}

		virtual void burst(const std::vector<int>& p_rSensorIds, TriggerContext const& context, int source, TriggerInterval const& interval)
		{
			INIT_EVENT(Burst);
			signaler().marshal(p_rSensorIds);
			signaler().marshal(context);
			signaler().marshal(source);
			signaler().marshal(interval);
			signaler().send();
		}

		virtual void cancel(const std::vector<int>& p_rSensorIds)
		{
			INIT_EVENT(Cancel);
			signaler().marshal(p_rSensorIds);
			signaler().send();
		}

	};

} // interface
} // precitec


#endif /*TRIGGERCMD_PROXY_H_*/
