/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			kir, wor, team
 *  @date			2012
 *  @brief			interface names, module names
 */


#ifndef INTERFACES_H
#define INTERFACES_H

#include "SystemManifest.h"
#include <map> // wg std::pair
#include "system/types.h"


// Im Falle von UnitTests nachfolgenden Kommentar entfernen.
// #define _UNITTEST

namespace precitec
{
namespace system
{
namespace module
{
	/**
	 *  Alle!! Schnittstellen
	 * Viele Module koennen die gleiche Schnittstelle bedienen
	 * Eine Schnittstelle kann eine Message-Schnittstelle (synchron)
	 * oder eine Event-Schnittstelle (asynchron) sein.
	 *
	 * Die Enum Interfaces enthaelt alle Schnittstellen, die inerhalb
	 * der Gesamtapplikation bedient werden koennen.
	 */
	enum SYSTEM_API Interfaces
	{
		AllInterfaces = -2,
		Client = -1, 					///< Client *bedient*=implementiert kein Interface,

		FirstMessage = 0,				///< ist natuerlich nur ein Marker
		// hier folgen alle Message-Schnitstellen
		Registrar = FirstMessage,		///< fuer jedes Modul extra, verwaltet veroeffentlichte I-Faces
		Receptor,						///< reines Anmelde-Interface mit fester Udp-Adresse, gemeinsame fuer alle Module
		Module, 						///< jedes Modul implementiert dieses Interface als Server
		Device,							///< All configurable Devices

		Analyzer,
		// hier werden weitere Messages eigefuegt
		Db,								///< TDb<Messages>
		Calibration,					///< Calibration Message Interface
		WeldHeadMsg,					///< WeldHeadMsg interface

		CalibDataMsg,                   ///< AppCalib -> Workflow/Analyzer: signal calibration data changes

		// weitere Message-Schnittstellen vor dieser Zeile eigefuegen
#ifdef _UNITTEST
		T3ControlDevice,				///< T3-test-Suite
		T3ControlWorker,				///< T3-test-Suite
		T3DataBase,						///< T3-test-Suite
		T3Access,						///< T3-test-Suite
		TestServer,						///< Inerface-Test/Demo
#endif
		VideoRecorderCmd,				///< video recorder parametrisation (wmHost -> VideoRecorder)
		GrabberStatus,					///< video recorder parametrisation (VideoRecorder -> Grabber)
		VideoRecorderTransfer,			///< video recorder transfer (VideoRecorder -> wmHost)
        SimulationCmd, ///< Simulation command interface (wmHost -> App_Simulation)
		CalibrationCoordinatesRequest, /// request calibration coordinates (App_Gui -> App_Calibration)
		NumMessages,					///< natuerlich nur Marker

		FirstEvent = 0x400,				///< 0x400 = willkuerliche Zahl

		// hier folgen alle Event-Schnitstellen
		ResultEvent = FirstEvent,
		Recorder,						///< TRecorder<Messages>
		Results,						///< Result-Filter an viele
		Sensor,							///< Sensor -> Input-Filter
		SystemStatus,					///< InspectManager -> SPS, etc.

		Trigger,						///< InspectManager -> Sensor
		Inspection, 					///< TInspection Status + Verwaltung
		InspectionCmd,
		InspectionOut,          		///< Diverse an InspectionControl

		EthercatInputs,					///< Daten der Eingangsmodule auf dem EtherCAT
		EthercatOutputs,				///< Daten fuer die Ausgangsmodule auf dem EtherCAT

		VideoRecorder,					///< video recorder data (Analyzer -> VideoRecorder)
		WeldHeadPublish,
		WeldHeadSubscribe,
		VIServiceToGUI,
		VIServiceFromGUI,
		TriggerCmd,						///< TriggerCmd ist jetzt auch ein Event!

		ProductTeachIn,
		DeviceNotification,
        StorageUpdate,
        DbNotification,
        QuerySystemStatus,
        RecorderPoll,
        EthercatInputsToService,
        S6K_InfoToProcesses,
        S6K_InfoFromProcesses,
        InspectionToS6k,
        ControlSimulation,
        SchedulerEvents,
		// hier werden weitere Events eigefuegt

#ifdef _UNITTEST
		TestSubscriber,					///< Demo-Subscriber
		T3InformControl,				///< T3-test-Suite
		T3SupplyWorker,					///< T3-test-Suite
		EventTest,						///< EventTestServer
#endif
		LastEvent,						///< nur Marker wg NumEvents
		NumEvents = LastEvent - FirstEvent
	};

