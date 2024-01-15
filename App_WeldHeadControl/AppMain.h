/*
 *
 *  Created on: 18.05.2010
 *      Author: f.agrawal
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
#include "module/baseModule.h"

#include "viWeldHead/outMotionDataServer.h"

#include "event/viWeldHeadPublish.proxy.h"
#include "event/viWeldHeadSubscribe.handler.h"

#include "message/weldHead.handler.h"

#include "event/sensor.proxy.h"
#include "event/results.proxy.h"

#include "event/triggerCmd.handler.h"
#include "viWeldHead/TriggerCmdServer.h"

#include "event/inspection.handler.h"
#include "viWeldHead/InspectionServer.h"

#include "event/inspectionCmd.handler.h"
#include "viWeldHead/InspectionCmdServer.h"

#include "event/ethercatInputs.handler.h"
#include "viWeldHead/EthercatInputsServer.h"

#include "message/device.handler.h"
#include "viWeldHead/deviceServer.h"

#include "viWeldHead/VICallbackBase.h"

#include "protocol/protocol.info.h"
#include "common/connectionConfiguration.h"

#include "viWeldHead/WeldingHeadControl.h"
#include "viWeldHead/ResultsServer.h"

#include "event/deviceNotification.proxy.h"
namespace precitec
{

namespace ethercat
{

class AppMain : public framework::module::BaseModule
{
public:
	AppMain();
	virtual ~AppMain();

	virtual void runClientCode();
	int init(int argc, char * argv[]);

private:
	TviWeldHeadPublish<EventProxy> 	outMotionDataProxy_;
	OutMotionDataServer    outMotionDataServer_;

	TSensor<EventProxy>    m_oSensorProxy;

	TEthercatOutputs<EventProxy>        m_oEthercatOutputsProxy;

	WeldingHeadControl     m_weldHeadControl;
	TviWeldHeadSubscribe<EventHandler>	viWeldHeadSubscribeHandler_;
	TWeldHeadMsg<MsgHandler> weldHeadMsgHandler_;

	TriggerCmdServer                    m_oTriggerCmdServer;
	TTriggerCmd<EventHandler>           m_oTriggerCmdHandler;

	InspectionServer                    m_oInspectionServer;
	TInspection<EventHandler>           m_oInspectionHandler;

	InspectionCmdServer                 m_oInspectionCmdServer;
	TInspectionCmd<EventHandler>        m_oInspectionCmdHandler;

	EthercatInputsServer                m_oEthercatInputsServer;
	TEthercatInputs<EventHandler>       m_oEthercatInputsHandler;

	DeviceServer						m_oDeviceServer;				///< Device server, to configure the VI_WeldHeadControl from wmMain
	TDevice<MsgHandler>					m_oDeviceHandler;				///< Device handler

    viCommunicator::ResultsServer     m_oResultsServer;
    TResults<EventHandler>            m_oResultsHandler;
    std::shared_ptr<TResults<EventProxy>> m_resultsProxy;
    std::shared_ptr<interface::TDeviceNotification<EventProxy>> m_deviceNotificationProxy;
};

} // namespace ethercat

} // namespace precitec

#endif /* APPMAIN_H_ */

