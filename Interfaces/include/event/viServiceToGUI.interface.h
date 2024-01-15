#ifndef VISERVICETOGUI_INTERFACE_H_
#define VISERVICETOGUI_INTERFACE_H_


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
	class TviServiceToGUI;

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
	class TviServiceToGUI<AbstractInterface>
	{
	public:
		TviServiceToGUI() {}
		virtual ~TviServiceToGUI() {}
	public:

		/*
		 * GetProcessImage.
		 * @size
		 * @data
		 */

		virtual void ProcessImage(ProcessDataVector& input, ProcessDataVector& output) = 0;
		virtual void SlaveInfoECAT(short count, SlaveInfo info) = 0;
		virtual void ConfigInfo(std::string config) = 0;

	};

    struct TviServiceToGUIMessageDefinition
    {
		EVENT_MESSAGE(ProcessImageMsg, ProcessDataVector, ProcessDataVector);
		EVENT_MESSAGE(SlaveInfoECATMsg, short, SlaveInfo);
		EVENT_MESSAGE(ConfigInfoMsg, std::string);

		MESSAGE_LIST(
			ProcessImageMsg,
			SlaveInfoECATMsg,
			ConfigInfoMsg
		);
    };

	//----------------------------------------------------------
	template <>
	class TviServiceToGUI<Messages> : public Server<Messages>, public TviServiceToGUIMessageDefinition
	{
	public:
		TviServiceToGUI<Messages>() : info(module::VIServiceToGUI, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 1*MBytes, replyBufLen = 100*Bytes, NumBuffers=64 };

	};


} // namespace interface
} // namespace precitec


#endif /*VISERVICETOGUI_INTERFACE_H_*/
