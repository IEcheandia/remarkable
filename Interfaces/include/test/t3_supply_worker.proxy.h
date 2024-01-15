#ifndef T3_SUPPLY_WORKER_PROXY_H
#define T3_SUPPLY_WORKER_PROXY_H
#pragma once

/**
 * Interfaces::t3_supply_worker.proxy.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_SupplyWorker, M_ControlWorker
 *  		Device:  E_SupplyWorker, E_SupplyWorker
 *  		Worker:	 E_SupplyWorker
 *  - 1 SharedMem: Data_ rw-Device r-Worker
 *
 * T3SupplyWorker:
 *  Schnittstelle, mit der der Controller Device steuert
 *  	- synchron
 *    - wenig Daten
 *    - mehrere Messages
 *
 */

#include "server/proxy.h"
#include "test/t3_supply_worker.interface.h"
#include "test/t3_device.h" // wg Device::State


namespace precitec
{
	using test::ShpData;
namespace interface
{

	template <>
	class TT3SupplyWorker<EventProxy> : public Server<EventProxy>, public TT3SupplyWorker<AbstractInterface> {

	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TT3SupplyWorker()
		: EVENT_PROXY_CTOR(TT3SupplyWorker), TT3SupplyWorker<AbstractInterface>() {}

		virtual ~TT3SupplyWorker() {}

	public:
		/*
		/// versichkt einen int
		virtual void send(int data) {
			CREATE_MESSAGE1(TT3SupplyWorker, send, int);
			signaler().initMessage(Msg::index);
			signaler().marshal(data);
			signaler().send();
		}

		/// verschickt eine Datenstruktur ueber einen SharedMem
		virtual void send(ShpData data) {
			CREATE_MESSAGE1(TT3SupplyWorker, send, ShpData);
			signaler().initMessage(Msg::index);
			signaler().marshal(data);
			signaler().send();
		}
		*/
		/// versichkt einen int
		virtual void deploy(int data) {
			INIT_MESSAGE1(TT3SupplyWorker, deploy, int);
			marshal(data);
			send();
		}

		/// verschickt eine Datenstruktur ueber einen SharedMem
		virtual void deploy(ShpData const& data) {
			INIT_MESSAGE1(TT3SupplyWorker, deploy, ShpData);
			marshal(data);
			send();
		}
		/// Bild uebertragen
		virtual void sendImage(int imageNum, SmpImage &image) {
			INIT_MESSAGE2(TT3SupplyWorker, sendImage, int, SmpImage);
			marshal(imageNum);
			marshal(image);
			send();
		}

	}; // class TT3SupplyWorker<MsgProxy>

} // namespace interface
} // namespace precitec

#endif /*T3_SUPPLY_WORKER_PROXY_H*/
