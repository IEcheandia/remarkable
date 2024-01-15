/**
 * @file
 * @brief   Headerfile zum DeviceServer von VI_InspectionControl
 *
 * @author  AL
 * @date    09.10.2014
 * @version 1.0
 */

#ifndef INSPECTION_DEVICESERVER_H_
#define INSPECTION_DEVICESERVER_H_

#include "message/device.interface.h"

#include "viInspectionControl/VI_InspectionControl.h"

namespace precitec
{
	using namespace interface;

namespace ethercat
{

class DeviceServer : public TDevice<AbstractInterface>
{
	public:
		DeviceServer(VI_InspectionControl& p_rInspectionControl);
		virtual ~DeviceServer();

		//TDeviceInterface

		/// das Einschalten
		int initialize(Configuration const& config, int subDevice=0);
		/// die Reset-Taste
		void uninitialize();
		/// kann beliebig oft durchgefuehrt werden
		void reinitialize();

		/// spezifischen Parameter setzen
		KeyHandle set(SmpKeyValue keyValue, int subDevice=0);
		/// mehrere Parameter setzen
		void set(Configuration config, int subDevice=0);

		/// spezifischen Parameter auslesen
		SmpKeyValue get(Key key, int subDevice=0);
		/// spezifischen Parameter auslesen
		SmpKeyValue get(KeyHandle handle, int subDevice=0);
		/// alle Parameter auslesen
		Configuration get(int subDevice=0);

	private:
		VI_InspectionControl& m_rInspectionControl;

};

} // namespace ethercat

} // namespace precitec

#endif /* INSPECTION_DEVICESERVER_H_ */

