/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       03.10.2011
 *  @brief      Part of the inspectionOut Interface
 *  @details
 */

#ifndef INSPECTIONOUT_INTERFACE_H_
#define INSPECTIONOUT_INTERFACE_H_

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/inspectionOut.h"

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
	class TInspectionOut;

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
	class TInspectionOut<AbstractInterface>
	{
	public:
		TInspectionOut() {}
		virtual ~TInspectionOut() {}
	public:
		/// Interface: inspectionOut : setSystemReady
		virtual void setSystemReady (bool onoff) = 0;
		/// Interface: inspectionOut : setSystemErrorField
		virtual void setSystemErrorField (int systemErrorField) = 0;
		/// Interface: inspectionOut : setSumErrorLatched
		virtual void setSumErrorLatched (bool onoff) = 0;
		/// Interface: inspectionOut : setQualityErrorField
		virtual void setQualityErrorField (int qualityErrorField) = 0;
		/// Interface: inspectionOut : setInspectCycleAckn
		virtual void setInspectCycleAckn (bool onoff) = 0;
		/// Interface: inspectionOut : setCalibrationFinished
		virtual void setCalibrationFinished (bool result) = 0;

	};

    struct TInspectionOutMessageDefinition
    {
		EVENT_MESSAGE(SetSystemReady, bool);
		EVENT_MESSAGE(SetSystemErrorField, int);
		EVENT_MESSAGE(SetSumErrorLatched, bool);
		EVENT_MESSAGE(SetQualityErrorField, int);
		EVENT_MESSAGE(SetInspectCycleAckn, bool);
		EVENT_MESSAGE(SetCalibrationFinished, bool);

		MESSAGE_LIST(
			SetSystemReady,
			SetSystemErrorField,
			SetSumErrorLatched,
			SetQualityErrorField,
			SetInspectCycleAckn,
			SetCalibrationFinished
		);
    };

	//----------------------------------------------------------
	template <>
	class TInspectionOut<Messages> : public Server<Messages>, public TInspectionOutMessageDefinition
	{
	public:
		TInspectionOut<Messages>() : info(system::module::InspectionOut, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
	};

} // namespace interface
} // namespace precitec

#endif /* INSPECTIONOUT_INTERFACE_H_ */

