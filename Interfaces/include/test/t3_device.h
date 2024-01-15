#ifndef T3_DEVICE_H_
#define T3_DEVICE_H_
#pragma once

/**
 * Interfaces::t3_device.h
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
 * T3Device: generisches Hardware-Device
 *  - ist Server fuer M-ControlDevice
 *  - ist Publisher fuer E_InformControl
 */

#include "system/sharedMem.h" // wg Device::State

namespace precitec
{
namespace test
{
	struct Data {
		int x[100];
		friend  std::ostream &operator <<(std::ostream &os, Data const& d) {
			os << "Data: " << d.x[0] << " "; return os;
		}
	};
	typedef system::ShMemPtr<Data> ShpData;

	class Device {
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

#endif // T3_DEVICE_H_
