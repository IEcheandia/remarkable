#ifndef T3_INFORM_CONTROL_SERVER_H
#define T3_INFORM_CONTROL_SERVER_H
#pragma once

/**
 * Interfaces::t3_inform_control.server.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_InformControl, M_ControlWorker
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

#include <string> // std::string

#include "server/interface.h"
#include "test/t3_inform_control.interface.h"
#include "module/interfaces.h" // wg appId
#include "test/t3_device.h" // wg Device::State


/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
using test::Controller;
namespace interface
{
	/**
	 * TT3InformControl ist eine primitive TT3InformControl-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3InformControl<EventServer> : public TT3InformControl<AbstractInterface>
	{
	public:
		TT3InformControl() {}
		virtual ~TT3InformControl() {}
	public:
		/// gibt Fehler an Controller weiter
		virtual void signalError(Error error) {

		}
		/// gibt Message an Controller weiter
		virtual void signalMessage(PvString const& message) {

		}
		/// Code wg Inhalststest
		virtual void signalCode(int codeNum, Code const& code) {
			code.tell(std::cout, codeNum);
			std::cout << "EventServer::signalCode: ok"  << std::endl;
		}

	};


} // namespace system
} // namespace precitec

#endif /*T3_INFORM_CONTROL_SERVER_H*/
