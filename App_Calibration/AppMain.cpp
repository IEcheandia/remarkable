/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB)
 *  @date		2011
 * 	@brief 		Calibration application - controls the application process.
 *  @details	The application controls the calibration process, which corresponds to a special state in the workflow state machine.
 */

// stl includes
#include <iostream>
#include <unistd.h>

// local includes
#include "AppMain.h"
// WM includes
#include <message/module.h>
#include <message/device.h>
#include <common/connectionConfiguration.h>
#include <common/systemConfiguration.h>

namespace {
    using namespace precitec;

    calibration::OCTApplicationType getOCTApplicationType()
    {
        if (!SystemConfiguration::instance().getBool("IDM_Device1Enable", false))
        {
            return precitec::calibration::OCTApplicationType::NoOCT;
        }
        if (SystemConfiguration::instance().getBool("Newson_Scanner1Enable", false))
        {
            return precitec::calibration::OCTApplicationType::OCTTrackApplication;
        }
        else
        {
            return precitec::calibration::OCTApplicationType::OCTEnabled;
        }


    }
}

namespace precitec {
namespace calibration {

AppMain::AppMain() : BaseModule(system::module::CalibrationModul),
	m_oCalibrationManager( m_oTriggerCmdProxy, m_oRecorderProxy, m_oCameraDeviceProxy, m_oWeldHeadMsgProxy, m_oCalibDataMsgProxy
        , m_oIDMDeviceProxy, m_oWeldHeadDeviceProxy, 
        getOCTApplicationType()
    ),
	m_oCalibrationServer( &m_oCalibrationManager ),
	m_oCalibrationHandler( &m_oCalibrationServer ),
	m_oCalibrationSensorServer( m_oCalibrationManager ),
	m_oCalibrationSensorHandler( &m_oCalibrationSensorServer ),
	m_oDeviceServer( m_oCalibrationManager ),
	m_oDeviceHandler( &m_oDeviceServer ),
	m_oCalibrationCoordinatesRequestServer(&m_oCalibrationManager ),
	m_oCalibrationCoordinatesRequestHandler(&m_oCalibrationCoordinatesRequestServer),
	m_isSimulation(false)
{
#if defined __QNX__ || defined __linux__
		// to enable the logger to manipulate system status
		// ... define and publish systemStatusProxy_ as for manual use, and the uncomment next line
		//thisModulesLogger_.setSignaler(systemStatusProxy_);
#endif

    registerSubscription(&m_oDeviceHandler, system::module::CalibrationModul);

	ConnectionConfiguration::instance().setInt( pidKeys[CALIBRATION_KEY_INDEX], getpid() ); // let ConnectServer know our pid
} // AppMain



AppMain::~AppMain()
{
} // ~AppMain


void AppMain::runClientCode()
{

} // runClientCode



int AppMain::init(int argc, char * argv[])
{
	processCommandLineArguments(argc, argv);
    for (int i = 1; i < argc; i++)
    {
        if (strcmp(argv[i], "--sim") == 0)
        {
            m_isSimulation = true;
            continue;
        }        
    }

    if (!m_isSimulation)
    {
        system::raiseRtPrioLimit();
        m_oCalibrationSensorHandler.setRealTimePriority(system::Priority::Sensors);
    }
    m_oCalibrationSensorServer.initSharedMemory(m_isSimulation);
    
    m_oCalibrationManager.configureAsSimulationStation(m_isSimulation);
    
	// Register proxys
	registerPublication( &m_oCameraDeviceProxy, system::module::GrabberModul );
    if (! m_oCalibrationManager.isSimulation())
    {
        registerPublication( &m_oTriggerCmdProxy );
        registerPublication( &m_oWeldHeadMsgProxy );
        registerPublication( &m_oRecorderProxy );
        registerPublication( &m_oWeldHeadDeviceProxy, system::module::VIWeldHeadControl);
    }
	registerPublication( &m_oCalibDataMsgProxy );

	// Register handler
	registerSubscription( &m_oCalibrationHandler );			// Receives the commands to enter the calibration state and to configure it
    if (! m_oCalibrationManager.isSimulation())
    {
        registerSubscription( &m_oCalibrationSensorHandler ); 	// Receives the data from the camera and analog devices
    }
	registerSubscription( &m_oCalibrationCoordinatesRequestHandler );

    if (! m_oCalibrationManager.isSimulation() && m_oCalibrationManager.isOCTEnabled())
    {
        registerPublication(&m_oIDMDeviceProxy, system::module::CHRCommunicationModul);
    }

	// Init BaseModule
	initialize(this);

	// notify already here instead of runClientCode
	notifyStartupFinished();

	return 0;

} // init



} 	// namespace calibration
}	// namespace precitec
