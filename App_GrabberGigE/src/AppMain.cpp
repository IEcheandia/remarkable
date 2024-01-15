// project includes
#include "AppMain.h"
// wm includes
#include "message/module.h"
#include "message/device.h"
#include "module/moduleLogger.h"
#include "common/connectionConfiguration.h"
#include "common/systemConfiguration.h"


namespace precitec {
namespace grabber  {


AppMain::AppMain() 
    : BaseModule                    ( system::module::GrabberModul )
    , m_oCamera                     ( m_oSensorProxy )
    , m_oDeviceServer               ( m_oCamera )
    , m_oDeviceHandler				( &m_oDeviceServer )
    , m_oTriggerCmdServer           ( m_oCamera )
    , m_oTriggerCmdHandler          ( &m_oTriggerCmdServer )
    , m_oGrabberStatusServer        ( )
    , m_oGrabberStatusHandler       ( &m_oGrabberStatusServer )
{

}


AppMain::~AppMain() 
{
	uninitialize();
}



void AppMain::runClientCode()
{
	notifyStartupFinished();    

    if (m_oCamera.isActiveForConfiguration())
    {
        m_oCamera.initializeWork();
    }
}



int AppMain::init(int argc, char * argv[])
{    
	processCommandLineArguments(argc, argv);

    m_oGrabberStatusServer.setCamera(&m_oCamera);

    if (m_oCamera.isActiveForConfiguration())
    {
        ConnectionConfiguration::instance().setInt( pidKeys[GRABBER_KEY_INDEX], getpid());
        // register proxies
        registerPublication(&m_oSensorProxy);
        registerPublication(&m_oInspectionCmdProxy);

        m_oTriggerCmdHandler.setRealTimePriority(system::Priority::InspectionControl);

        // register handler
        registerSubscription(&m_oDeviceHandler, system::module::GrabberModul);
        registerSubscription(&m_oTriggerCmdHandler);
        registerSubscription(&m_oGrabberStatusHandler);
    }
    // BaseModule initialsieren
    initialize(this);
    return 0;
}


} 	// grabber
}	// precitec
