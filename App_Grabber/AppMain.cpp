/**
 * @file
 * @brief  Schnittstelle auf Framegrabber und Kamera
 *
 * Main Programm des Grabber Projekts App_Grabber
 *
 * Stellt die Datenakquisition (grabber), den Device Server (Parametrierschnittstelle)
 * und den Trigger Server ( Triggerung der Bildakquisition) zur Verfuegung
 *
 * @author JS
 * @date   20.05.11
 * @version 0.1
 *
 */


//#define TESTSTANDALONE 1

#include <iostream>
#include <unistd.h>
#include "AppMain.h"
#include "event/inspection.proxy.h"
#include "message/module.h"
#include "message/device.h"
#include "module/moduleLogger.h"
#include "common/triggerContext.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"
#include "system/realTimeSupport.h"

namespace precitec{
namespace grabber{


AppMain::AppMain() :
	BaseModule				( system::module::GrabberModul ),		// wir brauchen immer die moduleId!!
	m_pGrabber				( std::unique_ptr<DataAcquire>(new DataAcquire(m_oSensorProxy)) ),
	m_oTriggerServer		( m_oSensorProxy , *m_pGrabber ),
	m_oTriggerHandler       ( &m_oTriggerServer ),
	m_oTriggerCmdServer		( m_oTriggerServer ),
	m_oTriggerCmdHandler	( &m_oTriggerCmdServer ),
	m_oDeviceServer			( *m_pGrabber,m_oTriggerCmdServer ), // device server kann den command server des triggers bekommen ...test
	m_oDeviceHandler		( &m_oDeviceServer ),
	m_oGrabberStatusServer	( *m_pGrabber, m_oTriggerServer ),
	m_oGrabberStatusHandler	( &m_oGrabberStatusServer ),
	m_inspectionCmd(std::make_shared<TInspectionCmd<EventProxy>>())
{
    m_pGrabber->setInspectionCmd(m_inspectionCmd);

    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_oHasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_oHasCamera = false;
        }
    }
    else
    {
        m_oHasCamera = false;
    }

    system::raiseRtPrioLimit();
}

AppMain::~AppMain() 
{
    uninitialize();
}

void AppMain::runClientCode()
{
    if (SystemConfiguration::instance().getInt("CameraInterfaceType", 0) == 0 && m_oHasCamera)
    {
        wmLog(eDebug, "Grabber::runClientCode()\n");
        std::cout<<" run client code..."<<std::endl;
        Configuration config;
        m_oDeviceServer.initialize(config,0);
    }
    notifyStartupFinished();
}


int AppMain::init(int argc, char * argv[])
{
    processCommandLineArguments(argc, argv);

    for (int i = 0; i < argc; ++i)
    {
        if( strstr(argv[i],"debug") != NULL )
        {
            m_pGrabber->setDbgOut(1);
            break;
        }
    }

    if ( SystemConfiguration::instance().getInt("CameraInterfaceType", 0) == 0 && m_oHasCamera) 
    {
        ConnectionConfiguration::instance().setInt( pidKeys[GRABBER_KEY_INDEX], getpid());
        registerPublication(&m_oSensorProxy);
        registerPublication(m_inspectionCmd.get());

        m_oTriggerCmdHandler.setRealTimePriority(system::Priority::InspectionControl);
        m_oTriggerHandler.setRealTimePriority(system::Priority::Sensors);

        //Registriere Device Handler
        registerSubscription(&m_oDeviceHandler, system::module::GrabberModul);

        // Registriere handler vom trigger Interface
		registerSubscription(&m_oTriggerHandler);

		// Registriere handler vom triggerCmd Interface
        registerSubscription(&m_oTriggerCmdHandler);

        // Registriere handler vom GrabberStatus Interface
        registerSubscription(&m_oGrabberStatusHandler);
    }
    // BaseModule initialsieren
	initialize(this);
	return 0;
}

} 	// grabber
}	// precitec
