#ifndef T3_CONTROL_WORKER_PROXY_H
#define T3_CONTROL_WORKER_PROXY_H
#pragma once

/**
 * Interfaces::t3_control_worker.proxy.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, worker, worker
 *  - 4 Schnittstellen:
 *  		Control: M_ControlWorker, M_ControlWorker
 *  		Worker:  E_InformControl, E_SupplyWorker
 *  		Worker:	 E_InformControl
 *  - 1 SharedMem: Data_ rw-Worker r-Worker
 *
 * T3ControlWorker:
 *  Schnittstelle, mit der der Controller Worker steuert
 *  	- synchron
 *    - wenig Daten
 *    - mehrere Messages
 *
 */

#include "server/proxy.h"
#include "test/t3_control_worker.interface.h"
#include "test/t3_worker.h" // wg Worker::State


namespace precitec
{
namespace interface
{

	template <>
	class TT3ControlWorker<MsgProxy> : public Server<MsgProxy>, public TT3ControlWorker<AbstractInterface> {

	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TT3ControlWorker()
		: PROXY_CTOR(TT3ControlWorker), TT3ControlWorker<AbstractInterface>() {}

		/// normalerweise wird das Protokoll gleich mitgeliefert
		TT3ControlWorker(SmpProtocolInfo & p)
		: PROXY_CTOR1(TT3ControlWorker,  p), TT3ControlWorker<AbstractInterface>() {}

		virtual ~TT3ControlWorker() {}

	public:
		/// initialisierung, ggf Dropback wg Statemachine
		virtual bool init(Worker::State state) {
			INIT_MESSAGE1(TT3ControlWorker, init, int);
			marshal(state);
			// Parameter ist Returnwert bei Kommunikaitonsfehler
			return sendWithReturn(Worker::InGeopardy);
		}

		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double param0, double param1) {
			INIT_MESSAGE3(TT3ControlWorker, configTask, int, double, double);
			marshal(taskNum);
			marshal(param0);
			marshal(param1);
			send();
		}

	}; // class TT3ControlWorker<MsgProxy>

} // namespace interface
} // namespace precitec

#endif /*T3_CONTROL_WORKER_PROXY_H*/
