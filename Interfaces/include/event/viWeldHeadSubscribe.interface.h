#ifndef VIWELDHEADSUBSCRIBE_INTERFACE_H_
#define VIWELDHEADSUBSCRIBE_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"


#include "event/viWeldHeadSubscribe.h"

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
	class TviWeldHeadSubscribe;

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
	class TviWeldHeadSubscribe<AbstractInterface>
	{
	public:
		TviWeldHeadSubscribe() {}
		virtual ~TviWeldHeadSubscribe() {}
	public:

		/*
		 * When VI is started, SetHeadMode has to be called to initialize the motion axis.
		 * When axis is configured as homeable this will perform an homing.
		 *
		 * @mode: the target state. {Position or Velocity}
		 */

		virtual void SetHeadMode(HeadAxisID axis, MotionMode mode, bool bHome) = 0 ;


		/*
		*	Set new target value (-> move axis)
		*	@value: new target value [mm]
		*	@mode: choose mode {Position_Relative or Position_Absolute}
		*/

		virtual void SetHeadValue(HeadAxisID axis, int value, MotionMode positioningMode) = 0;


		/*
		 *	Set line laser intensity
		 *	@value:
		 *			0 = 0%
		 *			10000 = 100%
		 */

		virtual void SetTrackerScanWidthControlled(int value) = 0;
		virtual void SetTrackerScanPosControlled(int value) = 0;
		virtual void SetTrackerDriverOnOff(bool onOff) = 0;

		virtual void doZCollDriving(DrivingType p_oDrivingType, int value) = 0;

		/*
		 * Request head information.
		 * An "TviWeldHeadPublish::headInfo" event will be sent.
		 */

		virtual void RequestHeadInfo(HeadAxisID axis) = 0;

		virtual void SetLCPowerOffset(int value) = 0;

		virtual void SetGenPurposeAnaOut(OutputID outputNo, int value) = 0;

		virtual void ScanmasterResult(ScanmasterResultType p_oResultType, ResultDoubleArray const& p_rResultDoubleArray) = 0;

        virtual void ScanmasterHeight(double height) = 0;
	};

    struct TviWeldHeadSubscribeMessageDefinition
    {
		EVENT_MESSAGE(SetHeadModeMsg, HeadAxisID, MotionMode, bool);
		EVENT_MESSAGE(SetHeadValueMsg, HeadAxisID, int, MotionMode);
		EVENT_MESSAGE(SetTrackerScanWidthControlledMsg, int);
		EVENT_MESSAGE(SetTrackerScanPosControlledMsg, int);
		EVENT_MESSAGE(SetTrackerDriverOnOffMsg, bool);
		EVENT_MESSAGE(DoZCollDriving, DrivingType, int);
		EVENT_MESSAGE(RequestHeadInfoMsg,HeadAxisID);
		EVENT_MESSAGE(SetLCPowerOffsetMsg, int);
		EVENT_MESSAGE(SetGenPurposeAnaOutMsg, OutputID, int);
		EVENT_MESSAGE(ScanmasterResultMsg, ScanmasterResultType, ResultDoubleArray);
        EVENT_MESSAGE(ScanmasterHeightMsg, double);

		MESSAGE_LIST(
			SetHeadModeMsg,
			SetHeadValueMsg,
			SetTrackerScanWidthControlledMsg,
			SetTrackerScanPosControlledMsg,
			SetTrackerDriverOnOffMsg,
			DoZCollDriving,
			RequestHeadInfoMsg,
			SetLCPowerOffsetMsg,
			SetGenPurposeAnaOutMsg,
			ScanmasterResultMsg,
            ScanmasterHeightMsg
		);
    };

	//----------------------------------------------------------
	template <>
	class TviWeldHeadSubscribe<Messages> : public Server<Messages>, public TviWeldHeadSubscribeMessageDefinition
	{
	public:
		TviWeldHeadSubscribe<Messages>() : info(system::module::WeldHeadSubscribe, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Kontanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 1000*Bytes, NumBuffers=512 };
	};


} // namespace interface
} // namespace precitec


#endif /*VIWELDHEADSUBSCRIBE_INTERFACE_H_*/
