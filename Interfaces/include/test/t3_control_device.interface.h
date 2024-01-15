#ifndef T3_CONTROL_DEVICE_INTERFACE_H
#define T3_CONTROL_DEVICE_INTERFACE_H
#pragma once

/**
 * Interfaces::t3_control_device.interface.h
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

#include "server/interface.h"
#include "message/serializer.h"
#include "test/t3_device.h" // wg Device::State

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using namespace  system;
	using namespace  message;
	using test::Device;
namespace interface
{

	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int mode>
	class TT3ControlDevice;

	/**
	 * PartTransport ist eine primitive PartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3ControlDevice<AbstractInterface>
	{
	public:
		TT3ControlDevice() {}
		virtual ~TT3ControlDevice() {}
	public:
		/// initialisierung, ggf Dropback wg Statemachine
		virtual bool init(Device::State state) = 0;
		/// Daten fuer Task laden (offline)
		virtual void configTask(int taskNum, double parm0, double param1) = 0;
		/// Device fuer Task vorbereiten (realTime)
		virtual void prepTask(int taskNum, double specialParam) = 0;
		/// Task initiieren (realTime)
		virtual void triggerTask(int taskNum) = 0;
		/// Bild uebertragen
		virtual void triggerImage(int imageNum) = 0;
	};


	//----------------------------------------------------------
	template <>
	class TT3ControlDevice<Messages> : public Server<Messages> {
	public:
		TT3ControlDevice() : info(module::T3ControlDevice, sendBufLen, replyBufLen, NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 200*Bytes, replyBufLen = 100*Bytes };
	public:
		MESSAGE_LIST5(
			module::T3ControlDevice,
			MESSAGE_NAME1(init, int),
			MESSAGE_NAME3(configTask, int, double, double),
			MESSAGE_NAME2(prepTask, int, double),
			MESSAGE_NAME1(triggerTask, int),
			MESSAGE_NAME1(triggerImage, int)
		);
	public:
		DEFINE_MSG1(bool, init, int);
		DEFINE_MSG3(void, configTask, int, double, double);
		DEFINE_MSG2(void, prepTask, int, double);
		DEFINE_MSG1(void, triggerTask, int);
		DEFINE_MSG1(void, triggerImage, int);
	};


} // namespace events
} // namespace precitec

#endif /*T3_CONTROL_DEVICE_INTERFACE_H*/
