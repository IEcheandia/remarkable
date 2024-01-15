#ifndef T3_CONTROL_DEVICE_PROXY_H
#define T3_CONTROL_DEVICE_PROXY_H
#pragma once

/**
 * Interfaces::t3_control_device.proxy.h
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

#include "server/proxy.h"
#include "test/t3_control_device.interface.h"
#include "test/t3_device.h" // wg Device::State


namespace precitec
{
	using test::Device;
namespace interface
{

	template <>
	class TT3ControlDevice<MsgProxy> : public Server<MsgProxy>, public TT3ControlDevice<AbstractInterface> {

	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TT3ControlDevice()
		: PROXY_CTOR(TT3ControlDevice), TT3ControlDevice<AbstractInterface>() {}

		/// normalerweise wird das Protokoll gleich mitgeliefert
		TT3ControlDevice(SmpProtocolInfo & p)
		: PROXY_CTOR1(TT3ControlDevice,  p), TT3ControlDevice<AbstractInterface>() {}

		virtual ~TT3ControlDevice() {}

	public:
		/// initialisierung, ggf Dropback wg Statemachine
		/*virtual bool init(Device::State state) {
			CREATE_MESSAGE1(TT3ControlDevice, init, int);
			sender().initMessage(Msg::index);
			sender().marshal(state);
			sender().send();
			try {
				bool ret;	sender().deMarshal(ret);
				return ret;
			} catch {
				return false;
			}
		}

		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double param0, double param1) {
			CREATE_MESSAGE3(TT3ControlDevice, configTask, int, double, double);
			sender().initMessage(Msg::index);
			sender().marshal(taskNum);
			sender().marshal(param0);
			sender().marshal(param1);
			sender().send();
		}

		/// Device fuer Task vorbereiten (realTime)
		virtual void prepTask(int taskNum, double specialParam) {
			CREATE_MESSAGE2(TT3ControlDevice, prepTask, int, double);
			sender().initMessage(Msg::index);
			sender().marshal(taskNum);
			sender().marshal(specialParam);
			sender().send();
		}

		/// Task initiieren (realTime)
		virtual void triggerTask(int taskNum) {
			CREATE_MESSAGE1(TT3ControlDevice, triggerTask, int);
			sender().initMessage(Msg::index);
			sender().marshal(taskNum);
			sender().send();
		}
*/
		virtual bool init(Device::State state) {
			INIT_MESSAGE1(TT3ControlDevice, init, int);
			marshal(state);
			// Parameter ist Returnwert bei Kommunikaitonsfehler
			return sendWithReturn(false);
		}

		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double param0, double param1) {
			INIT_MESSAGE3(TT3ControlDevice, configTask, int, double, double);
			marshal(taskNum);
			marshal(param0);
			marshal(param1);
			send();
		}

		/// Device fuer Task vorbereiten (realTime)
		virtual void prepTask(int taskNum, double specialParam) {
			INIT_MESSAGE2(TT3ControlDevice, prepTask, int, double);
			marshal(taskNum);
			marshal(specialParam);
			send();
		}

		/// Task initiieren (realTime)
		virtual void triggerTask(int taskNum) {
			INIT_MESSAGE1(TT3ControlDevice, triggerTask, int);
			marshal(taskNum);
			send();
		}

		/// Imageaufnahme antriggern
		virtual void triggerImage(int imageNum) {
			INIT_MESSAGE1(TT3ControlDevice, triggerImage, int);
			marshal(imageNum);
			send();
		}


	}; // class TT3ControlDevice<MsgProxy>

} // namespace interface
} // namespace precitec

#endif /*T3_CONTROL_DEVICE_PROXY_H*/
