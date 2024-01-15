#ifndef T3_INFORM_CONTROL_HANDLER_H
#define T3_INFORM_CONTROL_HANDLER_H
#pragma once

/**
 * Interfaces::t3_inform_control.handler.h
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

#include <iostream>
#include <string> // std::string

#include "server/eventHandler.h"
#include "test/t3_inform_control.interface.h"
#include "test/t3_control.h" // wg Device::State


namespace precitec
{
	using test::Controller;
namespace interface
{
	template <>
	class TT3InformControl<EventHandler> : public Server<EventHandler>
	{
	public:
		EVENT_HANDLER( TT3InformControl );
	public:
		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER1(TT3InformControl, signalError , int);
			REGISTER1(TT3InformControl, signalMessage, PvString);
			REGISTER2(TT3InformControl, signalCode,int, Code);
		}

		void CALLBACK1(signalError, int)(Receiver &receiver) {
			//std::cout << "TT3InformControl<EventHandler>::signalError(";
			int error; receiver.deMarshal(error);
			// std::cout << state << ")" << std::endl;
			// der eigentliche Server wird aufgerugen und der Returnwrt gleich verpackt (in den Puffer serialisiert)
			server_->signalError(Controller::Error(error));
		}

		void CALLBACK1(signalMessage, PvString)(Receiver &receiver) {
			//std::cout << "TT3InformControl<EventHandler>::signalMessage(";
			PvString message; receiver.deMarshal(message);
			server_->signalMessage(message);
		}

		void CALLBACK2(signalCode, int, Code)(Receiver &receiver) {
			//std::cout << "TT3InformControl<EventHandler>::signalCode(";
			int codeNum; receiver.deMarshal(codeNum);
			Code code; receiver.deMarshal(code);
			server_->signalCode(codeNum, code);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			std::cout << "eventHandler::signalCode: ok" << std::endl;
			std::cout << "eventHandler::signalCode: replied" << std::endl;
		}


	};

} // namespace system
} // namespace precitec

#endif /*T3_INFORM_CONTROL_HANDLER_H*/
