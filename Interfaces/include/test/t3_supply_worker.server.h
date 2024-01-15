#ifndef T3_SUPPLY_WORKER_SERVER_H
#define T3_SUPPLY_WORKER_SERVER_H
#pragma once

/**
 * Interfaces::t3_supply_worker.server.h
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

#include <string> // std::string

#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "system/sharedMem.h" // wg appId

#include "test/t3_supply_worker.interface.h"
#include "test/t3_device.h" // wg Device::State
#include "test/t3_worker.h" // wg Device::State



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
using image::AImage;
using system::ShMemPtr;

	/**
	 * TT3SupplyWorker ist eine primitive TT3SupplyWorker-Abstraktion. Sie kann bisher nur
	 * ein paar (ver)einfach(te) Befehle.
	 */
	template <>
	class TT3SupplyWorker<EventServer> : public TT3SupplyWorker<AbstractInterface> {
	public:
		TT3SupplyWorker(SharedMem &sm) :  shMem_(sm) {
			ShMemPtr<byte> sharedPtr(sm);
			byte *p = &sharedPtr(0);
			for (int i=0; i<100; ++i) {
				std::cout << int(p[i]) << " ";
			}
			std::cout << std::endl;
		}
		virtual ~TT3SupplyWorker() {}
	public:
		/// versichkt einen int
		virtual void deploy(int data) {
		}
		/// verschickt eine Datenstruktur ueber einen SharedMem

		virtual void deploy(ShpData const& data) {

		}
		/// Bild uebertragen
		virtual void sendImage(int imageNum, SmpImage &image) {
			std::cout << "imageNum: " << imageNum << std::endl;
			AImage &i(*image);

			std::cout << "image: " << i.size() << std::endl;

			for (int y=0; y<4; ++y) {
				for (int x=0; x<4; ++x) {
					std::cout << int(i(x, y)) << " ";
				}
				std::cout	<< std::endl;
			}
			std::cout << "server: sendimage ok: " << imageNum << std::endl;
		}
	private:
		/// vom Handler angelegtes Shered-Mem auf das die Pointer im SmpLImage zugreifen
		SharedMem &shMem_;
	};


} // namespace system
} // namespace precitec

#endif /*T3_SUPPLY_WORKER_SERVER_H*/
