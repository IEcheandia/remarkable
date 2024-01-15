/** @defgroup App_CHRCommunication App_CHRCommunication
 */

/*
 *  AppMain.h for App_CHRCommunication
 *
 *  Created on: 18.10.2017
 *      Author: a.egger
 */

#include <sys/resource.h>

#include "AppMain.h"
#include <common/systemConfiguration.h>


using precitec::grabber::CHRCommunication;

#define MY_NEW_PRIORITY (10)

namespace precitec
{
    using namespace interface;

namespace grabber
{

AppMain::AppMain() : BaseModule(system::module::CHRCommunicationModul),		// wir brauchen immer die moduleId!!
    m_oCHRCommunication(m_oSensorProxy),
    m_oTriggerCmdServer( m_oCHRCommunication ),
    m_oTriggerCmdHandler( &m_oTriggerCmdServer ),
    m_oInspectionServer( m_oCHRCommunication ),
    m_oInspectionHandler( &m_oInspectionServer ),
    m_oDeviceServer( m_oCHRCommunication ),
    m_oDeviceHandler( &m_oDeviceServer ),
    m_oDeviceCalibrationHandler( &m_oDeviceServer)
{
    system::raiseRtPrioLimit();

    if ((m_oCHRCommunication.isIDMDevice1Enabled()) || (m_oCHRCommunication.isCLS2Device1Enabled()))
    {
        m_oCHRCommunication.StartCHRReadThread();
        m_oCHRCommunication.StartCHRCyclicTaskThread(true); // first start of CHRCyclicTaskThread
        m_oCHRCommunication.StartCHRSpectrumTransferThread();
        m_oCHRCommunication.StartCHRWriteThread();
        m_oCHRCommunication.InitCHRCommunication();
    }

    if ((m_oCHRCommunication.isIDMDevice1Enabled()) || (m_oCHRCommunication.isCLS2Device1Enabled()))
    {
        registerPublication( &m_oSensorProxy);

        m_oTriggerCmdHandler.setRealTimePriority(precitec::Priority::InspectionControl);
        m_oInspectionHandler.setRealTimePriority(precitec::Priority::InspectionControl);

        registerSubscription(&m_oTriggerCmdHandler);
        registerSubscription(&m_oInspectionHandler);
    }

    registerSubscription(&m_oDeviceHandler, system::module::CHRCommunicationModul);

    initialize(this);

    ConnectionConfiguration::instance().setInt( pidKeys[CHRCOMMUNICATION_KEY_INDEX], getpid() ); // let ConnectServer know our pid
}

AppMain::~AppMain()
{
}

int AppMain::init(int argc, char * argv[])
{
    processCommandLineArguments(argc, argv);
    notifyStartupFinished();
    for (int i = 0; i < argc; ++i)
    {
    }
    return 0;
}

void AppMain::runClientCode()
{
    std::cout << "runClientCode" << std::endl;

    char wait = 0;
    std::cin >> wait;
}

} // namespace grabber

} // namespace precitec

