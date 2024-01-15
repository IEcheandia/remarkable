#ifndef T3_CONTROL_DEVICE_SERVER_H
#define T3_CONTROL_DEVICE_SERVER_H
#pragma once

/**
 * Interfaces::t3_control_device.server.h
 *
 *  Created on: 10.06.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 *
 * t3: ModulManager-Test
 * 	- 3 Modulen:	control, device, worker
 *  - 4 Schnittstellen:
 *  		Control: M_ControlDevice, M_ControlWorker
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

#include <string> // std::string

#include "server/interface.h"
#include "test/t3_control_device.interface.h"
#include "test/t3_supply_worker.proxy.h"
#include "module/interfaces.h" // wg appId
#include "test/t3_device.h" // wg Device::State
#include "geo/size.h" // wg Device::State


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
	using image::SmpImage;
	using image::LImage;
	using geo2d::Size;
	using interface::TT3SupplyWorker;
	/**
	 * TT3ControlDevice ist eine primitive TT3ControlDevice-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3ControlDevice<MsgServer> : public TT3ControlDevice<AbstractInterface>
	{
	public:
		TT3ControlDevice() {}
		TT3ControlDevice(TT3SupplyWorker<EventProxy> &w) : worker_(w) {}

		virtual ~TT3ControlDevice() {}
	public:
		/// initialisierung, ggf Dropback wg Statemachine
		virtual bool init(Device::State state) {
			std::cout << "T3ControlDevice<MsgServer>::init: (" ;
			std::cout << "state: " << state << ")" << std::endl;
//			switch (state) {
//			}
			return true; // Operation gelungen
		}

		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double param0, double param1) {
			std::cout << "T3ControlDevice<MsgServer>::configTask: (" ;
			std::cout << "taskNum: " << taskNum << ", ";
			std::cout << "param0: " << param0 << ", ";
			std::cout << "param1: " << param0 << ")" << std::endl;
		}
		/// Device fuer Task vorbereiten (realTime)
		virtual void prepTask(int taskNum, double specialParam) {
			std::cout << "T3ControlDevice<MsgServer>::prepTask: (" ;
			std::cout << "taskNum: " << taskNum << ", ";
			std::cout << "specialParam: " << specialParam << ")" << std::endl;
		}
		/// Task initiieren (realTime)
		virtual void triggerTask(int taskNum) {
			std::cout << "T3ControlDevice<MsgServer>::triggerTask: (" ;
			std::cout << "taskNum: " << taskNum << ")" << std::endl;
		}
		/// Bild triggern
		virtual void triggerImage(int imageNum) {
			SmpImage image(new LImage(Size(10, 20)));
			worker_.sendImage(imageNum, image);
		}
	private:
		TT3SupplyWorker<EventProxy> worker_;
	};


} // namespace system
} // namespace precitec

#endif /*T3_CONTROL_DEVICE_SERVER_H*/
