/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		KIR, WOR, EA, AL, AB, SB, HS
 * 	@date		2009
 * 	@brief		Triggers diverse actions in analayzer and workflow.
 */

#ifndef INSPECTIONCMD_HANDLER_H_
#define INSPECTIONCMD_HANDLER_H_


#include "event/inspectionCmd.h"
#include "event/inspectionCmd.interface.h"
#include "server/eventHandler.h"

#include "module/moduleLogger.h"

namespace precitec
{
namespace interface
{
	template <>
	class TInspectionCmd<EventHandler> : public Server<EventHandler>, public TInspectionCmdMessageDefinition
	{
	public:
		EVENT_HANDLER( TInspectionCmd );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(SignalNotReady, signalNotReady);
			REGISTER_EVENT(RequestCalibration, requestCalibration);
			REGISTER_EVENT(StartLivemode, startLivemode);
			REGISTER_EVENT(StopLivemode, stopLivemode);
			REGISTER_EVENT(StartSimulation, startSimulation);
			REGISTER_EVENT(StopSimulation, stopSimulation);
			REGISTER_EVENT(StartProductTeachInMode, startProductTeachInMode);
			REGISTER_EVENT(AbortProductTeachInMode, abortProductTeachInMode);
			REGISTER_EVENT(QuitSystemFault, quitSystemFault);
			REGISTER_EVENT(ResetSumErrors, resetSumErrors);
			REGISTER_EVENT(EmergencyStop, emergencyStop);
			REGISTER_EVENT(ResetEmergencyStop, resetEmergencyStop);
            REGISTER_EVENT(SignalReady, signalReady);
		}

		void signalNotReady(Receiver &receiver)
		{
			int relatedException; receiver.deMarshal(relatedException);
			getServer()->signalNotReady(relatedException);
						}
		void requestCalibration(Receiver &receiver)
		{
			int type; receiver.deMarshal(type);

			getServer()->requestCalibration(type);
		}

		void startLivemode(Receiver &receiver)
		{
			Poco::UUID productID; receiver.deMarshal(productID);
			int seamseries; receiver.deMarshal(seamseries);

			int seam; receiver.deMarshal(seam);

			getServer()->startLivemode(productID,seamseries,seam);
		}

		void stopLivemode(Receiver &receiver)
		{
			getServer()->stopLivemode();
		}

		void startSimulation(Receiver &receiver)
		{
			Poco::UUID productID; receiver.deMarshal(productID);
			int mode; receiver.deMarshal(mode);

			server_->startSimulation( productID, mode);
		}
		void stopSimulation(Receiver &receiver)
		{
			server_->stopSimulation();
		}
		void startProductTeachInMode(Receiver &receiver)
		{
			server_->startProductTeachInMode();
		}
		void abortProductTeachInMode(Receiver &receiver)
		{
			server_->abortProductTeachInMode();
		}
		void quitSystemFault(Receiver &receiver)
		{
			server_->quitSystemFault();
		}
		void resetSumErrors(Receiver &receiver)
		{
			server_->resetSumErrors();
		}

		void emergencyStop(Receiver &receiver)
		{
			server_->emergencyStop();
		}
		void resetEmergencyStop(Receiver &receiver)
		{
			server_->resetEmergencyStop();
		}

        void signalReady(Receiver &receiver)
        {
            int relatedException;
            receiver.deMarshal(relatedException);
            server_->signalReady(relatedException);
        }


	private:
		TInspectionCmd<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec



#endif /*INSPECTIONCMD_HANDLER_H_*/
