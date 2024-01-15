/**
 *	@file
 *  @copyright	Precitec Vision GmbH & Co. KG
 *  @author		Simon Hilsenbeck (HS)
 *  @date		2011
 * 	@brief 		VideoRecorder application - controls the application process.
 *  @details
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_

// WM includes
#include "message/module.h" 							///< globale moduleId
#include "module/baseModule.h"							///< base class
#include "message/grabberStatus.proxy.h"				///< grabber status info, like current image number or nb of buffers
#include "event/schedulerEvents.proxy.h"				///< interface to send events to a scheduler
#include "event/videoRecorder.handler.h"				///< video recorder handler (gets called, when new video data was sent by the analyzer)
#include "message/device.handler.h"						///< device handler (gets called by wmHost parameter exchange)
// modul includes
#include "videoRecorder/videoRecorderServer.h" 			///< the code that gets called when the new video data is ready
#include "videoRecorder/deviceServer.h" 				///< Implements the TDevice interface for paremtrization.
#include "videoRecorder/videoRecorder.h"				///< recording manager



namespace precitec {
namespace vdr {

class AppMain : public framework::module::BaseModule
{
public:

	/// CTor.
	AppMain();
	/// DTor.
	virtual ~AppMain();
	/// Actual main function.
	virtual void runClientCode();
	/// Init the module.
	int init(int argc, char * argv[]);
	/// Uninit the module.
	/*virtual*/ void uninitialize();

private:

	VideoRecorder								m_oVideoRecorder;				///< VideoRecorder, which controls the actual videoRecorder process.
	TGrabberStatus<MsgProxy>		 			m_oGrabberStatusProxy;      	///< GrabberStatus proxy, which requests status info from the frame grabber.
	TSchedulerEvents<EventProxy>		 		m_oSchedulerEventsProxy;      	///< interface to send events to a scheduler.
	VideoRecorderServer							m_oVideoRecorderServer;			///< VideoRecorder server, which receives the data from the analyzer.
	interface::TVideoRecorder<EventHandler>		m_oVideoRecorderHandler;		///< VideoRecorder handler, which calls the VideoRecorder server.
	DeviceServer								m_oDeviceServer;				///< Device server, which receives the parameter from the wmHost.
	interface::TDevice<MsgHandler>				m_oDeviceHandler;				///< Device handler, which calls the Device server.
};

} // namespache vdr
} // namespace precitec

#endif /* APPMAIN_H_ */
