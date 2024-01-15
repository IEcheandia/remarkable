/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       03.10.2011
 *  @brief      Part of the inspectionOut Interface
 *  @details
 */

#ifndef INSPECTIONOUT_PROXY_H_
#define INSPECTIONOUT_PROXY_H_

#include "event/inspectionOut.h"
#include "event/inspectionOut.interface.h"
#include "server/eventProxy.h"

namespace precitec
{

namespace interface
{

	template <>
	class TInspectionOut<EventProxy> : public Server<EventProxy>, public TInspectionOut<AbstractInterface>, public TInspectionOutMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TInspectionOut() : EVENT_PROXY_CTOR(TInspectionOut), TInspectionOut<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TInspectionOut() {}

	public:

		virtual void setSystemReady(bool onoff)
		{
			INIT_EVENT(SetSystemReady);
			signaler().marshal(onoff);
			signaler().send();
		}

		virtual void setSystemErrorField(int systemErrorField)
		{
			INIT_EVENT(SetSystemErrorField);
			signaler().marshal(systemErrorField);
			signaler().send();
		}

		virtual void setSumErrorLatched(bool onoff)
		{
			INIT_EVENT(SetSumErrorLatched);
			signaler().marshal(onoff);
			signaler().send();
		}

		virtual void setQualityErrorField(int qualityErrorField)
		{
			INIT_EVENT(SetQualityErrorField);
			signaler().marshal(qualityErrorField);
			signaler().send();
		}

		virtual void setInspectCycleAckn(bool onoff)
		{
			INIT_EVENT(SetInspectCycleAckn);
			signaler().marshal(onoff);
			signaler().send();
		}

		virtual void setCalibrationFinished(bool result)
		{
			INIT_EVENT(SetCalibrationFinished);
			signaler().marshal(result);
			signaler().send();
		}

	};

} // namespace interface
} // namespace precitec

#endif /* INSPECTIONOUT_PROXY_H_ */

