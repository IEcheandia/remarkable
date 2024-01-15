#ifndef T3_CONTROL_WORKER_HANDLER_H
#define T3_CONTROL_WORKER_HANDLER_H
#pragma once

/**
 * Interfaces::t3_control_worker.handler.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, worker, worker
 *  - 4 Schnittstellen:
 *  		Control: M_T3ControlWorker, M_ControlWorker
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

#include <iostream>
#include <string> // std::string

#include "server/handler.h"
#include "test/t3_control_worker.interface.h"
#include "test/t3_worker.h" // wg Worker::State


namespace precitec
{
	using test::Worker;
namespace interface
{
	template <>
	class TT3ControlWorker<MsgHandler> : public Server<MsgHandler>
	{
	public:
		MSG_HANDLER( TT3ControlWorker );
	public:
		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER1(TT3ControlWorker, init , int);
			REGISTER3(TT3ControlWorker, configTask, int, double, double);
		}

		void CALLBACK1(init, int)(Receiver &receiver) {
			//std::cout << "TT3ControlWorker<MsgHandler>::init(";
			int state; receiver.deMarshal(state);
			// std::cout << state << ")" << std::endl;
			// der eigentliche Server wird aufgerugen und der Returnwrt gleich verpackt (in den Puffer serialisiert)
			receiver.marshal(server_->init(Worker::State(state)));
			// der Message-Reply wird abgesetzt
			receiver.reply();
		}

		void CALLBACK3(configTask, int, double, double)(Receiver &receiver) {
			//std::cout << "TT3ControlWorker<MsgHandler>::configTask(";
			int taskNum; receiver.deMarshal(taskNum);
			// std::cout << taskNum << ", ";
			double param0; receiver.deMarshal(param0);
			// std::cout << param0 << ", ";
			double param1; receiver.deMarshal(param1);
			// std::cout << param1 << ")" << std::endl;
			server_->configTask(taskNum, param0, param1);
			// der Message-Reply wird abgesetzt
			receiver.reply();
		}


	};

} // namespace system
} // namespace precitec

#endif /*T3_CONTROL_WORKER_HANDLER_H*/
