/*
 *  AppMain.h for App_Trigger
 *
 *  Created on: 26.2.2019
 *      Author: a.egger
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
#include "module/baseModule.h"

#include "event/triggerCmd.handler.h"
#include "Trigger/TriggerCmdServer.h"

#include "event/trigger.proxy.h"

#include "common/connectionConfiguration.h"

#include "Trigger/Trigger.h"

namespace precitec
{

namespace trigger
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

    TriggerCmdServer                     m_oTriggerCmdServer;
    TTriggerCmd<EventHandler>            m_oTriggerCmdHandler;

    TTrigger<EventProxy>                 m_oTriggerInterfaceProxy;

    Trigger                              m_oTrigger;
};

} // namespace trigger

} // namespace precitec

#endif /* APPMAIN_H_ */

