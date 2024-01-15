#include "viWeldHead/outMotionDataServer.h"

namespace precitec {

namespace viWeldHead {

OutMotionDataServer::OutMotionDataServer(TviWeldHeadPublish<EventProxy>& outMotionDataProxy) :
		outMotionDataProxy_(outMotionDataProxy)
{
}

OutMotionDataServer::~OutMotionDataServer()
{
}

void OutMotionDataServer::headError(HeadAxisID axis, ErrorCode errorCode, int value) // Interface: viWeldHeadPublish (event)
{
	outMotionDataProxy_.headError(axis,errorCode,value);
}

void OutMotionDataServer::headInfo(HeadAxisID axis, HeadInfo info) // Interface: viWeldHeadPublish (event)
{
	outMotionDataProxy_.headInfo(axis,info);
}

void OutMotionDataServer::headIsReady(HeadAxisID axis, MotionMode currentMode) // Interface: viWeldHeadPublish (event)
{
	outMotionDataProxy_.headIsReady(axis,currentMode);
}

void OutMotionDataServer::headValueReached(HeadAxisID axis, MotionMode currentMode, int currentValue) // Interface: viWeldHeadPublish (event)
{
	outMotionDataProxy_.headValueReached(axis,currentMode,currentValue);
}

void OutMotionDataServer::trackerInfo(TrackerInfo p_oTrackerInfo) // Interface: viWeldHeadPublish (event)
{
	outMotionDataProxy_.trackerInfo(p_oTrackerInfo);
}

} // namepace viWeldHead

} // namespace precitec

