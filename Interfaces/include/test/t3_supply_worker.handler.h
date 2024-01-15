#ifndef T3_SUPPLY_WORKER_HANDLER_H
#define T3_SUPPLY_WORKER_HANDLER_H
#pragma once

/**
 * Interfaces::t3_supply_worker.handler.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_T3SupplyWorker, M_ControlWorker
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

#include <iostream>
#include <string> // std::string

#include "server/eventHandler.h"
#include "test/t3_supply_worker.interface.h"
#include "test/t3_device.h" // wg Device::State
#include "test/t3_worker.h" // wg Device::State
#include "image/image.h" // wg Device::State


namespace precitec
{
	using test::Device;
	using test::ShpData;
	using image::SmpImage;

namespace interface
{
	template <>
	class TT3SupplyWorker<EventHandler> : public Server<EventHandler>
	{
	public:
		EVENT_HANDLER( TT3SupplyWorker );
	private:
		SharedMem shMem_;
	public:
		//virtual void init() {}
		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER1(TT3SupplyWorker, deploy, int);
			REGISTER1(TT3SupplyWorker, deploy, ShpData);
			REGISTER2(TT3SupplyWorker, sendImage, int, SmpImage);
		}

		void CALLBACK1(deploy, int)(Receiver &receiver) {
			//std::cout << "TT3SupplyWorker<EventHandler>::signalError(";
			int data; receiver.deMarshal(data);
			// std::cout << state << ")" << std::endl;
			// der eigentliche Server wird aufgerugen und der Returnwrt gleich verpackt (in den Puffer serialisiert)
			server_->deploy(Device::State(data));
			// der Message-Reply wird abgesetzt
			//receiver.reply();
		}

		void CALLBACK1(deploy, ShpData)(Receiver &receiver) {
			//std::cout << "TT3SupplyWorker<EventHandler>::send(";
			ShpData data; receiver.deMarshal(data);
			// std::cout << taskNum << ")" << std::endl;
			server_->deploy(data);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			//receiver.reply();
		}

		void CALLBACK2(sendImage, int, SmpImage)(Receiver &receiver) {
			//std::cout << "TT3ControlDevice<MsgHandler>::triggerTask(";
			int imageNum; receiver.deMarshal(imageNum);
			SmpImage image; receiver.deMarshal(image);
			// std::cout << taskNum << ")" << std::endl;
			server_->sendImage(imageNum, image);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			std::cout << "handler: sendimage ok: " << imageNum << std::endl;
			//receiver.reply();
			//std::cout << "handler: reply ok: " << imageNum << std::endl;
		}
	};

} // namespace system
} // namespace precitec

#endif /*T3_SUPPLY_WORKER_HANDLER_H*/
