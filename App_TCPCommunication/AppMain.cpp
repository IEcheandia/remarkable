/** @defgroup App_TCPCommunication App_TCPCommunication
 */

/*
 *  AppMain.h for App_TCPCommunication
 *
 *  Created on: 22.5.2019
 *      Author: a.egger
 */

#include "AppMain.h"

namespace precitec
{

namespace tcpcommunication
{

using namespace interface;

AppMain::AppMain() : BaseModule(system::module::TCPCommunicationModul),    // wir brauchen immer die moduleId!!
    m_oS6K_InfoToProcessesServer( m_oTCPCommunication ),
    m_oS6K_InfoToProcessesHandler( &m_oS6K_InfoToProcessesServer ),
    m_oDbNotificationServer( m_oTCPCommunication ),
    m_oDbNotificationHandler( &m_oDbNotificationServer ),
    m_oInspectionOutServer( m_oTCPCommunication ),
    m_oInspectionOutHandler( &m_oInspectionOutServer ),
    m_oTCPCommunication(m_oS6K_InfoFromProcessesProxy, m_oDbProxy)
{
    registerSubscription(&m_oS6K_InfoToProcessesHandler);
    registerPublication(&m_oS6K_InfoFromProcessesProxy);
    registerSubscription(&m_oDbNotificationHandler);
    registerPublication(&m_oDbProxy);
    registerSubscription(&m_oInspectionOutHandler);

    initialize(this);

    ConnectionConfiguration::instance().setInt( pidKeys[TCPCOMMUNICATION_KEY_INDEX], getpid() ); // let ConnectServer know our pid
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

} // namespace tcpcommunication

} // namespace precitec

