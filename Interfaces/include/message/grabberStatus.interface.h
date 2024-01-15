/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2012
 *  @brief			GrabberStatus Interface
 */

#ifndef GRABBERSTATUS_INTERFACE_H_
#define GRABBERSTATUS_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

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
	class TGrabberStatus;

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
	class TGrabberStatus<AbstractInterface>
	{
	public:
		TGrabberStatus() {}
		virtual ~TGrabberStatus() {}
	public:
		// returns if the given image number belongs to an image in the grabber dma buffer (true), or if it is expired (false)
		virtual bool isImgNbInBuffer(uint32_t p_oImageNb) = 0;

		// information of upcoming activity
		virtual void preActivityNotification(uint32_t _productNumber) = 0;
	};

    struct TGrabberStatusMessageDefinition
    {
		MESSAGE(bool, IsImgNbInBuffer, uint32_t);
		MESSAGE(void, PreActivityNotification, uint32_t);

		MESSAGE_LIST(
			IsImgNbInBuffer,
			PreActivityNotification
		);
    };

	//----------------------------------------------------------
	template <>
	class TGrabberStatus<Messages> : public Server<Messages>, public TGrabberStatusMessageDefinition
	{
	public:
		TGrabberStatus() : info(system::module::GrabberStatus, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 100*Bytes, replyBufLen = 100*Bytes }; // bool(int)
	};


} // namespace interface
} // namespace precitec


#endif /*VIDEORECORDER_INTERFACECMD_H_*/
