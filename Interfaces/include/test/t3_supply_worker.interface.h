#ifndef T3_SUPPLY_WORKER_INTERFACE_H
#define T3_SUPPLY_WORKER_INTERFACE_H
#pragma once

/**
 * Interfaces::t3_supply_worker.interface.h
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

#include "server/interface.h"
#include "message/serializer.h"
#include "test/t3_worker.h" // wg Device::State
#include "test/t3_device.h" // wg Device::State
#include "image/image.h"

/*
 * Hier werden die abstrakten Baisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
	using test::ShpData;

namespace interface
{
	using namespace  system;
	using namespace  message;
	using image::SmpImage;

	/// Vorwaertsdeklaration des generellen Templates das hier Spezialisiert wird
	template <int mode>
	class TT3SupplyWorker;

	/**
	 * PartTransport ist eine primitive PartTransport-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3SupplyWorker<AbstractInterface> {
	public:
		TT3SupplyWorker() {}
		virtual ~TT3SupplyWorker() {}
	public:
		/// versichkt einen int
		virtual void deploy(int data) = 0;
		/// verschickt eine Datenstruktur ueber einen SharedMem
		virtual void deploy(ShpData const& data) = 0;
		/// Bild uebertragen
		virtual void sendImage(int imageNum, SmpImage &image) = 0;
	};

	//----------------------------------------------------------
	template <>
	class TT3SupplyWorker<Messages> : public Server<Messages> {
	public:
		TT3SupplyWorker() : info(module::T3SupplyWorker, sendBufLen, replyBufLen, NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 2000*Bytes, replyBufLen = 100*Bytes };
	public:
		MESSAGE_LIST3(
			module::T3SupplyWorker,
			MESSAGE_NAME1(deploy, int),
			MESSAGE_NAME1(deploy, ShpData),
			MESSAGE_NAME2(sendImage, int, SmpImage)
		);
	public:
		DEFINE_MSG1(void, deploy, int);
		DEFINE_MSG1(void, deploy, ShpData);
		DEFINE_MSG2(void, sendImage, int, SmpImage);
	};


} // namespace events
} // namespace precitec

#endif /*T3_SUPPLY_WORKER_INTERFACE_H*/
