/**
 * @file
 * @brief This device server is secured and only permits changes when the state is "normal mode" / aka as eOperate.
 *
 * @author 	SB
 * @date	06.09.12
 */
#ifndef SECUREDDEVICESERVER_H_
#define SECUREDDEVICESERVER_H_

// clib includes
#include <string>
// WM includes
#include "event/deviceNotification.interface.h"
#include "message/device.h"
#include "message/device.interface.h"
#include "message/device.proxy.h"

namespace precitec { using namespace interface;

namespace framework
{
namespace module
{
class ModuleManagerConnector;
}
}

namespace analyzer {

class SecuredDeviceHandler;

/**
 * @ingroup Workflow
 * @brief Device server that is secured and only permits changes when the state is "normal mode" / aka as eOperate.
 */
class SecuredDeviceServer : public TDevice<AbstractInterface>
{

public:
    SecuredDeviceServer(std::string p_oName, precitec::system::module::Modules module, const Poco::UUID &id);

    SecuredDeviceServer(std::string p_oName, TDevice<AbstractInterface> *device, const Poco::UUID &id);

	/**
	 * @brief Initialize device / process. Function is not called right now - so we do not call anything either.
	 * @param p_rConfig reference to a configuration object (std::vector of key-value objects).
	 * @param p_oSubDevice number of sub-device that should be initialized, e.g. in the case of multiple cameras connected to a single grabber. Default: 0.
	 * @return ???
	 */
	int initialize( Configuration const& p_rConfig, int p_oSubDevice=0 );
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

	/**
	 * @brief Checks if it is save to change the configuration, i.e. if the system is not in automatic mode. Right now, only the "normal mode" / eOperate and "live mode" are allowed.
	 * @return True if it is save to call the remote proxy.
	 */
	bool isSecure();

	/**
	 * @brief Lock the server, i.e. access to the device interface is blocked (automatic mode).
	 * @param p_oLocked if true, the server should blocks the calls.
	 */
	void lock( bool p_oLocked );

    void setDeviceNotification(const std::shared_ptr<TDeviceNotification<AbstractInterface>> &deviceNotification)
    {
        m_deviceNotification = deviceNotification;
    }

    void publish(precitec::framework::module::ModuleManagerConnector *moduleManager);

    virtual void init();

protected:
    const std::string &name() const
    {
        return m_oName;
    }

    TDevice<AbstractInterface> *proxy()
    {
        return m_device;
    }

private:

	std::string			m_oName;			///< Name of interface.
	std::unique_ptr<TDevice<MsgProxy>> 	m_pProxy;			///< Remote proxy to actual device server.
	bool				m_oLocked;			///< Is the device manager locked, i.e. access to the device interfaces is blocked (automatic mode).
	TDevice<AbstractInterface> *m_device = nullptr;

    void notify(const std::string &key);

    std::shared_ptr<TDeviceNotification<AbstractInterface>> m_deviceNotification;
    const Poco::UUID &m_id;
    precitec::system::module::Modules m_module = precitec::system::module::AnyModule;
};

} // namespace workflow
} // namespace precitec

#endif /* SECUREDDEVICESERVER_H_ */
