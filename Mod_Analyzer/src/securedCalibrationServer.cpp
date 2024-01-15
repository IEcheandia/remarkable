/*
 * SecuredCalibrationServer.cpp
 *
 *  Created on: Oct 18, 2013
 *      Author: abeschorner
 */

#include "analyzer/securedCalibrationServer.h"
#include "module/moduleLogger.h"


namespace precitec {
namespace analyzer {

SecuredCalibrationServer::SecuredCalibrationServer( std::string p_oControlName, precitec::system::module::Modules module, const Poco::UUID &id ) :
		SecuredDeviceServer(p_oControlName, module, id)
{
}

SecuredCalibrationServer::~SecuredCalibrationServer()
{
	// TODO Auto-generated destructor stub
}

KeyHandle SecuredCalibrationServer::set( SmpKeyValue p_pKeyValue, int p_oSubDevice )
{
	// is the server locked?
	if (!isSecure())
	{
		wmLogTr( eError, "Error.Device.SetNotSave", "Cannot change configuration of %s (%s) in this state!\n", name().c_str(), p_pKeyValue->key().c_str() );
		return KeyHandle();
	}


	return SecuredDeviceServer::set( p_pKeyValue, p_oSubDevice );

} // set

} /* namespace analyzer */
} /* namespace precitec */
