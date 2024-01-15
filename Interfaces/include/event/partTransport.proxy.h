#ifndef PART_TRANSPORT_PROXY_H
#define PART_TRANSPORT_PROXY_H

#include "server/eventProxy.h"
#include "event/partTransport.interface.h"


namespace precitec
{
namespace interface
{


	template <>
	class TPartTransport<EventProxy> : public Server<EventProxy>, TPartTransport<AbstractInterface>
	{

	public:
		/// beide Basisklassen muessen geeignet initialisiert werden
		TPartTransport() : EVENT_PROXY_CTOR(TPartTransport), TPartTransport<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}	
	public:		
		virtual void moveto(int state)
		{
			INIT_EVENT1(Messages, moveto, int);
			signaler().marshal(state);
			signaler().send();
		}
		
		virtual void init()
		{
			INIT_EVENT0(Messages, init);
			signaler().send();
		}
		
		virtual void pause()
		{
			INIT_EVENT0(Messages, pause);
			signaler().send();
		}
		
		virtual void resume()
		{
			INIT_EVENT1(Messages, resume);
			signaler().send();
		}

		virtual void stop()
		{
			INIT_EVENT1(Messages, stop);
			signaler().send();
		}
	};

} // namespace interface
} // namespace precitec

#endif /*PART_TRANSPORT_PROXY_H*/
