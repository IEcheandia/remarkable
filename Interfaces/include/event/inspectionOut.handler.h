/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       03.10.2011
 *  @brief      Part of the inspectionOut Interface
 *  @details
 */

#ifndef INSPECTIONOUT_HANDLER_H_
#define INSPECTIONOUT_HANDLER_H_

#include "event/inspectionOut.h"
#include "event/inspectionOut.interface.h"
#include "server/eventHandler.h"

namespace precitec
{

namespace interface
{

	using namespace  message;

	template <>
	class TInspectionOut<EventHandler> : public Server<EventHandler>, public TInspectionOutMessageDefinition
	{
	public:
		EVENT_HANDLER( TInspectionOut );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(SetSystemReady, setSystemReady);
			REGISTER_EVENT(SetSystemErrorField, setSystemErrorField);
			REGISTER_EVENT(SetSumErrorLatched, setSumErrorLatched);
			REGISTER_EVENT(SetQualityErrorField, setQualityErrorField);
			REGISTER_EVENT(SetInspectCycleAckn, setInspectCycleAckn);
			REGISTER_EVENT(SetCalibrationFinished, setCalibrationFinished);
		}

		void setSystemReady(Receiver &receiver)
		{
			bool onoff; receiver.deMarshal(onoff);
			getServer()->setSystemReady(onoff);
		}

		void setSystemErrorField(Receiver &receiver)
		{
			int systemErrorField; receiver.deMarshal(systemErrorField);
			getServer()->setSystemErrorField(systemErrorField);
		}

		void setSumErrorLatched(Receiver &receiver)
		{
			bool onoff; receiver.deMarshal(onoff);
			getServer()->setSumErrorLatched(onoff);
		}

		void setQualityErrorField(Receiver &receiver)
		{
			int qualityErrorField; receiver.deMarshal(qualityErrorField);
			getServer()->setQualityErrorField(qualityErrorField);
		}

		void setInspectCycleAckn(Receiver &receiver)
		{
			bool onoff; receiver.deMarshal(onoff);
			getServer()->setInspectCycleAckn(onoff);
		}

		void setCalibrationFinished(Receiver &receiver)
		{
			bool result; receiver.deMarshal(result);
			getServer()->setCalibrationFinished(result);
		}

	private:
		TInspectionOut<AbstractInterface> * getServer()
		{
			return server_;
		}

	};

} // namespace interface
} // namespace precitec

#endif /* INSPECTIONOUT_HANDLER_H_ */

