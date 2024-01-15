
#pragma once
#include  <iostream>

// fuer die globale moduleId
#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"

//trigger part
#include "event/triggerCmd.handler.h"

#include "event/inspectionCmd.proxy.h"

#include "message/device.server.h"
#include "message/device.proxy.h"
#include "message/device.h"

#include "trigger/commandServer.h"


//grabber part
#include "event/sensor.proxy.h"
#include "message/device.handler.h"
#include "event/trigger.handler.h"
#include "message/grabberStatus.handler.h"

#include "grabber/deviceServerNoHw.h"
#include "trigger/triggerServerNoHw.h"
#include "trigger/commandServerNoHw.h"
#include "grabber/grabberStatusServerNoHw.h"

//#include "grabber/dataAcquire.h"


namespace precitec{
namespace grabber{
    
using framework::module::BaseModule;
using interface::TInspectionCmd;
using interface::TTriggerCmd;
using interface::TTrigger;
using namespace trigger;

class AppMain : public BaseModule
{
public:
    
    AppMain();
    
    virtual ~AppMain();
    
    virtual void runClientCode();

    int init(int argc, char * argv[]);
    
    virtual void uninitialize(); 
    
private:

    TSensor<EventProxy> m_sensorProxy;
    std::unique_ptr<DataAcquire> m_grabber;
    TriggerServerNoHw m_triggerServer;
    TTrigger<EventHandler> m_triggerHandler;
    CommandServerNoHw m_triggerCmdServer;
    TTriggerCmd<EventHandler> m_triggerCmdHandler;
    DeviceServerNoHw m_deviceServer;
    TDevice<MsgHandler> m_deviceHandler;
    GrabberStatusServerNoHw m_grabberStatusServer;
    TGrabberStatus<MsgHandler> m_grabberStatusHandler;
    bool m_hasCamera;

};

}
}

