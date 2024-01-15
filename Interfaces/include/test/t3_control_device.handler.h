#ifndef T3_CONTROL_DEVICE_HANDLER_H
#define T3_CONTROL_DEVICE_HANDLER_H
#pragma once

/**
 * Interfaces::t3_control_device.handler.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_T3ControlDevice, M_ControlWorker
 *  		Device:  E_InformControl, E_SupplyWorker
 *  		Worker:	 E_InformControl
 *  - 1 SharedMem: Data_ rw-Device r-Worker
 *
 * T3ControlDevice:
 *  Schnittstelle, mit der der Controller Device steuert
 *  	- synchron
 *    - wenig Daten
 *    - mehrere Messages
 *
 */

#include <iostream>
#include <string> // std::string

#include "server/handler.h"
#include "test/t3_control_device.interface.h"
#include "test/t3_device.h" // wg Device::State


namespace precitec
{
	using test::Device;
namespace interface
{
	template <>
	class TT3ControlDevice<MsgHandler> : public Server<MsgHandler>
	{
	public:
		MSG_HANDLER( TT3ControlDevice );
	public:
		void registerCallbacks() {
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER1(TT3ControlDevice, init , int);
			REGISTER3(TT3ControlDevice, configTask, int, double, double );
			REGISTER2(TT3ControlDevice, prepTask, int, double);
			REGISTER1(TT3ControlDevice, triggerTask, int);
			REGISTER1(TT3ControlDevice, triggerImage, int);
		}

		void CALLBACK1(init, int)(Receiver &receiver) {
			//std::cout << "TT3ControlDevice<MsgHandler>::init(";
			int state; receiver.deMarshal(state);
			// std::cout << state << ")" << std::endl;
			// der eigentliche Server wird aufgerugen und der Returnwrt gleich verpackt (in den Puffer serialisiert)
			receiver.marshal(server_->init(Device::State(state)));
			// der Message-Reply wird abgesetzt
			receiver.reply();
		}

		void CALLBACK3(configTask, int, double, double)(Receiver &receiver) {
			//std::cout << "TT3ControlDevice<MsgHandler>::configTask(";
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

		void CALLBACK2(prepTask, int, double)(Receiver &receiver) {
			//std::cout << "TT3ControlDevice<MsgHandler>::configTask(";
			int taskNum; receiver.deMarshal(taskNum);
			// std::cout << taskNum << ", ";
			double specialParam; receiver.deMarshal(specialParam);
			// std::cout << specialParam << ")" << std::endl;
			server_->prepTask(taskNum, specialParam);
			// der Message-Reply wird abgesetzt
			receiver.reply();
		}

		void CALLBACK1(triggerTask, int)(Receiver &receiver) {
			//std::cout << "TT3ControlDevice<MsgHandler>::triggerTask(";
			int taskNum; receiver.deMarshal(taskNum);
			// std::cout << taskNum << ")" << std::endl;
			server_->triggerTask(taskNum);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

		void CALLBACK1(triggerImage, int)(Receiver &receiver) {
			//std::cout << "TT3ControlDevice<MsgHandler>::triggerTask(";
			int imageNum; receiver.deMarshal(imageNum);
			// std::cout << taskNum << ")" << std::endl;
			server_->triggerImage(imageNum);
			// der Message-Reply mit dem Returnwert wird abgesetzt
			receiver.reply();
		}

	};

} // namespace system
} // namespace precitec

#endif /*T3_CONTROL_DEVICE_HANDLER_H*/
