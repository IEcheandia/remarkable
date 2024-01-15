/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Claudius Batzlen (CB)
 * 	@date		2018
 * 	@brief		Implements the UPS in Framework for Shutdown and User infos.
 */

#define BUFFER_LENGTH 200
#define RECVPORT 55555  //Port defined in NUT notifycmd script


#include "workflow/upsMonitorServer.h"

#include <unistd.h>
#include <pthread.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>

#include "module/moduleLogger.h"
#include "event/systemStatus.h"
#include "common/connectionConfiguration.h"

#include <signal.h>
#include <sys/prctl.h>

using namespace precitec::interface;

namespace precitec{
    

    
namespace workflow{
    
///////////////////////////////////////////////////////////
// Prototyp fuer Thread Funktions
///////////////////////////////////////////////////////////

// Thread Funktion muss ausserhalb der Klasse sein

void* UDPsocketThread(void *p_pArg);

///////////////////////////////////////////////////////////
// global variables 
///////////////////////////////////////////////////////////


static bool s_threadsStopped = false;

///////////////////////////////////////////////////////////
// Constructor
///////////////////////////////////////////////////////////

UpsMonitorServer::UpsMonitorServer(TSystemStatus<AbstractInterface>& p_rSystemStatusProxy):
                    m_rSystemStatusProxy( p_rSystemStatusProxy )
{
    //printf("UpsMonitorServer: Constructor\n");
}
    
///////////////////////////////////////////////////////////
// Destructor
///////////////////////////////////////////////////////////

UpsMonitorServer::~UpsMonitorServer(void)
{
    //printf("UpsMonitorServer: Destructor\n");  
    Stop();
    
}  

///////////////////////////////////////////////////////////
// Class functions
///////////////////////////////////////////////////////////

void UpsMonitorServer::Start(void)
{
    ///////////////////////////////////////////////////////
    // start thread for ups monitoring
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToUDPsocketThread.m_pUpsMonitorServer = this;

    if (pthread_create(&m_oUDPsocketThread, &oPthreadAttr, &UDPsocketThread, &m_oDataToUDPsocketThread) != 0)
    {
        wmLog(eDebug, "UpsMonitorServer: Cannot start thread for process monitoring\n");
    }
}

void UpsMonitorServer::Stop(void)
{
    s_threadsStopped = true;
    pthread_cancel(m_oUDPsocketThread);
    pthread_join(m_oUDPsocketThread, nullptr);
    
    wmLog(eDebug, "UpsMonitorServer: UDPsocketThread stopped\n");
    //printf("UpsMonitorServer: UDPsocketThread stopped\n");
    
}


void UpsMonitorServer::Handler_UDPsocket(const char *upsEvent)
{
    //wmLog(eDebug, "UpsMonitorServer: UDPsocketThread Handler()\n");
    
    if ((strcmp("UPS_ONLINE", upsEvent) == 0) || (strcmp("ONLINE", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS power restored\n");
        m_rSystemStatusProxy.upsState(Online);
 
    }
    else if ((strcmp("UPS_ONBATT", upsEvent) == 0) || (strcmp("ONBATT", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS is on Battery!\n");
        m_rSystemStatusProxy.upsState(OnBattery);
    }
    else if ((strcmp("UPS_LOWBATT", upsEvent) == 0) || (strcmp("LOWBATT", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS low Battery!\n");
        m_rSystemStatusProxy.upsState(LowBattery);
    }
    else if ((strcmp("UPS_COMMOK", upsEvent) == 0 )|| (strcmp("COMMOK", upsEvent) == 0))
    {
        wmLog(eDebug, "UPS communication OK!\n");
        m_rSystemStatusProxy.upsState(Online);
    }
    else if ((strcmp("UPS_COMMBAD", upsEvent) == 0 )|| (strcmp("COMMBAD", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS communication BAD!\n");
        m_rSystemStatusProxy.upsState(NoCommunication);
    }
    else if ((strcmp("UPS_REPLBATT", upsEvent) == 0) || (strcmp("REPLBATT", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS need replace Battery!\n");
        m_rSystemStatusProxy.upsState(ReplaceBattery);
    }
    else if ((strcmp("UPS_NOCOMM", upsEvent) == 0) || (strcmp("NOCOMM", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS no communication!\n");
        m_rSystemStatusProxy.upsState(NoCommunication);
    }
    else if ((strcmp("UPS_NOPARENT", upsEvent) == 0) || (strcmp("NOPARENT", upsEvent) == 0))
    {
        wmLog(eDebug, "UpsMonitorServer: UPS no parents!\n");
    }
    else if ((strcmp("UPS_INIT", upsEvent) == 0) || (strcmp("INIT", upsEvent) == 0))
    {
        m_rSystemStatusProxy.upsState(Online);
    }
    else if ((strcmp("UPS_SHUTDOWN", upsEvent) == 0) || (strcmp("SHUTDOWN", upsEvent) == 0))
    {
        // shutdown request to ConnectServer in function UDPsocketThread()
        wmLog(eDebug, "UpsMonitorServer: UPS request shutdown!\n");
        m_rSystemStatusProxy.upsState(Shutdown);
    }
     else
    {
        wmLog(eDebug, "UpsMonitorServer: UDPsocketThread Handler() find no EVENT flag: %s\n", upsEvent);
    }
   
}


// Thread Funktion muss ausserhalb der Klasse sein
void *UDPsocketThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "UPS Monitor");
    struct DataToUDPsocketThread* pDataToUDPsocketThread;
    UpsMonitorServer* pUpsMonitorServer;

    pDataToUDPsocketThread = static_cast<struct DataToUDPsocketThread *>(p_pArg);
    pUpsMonitorServer = pDataToUDPsocketThread->m_pUpsMonitorServer;

    wmLog(eDebug, "UpsMonitorServer: UDPsocketThread is started\n");
//    printf("UpsMonitorServer: UDPsocketThread is started\n");

    sleep(30); // 30 seconds delay before monitoring processes
     

    // create UDP Socket
    struct sockaddr_in serverAddr;
        int upsSocket, recv_len;
    
    upsSocket = socket(AF_INET, SOCK_DGRAM, 0);
    if (upsSocket == -1)
    {
        wmLog(eDebug, "UpsMonitorServer: Error while creating UDP socket\n");
//        printf("UpsMonitorServer: Error while creating UDP socket\n");
        return NULL;
    }
    
    memset((char*)&serverAddr, 0 , sizeof(serverAddr));
    serverAddr.sin_family= AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    serverAddr.sin_port = htons(RECVPORT);

    if (bind(upsSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
    {
        wmLog(eDebug, "UpsMonitorServer: Error while binding UDP socket\n");
//        printf("UpsMonitorServer: Error while binding UDP socket\n");
        return NULL;
    }

    // init upsState
    pUpsMonitorServer->Handler_UDPsocket("INIT");
    
    wmLog(eDebug, "UpsMonitorServer: UDP socket is active\n");
//    printf("UpsMonitorServer: UDP socket is active\n");
   
    while(!s_threadsStopped)
    {
        char recvBuffer[BUFFER_LENGTH];
 
        if ( (recv_len = recvfrom(upsSocket, recvBuffer, sizeof(recvBuffer), 0, NULL, NULL) ) == -1 )
        {
            wmLog(eDebug, "UpsMonitorServer: Error while recieving UDP socket\n");
            //printf("UpsMonitorServer: Error while recieving UDP socket\n");
            continue;
        }
        std::string str_tmp(recvBuffer);
        std::string event =  str_tmp.substr(0, str_tmp.find("\n", 0));
        
        //printf("UpsMonitorServer: Event vor Handler() %s \n", event.c_str()), 

        pUpsMonitorServer->Handler_UDPsocket(event.c_str());
        
        // Shutdown request from ups
        if ( (event == "UPS_SHUTDOWN") || (event == "SHUTDOWN") )
        {
            wmLog(eDebug, "xxxxxxxxxxxxx Shutdown from UpsMonitorServer xxxxxxxxxxxx\n");
            
            ConnectionConfiguration simulationConfig(std::string("SIMULATION"));
            int simulationPid = simulationConfig.getInt(std::string("ConnectServerPid"), 0);
            if (simulationPid != 0)
            {
                ::kill(simulationPid, SIGTERM);
            }

            ConnectionConfiguration hardwareConfig(getenv("WM_STATION_NAME"));
            int hardwarePid = hardwareConfig.getInt(std::string("ConnectServerPid"), 0);
            if (hardwarePid != 0)
            {
                ::kill(hardwarePid, SIGUSR1);
            }
        }
    }
    
    close(upsSocket);

    return NULL;
}


}	// namespace workflow
}	// namespace precitec

