#ifndef INSPECTIONCMDSERVER_H_
#define INSPECTIONCMDSERVER_H_

#include "Mod_Workflow.h"

#include "event/inspectionCmd.h"
#include "event/inspectionCmd.interface.h"

#include "stateMachine/stateContext.h"

#include "system/tools.h"


namespace precitec {
using namespace interface;
namespace workflow { 

	// Nimmt die Datenpakete vom VMI-Sensor entgegen und verarbeitet diese
	class MOD_WORKFLOW_API InspectionCmdServer : public TInspectionCmd<AbstractInterface>
	{
		public:
			InspectionCmdServer(SmStateContext stateContext);

		public:

			// Kalibration anfordern.
			void requestCalibration( int p_oType )
			{
				try {
					stateContext_->calibrate( p_oType );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch	
			}

			// Livebetrieb start.
			void startLivemode( const Poco::UUID& p_oProductID, int p_oSeamseries, int p_oSeam )
			{
				try {
					stateContext_->startLive( p_oProductID, p_oSeamseries, p_oSeam );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			// Livebetrieb stop.
			void stopLivemode()
			{
				try {
					stateContext_->stopLive();
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch	
		}

			// Im Simulationsbetrieb wird praktisch das gleiche gemacht wie im Automatikbetrieb, ausser
			// dass das Kommando InspectionStart geleich mit generiert wird
			void startSimulation(const Poco::UUID& p_oProductID, int mode)
			{
				try {
                    if (stateContext_->isSimulationStation())
                    {
                        // Muss ein bestehender Modus beendet werden?
                        if (stateContext_->currentState() == eAutomaticMode)
                            stateContext_->stopAuto();
                        if (stateContext_->currentState() == eLiveMode)
                            stateContext_->stopLive();

                        stateContext_->setProductID( p_oProductID, 0 );

                        // Change into automatic state - product type is 0 and serial number is 0 as well ...
                        stateContext_->startAuto( 0, 0, "no info" );
                        if (stateContext_->currentState() == eAutomaticMode)
                        {
                            stateContext_->setSeamseries(0);
                            stateContext_->beginInspect(0);
                        }
                    } else
                    {
                        wmLog( eError, "Error: startSimulation not supported on the real-time system!\n" );
                    }
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			void stopSimulation()
			{
				try {
					stateContext_->endInspect();
					stateContext_->stopAuto();
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			void startProductTeachInMode()
			{
				try {
					stateContext_->startProductTeachIn();
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			void abortProductTeachInMode()
			{
				try {
					stateContext_->abortProductTeachIn();
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			/**
			 * @brief Kritischer Fehler -> gehe in NotReady Zustand.
			 * @param p_oErrorCode error-code of the exception, as defined in module/moduleLogger.h.
			 */
			void signalNotReady(int p_oErrorCode)
			{
				try {
					stateContext_->signalNotReady( p_oErrorCode );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			/**
			 * @brief Kritischer Fehler aufgehoben, wenn moeglich gehe wieder in Ready Zustand.
			 * @param p_oErrorCode error-code of the exception, that has caused the NotReady state and is no longer a problem. Attention: There might still be other faulty
			 * components, so that the system is not able to go into the Ready state.
			 */
			void signalReady( int p_oErrorCode ) override
			{
				try {
					stateContext_->signalReady( p_oErrorCode );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			// anstehenden Systemfehler quittieren
			void quitSystemFault()
			{
				try {
					stateContext_->quitSystemFault( );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			// Reset Summenfehler
			void resetSumErrors()
			{
				try {
					stateContext_->inspectManager().resetSumErrors();
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			// Notaus aktiviert
			void emergencyStop(){
				try {
					stateContext_->emergencyStop( );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

			// Notaus deaktiviert
			void resetEmergencyStop(){
				try {
					stateContext_->resetEmergencyStop( );
				} // try
				catch(...) {
					logExcpetion(__FUNCTION__, std::current_exception());
				} // catch
			}

		private:
			SmStateContext stateContext_;
	};

}	// workflow
}	// precitec

#endif /*INSPECTIONCMDSERVER_H_*/