	inline bool isMessageInterface(Interfaces i) { return (i>=FirstMessage) && (i<NumMessages); }
	inline bool isEventInterface(Interfaces i) 	{ return (i>=FirstEvent) && (i<LastEvent); }

	const int AllEvents  = -2; // = alle Events eines E-Interfaces


	/**
	 * Jeder Server hat hier ein Eintrag
	 *
	 * Beispiel: der Analyzer (1*Code) hat hier zwei Eintraege
	 * den ersten fuer die Echtzeit-(QNX)-Analyse
	 * den Zweiten fuer die Offline-(Win)-Analyse
	 * Obwohl beide Apps den gleichen Code umfassen, bekommen sie unterschiedliche Nachrichten
	 *
	 * Der ModuleManager hat gleich drei Eintraege 2unter QNX einen unter Win
	 * Der erste QNX-MM wird von Root gestartet, und kann infolge dessen Module
	 *   staten, die ebenfalls Root-Rechte brauchen.
	 * Der zweite QNX-MM wird als normale App mit User-Rechten gestartet und kann Apps nur
	 *   mit User-Rechten starten.
	 *
	 * Ggf gilt fuer den Logger (QNX-RealTime-Logger vs. WIN-Logger)
	 *
	 * Fuer das Primitive Datenbank Interface wird es sicher mehrere Implementierungen geben
	 */
	enum SYSTEM_API Modules {
		AnyModule = -1,
		InvalidModule = 0,
		// ab jetzt kommen 'echte' nicht-test-Module
		ModuleManager,
		WorkflowModul,
		TriggerModul,
		GrabberModul,
		VIWeldHeadControl,
		VIInspectionControl,
		VIServiceModul,
		CalibrationModul,
		VideoRecorderModul,
		LoggerServer,
		LoggerConsole,				///< App_LoggerQnx, a simple GUI replacement to test App_LoggerServer.
		EtherCATMasterModul,
        SimulationModul,
        StorageModul,
        UserInterfaceModul,
        CHRCommunicationModul,
        MockAxisModul,
        FieldbusModul,
        TCPCommunicationModul,
        SchedulerModul,

// 	test modules, bei Bedarf einbinden
#ifdef _UNITTEST
		TestServerModule,
		TestServerModule1,
		TestServerModule2,
		TestClientModule,
		TestClientModule1,
		TestClientModule2,
		TestSubscriberModule,
		T3Control,
		T3Device,
		T3Worker,
#endif

		NumApplications

	};

	/**
	 * Hier steht fuer jedes Modul, welche interfaces es implementiert
	 * Module taucht hier nicht auf, da es implizit von jedem Modul implementiert wird
	 * Eine App, die von mehreren Servern ableitet, kann so mehrere interfaces implementieren
	 * Eine solche App taucht hier mehrfach auf.
	 * Nicht jedes Interface muss tatsaechlich implementiert sein :-)
	 */

	typedef std::pair<int, PvString> IdAndString;
	typedef std::map<int, PvString>  IdToString;
	#define ID_PAIR(Id) IdAndString(Id, PvString(#Id))
	class InterfaceNames {
	public:
		typedef std::map<int, PvString> IdNameMap;
		InterfaceNames() {
			map_.insert(ID_PAIR(Registrar));
			map_.insert(ID_PAIR(Receptor));
			map_.insert(ID_PAIR(Module));
			map_.insert(ID_PAIR(Device));

			map_.insert(ID_PAIR(Analyzer));
			map_.insert(ID_PAIR(Db));
			map_.insert(ID_PAIR(DbNotification));

			map_.insert(ID_PAIR(Calibration));
			map_.insert(ID_PAIR(WeldHeadMsg));

			map_.insert(ID_PAIR(CalibDataMsg));
			map_.insert(ID_PAIR(CalibrationCoordinatesRequest));

			// events
			map_.insert(ID_PAIR(ResultEvent));
			map_.insert(ID_PAIR(Recorder));
			map_.insert(ID_PAIR(Results));
			map_.insert(ID_PAIR(Sensor));
			map_.insert(ID_PAIR(SystemStatus));

			map_.insert(ID_PAIR(Trigger));
			map_.insert(ID_PAIR(Inspection));
			map_.insert(ID_PAIR(InspectionCmd));
			map_.insert(ID_PAIR(InspectionOut));
			map_.insert(ID_PAIR(EthercatInputs));
			map_.insert(ID_PAIR(EthercatOutputs));
			map_.insert(ID_PAIR(VideoRecorderCmd));
			map_.insert(ID_PAIR(GrabberStatus));

			map_.insert(ID_PAIR(VideoRecorder));
			map_.insert(ID_PAIR(WeldHeadPublish));
			map_.insert(ID_PAIR(WeldHeadSubscribe));
			map_.insert(ID_PAIR(VIServiceToGUI));
			map_.insert(ID_PAIR(TriggerCmd));

			map_.insert(ID_PAIR(ProductTeachIn));
            map_.insert(ID_PAIR(SimulationCmd));
            map_.insert(ID_PAIR(StorageUpdate));
            map_.insert(ID_PAIR(InspectionToS6k));

            map_.insert(ID_PAIR(QuerySystemStatus));
            map_.insert(ID_PAIR(RecorderPoll));
            map_.insert(ID_PAIR(EthercatInputsToService));
            map_.insert(ID_PAIR(S6K_InfoToProcesses));
            map_.insert(ID_PAIR(S6K_InfoFromProcesses));
            map_.insert(ID_PAIR(ControlSimulation));
            map_.insert(ID_PAIR(SchedulerEvents));
            map_.insert(ID_PAIR(VIServiceFromGUI));
            map_.insert(ID_PAIR(DeviceNotification));

			// test interfaces, bei Bedarf einbinden
#ifdef _UNITTEST
			map_.insert(ID_PAIR(TestServer));
			map_.insert(ID_PAIR(TestSubscriber));
			map_.insert(ID_PAIR(T3ControlDevice));
			map_.insert(ID_PAIR(T3ControlWorker));
			map_.insert(ID_PAIR(T3SupplyWorker));
			map_.insert(ID_PAIR(T3InformControl));
			map_.insert(ID_PAIR(T3DataBase));
			map_.insert(ID_PAIR(T3Access));
			map_.insert(ID_PAIR(EventTest));
#endif
		}
	public:
		//PvString operator [] (Interfaces i) { return map_[i]; }
		PvString operator [] (int i) const { return map_[i]; }
	private:
		mutable IdToString map_;
	};

