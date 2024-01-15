/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, WOR, EA, AL, AB, SB, HS
 * 	@date		2009
 * 	@brief		Triggers diverse actions in analayzer and workflow.
 */

#ifndef INSPECTIONCMD_SERVER_H_
#define INSPECTIONCMD_SERVER_H_



#include "event/inspectionCmd.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TInspectionCmd<EventServer> : public TInspectionCmd<AbstractInterface>
	{
	public:
		TInspectionCmd(){}
		virtual ~TInspectionCmd() {}
	public:

		// Kritischer Fehler -> gehe in NotReady Zustand
		virtual void signalNotReady(int relatedException){}
		// Kalibration anfordern
		virtual void requestCalibration(int type) {}
		// Livebetrieb start
		virtual void startLivemode(const Poco::UUID& ProductId, int seam, int seamseries) {}
		// Livebetrieb stop
		virtual void stopLivemode() {}
		// Simulation start
		virtual void startSimulation(const Poco::UUID& ProductId, int mode) {}
		// Simulation stop
		virtual void stopSimulation() {}
		// Start ProductTeachInMode
		virtual void startProductTeachInMode() {}
		// Abort ProductTeachInMode
		virtual void abortProductTeachInMode() {}
		// anstehenden Systemfehler quittieren
		virtual void quitSystemFault() {}
		// Reset Summenfehler
		virtual void resetSumErrors(){}
		// Notaus aktiviert
		virtual void emergencyStop(){}
		// Notaus deaktiviert
		virtual void resetEmergencyStop(){}

        void signalReady(int relatedException) override {}
	};


} // interface
} // precitec



#endif /*INSPECTIONCMD_SERVER_H_*/
