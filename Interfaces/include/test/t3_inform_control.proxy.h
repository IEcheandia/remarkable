#ifndef T3_INFORM_CONTROL_PROXY_H
#define T3_INFORM_CONTROL_PROXY_H
#pragma once

/**
 * Interfaces::t3_inform_control.proxy.h
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

#include "server/eventProxy.h"
#include "test/t3_inform_control.interface.h"
#include "test/t3_device.h" // wg Device::State
#include "module/interfaces.h"


namespace precitec
{
	using test::Controller;

namespace interface
{

	template <>
	class TT3InformControl<EventProxy>
	: public Server<EventProxy>, public TT3InformControl<AbstractInterface> {

	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TT3InformControl()
		: EVENT_PROXY_CTOR(TT3InformControl), TT3InformControl<AbstractInterface>() {}

		virtual ~TT3InformControl() {}

	public:
		/// Fehlermeldung
		virtual void signalError(Controller::Error error) {
			INIT_MESSAGE1(TT3InformControl, signalError, int);
			marshal(int(error));
			send();
		}

		/// Textmeldung (ggf. Basis fuer Internationalisierungs-Test
		virtual void signalMessage(PvString const& message) {
			INIT_MESSAGE1(TT3InformControl, signalMessage, PvString);
			marshal(message);
			send();
		}

		/// Code wg Inhalststest
		virtual void signalCode(int codeNum, Code const& code) {
			INIT_MESSAGE2(TT3InformControl, signalCode, int, Code);
			marshal(codeNum);
			marshal(code);
			send();
		}

	}; // class TT3InformControl<MsgProxy>

} // namespace interface
} // namespace precitec

#endif /*T3_INFORM_CONTROL_PROXY_H*/
