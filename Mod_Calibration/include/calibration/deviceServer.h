/**
 *  @file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2013
 *  @brief 		Calibration device server - used to parameterize the calibration, but also to report results back to the user / gui side.
 */

#ifndef CAL_DEVICESERVER_H_
#define CAL_DEVICESERVER_H_

// clib includes
#include <string>
// project includes
#include <message/device.h>
#include <message/device.interface.h>
#include <math/calibrationData.h>
#include <calibration/calibrationManager.h>

namespace precitec {
using namespace interface;
namespace calibration {

/**
 * @ingroup Calibration
 * @brief Calibration device server - used to parameterize the calibration, but also to report results back to the user / gui side.
 */
class DeviceServer : public TDevice<AbstractInterface>
{
public:

	/**
	 * @brief CTor.
	 */
	DeviceServer( CalibrationManager& p_rCalibrationManager);
	/**
	 * @brief DTor.
	 */
	virtual ~DeviceServer();

	/**
	 * @brief Initialize device / process. Function is not called right now!
	 * @param p_rConfig reference to a configuration object (std::vector of key-value objects).
	 * @param p_oSubDevice number of sub-device that should be initialized, e.g. in the case of multiple cameras connected to a single grabber. Default: 0.
	 * @return ???
	 */
	int initialize(Configuration const& config, int subDevice=0);
	/**
	 * @brief Turn device off. Function is not called right now.
	 */
	void uninitialize();
	/**
	 * @brief Reset a device. Function is not called right now.
	 */
	void reinitialize();

	/**
	 * @brief Set a single key.
	 * @param p_pKeyValue Shared pointer to key value object.
	 * @param p_oSubDevice Number of sub-device.
	 * @return Returns a handle to the key (the handle is not implemented correctly in the device servers right now).
	 */
	KeyHandle set(SmpKeyValue keyValue, int subDevice=0);
	/**
	 * @brief Set multiple parameters. Currently not supported by the calibration device server!
	 * @param p_oConfig Configuration object, which is a vector of single key-value object.
	 * @param p_oSubDevice Number of sub-device.
	 */
	void set( Configuration p_oConfig, int p_oSubDevice=0 );

	/**
	 * @brief Get a single parameter.
	 * @param p_oKey Key object (e.g. 'ExposureTime').
	 * @param p_oSubDevice Number of sub-device.
	 * @return
	 */
	SmpKeyValue get(Key key, int subDevice=0);
	/**
	 * @brief Get a single parameter based on the key handle. Currently not supported by the calibration server.
	 * @param p_oHandle KeyHandle object.
	 * @param p_oSubDevice Number of sub-device.
	 */
	SmpKeyValue get(KeyHandle handle, int subDevice=0);

	/**
	 * @brief Get all parameters of the calibration device.
	 * @param p_oSubDevice Number of sub-device.
	 * @return Configuration object (std::vector of key-value objects).
	 */
	Configuration get(int subDevice=0);


private:

	CalibrationManager& m_rCalibrationManager;	///< Reference to calibration manager.

};


} // namespace calibration
} // namespace precitec


#endif /*DEVICESERVER_H_*/
