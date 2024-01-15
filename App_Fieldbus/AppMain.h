/*
 *  AppMain.h for App_Fieldbus
 *
 *  Created on: 25.2.2019
 *      Author: a.egger
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
#include "module/baseModule.h"

#include "event/ethercatInputs.proxy.h"
#include "event/ethercatInputsToService.proxy.h"

#include "event/ethercatOutputs.handler.h"
#include "Fieldbus/FieldbusOutputsServer.h"

#include "event/systemStatus.handler.h"
#include "Fieldbus/systemStatusServer.h"

#include "common/connectionConfiguration.h"

#include "Fieldbus/Fieldbus.h"

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

    Fieldbus                            m_oFieldbus;

    TEthercatInputs<EventProxy>         m_oFieldbusInputsProxy;
    TEthercatInputsToService<EventProxy> m_oFieldbusInputsToServiceProxy;

    FieldbusOutputsServer               m_oFieldbusOutputsServer;
    TEthercatOutputs<EventHandler>      m_oFieldbusOutputsHandler;

    SystemStatusServer                  m_systemStatusServer;
    TSystemStatus<EventHandler>         m_systemStatusHandler;
};

} // namespace ethercat

} // namespace precitec

#endif /* APPMAIN_H_ */

