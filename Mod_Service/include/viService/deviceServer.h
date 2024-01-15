/**
 * @file
 * @brief   Headerfile zum DeviceServer von VI_Service
 *
 * @author  EA
 * @date    14.02.2014
 * @version 1.0
 */

#ifndef SERVICE_DEVICESERVER_H_
#define SERVICE_DEVICESERVER_H_

#include "message/device.h"
#include "message/device.interface.h"

namespace precitec
{
	using namespace interface;

namespace ethercat
{

class DeviceServer : public TDevice<AbstractInterface>
{
	public:
		DeviceServer();
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
};

} // namespace ethercat

} // namespace precitec

#endif /* SERVICE_DEVICESERVER_H_ */

