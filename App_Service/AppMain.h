/*
 *
 *  Created on: 13.08.2010
 *      Author: f.agrawal
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"


#include "event/viServiceToGUI.proxy.h"
#include "event/viServiceToGUI.handler.h"

#include "event/sensor.h"
#include "event/sensor.proxy.h"



#include "viService/serviceToGuiServer.h"
#include "viService/serviceFromGuiServer.h"

#include "event/viServiceFromGUI.handler.h"

#include "event/viServiceToGUI.proxy.h"
#include "event/viServiceToGUI.handler.h"


#include "event/triggerCmd.h"
#include "event/triggerCmd.interface.h"
#include "event/triggerCmd.handler.h"

#include "event/ethercatInputsToService.handler.h"
#include "viService/EthercatInputsServer.h"

#include "event/ethercatOutputs.proxy.h"

#include "message/device.handler.h"
#include "viService/deviceServer.h"

#include "protocol/protocol.info.h"
#include "common/connectionConfiguration.h"


namespace precitec
{
	using framework::module::BaseModule;
	using namespace viService;
	using namespace interface;

namespace ethercat{


class AppMain : public BaseModule {
public:
	AppMain();
	virtual ~AppMain();

protected:
	virtual void runClientCode();

private:
	TviServiceToGUI<EventProxy> 	viServiceToGuiProxy_;

public:
	ServiceToGuiServer serviceToGuiServer_;
	ServiceFromGuiServer	serviceFromGuiServer_;
	TviServiceFromGUI<EventHandler>	viServiceFromGUIHandler_;

	TEthercatOutputs<EventProxy>    m_oEthercatOutputsProxy;

	EthercatInputsServer            m_oEthercatInputsServer;
	TEthercatInputsToService<EventHandler>   m_oEthercatInputsHandler;

private:
	DeviceServer						m_oDeviceServer;				///< Device server, to configure the VI_Service from wmMain
	TDevice<MsgHandler>					m_oDeviceHandler;				///< Device handler
};
}
}

#endif /* APPMAIN_H_ */
