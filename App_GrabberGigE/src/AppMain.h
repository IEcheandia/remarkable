#pragma once

// stl includes
#include  <iostream>
// wm includes
#include "message/module.h"
#include "module/baseModule.h"
#include "message/device.handler.h"
#include "message/grabberStatus.handler.h"
#include "event/triggerCmd.handler.h"
#include "event/sensor.proxy.h"
#include "event/inspectionCmd.proxy.h"
// project includes
#include "camera.h"
#include "triggerCommandServer.h"
#include "grabberStatusServer.h"
#include "deviceServer.h"

namespace precitec {
namespace grabber  {


class AppMain : public framework::module::BaseModule
{

public:
    
    AppMain();
    virtual ~AppMain();
    virtual void runClientCode();

public:
    
    int init(int argc, char * argv[]);
    
private:
    
    Camera                                      m_oCamera;

    TSensor<EventProxy>                         m_oSensorProxy;
    TInspectionCmd<EventProxy>                  m_oInspectionCmdProxy;

    DeviceServer                                m_oDeviceServer;
    interface::TDevice<MsgHandler>				m_oDeviceHandler;

    TriggerCommandServer	 	                m_oTriggerCmdServer;
    TTriggerCmd<EventHandler>	                m_oTriggerCmdHandler;
    
    GrabberStatusServer			                m_oGrabberStatusServer;
    TGrabberStatus<MsgHandler>	                m_oGrabberStatusHandler;
    
};


} // namespache grabber
} // namespace precitec
