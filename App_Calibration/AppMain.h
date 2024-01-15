 /**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Stefan Birmanns (SB), Andreas Beschorner (BA)
 *  @date		2011
 * 	@brief 		Calibration application - controls the application process.
 *  @details	The application controls the calibration process, which corresponds to a special state in the workflow state machine.
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

// stl includes
#include <iostream>
// WM includes
#include <message/module.h> 						// globale moduleId
#include <module/baseModule.h>						// base class
#include <message/device.proxy.h>
#include <event/triggerCmd.proxy.h>					// trigger (sends trigger commands to the trigger module)
#include <event/results.proxy.h>					// results (sends results out, e.g. to the hardware modules)
#include <event/recorder.proxy.h>					// recorder proxy (sends images to windows)
#include <event/sensor.handler.h>					// sensor handler (gets called, when a new image was shot by the camera)
#include <calibration/calibrationSensorServer.h> 	// the code that gets called when the new image is ready
#include <message/calibration.handler.h> 			// out own handler for the calibration state
#include <calibration/calibrationServer.h> 			// server
#include <calibration/calibrationManager.h>			// manager
#include <calibration/deviceServer.h>				// device server - calibration configuration interface
#include <message/device.handler.h>					// device handler - calibration configuration interface
#include <message/weldHead.interface.h>				// new weldhead message interface (setHeadPos blocking).
#include <message/weldHead.proxy.h>					// new weldhead message proxy (setHeadPos blocking).
#include <message/calibDataMessenger.proxy.h>     // send calib data changes to analyzer
#include <message/calibrationCoordinatesRequest.handler.h>  // receives coordinate request from gui
#include <calibration/deviceServer.h>
#include <calibration/calibrationCoordinatesRequestServer.h>
#include "Poco/RunnableAdapter.h"

namespace precitec {
namespace calibration {

class AppMain : public framework::module::BaseModule
{
public:

	/// CTor.
	AppMain();
	/// DTor.
	virtual ~AppMain();
	/// Actual main function.
	virtual void runClientCode();

public:

	/// Init the module.
	int init(int argc, char * argv[]);

private:
	TDevice<MsgProxy>					m_oCameraDeviceProxy;					///< Device proxy - used to set and get parameters of the camera.
	TTriggerCmd<EventProxy>				m_oTriggerCmdProxy;				///< Trigger-cmd proxy - used to send single trigger impulses to the grabber.
	TRecorder<EventProxy>				m_oRecorderProxy;				///< Recorder proxy - used to send images to the wmMain.
	TCalibDataMsg<MsgProxy>				m_oCalibDataMsgProxy;			///< CalibDataMsg proxy - signal calibration data changes to workflow/analzyer
	TDevice<MsgProxy>					m_oIDMDeviceProxy;
	TDevice<MsgProxy>					m_oWeldHeadDeviceProxy;         ///< Device proxy for the weldhead device interface

	CalibrationManager					m_oCalibrationManager;			///< Calibration manager, which controls the actual calibration process.
	CalibrationServer					m_oCalibrationServer;			///< Calibration server, server object of the calibration interface.
	TCalibration<MsgHandler>			m_oCalibrationHandler;

	CalibrationSensorServer				m_oCalibrationSensorServer;		///< Sensor server, which receives the data from the sensors.
	TSensor<EventHandler>				m_oCalibrationSensorHandler;	///< Sensor handler, which calls the sensor server.

	DeviceServer						m_oDeviceServer;				///< Device server, to configure the calibration from wmMain
	TDevice<MsgHandler>					m_oDeviceHandler;				///< Device handler

	TWeldHeadMsg<MsgProxy> 				m_oWeldHeadMsgProxy;			///< Msg proxy for the weldhead interface
	
	CalibrationCoordinatesRequestServer  m_oCalibrationCoordinatesRequestServer;
	TCalibrationCoordinatesRequest<MsgHandler> m_oCalibrationCoordinatesRequestHandler;
    
    bool m_isSimulation;
};

} // namespache calibration
} // namespace precitec

#endif /* APPMAIN_H_ */
