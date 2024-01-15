/*
 * SecuredCalibrationServer.h
 *
 *  Created on: Oct 18, 2013
 *      Author: abeschorner
 */

#ifndef SECUREDCALIBRATIONSERVER_H_
#define SECUREDCALIBRATIONSERVER_H_

#include "securedDeviceHandler.h"

namespace precitec {
namespace analyzer {

class SecuredCalibrationServer: public precitec::analyzer::SecuredDeviceServer {
public:
	SecuredCalibrationServer( std::string p_oControlName, precitec::system::module::Modules module, const Poco::UUID &id );
	virtual ~SecuredCalibrationServer();

	/**
	 * @brief Set a single key.
	 * @param p_pKeyValue Shared pointer to key value object.
	 * @param p_oSubDevice Number of sub-device.
	 * @return Returns a handle to the key (the handle is not implemented correctly in the device servers right now).
	 */
	KeyHandle set( SmpKeyValue p_pKeyValue, int p_oSubDevice=0 );
};

} /* namespace analyzer */
} /* namespace precitec */
#endif /* SECUREDCALIBRATIONSERVER_H_ */
