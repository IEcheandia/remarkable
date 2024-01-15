/**
 * @file
 * @brief The device handler corresponding to the secured device server. It receives the function calls from the windows side and redirects them to the server.
 *
 * @author 	SB
 * @date	06.09.12
 */
#ifndef SECUREDDEVICEHANDLER_H_
#define SECUREDDEVICEHANDLER_H_

// clib includes
#include <string>
// WM includes
#include "message/module.h"
#include "message/device.h"
#include "message/device.interface.h"
#include "message/device.handler.h"
#include "analyzer/securedDeviceServer.h"

namespace precitec { using namespace interface;

namespace framework
{
namespace module
{
class ModuleManagerConnector;
}
}

namespace analyzer {

/**
 * @ingroup Workflow
 * @brief The device handler corresponding to the secured device server. It receives the function calls from the windows side and redirects them to the server.
 */
class SecuredDeviceHandler
{

public:

	/**
	 * @brief CTor.
	 * @param p_oName Name of the device interface, e.g. "VideoRecorder" - used only for debugging and logging.
	 * @param p_oRemotePort std::string object with the port number of the windows/wmHost side.
	 */
	SecuredDeviceHandler( std::shared_ptr<SecuredDeviceServer> &&p_pServer, std::string p_oName);

	/**
	 * @brief Get a reference to the server object.
	 * @return Shared pointer to the secured device server.
	 */
	std::shared_ptr<SecuredDeviceServer> getServer();

	/**
	 * @brief Lock the handler, i.e. access to the device interface is blocked (automatic mode).
	 * @param p_oLocked if true, the handler should blocks the calls.
	 */
	void lock( bool p_oLocked );

    void subscribe(precitec::framework::module::ModuleManagerConnector *moduleManager);

protected:

	std::shared_ptr<SecuredDeviceServer>	m_pServer;		///< Secure server that does the proxy call to the remote server at the actual qnx process.
	TDevice<MsgHandler> 					m_oHandler;		///< Handler where the device interface calls from the gui arrive.

private:
    std::string m_name;

};

} // namespace workflow
} // namespace precitec


#endif /* SECUREDDEVICEHANDLER_H_ */
