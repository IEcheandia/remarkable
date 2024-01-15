#ifndef VIWELDHEADPUBLISH_INTERFACE_H_
#define VIWELDHEADPUBLISH_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"


#include "event/viWeldHeadPublish.h"

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
	class TviWeldHeadPublish;

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
	class TviWeldHeadPublish<AbstractInterface>
	{
	public:
		TviWeldHeadPublish() {}
		virtual ~TviWeldHeadPublish() {}
	public:

		/*
		 * An error occurred.
		 * @errorCode
		 * @value
		 */

		virtual void headError(HeadAxisID axis, ErrorCode errorCode, int value) = 0;


		/*
		 * Sends the requested current head informations.
		 * (Is triggered by TviWeldHeadSubscribe::RequestHeadInfo)
		 */

		virtual void headInfo(HeadAxisID axis, HeadInfo info) = 0;

		/*
		 * Is sent when TviWeldHeadSubscribe::SetHeadReady has finished.
		 * @currentMode {Position or Velocity}
		 */

		virtual void headIsReady(HeadAxisID axis, MotionMode currentMode) = 0;

		/*
		 * Head reached requested value.
		 * @currentMode
		 * @currentValue
		 */

		virtual void headValueReached(HeadAxisID axis, MotionMode currentMode, int currentValue) = 0;

		virtual void trackerInfo(TrackerInfo p_oTrackerInfo) = 0;

	};

    struct TviWeldHeadPublishMessageDefinition
    {
		EVENT_MESSAGE(HeadError, HeadAxisID,ErrorCode,int);
		EVENT_MESSAGE(HeadInfoMsg, HeadAxisID,HeadInfo);
		EVENT_MESSAGE(HeadIsReady, HeadAxisID,MotionMode);
		EVENT_MESSAGE(HeadValueReached, HeadAxisID,MotionMode, int);
		EVENT_MESSAGE(TrackerInfoMsg, TrackerInfo);

		MESSAGE_LIST(
			HeadError,
			HeadInfoMsg,
			HeadIsReady,
			HeadValueReached,
			TrackerInfoMsg
		);
    };

	//----------------------------------------------------------
	template <>
	class TviWeldHeadPublish<Messages> : public Server<Messages>, public TviWeldHeadPublishMessageDefinition
	{
	public:
		TviWeldHeadPublish<Messages>() : info(system::module::WeldHeadPublish, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 5000*Bytes, replyBufLen = 100*Bytes, NumBuffers=32 };

	};


} // namespace interface
} // namespace precitec


#endif /*VIWELDHEADPUBLISH_INTERFACE_H_*/
