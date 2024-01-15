/**
 * @file
 * @brief   Headerfile zum DeviceServer von VI_WeldHeadControl
 *
 * @author  EA
 * @date    15.05.2013
 * @version 1.0
 */

#ifndef WELDHEAD_DEVICESERVER_H_
#define WELDHEAD_DEVICESERVER_H_

#include "message/device.h"
#include "message/device.interface.h"

#include "viWeldHead/WeldingHeadControl.h"

namespace precitec
{
	using namespace interface;

namespace ethercat
{

class DeviceServer : public TDevice<AbstractInterface>
{
	public:
		DeviceServer(WeldingHeadControl& p_rWeldingHeadControl);
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
		WeldingHeadControl& m_rWeldingHeadControl;

		//bool m_oTestStatus1;
		//bool m_oTestStatus2;
		//int m_oTestValue1;
};

} // namespace ethercat

} // namespace precitec

#endif /* WELDHEAD_DEVICESERVER_H_ */

