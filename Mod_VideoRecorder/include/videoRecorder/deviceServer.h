/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2012
 * 	@brief		Implements the TDevice interface for key value exchange.
 */

#ifndef DEVICESERVER_H_20120319_INCLUDED
#define DEVICESERVER_H_20120319_INCLUDED

// project includes
#include "message/device.interface.h"
#include "videoRecorder/videoRecorder.h"
// stl includes
#include <iostream>

namespace precitec {
namespace vdr {


/**
 * @brief	Device server implementation for video recorder parametrization.
 */
class DeviceServer : public TDevice<AbstractInterface>
{
public:
	/**
	 * @brief CTOR.
	 * @param p_rVideoRecorder Reference to VideoRecorder that is parametrized.
	 */
	DeviceServer(VideoRecorder &p_rVideoRecorder) :
		m_rVideoRecorder( p_rVideoRecorder )
	{} // DeviceServer

	/**
	 * @brief	? - not implemented.
	 */
	/*virtual*/ int initialize(const Configuration &p_rConfig, int p_oSubDevice = 0) {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; // TODDO LOGGER
		return -1;
	} // initialize

	/**
	 * @brief	? - not implemented.
	 */
	/*virtual*/ void uninitialize() {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; // TODDO LOGGER
	} // uninitialize

	/**
	 * @brief	? - not implemented.
	 */
	/*virtual*/ void reinitialize() {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; // TODDO LOGGER
	} // reinitialize

	/**
	 * @brief	set a value
	 * @param	p_oSmpKeyValue	key and value
	 * @return	KeyHandle		handle (token)
	 */
	/*virtual*/ KeyHandle set(SmpKeyValue p_oSmpKeyValue, int p_oSubDevice=0) {
		return m_rVideoRecorder.set(p_oSmpKeyValue);
	} // set

	/**
	 * @brief	set a value
	 * @param	p_rConfig		key value container
	 * @return	void
	 */
	/*virtual*/ void set(Configuration p_rConfig, int p_oSubDevice=0) {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; // TODDO LOGGER
	} // set

	/**
	 * @brief	get a value by key
	 * @param	p_oKey			key
	 * @return	SmpKeyValue		key and value
	 */
	/*virtual*/  SmpKeyValue get(Key p_oKey, int p_oSubDevice=0) {
		return m_rVideoRecorder.get(p_oKey);
	} // get

	/**
	 * @brief	get a value by handle (token)
	 * @param	p_oKeyHandle	handle (token)
	 * @return	SmpKeyValue		key and value
	 */
	/*virtual*/ SmpKeyValue get(KeyHandle p_oKeyHandle, int p_oSubDevice=0) {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; // TODDO LOGGER
		return SmpKeyValue(new KeyValue(TInt,"?",-1) );
	} // get

	/**
	 * @brief	get the configuration
	 * @return	Configuration	key value container
	 */
	/*virtual*/ Configuration get(int p_oSubDevice=0) {
		return m_rVideoRecorder.getConfig();
	} // get

private:
	VideoRecorder&			m_rVideoRecorder; ///< Reference to videoRecorder.
}; // DeviceServer


}	// vdr
}	// precitec

#endif // DEVICESERVER_H_20120319_INCLUDED
