#ifndef INSPECTION_PROXY_H_
#define INSPECTION_PROXY_H_


#include "server/eventProxy.h"
#include "event/inspection.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TInspection<EventProxy> : public Server<EventProxy>, public TInspection<AbstractInterface>, public TInspectionMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TInspection() : EVENT_PROXY_CTOR(TInspection), TInspection<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TInspection() {}

	public:

		virtual void startAutomaticmode(uint32_t producttype, uint32_t productnumber, const std::string& p_rExtendedProductInfo)
		{
			INIT_EVENT(StartAutomaticmode);
			//signaler().initMessage(Msg::index);
			signaler().marshal(producttype);
			signaler().marshal(productnumber);
			signaler().marshal(p_rExtendedProductInfo);
			signaler().send();
		}

		virtual void stopAutomaticmode()
		{
			INIT_EVENT(StopAutomaticmode);
			//signaler().initMessage(Msg::index);
			signaler().send();
		}

		virtual void start(int seamnumber)
		{
			INIT_EVENT(Start);
			signaler().marshal(seamnumber);
			signaler().send();
		}

		virtual void end(int seamnumber)
		{
			INIT_EVENT(End);
			signaler().marshal(seamnumber);
			signaler().send();
		}

		virtual void info(int seamsequence)
		{
			INIT_EVENT(Info);
			signaler().marshal(seamsequence);
			signaler().send();
		}

		virtual void linelaser(bool onoff)
		{
			INIT_EVENT(Linelaser);
			signaler().marshal(onoff);
			signaler().send();
		}

		virtual void startCalibration()
		{
			INIT_EVENT(StartCalibration);
			signaler().send();
		}

		virtual void stopCalibration()
		{
			INIT_EVENT(StopCalibration);
			signaler().send();
		}

		virtual void seamPreStart(int seamnumber)
		{
			INIT_EVENT(SeamPreStart);
			signaler().marshal(seamnumber);
			signaler().send();
		}
	};

} // interface
} // precitec


#endif /*INSPECTION_PROXY_H_*/
