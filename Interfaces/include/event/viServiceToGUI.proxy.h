#ifndef VISERVICETOGUI_PROXY_H_
#define VISERVICETOGUI_PROXY_H_


#include "server/eventProxy.h"
#include "event/viServiceToGUI.interface.h"
#include "Poco/UUID.h"


namespace precitec
{
namespace interface
{

	template <>
	class TviServiceToGUI<EventProxy> : public Server<EventProxy>, public TviServiceToGUI<AbstractInterface>, public TviServiceToGUIMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TviServiceToGUI() : EVENT_PROXY_CTOR(TviServiceToGUI), TviServiceToGUI<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TviServiceToGUI() {}

	public:
		virtual void ProcessImage(ProcessDataVector& input, ProcessDataVector& output){
			INIT_EVENT(ProcessImageMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(input);
			signaler().marshal(output);
			signaler().send();
		}

		virtual void SlaveInfoECAT(short count, SlaveInfo info){
			INIT_EVENT(SlaveInfoECATMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(count);
			signaler().marshal(info);
			signaler().send();
		}

		virtual void ConfigInfo(stdString config){
			INIT_EVENT(ConfigInfoMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(config);
			signaler().send();
		}

	};

} // interface
} // precitec


#endif /*VISERVICETOGUI_PROXY_H_*/
