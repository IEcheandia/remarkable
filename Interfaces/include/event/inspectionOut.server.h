/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     EA
 *  @date       03.10.2011
 *  @brief      Part of the inspectionOut Interface
 *  @details
 */

#ifndef INSPECTIONOUT_SERVER_H_
#define INSPECTIONOUT_SERVER_H_

#include "event/inspectionOut.interface.h"

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TInspectionOut<EventServer> : public TInspectionOut<AbstractInterface>
	{
	public:
		TInspectionOut(){}
		virtual ~TInspectionOut() {}
	public:
		/// interface inspectionOut : setSystemReady
		virtual void setSystemReady (bool onoff) {}
		/// interface inspectionOut : setSystemErrorField
		virtual void setSystemErrorField (int systemErrorField) {}
		/// interface inspectionOut : setSumErrorLatched
		virtual void setSumErrorLatched (bool onoff) {}
		/// interface inspectionOut : setQualityErrorField
		virtual void setQualityErrorField (int qualityErrorField) {}
		/// interface inspectionOut : setInspectCycleAckn
		virtual void setInspectCycleAckn (bool onoff) {}
		/// interface inspectionOut : setCalibrationFinished
		virtual void setCalibrationFinished (bool result) {}
	};


} // namespace interface
} // namespace precitec

#endif /* INSPECTIONOUT_SERVER_H_ */

