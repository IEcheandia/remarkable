/**
* @file
* @brief  Schnittstelle auf Framegrabber und Kamera
*
* Header zum Main Programm des Grabber Projekts App_Grabber
*
* @author JS
* @date   20.05.10
* @version 0.1
* Erster Wurf
*/


#ifndef APPMAIN_H_
#define APPMAIN_H_

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

#include "grabber/deviceServer.h"
#include "grabber/triggerServer.h"
#include "grabber/grabberStatusServer.h"

#include "grabber/dataAcquire.h"


namespace precitec
{
    using framework::module::BaseModule;
    using interface::TInspectionCmd;
    using interface::TTriggerCmd;
    using interface::TTrigger;
    using namespace trigger;


namespace grabber
{

    // Diese Klasse wird als Applikationsobjekt gestartet
    class AppMain : public BaseModule
    {
    public:
        AppMain();
        virtual ~AppMain();
        virtual void runClientCode();

    public:
        // ersetzt die main Methode
        int init(int argc, char * argv[]);
        virtual void uninitialize() {
            //if (grabber_) delete  grabber_;
            m_oDeviceServer.uninitialize();
            m_oTriggerServer.uninit();
        }

    private:

        TSensor<EventProxy>		 	m_oSensorProxy;       		///< Sensor Interface des workflow/analyzer
        std::unique_ptr<DataAcquire> m_pGrabber;		   		///< Grabber/Kamera
        TriggerServer				m_oTriggerServer;     		///< TriggerServer setzt die TriggerEvents um
        TTrigger<EventHandler>      m_oTriggerHandler; 		///< handler fuer das receive des trigger servers

        //Trigger timer part
        CommandServer			 	m_oTriggerCmdServer;		///<TriggerCmdServer empfaengt Steuerbefehle fuer den Triggerserver
        TTriggerCmd<EventHandler>	m_oTriggerCmdHandler; 		///< handler fuer das receive des trigger servers


        DeviceServer				m_oDeviceServer;      		///< server zur Parametrierung - key Value
        TDevice<MsgHandler>			m_oDeviceHandler;     		///< callback handler, ruft device server auf

/*
        //Trigger timer part
        CommandServer			 	m_oTriggerCmdServer;		///<TriggerCmdServer empfaengt Steuerbefehle fuer den Triggerserver
        TTriggerCmd<EventHandler>	m_oTriggerCmdHandler; 		///< handler fuer das receive des trigger servers

*/

        GrabberStatusServer			m_oGrabberStatusServer;		///< server gets called by handler
        TGrabberStatus<MsgHandler>	m_oGrabberStatusHandler;  	///< receives messages from the video recorder module

        bool m_oHasCamera;

        std::shared_ptr<TInspectionCmd<EventProxy>> m_inspectionCmd;

    };

}	// namespache grabber
} // namespace precitec

#endif /*APPMAIN_H_*/
