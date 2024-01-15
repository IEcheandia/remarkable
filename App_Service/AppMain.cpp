/** @defgroup VI_Service VI_Service
 */

/*
 *
 *  Created on: 13.08.2010
 *      Author: f.agrawal
 */
#include "AppMain.h"
#include "system/realTimeSupport.h"

namespace precitec
{
 	 using precitec::viService::ServiceToGuiServer;
	namespace ethercat
{

AppMain::AppMain():
	BaseModule(system::module::VIServiceModul),		// wir brauchen immer die moduleId!!

	serviceToGuiServer_( viServiceToGuiProxy_ ),
	serviceFromGuiServer_(serviceToGuiServer_, m_oEthercatOutputsProxy),
	viServiceFromGUIHandler_(&serviceFromGuiServer_),
	m_oEthercatInputsServer( serviceToGuiServer_ ,serviceFromGuiServer_ ),
	m_oEthercatInputsHandler( &m_oEthercatInputsServer ),
	m_oDeviceHandler( &m_oDeviceServer )
{
    system::raiseRtPrioLimit();

    m_oEthercatInputsHandler.setRealTimePriority(system::Priority::EtherCATDependencies);

	registerSubscription(&m_oEthercatInputsHandler);
	registerSubscription(&viServiceFromGUIHandler_);

	registerPublication(&m_oEthercatOutputsProxy);
	registerPublication(&viServiceToGuiProxy_);

	//start server part (Win->QNX)
	//start client part (QNX->WIN)

    registerSubscription(&m_oDeviceHandler, system::module::VIServiceModul);

	initialize(this);

	ConnectionConfiguration::instance().setInt( pidKeys[SERVICE_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
}

void AppMain::runClientCode()
{
    m_oEthercatOutputsProxy.ecatRequestSlaveInfo();
    notifyStartupFinished();

	char wait = 0;
	std::cin >> wait;
}

} // namespace ethercat
} // namespace precitec

