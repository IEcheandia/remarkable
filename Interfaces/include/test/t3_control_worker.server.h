#ifndef T3_CONTROL_WORKER_SERVER_H
#define T3_CONTROL_WORKER_SERVER_H
#pragma once

/**
 * Interfaces::t3_control_worker.server.h
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

#include <string> // std::string

#include "server/interface.h"
#include "test/t3_control_worker.interface.h"
#include "module/interfaces.h" // wg appId
#include "test/t3_worker.h" // wg Worker::State


/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
namespace interface
{
	/**
	 * TT3ControlWorker ist eine primitive TT3ControlWorker-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3ControlWorker<MsgServer> : public TT3ControlWorker<AbstractInterface>
	{
	public:
		TT3ControlWorker() {}
		virtual ~TT3ControlWorker() {}
	public:
		/// initialisierung, ggf Dropback wg Statemachine
		virtual bool init(Worker::State state) {
			std::cout << "T3ControlWorker<MsgServer>::init: (" ;
			std::cout << "state: " << state << ")" << std::endl;
//			switch (state) {
//			}
			return true; // Operation gelungen
		}

		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double param0, double param1) {
			std::cout << "T3ControlWorker<MsgServer>::configTask: (" ;
			std::cout << "taskNum: " << taskNum << ", ";
			std::cout << "param0: " << param0 << ", ";
			std::cout << "param1: " << param0 << ")" << std::endl;
		}


	};


} // namespace system
} // namespace precitec

#endif /*T3_CONTROL_WORKER_SERVER_H*/
