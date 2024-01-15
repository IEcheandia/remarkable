#ifndef PRODUCTTEACHIN_INTERFACE_H_
#define PRODUCTTEACHIN_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/productTeachIn.h"

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
	class TProductTeachIn;

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
	class TProductTeachIn<AbstractInterface>
	{
	public:
		TProductTeachIn() {}
		virtual ~TProductTeachIn() {}
	public:
		// Inspection Start
		virtual void start (int seamSeries, int seam) = 0;
		// Inspection Ende
		virtual void end (system::Timer::Time duration) = 0;
		// startAutomatic
		virtual void startAutomatic(int code) = 0;
		// stopAutomatic
		virtual void stopAutomatic() = 0;
	};

    struct TProductTeachInMessageDefinition
    {
		EVENT_MESSAGE(Start, int, int);
		EVENT_MESSAGE(End, xLong);
		EVENT_MESSAGE(StartAutomatic, int);
		EVENT_MESSAGE(StopAutomatic, void);

		MESSAGE_LIST(
			Start,
			End,
			StartAutomatic,
			StopAutomatic
		);
    };

	//----------------------------------------------------------
	template <>
	class TProductTeachIn<Messages> : public Server<Messages>, public TProductTeachInMessageDefinition
	{
	public:
		TProductTeachIn<Messages>() : info(system::module::ProductTeachIn, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=32 };
	};


} // namespace interface
} // namespace precitec


#endif /*PRODUCTTEACHIN_INTERFACE_H_*/
