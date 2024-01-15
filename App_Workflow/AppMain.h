/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Ralph Kirchner, Wolfgang Reichl (WoR), Andreas Beschorner (BA), Stefan Birmanns (SB), Simon Hilsenbeck (HS)
 *  @date       2009
 *  @brief      Main application, which controls the workflow.
 */

#ifndef APPMAIN_H_
#define APPMAIN_H_


// fuer die globale moduleId
#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"

#include "event/inspection.handler.h"
#include "event/inspectionCmd.handler.h"
#include "event/sensor.handler.h"
#include "event/querySystemStatus.handler.h"
#include "event/recorderPoll.handler.h"

#include "event/deviceNotification.proxy.h"
#include "event/results.proxy.h"
#include "event/recorder.proxy.h"
#include "event/systemStatus.proxy.h"
#include "event/productTeachIn.proxy.h"
#include "event/inspectionOut.proxy.h"
#include "event/videoRecorder.proxy.h"

#include "event/dbNotification.handler.h"
#include "message/calibration.proxy.h"
#include "message/weldHead.interface.h"
#include "message/weldHead.proxy.h"
#include "message/calibDataMessenger.handler.h"
#include "message/device.handler.h"						///< device handler (gets called by wmHost parameter exchange)
#include "message/grabberStatus.proxy.h"				///< grabber notification of coming automatic mode

#include "workflow/inspectionServer.h"
#include "workflow/inspectionCmdServer.h"
#include "workflow/dbNotificationServer.h"
#include "workflow/deviceServer.h" 				///< Implements the TDevice interface for paremtrization.
#include "workflow/upsMonitorServer.h"
#include "workflow/querySystemStatusServer.h"

#include "workflow/stateMachine/stateContext.h"

#include "analyzer/inspectManager.h"
#include "analyzer/sensorServer.h"
#include "analyzer/centralDeviceManager.h"
#include "analyzer/calibDataMessengerServer.h"
#include "calibration/calibrationManager.h"
#include "math/calibrationData.h"

namespace precitec
{
	using framework::module::BaseModule;
	using interface::TInspection;
	using interface::TInspectionCmd;
	using interface::TInspectionOut;
	using interface::TProductTeachIn;
	using interface::TVideoRecorder;

	using analyzer::InspectManager;
	using analyzer::SensorServer;

namespace workflow
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

        void uninitialize() override;

	private:

		// Initalisierungsreihenfolge beachten! Erst Proxy dann Server dann Handler
		// WoR:
		// 		richtig ist: erst Server, dann Handler (der braucht Server fuer CTor)
		// 		ABER!!!  proxy ist frei: wobei mir "Proxy nach Server" besser erscheint
		// 		(Proxy hat Abhaengigkeiten, Server erfuellt Abhaengigkeiten)

		/// Hiermit ruft der Workflow aktiv Win-DB-Info ab
		TDb<MsgProxy>					dbProxy_;
		/// Konfiguriert das QNX-Triggermodul (SingleShot-Mode, Burst-Mode, ...)
		TTriggerCmd<EventProxy>			triggerCmdProxy_;
		/// Schickt Resultate an jedwede Empfaenger
		TResults<EventProxy>			resultsProxy_;
		/// Schickt Bild-/Sensor-Daten an Win-"Host"
		TRecorder<EventProxy>			recorderProxy_;
		/// Informiert jedweden Empfaenger ueber Workflow-Status, HW-Status, allg. FehlerZustaende
		TSystemStatus<EventProxy>		systemStatusProxy_;
		// Proxy fuer den Calibration State
		TCalibration<MsgProxy>			calibrationProxy_;

		//Proxy fuer die Verbindung zum VI_Inspection (spricht die HW an)
		TInspectionOut<EventProxy>		inspectionOutProxy_;

		//Proxy um die Daten des ProduktTeachIn zur GUI zu senden
		TProductTeachIn<EventProxy>		productTeachInProxy_;

		//MsgProxy fuer weldHeadMsg
		TWeldHeadMsg<MsgProxy>			weldHeadMsgProxy_;

		//Event Proxy fuer Videorecorder
		TVideoRecorder<EventProxy>		videoRecorderProxy_;

		// Ab hier Analyzerzeugs

		analyzer::CentralDeviceManager	centralDeviceManager_;
		InspectManager						inspectManager_;
		TGrabberStatus<MsgProxy>		 	m_oGrabberStatusProxy;      	///< GrabberStatus proxy, which notifies the grabber of coming activity.
		SmStateContext						stateContext_;

		// Inspection Command Server: Kommando-Schnittstelle zum Win-"Host"
		InspectionCmdServer 				inspectCmdServer_;
		TInspectionCmd<EventHandler>		inspectCmdHandler_;

		// Inspection Server: Schnittstelle fuer Hardware-Signale (start, stop, LinienLaser, ...)
		InspectionServer					inspectServer_;
		TInspection<EventHandler>			inspectHandler_;

		// Schnittstelle fuer Win-Aenderungen von Datenbank-Inhalten, Werte werden uebernommen, nicht gesetzt
		DbNotificationServer				dbNotificationServer_;
		TDbNotification<EventHandler>		dbNotificationHandler_;

		// Sensor wird vom MM bedient
		SensorServer						sensorServer_;
		TSensor<EventHandler>				sensorHandler_;

		// Sensor wird vom Windows System bedient (Simulation)
		// TODO: Muss wieder entfernt werden, wenn der MM auch mit Win umgehen kann (ist z.Zt. nicht geplant)
		SensorServer						sensorServerSim_;
		TSensor<EventHandler>				sensorHandlerSim_;

		// CalibDataMsg : Nimmt Befehle von der AppCalibration entgegen
		analyzer::CalibDataMessengerServer  calibDataMsgServer_;
		TCalibDataMsg<MsgHandler>           calibDataMsgHandler_;

		DeviceServer						m_oDeviceServerQnx;				///< Device server, which receives the parameter from the secured device server.

        UpsMonitorServer                    m_oUpsMonitorServer;

        std::shared_ptr<interface::TDeviceNotification<EventProxy>> m_deviceNotificationProxy;

        QuerySystemStatusServer m_querySystemStatusServer;
        interface::TQuerySystemStatus<EventHandler> m_querySystemStatusHandler;

        interface::TRecorderPoll<EventHandler> m_recorderPollHandler;
	};

}	// namespace workflow
} // namespace precitec

#endif /*APPMAIN_H_*/