	class SYSTEM_API ModuleNames {
	public:
		typedef std::map<int, PvString> IdNameMap;
		ModuleNames() {
			map_.insert(ID_PAIR(InvalidModule));
			map_.insert(ID_PAIR(ModuleManager));
			map_.insert(ID_PAIR(WorkflowModul));
			map_.insert(ID_PAIR(TriggerModul));
			map_.insert(ID_PAIR(GrabberModul));

			// bei Bedarf einbinden
#ifdef _UNITTEST
			map_.insert(ID_PAIR(T3Device));
			map_.insert(ID_PAIR(T3Worker));
			map_.insert(ID_PAIR(T3Control));
#endif
			map_.insert(ID_PAIR(VIWeldHeadControl));
			map_.insert(ID_PAIR(VIInspectionControl));
			map_.insert(ID_PAIR(VIServiceModul));
			map_.insert(ID_PAIR(CalibrationModul));
			map_.insert(ID_PAIR(VideoRecorderModul));
			map_.insert(ID_PAIR(LoggerServer));
			map_.insert(ID_PAIR(LoggerConsole));
			map_.insert(ID_PAIR(EtherCATMasterModul));
			map_.insert(ID_PAIR(SimulationModul));
            map_.insert(ID_PAIR(StorageModul));
			map_.insert(ID_PAIR(UserInterfaceModul));			
			map_.insert(ID_PAIR(CHRCommunicationModul));
            map_.insert(ID_PAIR(MockAxisModul));
            map_.insert(ID_PAIR(FieldbusModul));
            map_.insert(ID_PAIR(TCPCommunicationModul));
            map_.insert(ID_PAIR(SchedulerModul));

			// bei Bedarf einbinden!
#ifdef _UNITTEST
			map_.insert(ID_PAIR(TestServerModule));
			map_.insert(ID_PAIR(TestServerModule1));
			map_.insert(ID_PAIR(TestServerModule2));
			map_.insert(ID_PAIR(TestClientModule));
			map_.insert(ID_PAIR(TestClientModule1));
			map_.insert(ID_PAIR(TestClientModule2));
			map_.insert(ID_PAIR(TestSubscriberModule));
#endif
		}

	public:
		PvString operator [] (int i) const { return map_[i]; }
	private:
		mutable IdToString map_;
	};
	#undef ID_PAIR

	extern SYSTEM_API const InterfaceNames InterfaceName;
	extern SYSTEM_API const ModuleNames 	ModuleName;


	inline std::ostream &operator <<(std::ostream &os, Interfaces const& i) {
		os << InterfaceName[i]; return os;
	}

	inline std::ostream &operator <<(std::ostream &os, Modules const& m) {
		os << ModuleName[m]; return os;
	}

} // namespace module
} // namespace system
} // namespace precitec

#endif /*INTERFACES_H*/
