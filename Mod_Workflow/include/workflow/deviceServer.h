/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2016
 * 	@brief		Implements the TDevice interface for key value exchange.
 */

#ifndef DEVICESERVER_H_20161005_INCLUDED
#define DEVICESERVER_H_20161005_INCLUDED

// project includes
#include "message/device.interface.h"
#include "analyzer/inspectManager.h"
#include "analyzer/deviceParameter.h"
// stdlib includes
#include <iostream>

namespace precitec {
namespace workflow {


/**
 * @brief	Device server implementation for video recorder parametrization.
 */
class DeviceServer : public TDevice<AbstractInterface>
{
public:
	/**
	 * @brief CTOR.
	 * @param p_rInspectManager Reference to InspectManager that is parametrized.
	 */
	DeviceServer(analyzer::InspectManager& p_rInspectManager) :
        m_rInspectManager   ( p_rInspectManager ),
        m_rDeviceParameters ( m_rInspectManager.getDeviceParameter() )
	{} // DeviceServer

	/**
	 * @brief	? - not implemented.
	 */
	/*virtual*/ int initialize(const Configuration &p_rConfig, int p_oSubDevice = 0) {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; 
		return -1;
	} // initialize

	/**
	 * @brief	? - not implemented.
	 */
	/*virtual*/ void uninitialize() {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; 
	} // uninitialize

	/**
	 * @brief	? - not implemented.
	 */
	/*virtual*/ void reinitialize() {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; 
	} // reinitialize

	/**
	 * @brief	set a value
	 * @param	p_oSmpKeyValue	key and value
	 * @return	KeyHandle		handle (token)
	 */
	/*virtual*/ KeyHandle set(SmpKeyValue p_oSmpKeyValue, int p_oSubDevice=0) {
		return m_rDeviceParameters.set(p_oSmpKeyValue);
	} // set

	/**
	 * @brief	set a value
	 * @param	p_rConfig		key value container
	 * @return	void
	 */
	/*virtual*/ void set(Configuration p_rConfig, int p_oSubDevice=0) {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n";
	} // set

	/**
	 * @brief	get a value by key
	 * @param	p_oKey			key
	 * @return	SmpKeyValue		key and value
	 */
	/*virtual*/  SmpKeyValue get(Key p_oKey, int p_oSubDevice=0) {
		return m_rDeviceParameters.get(p_oKey);
	} // get

	/**
	 * @brief	get a value by handle (token)
	 * @param	p_oKeyHandle	handle (token)
	 * @return	SmpKeyValue		key and value
	 */
	/*virtual*/ SmpKeyValue get(KeyHandle p_oKeyHandle, int p_oSubDevice=0) {
		std::cerr << "WARNING\t" <<  __FUNCTION__ << "\t: Not implemented.\n"; 
		return SmpKeyValue(new KeyValue(TInt,"?",-1) );
	} // get

	/**
	 * @brief	get the configuration
	 * @return	Configuration	key value container
	 */
	/*virtual*/ Configuration get(int p_oSubDevice=0) {
        const auto oConf = makeConfiguration(m_rDeviceParameters);
        return oConf;
	} // get

private:

	analyzer::InspectManager&			m_rInspectManager; ///< Reference to analyzer.
    analyzer::DeviceParameter&          m_rDeviceParameters;
}; // DeviceServer


}	// workflow
}	// precitec

#endif // DEVICESERVER_H_20161005_INCLUDED
