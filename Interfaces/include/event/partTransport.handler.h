#ifndef PART_TRANSPORT_HANDLER_H
#define PART_TRANSPORT_HANDLER_H

#include <iostream> 
#include <string> // std::string

#include "server/eventHandler.h"
#include "event/partTransport.interface.h"


namespace precitec
{
namespace interface
{
	
	template <>
	class TPartTransport<EventHandler> : public Server<EventHandler>
	{
	public:
		EVENT_HANDLER( TPartTransport );
	public:
		void registerCallbacks() 
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER1(TPartTransport, moveto, int );
			REGISTER0(TPartTransport, init);
			REGISTER0(TPartTransport, pause);
			REGISTER0(TPartTransport, resume);
			REGISTER0(TPartTransport, stop);
		}

		void moveto(Receiver &receiver)
		{
			std::cout << "TPartTransport<EventHandler>::moveto" << std::endl;
			//TSignaler<Messages>::MESSAGE_NAME1(setState, int) msg();
			int arg0; receiver.deMarshal(arg0);
			server_->moveto(arg0);
		}
		
		void init(Receiver &receiver)
		{ 
			std::cout << "TPartTransport<EventHandler>::init" << std::endl; 
			server_->init();
		}
		
		void pause(Receiver &receiver)
		{ 
			std::cout << "TPartTransport<EventHandler>::pause" << std::endl; 
			server_->stop();
		}
		
		void resume(Receiver &receiver)
		{ 
			std::cout << "TPartTransport<EventHandler>::resume" << std::endl; 
			server_->stop();
		}
		
		void stop(Receiver &receiver)
		{ 
			std::cout << "TPartTransport<EventHandler>::stop" << std::endl; 
			server_->stop();
		}
	};

} // namespace system
} // namespace precitec

#endif /*PART_TRANSPORT_HANDLER_H*/
