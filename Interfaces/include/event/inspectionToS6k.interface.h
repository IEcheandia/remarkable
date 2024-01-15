#pragma once

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
	class TInspectionToS6k;

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
	class TInspectionToS6k<AbstractInterface>
	{
	public:
		TInspectionToS6k() {}
		virtual ~TInspectionToS6k() {}
	public:
		/// Interface: inspectionToS6k : setS6K_QualityResults
		virtual void setS6K_QualityResults (int32_t p_oSeamNo, struct S6K_QualityData_S1S2 p_oQualityData) = 0;
		/// Interface: inspectionToS6k : setS6K_CS_DataBlock
		virtual void setS6K_CS_DataBlock (CS_TCP_MODE p_oSendCmd, uint16_t p_oSeamNo, uint16_t p_oBlockNo, uint16_t p_oFirstMeasureInBlock,
                                          uint16_t p_oMeasureCntInBlock, uint16_t p_oMeasuresPerResult, uint16_t p_oValuesPerMeasure, CS_BlockType p_oCS_DataBlock) = 0;

	};

    struct TInspectionToS6kMessageDefinition
    {
		EVENT_MESSAGE(SetS6K_QualityResults, int32_t, struct S6K_QualityData_S1S2);
		EVENT_MESSAGE(SetS6K_CS_DataBlock, CS_TCP_MODE, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, CS_BlockType);

		MESSAGE_LIST(
			SetS6K_QualityResults,
			SetS6K_CS_DataBlock
		);
    };

	//----------------------------------------------------------
	template <>
	class TInspectionToS6k<Messages> : public Server<Messages>, public TInspectionToS6kMessageDefinition
	{
	public:
		TInspectionToS6k<Messages>() : info(system::module::InspectionToS6k, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
	};

} // namespace interface
} // namespace precitec

