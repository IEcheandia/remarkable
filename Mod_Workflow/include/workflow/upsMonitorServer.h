/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Claudius Batzlen (CB)
 * 	@date		2018
 * 	@brief		Implements the UPS in Framework for Shutdown and User infos.
 */

#ifndef UPSMONITORSERVER_H_
#define UPSMONITORSERVER_H_

#include <pthread.h>

// project includes
#include "event/systemStatus.proxy.h"


namespace precitec {
    
using namespace interface;

namespace workflow {

class UpsMonitorServer; // forward declaration    

struct DataToUDPsocketThread
{
	UpsMonitorServer* m_pUpsMonitorServer;
};
    
    
class UpsMonitorServer 
{

public:

    UpsMonitorServer(TSystemStatus<AbstractInterface>& p_rSystemStatusProxy);
         
	virtual ~UpsMonitorServer(void);
    
    void Start(void);
    void Stop(void);
	void Handler_UDPsocket(const char *string);

private:

    TSystemStatus<AbstractInterface>&		m_rSystemStatusProxy;

    pthread_t m_oUDPsocketThread;
    struct DataToUDPsocketThread m_oDataToUDPsocketThread;

};    
    
    
}	// workflow
}	// precitec

#endif /* UPSMONITORSERVER_H_ */
