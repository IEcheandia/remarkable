/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		A secured device server for the weld-head process. This class was introduced because of the switch for the line laser. The line laser is controlled either by the weldhead or by the grabber / camera process.
 */
#ifndef SECUREDWELDHEADSERVER_H_
#define SECUREDWELDHEADSERVER_H_

// clib includes
#include <string>
// WM includes
#include <analyzer/securedDeviceServer.h>
#include <analyzer/securedDeviceHandler.h>

namespace precitec { using namespace interface;
namespace analyzer {

/**
 * @ingroup Workflow
 * @brief Device server that is secured and knows about the grabber handler as well.
 */
class SecuredWeldHeadServer : public SecuredDeviceServer
{
public:

	/**
	 * @brief CTor.
	 * @param p_pGrabberHandler shared_ptr to grabber handler, to be able to call the grabber to control the line laser.
	 * @param p_oLocalPort std::string object with the port number of the qnx device server (e.g. Grabber).
	 * @param id the unique id of the device
	 */
	SecuredWeldHeadServer( const std::shared_ptr< SecuredDeviceHandler > &p_pGrabberHandler, precitec::system::module::Modules module, const Poco::UUID &id );

	/**
	 * @brief Set a single key.
	 * @param p_pKeyValue Shared pointer to key value object.
	 * @param p_oSubDevice Number of sub-device.
	 * @return Returns a handle to the key (the handle is not implemented correctly in the device servers right now).
	 */
	KeyHandle set( SmpKeyValue p_pKeyValue, int p_oSubDevice=0 );
	/**
	 * @brief Set a single key, and do not check the current state of the machine. Attention, this will change the device settings even in automatic mode.
	 * @param p_pKeyValue Shared pointer to key value object.
	 * @param p_oSubDevice Number of sub-device.
	 * @return Returns a handle to the key (the handle is not implemented correctly in the device servers right now).
	 */
	virtual KeyHandle force( SmpKeyValue p_pKeyValue, int p_oSubDevice=0 );
	/**
	 * @brief Set multiple parameters.
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
	SmpKeyValue get( Key p_oKey, int p_oSubDevice=0 );
	/**
	 * @brief Get a single parameter.
	 * @param p_oHandle KeyHandle object.
	 * @param p_oSubDevice Number of sub-device.
	 */
	SmpKeyValue get( KeyHandle p_oHandle, int p_oSubDevice=0 );
	/**
	 * @brief Get all configuration parameters of a device.
	 * @param p_oSubDevice Number of sub-device.
	 * @return Configuration object (std::vector of key-value objects).
	 */
	Configuration get( int p_oSubDevice=0 );

    void init() override;

private:

	std::shared_ptr< SecuredDeviceHandler > m_pGrabberHandler;		///< Pointer to grabber handler, to be able to call the grabber to control the line laser via the camera.
	bool									m_oLineLaser1ViaCamera;	///< State of the switch of the first line laser.
	bool									m_oLineLaser1Enabled;	///< Only for camera connected line lasers: Is the line laser 1 enabled?
	int										m_oLineLaser1Intensity; ///< Only for camera connected line lasers: Current line laser 1 intensity.
	bool									m_oLineLaser2ViaCamera; ///< State of the switch of the second line laser.
	bool									m_oLineLaser2Enabled;	///< Is the line laser 2 enabled?
	int										m_oLineLaser2Intensity; ///< Only for camera connected line lasers: Current line laser 2 intensity.
	bool									m_oFieldLight1ViaCamera;///< State of the switch of the first field light.
	bool									m_oFieldLight1Enabled;	///< Is the field light 1 enabled?
	int										m_oFieldLight1Intensity;///< Only for camera connected field lights: Current field light 1 intensity.
	int										m_oLineLaserCameraCommandSet; ///< Only for camera connected line lasers: Current line laser command set

};

} // namespace workflow
} // namespace precitec

#endif /* SECUREDWELDHEADSERVER_H_ */
