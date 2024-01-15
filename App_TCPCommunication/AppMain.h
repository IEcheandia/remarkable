/*
 *  AppMain.h for App_TCPCommunication
 *
 *  Created on: 22.5.2019
 *      Author: a.egger
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
#include "module/baseModule.h"

#include "common/connectionConfiguration.h"

#include "event/S6K_InfoToProcesses.handler.h"
#include "TCPCommunication/S6K_InfoToProcessesServer.h"

#include "event/S6K_InfoFromProcesses.proxy.h"

#include "event/dbNotification.handler.h"
#include "TCPCommunication/DbNotificationServer.h"

#include "message/db.proxy.h"

#include "event/inspectionToS6k.handler.h"
#include "TCPCommunication/InspectionOutServer.h"

#include "TCPCommunication/TCPCommunication.h"

namespace precitec
{

namespace tcpcommunication
{

class AppMain : public framework::module::BaseModule
{
public:
    AppMain();
    virtual ~AppMain();

    virtual void runClientCode();
    int init(int argc, char * argv[]);

private:
    S6K_InfoToProcessesServer            m_oS6K_InfoToProcessesServer;
    TS6K_InfoToProcesses<EventHandler>   m_oS6K_InfoToProcessesHandler;

    TS6K_InfoFromProcesses<EventProxy>   m_oS6K_InfoFromProcessesProxy;

    DbNotificationServer                 m_oDbNotificationServer;
    TDbNotification<EventHandler>        m_oDbNotificationHandler;

    TDb<MsgProxy>                        m_oDbProxy;

    InspectionOutServer                  m_oInspectionOutServer;
    TInspectionToS6k<EventHandler>       m_oInspectionOutHandler;

    TCPCommunication                     m_oTCPCommunication;
};

} // namespace tcpcommunication

} // namespace precitec

#endif /* APPMAIN_H_ */

