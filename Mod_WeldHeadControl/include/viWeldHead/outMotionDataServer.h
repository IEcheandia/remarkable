#ifndef OUTMOTIONDATASERVER_H_
#define OUTMOTIONDATASERVER_H_

#include "event/viWeldHeadPublish.h"
#include "event/viWeldHeadPublish.interface.h"
#include "event/viWeldHeadPublish.proxy.h"

namespace precitec
{
	using namespace interface;

namespace viWeldHead
{

	// Verarbeitet MotionData
	class OutMotionDataServer : public TviWeldHeadPublish<AbstractInterface>
	{
		public:
			OutMotionDataServer( TviWeldHeadPublish<EventProxy>& outMotionDataProxy );
			virtual ~OutMotionDataServer();

		public:
			//current errorCode
			virtual void headError(HeadAxisID axis, ErrorCode errorCode, int value); // Interface: viWeldHeadPublish (event)

			//current head informations
			virtual void headInfo(HeadAxisID axis, HeadInfo info);                   // Interface: viWeldHeadPublish (event)

			//signal head is ready in "mode"
			virtual void headIsReady(HeadAxisID axis, MotionMode currentMode);       // Interface: viWeldHeadPublish (event)

			//head reached requested value... (currentValue in mode)
			virtual void headValueReached(HeadAxisID axis, MotionMode currentMode, int currentValue); // Interface: viWeldHeadPublish (event)

			//info about the ScanTracker status
			virtual void trackerInfo(TrackerInfo p_oTrackerInfo); // Interface: viWeldHeadPublish (event)

		private:
			TviWeldHeadPublish<EventProxy>& outMotionDataProxy_;
	};

} // namespace viWeldHead

} // namespace precitec

#endif /*OUTMOTIONDATASERVER_H_*/

