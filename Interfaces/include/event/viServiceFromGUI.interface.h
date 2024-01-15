#ifndef VISERVICEFROMGUI_INTERFACE_H_
#define VISERVICEFROMGUI_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"


#include "event/viService.h"

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
	class TviServiceFromGUI;

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
	class TviServiceFromGUI<AbstractInterface>
	{
	public:
		TviServiceFromGUI() {}
		virtual ~TviServiceFromGUI() {}
	public:

		/*
		 * GetProcessImage.
		 * @size
		 * @data
		 */

		virtual void SetTransferMode(bool onOff) = 0;
		virtual void OutputProcessData(short physAddr, ProcessData& data, ProcessData& mask, short type = 1) = 0;
        virtual void requestSlaveInfo() = 0;

	};

    struct TviServiceFromGUIMessageDefinition
    {
		EVENT_MESSAGE(SetTransferModeMsg, bool);
		EVENT_MESSAGE(OutputProcessDataMsg, short, ProcessData, ProcessData, short);
		EVENT_MESSAGE(RequestSlaveInfo, void);

		MESSAGE_LIST(
			SetTransferModeMsg,
			OutputProcessDataMsg,
			RequestSlaveInfo
		);
    };

	//----------------------------------------------------------
	template <>
	class TviServiceFromGUI<Messages> : public Server<Messages>, public TviServiceFromGUIMessageDefinition
	{
	public:
		TviServiceFromGUI<Messages>() : info(module::VIServiceFromGUI, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 20*KBytes, replyBufLen = 100*Bytes, NumBuffers=64 };

	};


} // namespace interface
} // namespace precitec


#endif /*VISERVICEFROMGUI_INTERFACE_H_*/
