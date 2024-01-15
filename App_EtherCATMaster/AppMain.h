/*
 *  AppMain.h for App_EtherCATMaster
 *
 *  Created on: 20.11.2016
 *      Author: a.egger
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
#include "module/baseModule.h"

#include "event/ethercatInputs.proxy.h"
#include "event/ethercatInputsToService.proxy.h"

#include "event/ethercatOutputs.handler.h"
#include "EtherCATMaster/EthercatOutputsServer.h"

#include "event/systemStatus.handler.h"
#include "EtherCATMaster/systemStatusServer.h"

#include "protocol/protocol.info.h"

#include "common/connectionConfiguration.h"

#include "EtherCATMaster/EtherCATMaster.h"

#include <termios.h>

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
	void stack_prefault(void);

    void ResetTerminalMode(void);
    void SetTerminalMode(void);
    int kbhit(void);
    int getch(void);
    void readKeyboard(void);

	TEthercatInputs<EventProxy>         m_oEthercatInputsProxy;
	TEthercatInputsToService<EventProxy> m_oEthercatInputsToServiceProxy;
	EtherCATMaster                      m_oEtherCATMaster;

	EthercatOutputsServer               m_oEthercatOutputsServer;
	TEthercatOutputs<EventHandler>      m_oEthercatOutputsHandler;

    SystemStatusServer                  m_systemStatusServer;
    TSystemStatus<EventHandler>         m_systemStatusHandler;

    struct termios                      m_oTermiosBackup;
};

} // namespace ethercat

} // namespace precitec

#endif /* APPMAIN_H_ */

