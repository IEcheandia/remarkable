/** @defgroup VI_WeldHead VI_WeldHead
 */

/*
 *
 *  Created on: 18.05.2010
 *      Author: f.agrawal
 */

#include "AppMain.h"

#include <sys/resource.h>

#include "common/systemConfiguration.h"

using precitec::ethercat::WeldingHeadControl;

namespace precitec
{

	using precitec::viWeldHead::OutMotionDataServer;
	using namespace interface;
	using namespace hardware;

namespace ethercat
{

AppMain::AppMain() : BaseModule(system::module::VIWeldHeadControl),		// wir brauchen immer die moduleId!!
	outMotionDataServer_( outMotionDataProxy_ ),
	m_weldHeadControl( outMotionDataServer_, m_oSensorProxy, m_oEthercatOutputsProxy ),
	viWeldHeadSubscribeHandler_( &m_weldHeadControl ),
	weldHeadMsgHandler_( &m_weldHeadControl ),
	m_oTriggerCmdServer( m_weldHeadControl ),
	m_oTriggerCmdHandler( &m_oTriggerCmdServer ),
	m_oInspectionServer( m_weldHeadControl ),
	m_oInspectionHandler( &m_oInspectionServer ),
	m_oInspectionCmdServer( m_weldHeadControl ),
	m_oInspectionCmdHandler( &m_oInspectionCmdServer ),
	m_oEthercatInputsServer( m_weldHeadControl ),
	m_oEthercatInputsHandler( &m_oEthercatInputsServer ),
	m_oDeviceServer( m_weldHeadControl ),
	m_oDeviceHandler( &m_oDeviceServer ),
    m_oResultsServer(m_weldHeadControl),
    m_oResultsHandler(&m_oResultsServer),
    m_resultsProxy(new TResults<EventProxy>{}),
    m_deviceNotificationProxy(std::make_shared<interface::TDeviceNotification<EventProxy>>())
{

	registerPublication( &outMotionDataProxy_ );
	registerPublication( &m_oSensorProxy);
    registerPublication(m_resultsProxy.get());

	registerPublication(&m_oEthercatOutputsProxy);

    weldHeadMsgHandler_.setRealTimePriority(system::Priority::InspectionControl);
    m_oTriggerCmdHandler.setRealTimePriority(system::Priority::InspectionControl);
    m_oInspectionHandler.setRealTimePriority(system::Priority::InspectionControl);
    m_oInspectionCmdHandler.setRealTimePriority(system::Priority::InspectionControl);
    m_oEthercatInputsHandler.setRealTimePriority(system::Priority::EtherCATDependencies);
    m_oResultsHandler.setRealTimePriority(system::Priority::Results);

	registerSubscription(&viWeldHeadSubscribeHandler_);
	registerSubscription(&weldHeadMsgHandler_);
	registerSubscription(&m_oTriggerCmdHandler);
	registerSubscription(&m_oInspectionHandler);
	registerSubscription(&m_oInspectionCmdHandler);
	registerSubscription(&m_oEthercatInputsHandler);
    registerSubscription(&m_oDeviceHandler, system::module::VIWeldHeadControl);
    registerSubscription(&m_oResultsHandler);
    registerPublication(m_deviceNotificationProxy.get());
    
	initialize(this);

    m_weldHeadControl.getScanlab()->setResultsProxy(m_resultsProxy);
	m_weldHeadControl.getScanlab()->setDeviceNotificationProxy(m_deviceNotificationProxy);
    m_oResultsServer.setResultsProxy(m_resultsProxy);

	ConnectionConfiguration::instance().setInt( pidKeys[WELDHEADCONTROL_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
}

int AppMain::init(int argc, char * argv[])
{
	processCommandLineArguments(argc, argv);
	for (int i = 0; i < argc; ++i)
	{
	}
	return 0;
}

void AppMain::runClientCode()
{
	std::cout << "runClientCode" << std::endl;
	notifyStartupFinished();

	char wait = 0;
	std::cin >> wait;
}

} // namespace ethercat

} // namespace precitec

