#ifndef T3_INFORM_CONTROL_INTERFACE_H
#define T3_INFORM_CONTROL_INTERFACE_H
#pragma once

/**
 * Interfaces::t3_inform_control.interface.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_T3InformControl, M_ControlWorker
 *  		Device:  E_InformControl, E_SupplyWorker
 *  		Worker:	 E_InformControl
 *  - 1 SharedMem: Data_ rw-Device r-Worker
 *
 * T3InformControl:
 *  Schnittstelle, mit der der Controller Device steuert
 *  	- synchron
 *    - wenig Daten
 *    - mehrere Messages
 *
 */

#include "server/interface.h"
#include "message/serializer.h"
#include "test/t3_control.h" // wg Controller::Error

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using namespace  system;
	using namespace  message;
	using test::Controller;

	namespace interface
{
	typedef Controller::Code Code;
	typedef Controller::Error Error;
	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int mode>
	class TT3InformControl;

	/**
	 * PartTransport ist eine primitive PartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3InformControl<AbstractInterface> {
	public:
		TT3InformControl() {}
		virtual ~TT3InformControl() {}
	public:
		/// gibt Fehler an Controller weiter
		virtual void signalError(Error error) = 0;
		/// gibt Code an Controller weiter
		virtual void signalCode(int codeNum, Code const&code) = 0;
		/// gibt Message an Controller weiter
		virtual void signalMessage(PvString const& message) = 0;
	};

	//----------------------------------------------------------
	template <>
	class TT3InformControl<Messages> : public Server<Messages> {
	public:
		TT3InformControl() : info(module::T3InformControl, sendBufLen, replyBufLen, NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 200*KBytes, replyBufLen = 100*Bytes };
	public:
		MESSAGE_LIST3(
			module::T3InformControl,
			MESSAGE_NAME1(signalError, int),
			MESSAGE_NAME1(signalMessage, PvString),
			MESSAGE_NAME2(signalCode, int, Code)
		);
	public:
		DEFINE_MSG1(void, signalError, int);
		DEFINE_MSG1(void, signalMessage, PvString);
		DEFINE_MSG2(void, signalCode, int, Code);
	};


} // namespace events
} // namespace precitec

#endif /*T3_INFORM_CONTROL_INTERFACE_H*/
