#ifndef T3_CONTROL_WORKER_INTERFACE_H
#define T3_CONTROL_WORKER_INTERFACE_H
#pragma once

/**
 * Interfaces::t3_control_worker.interface.h
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

#include  "server/interface.h"
#include  "message/serializer.h"
#include "test/t3_worker.h" // wg Worker::State

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using test::Worker;
namespace interface
{
	using namespace  system;
	using namespace  message;

	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int mode>
	class TT3ControlWorker;

	/**
	 * PartTransport ist eine primitive PartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3ControlWorker<AbstractInterface>
	{
	public:
		TT3ControlWorker() {}
		virtual ~TT3ControlWorker() {}
	public:
		/// initialisierung, ggf Dropback wg Statemachine
		virtual bool init(Worker::State state) = 0;
		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double parm0, double param1) = 0;


	};


	//----------------------------------------------------------
	template <>
	class TT3ControlWorker<Messages> : public Server<Messages> {
	public:
		TT3ControlWorker() : info(module::T3ControlWorker, sendBufLen, replyBufLen, NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 200*Bytes, replyBufLen = 100*Bytes };
	public:
		MESSAGE_LIST2(
			module::T3ControlWorker,
			MESSAGE_NAME1(init, int),
			MESSAGE_NAME3(configTask, int, double, double)

		);
	public:
		DEFINE_MSG1(bool, init, int);
		DEFINE_MSG3(void, configTask, int, double, double);

	};


} // namespace events
} // namespace precitec

#endif /*T3_CONTROL_WORKER_INTERFACE_H*/
