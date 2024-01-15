/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Ralph Kirchner, Wolfgang Reichl (WoR), Andreas Beschorner (BA), Stefan Birmanns (SB), Simon Hilsenbeck (HS)
 *  @date       2009
 *  @brief      Main application, which controls the workflow.
 */

#include "AppMain.h"

#include <string.h>
#include <errno.h>

#include "message/module.h"
#include "fliplib/Fliplib.h"

#include "Poco/Version.h"
#include "Poco/UUID.h"
#include "Poco/Thread.h"

#include "workflow/stateMachine/stateContext.h"
#include "common/connectionConfiguration.h"
#include "analyzer/centralDeviceManager.h"
#include "system/tools.h"

namespace precitec
{
	using namespace interface;
	using namespace system;
	using namespace message;

namespace workflow
{

AppMain::AppMain() :
	BaseModule(system::module::WorkflowModul),
	dbProxy_(),
	triggerCmdProxy_(),
	resultsProxy_(),
	recorderProxy_(),
	systemStatusProxy_(),
	calibrationProxy_(),
	inspectionOutProxy_(),
	productTeachInProxy_(),
	weldHeadMsgProxy_(),
	videoRecorderProxy_(),
	centralDeviceManager_( ),
	inspectManager_(
		&dbProxy_,
		&weldHeadMsgProxy_,
		&triggerCmdProxy_,
		&resultsProxy_,
		&recorderProxy_,
		&systemStatusProxy_,
		&videoRecorderProxy_,
		&centralDeviceManager_,
        false),
    m_oGrabberStatusProxy(),
	stateContext_(
		new StateContext(
				Poco::UUID( ConnectionConfiguration::instance().getString("Station.UUID", Poco::UUID::null().toString() ) ),
				dbProxy_,
				inspectManager_,
				systemStatusProxy_,
				calibrationProxy_ ,
				inspectionOutProxy_,
				centralDeviceManager_,
				productTeachInProxy_,
				weldHeadMsgProxy_,
				videoRecorderProxy_,
				m_oGrabberStatusProxy)
	),
	inspectCmdServer_			( stateContext_ ),
	inspectCmdHandler_			( &inspectCmdServer_ ),
	inspectServer_				( stateContext_ ),
	inspectHandler_				( &inspectServer_ ),
	dbNotificationServer_		( stateContext_ ),
	dbNotificationHandler_ 		( &dbNotificationServer_ ),
	sensorServer_				( &inspectManager_ ),
	sensorHandler_				( &sensorServer_ ),
	sensorServerSim_			( &inspectManager_ ),
	sensorHandlerSim_			( &sensorServerSim_ ),
	calibDataMsgServer_         ( &inspectManager_ ),
	calibDataMsgHandler_        ( &calibDataMsgServer_ ),
	m_oDeviceServerQnx			( inspectManager_ ),
    m_oUpsMonitorServer         ( systemStatusProxy_ ),
	m_deviceNotificationProxy(std::make_shared<interface::TDeviceNotification<EventProxy>>()),
	m_querySystemStatusServer(),
	m_querySystemStatusHandler(&m_querySystemStatusServer),
	m_recorderPollHandler(inspectManager_.recorderPollServer().get())



{
    m_querySystemStatusServer.setStateContext(stateContext_);
	ConnectionConfiguration::instance().setInt( pidKeys[WORKFLOW_KEY_INDEX], getpid() ); // let ConnectServer know our pid
    centralDeviceManager_.initWorkflowDevice(&m_oDeviceServerQnx);
    centralDeviceManager_.setDeviceNotification(m_deviceNotificationProxy);
}

AppMain::~AppMain()
{
    //m_oUpsMonitorServer.StopUDPsocketThread();
}

void AppMain::runClientCode()
{
	// Es wird immer wieder versucht die Verbindung aufzubauen, bis der Prozess ordentlich beendet werden
	// soll. -> Ende StateMachine
	while (true)
	{
		try
		{
			wmLog( eDebug, "Trying to connect to wmHost...\n" );

#if !defined(NDEBUG)
			ConnectionConfiguration::instance().dump();
#endif

			wmLog( eDebug, "Host connection established...[OK]\n" );

            centralDeviceManager_.init();

			// Ab jetzt gehts rund
			wmLog( eDebug, "Startup workflow manager...\n" );
			stateContext_->reset();
			wmLog( eDebug, "Initalizing workflow manager...\n" );
			stateContext_->initialize();
			wmLog( eDebug, "Running workflow manager...\n" );

            m_oUpsMonitorServer.Start();
			notifyStartupFinished();

			//stateContext_->getCalibration().start(eInitCalibrationData);
			stateContext_->waitForTermination();
			stateContext_->exit();


			wmLog( eInfo, "Terminate workflow manager...\n");
			return; // full exit
		}
		catch(Poco::Exception& p_rException)
		{
			wmLog( eError, "Poco::Exception: %s:\n", p_rException.what() );
			wmLog( eError, "'%s'\n", p_rException.message().c_str() );
		}
		catch(std::exception& p_rException)
		{
			wmLog( eError, "Exception: %s\n", p_rException.what() );
		}
		catch (...)
		{
			wmLog( eError, "Unhandled exception during startup of App_Workflow!\n");
		}
	}
}

int AppMain::init(int argc, char * argv[])
{
	processCommandLineArguments(argc, argv);
	wmLog( eDebug, "Assigned station id: %s\n", stateContext_->getStationID().toString().c_str() );


	// Handler registrieren
	//registerPublication( &dbProxy_ );

	registerPublication( &inspectionOutProxy_ ); // Ausloeser BUG
	registerPublication( &triggerCmdProxy_ );
	registerPublication( &resultsProxy_ );
	registerPublication( &recorderProxy_ );
	registerPublication( &systemStatusProxy_ );
	registerPublication( &calibrationProxy_ );
	registerPublication( &weldHeadMsgProxy_ );
	registerPublication( &videoRecorderProxy_ );
	//registerPublication(&productTeachInProxy_);
	registerPublication( &m_oGrabberStatusProxy );
	registerPublication( m_deviceNotificationProxy.get() );
    registerPublication( &dbProxy_ );

    sensorHandler_.setRealTimePriority(system::Priority::Sensors);
    inspectCmdHandler_.setRealTimePriority(system::Priority::InspectionControl);
    inspectHandler_.setRealTimePriority(system::Priority::InspectionControl);

	// nun sagen wir was wir anbieten (alle handler) und was wir brauchen (hier keinen proxy)
	registerSubscription(&inspectCmdHandler_);
	registerSubscription(&inspectHandler_);
	registerSubscription(&sensorHandler_);
	registerSubscription(&calibDataMsgHandler_);
    registerSubscription(&dbNotificationHandler_);
    registerSubscription(&m_querySystemStatusHandler);
    registerSubscription(&m_recorderPollHandler);

    centralDeviceManager_.publishAllProxies(this);
    centralDeviceManager_.subscribeAllHandlers(this);

	// BaseModule initialsieren
	initialize(this);
    
	return 0;
}

void AppMain::uninitialize()
{
    stateContext_->beginTermination();
}

}
}
