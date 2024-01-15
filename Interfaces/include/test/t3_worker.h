#ifndef T3_WORKER_H_
#define T3_WORKER_H_
#pragma once

/**
 * Interfaces::t3_worker.h
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
 * T3Worker: generischer Worker
 *  - ist Server fuer M-ControlWorker
 *  - ist Subscriber fuer E_SupplyControl
 */


namespace precitec
{
namespace test
{
	class Worker {
	public:
		enum State {
			UnInitialized = -1,
			Initialized,
			ReadyForTask,
			InGeopardy
		};
	};
} // namespace test
} // namespace precitec

#endif // T3_WORKER_H_
