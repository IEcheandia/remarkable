#ifndef TRIGGER_INTERFACE_H_
#define TRIGGER_INTERFACE_H_

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/trigger.h"

/*
 * Hier werden die abstrakten Basisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
namespace interface
{
	using namespace  system;
	using namespace  message;
	//----------------------------------------------------------
	// Hier folgen die Template-Definitionen fuer die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TTrigger;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Signaler zeigt auf primitive Weise den Systemzustand an.
	 * Der State-Enum bietet drei Zustaende an. Verschiedene
	 * Handler koennen diese Zustaende unterschiedlich darstellen.
	 */
	template<>
	class TTrigger<AbstractInterface>
	{
	public:
		TTrigger() {}
		virtual ~TTrigger() {}
	public:
		// Triggersignal fuer ein Bild
		virtual void trigger(TriggerContext const& context) = 0;
		// Triggersignal fuer n oder endlos Anzahl Bilder (nur Grabcontroled)
		virtual void trigger(TriggerContext const& context, TriggerInterval const& interval) = 0;

		// TriggerMode setzen
		virtual void triggerMode(int mode) = 0;

		// Trigger stoppen - im grabber conntrolled mode
		virtual void triggerStop(int flag) = 0;
	};

    struct TTriggerMessageDefinition
    {
		EVENT_MESSAGE(Trigger, TriggerContext);
		EVENT_MESSAGE(TriggerWithInterval, TriggerContext, TriggerInterval);
		EVENT_MESSAGE(TriggerMode,int);
		EVENT_MESSAGE(TriggerStop,int);

		MESSAGE_LIST(
			Trigger,
			TriggerWithInterval,
			TriggerMode,
			TriggerStop
		);
    };

	//----------------------------------------------------------
	template <>
	class TTrigger<Messages> : public Server<Messages>, public TTriggerMessageDefinition	{
	public:
		TTrigger<Messages>() : info(system::module::Trigger, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=32 };
	};


} // namespace interface
} // namespace precitec

#endif /*TRIGGER_INTERFACE_H_*/
