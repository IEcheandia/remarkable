/**
 * @file
 * @brief The device handler corresponding to the secured device server. It receives the function calls from the windows side and redirects them to the server.
 *
 * @author 	SB
 * @date	06.09.12
 */

#include "analyzer/securedDeviceHandler.h"
#include "analyzer/securedDeviceServer.h"
#include "common/connectionConfiguration.h"
#include "module/moduleManagerConnector.h"

namespace precitec { using namespace interface;
namespace analyzer {



SecuredDeviceHandler::SecuredDeviceHandler( std::shared_ptr<SecuredDeviceServer> &&p_pServer, std::string p_oName) :
	m_pServer( std::move(p_pServer) ),
	m_oHandler( m_pServer.get() ),
	m_name(p_oName)
{
	wmLog( eDebug, "SecuredDeviceHandler initializing %s interface.\n", p_oName.c_str() );

} // CTor

void SecuredDeviceHandler::subscribe(precitec::framework::module::ModuleManagerConnector *moduleManager)
{
    moduleManager->registerSubscription(&m_oHandler, system::module::WorkflowModul, m_name);
}

std::shared_ptr<SecuredDeviceServer> SecuredDeviceHandler::getServer()
{
	return m_pServer;

} // server



void SecuredDeviceHandler::lock( bool p_oLocked )
{
	m_pServer->lock( p_oLocked );

} // lock



} // namespace workflow
} // namespace precitec
