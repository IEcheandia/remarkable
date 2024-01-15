/*
 *  AppMain.h for App_CHRCommunication
 *
 *  Created on: 18.10.2017
 *      Author: a.egger
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

#include "message/module.h"
#include "module/baseModule.h"

#include "common/connectionConfiguration.h"

#include "event/sensor.proxy.h"

#include "event/triggerCmd.handler.h"
#include "CHRCommunication/TriggerCmdServer.h"

#include "event/inspection.handler.h"
#include "CHRCommunication/InspectionServer.h"

#include "message/device.handler.h"
#include "CHRCommunication/deviceServer.h"

#include "CHRCommunication/CHRCommunication.h"

namespace precitec
{

namespace grabber
{

class AppMain : public framework::module::BaseModule
{
public:
    AppMain();
    virtual ~AppMain();

    virtual void runClientCode();
    int init(int argc, char * argv[]);

private:
    CHRCommunication                   m_oCHRCommunication;

    TSensor<EventProxy>                m_oSensorProxy;

    TriggerCmdServer                   m_oTriggerCmdServer;
    TTriggerCmd<EventHandler>          m_oTriggerCmdHandler;

    InspectionServer                   m_oInspectionServer;
    TInspection<EventHandler>          m_oInspectionHandler;

    DeviceServer                       m_oDeviceServer;      ///< Device server
    TDevice<MsgHandler>                m_oDeviceHandler;     ///< Device handler
    
    TDevice<MsgHandler>                m_oDeviceCalibrationHandler;     ///< Device handler
    
};

} // namespace grabber

} // namespace precitec

#endif /* APPMAIN_H_ */

