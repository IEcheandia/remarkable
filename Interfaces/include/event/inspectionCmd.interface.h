/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, WOR, EA, AL, AB, SB, HS
 * 	@date		2009
 * 	@brief		Triggers diverse actions in analayzer and workflow.
 */

#ifndef INSPECTIONCMD_INTERFACE_H_
#define INSPECTIONCMD_INTERFACE_H_


#include "server/interface.h"
#include "module/interfaces.h" // wg appId
#include "protocol/protocol.info.h"

#include "event/inspectionCmd.h" // Header ist leer ????

/*
 * Hier werden die abstrakten Basisklassen, sowie die
 * Messages definiert. Dieser Header wird sowohl fuer
 * den lokalen (Implementation, MsgHandler) sowie den
 * remote-Teil (Proxyer) benoetigt.
 */
namespace precitec
{
namespace interface
{
	//----------------------------------------------------------
	// Hier folgen die Template-Definitionen fuer die verschiedenen
	// Spezialisierungen  <Implementation> <MsgHandler> <Proxyer> <Messages>

	template <int mode>
	class TInspectionCmd;

	//----------------------------------------------------------
	// Abstrakte Basis Klassen  = Interface
	// Die <Implementation> und <Proxy> Spezialisierungen leiten von diesen
	// Basisklassen ab.

	/**
	 * Signaler zeigt auf primitive Weise den Systemzustand an.
	 * Der State-Enum bietet drei Zustaende an. Verschiedene
	 * Handler koennen diese Zustaende unterschiedlich darstellen.
	 */
	template<>
	class TInspectionCmd<AbstractInterface>
	{
	public:
		TInspectionCmd() {}
		virtual ~TInspectionCmd() {}
	public:

		// Kritischer Fehler -> gehe in NotReady Zustand
		virtual void signalNotReady(int relatedException) = 0;
		// Kalibration anfordern
		virtual void requestCalibration(int type) = 0;
		// Livebetrieb start
		virtual void startLivemode(const Poco::UUID& productId, int seamseries, int seam) = 0;
		// Livebetrieb stop
		virtual void stopLivemode() = 0;
		// Initialiert und startet die Simulation mit einer bestimmten Produktnummer.
		// mode beschreibt, ob die HW deaktiviert werden soll usw. Definition noch offen
		virtual void startSimulation(const Poco::UUID& productId, int mode) = 0;
		// Simulation stop
		virtual void stopSimulation() = 0;
		// Start ProductTeachInMode
		virtual void startProductTeachInMode() = 0;
		// Abort ProductTeachInMode ()
		virtual void abortProductTeachInMode() = 0;
		// anstehenden Systemfehler quittieren
		virtual void quitSystemFault() = 0;
		// Reset Summenfehler
		virtual void resetSumErrors() = 0;
		// Notaus aktiviert
		virtual void emergencyStop() = 0;
		// Notaus deaktiviert
		virtual void resetEmergencyStop() = 0;

        /**
         * Removes the critical error set by signalNotReady -> goes back in Ready state if possible
         **/
        virtual void signalReady(int relatedException) = 0;
	};

    struct TInspectionCmdMessageDefinition
    {
		EVENT_MESSAGE(SignalNotReady, int);
		EVENT_MESSAGE(RequestCalibration, int);
		EVENT_MESSAGE(StartLivemode, Poco::UUID, int, int);
		EVENT_MESSAGE(StopLivemode, void);
		EVENT_MESSAGE(StartSimulation, Poco::UUID, int);
		EVENT_MESSAGE(StopSimulation, void);
		EVENT_MESSAGE(StartProductTeachInMode, void);
		EVENT_MESSAGE(AbortProductTeachInMode, void);
		EVENT_MESSAGE(QuitSystemFault, void);
		EVENT_MESSAGE(ResetSumErrors, void);
		EVENT_MESSAGE(EmergencyStop, void);
		EVENT_MESSAGE(ResetEmergencyStop, void);
        EVENT_MESSAGE(SignalReady, int);

		MESSAGE_LIST(
			SignalNotReady,
			RequestCalibration,
			StartLivemode,
			StopLivemode,
			StartSimulation,
			StopSimulation,
			StartProductTeachInMode,
			AbortProductTeachInMode,
			QuitSystemFault,
			ResetSumErrors,
			EmergencyStop,
			ResetEmergencyStop,
            SignalReady
        );
    };

	//----------------------------------------------------------
	template <>
	class TInspectionCmd<Messages> : public Server<Messages>, public TInspectionCmdMessageDefinition
	{
	public:
		TInspectionCmd<Messages>() : info(system::module::InspectionCmd, sendBufLen, replyBufLen, MessageList::NumMessages, NumBuffers) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 500*Bytes, replyBufLen = 100*Bytes, NumBuffers=128 };
	};


} // namespace interface
} // namespace precitec


#endif /*INSPECTIONCMD_INTERFACE_H_*/
