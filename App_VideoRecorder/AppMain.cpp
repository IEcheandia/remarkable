/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Simon Hilsenbeck (HS)
 *  @date		2011
 * 	@brief 		VideoRecorder application - controls the application process.
 *  @details
 */

// stl includes
#include <iostream>
#include <unistd.h>
// local includes
#include "AppMain.h"
// WM includes
#include "message/module.h"
#include "module/moduleLogger.h"
#include "message/device.h"
#include "common/connectionConfiguration.h"

namespace precitec {
namespace vdr {



AppMain::AppMain() :
BaseModule(system::module::VideoRecorderModul),
	m_oVideoRecorder				( m_oGrabberStatusProxy, m_oSchedulerEventsProxy),
	m_oVideoRecorderServer			( m_oVideoRecorder ),
	m_oVideoRecorderHandler			( &m_oVideoRecorderServer ),
	m_oDeviceServer					( m_oVideoRecorder ),
	m_oDeviceHandler				( &m_oDeviceServer )
{
	ConnectionConfiguration::instance().setInt( pidKeys[VIDEORECORDER_KEY_INDEX], getpid() ); // let ConnectServer know our pid
} // AppMain



AppMain::~AppMain() {
	uninitialize();
} // ~AppMain


void AppMain::runClientCode() {

    wmLog( eDebug, "App_VideoRecorder::runClientCode()\n");

    notifyStartupFinished();
} // runClientCode



int AppMain::init(int argc, char * argv[]) {
	// Proxys registrieren
	processCommandLineArguments(argc, argv);

	registerPublication( &m_oGrabberStatusProxy );
	registerPublication( &m_oSchedulerEventsProxy );

	// Register handler
	registerSubscription( &m_oVideoRecorderHandler ); 			// Receives the data from the analyzer
    registerSubscription(&m_oDeviceHandler, system::module::VideoRecorderModul);

	// Init BaseModule
	initialize(this);

	return 0;

} // init



/*virtual*/ void AppMain::uninitialize() {
	try {
		m_oVideoRecorder.uninitialize();
	}
	catch(const Poco::Exception& ex) {
		wmLog( eError, "Poco::Exception %s\n", ex.what() );
	}
	catch(const std::exception& exs) {
		wmLog( eError, "exception %s\n", exs.what() );
	}
	catch (...)  {
		std::cout << "unhandled exception!" << std::endl;
	}
} // uninitialize


} 	// namespace vdr
}	// namespace precitec
