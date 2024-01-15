#ifndef VIWELDHEADPUBLISH_SERVER_H_
#define VIWELDHEADPUBLISH_SERVER_H_



#include "event/viWeldHeadPublish.interface.h"


namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TviWeldHeadPublish<EventServer> : public TviWeldHeadPublish<AbstractInterface>
	{
	public:
		TviWeldHeadPublish(){}
		virtual ~TviWeldHeadPublish() {}
	public:


		//current errorCode
		virtual void headError(HeadAxisID axis, ErrorCode errorCode, int value){}

		//current head informations
		virtual void headInfo(HeadAxisID axis, HeadInfo info){}


		//signal head is ready in "mode"
		virtual void headIsReady(HeadAxisID axis, MotionMode currentMode){}

		//head reached requested value... (currentValue in mode)
		virtual void headValueReached(HeadAxisID axis, MotionMode currentMode, int currentValue){}

		//info about the ScanTracker status
		virtual void trackerInfo(TrackerInfo p_oTrackerInfo){}



	};


} // interface
} // precitec



#endif /*VIWELDHEADPUBLISH_SERVER_H_*/
