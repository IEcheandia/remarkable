#ifndef VISERVICEFROMGUI_PROXY_H_
#define VISERVICEFROMGUI_PROXY_H_


#include "server/eventProxy.h"
#include "event/viServiceFromGUI.interface.h"
#include "Poco/UUID.h"


namespace precitec
{
namespace interface
{

	template <>
	class TviServiceFromGUI<EventProxy> : public Server<EventProxy>, public TviServiceFromGUI<AbstractInterface>, public TviServiceFromGUIMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TviServiceFromGUI() : EVENT_PROXY_CTOR(TviServiceFromGUI), TviServiceFromGUI<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TviServiceFromGUI() {}

	public:
		virtual void SetTransferMode(bool onOff){
			INIT_EVENT(SetTransferModeMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(onOff);
			signaler().send();
		}

		virtual void OutputProcessData(short physAddr, ProcessData& data, ProcessData& mask, short type){
			INIT_EVENT(OutputProcessDataMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(physAddr);
			signaler().marshal(data);
			signaler().marshal(mask);
			signaler().marshal(type);
			signaler().send();
		}

		void requestSlaveInfo() override
		{
			INIT_EVENT(RequestSlaveInfo);
			signaler().send();
		}

	};

} // interface
} // precitec


#endif /*VISERVICEFROMGUI_PROXY_H_*/
