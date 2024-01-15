#ifndef VIWELDHEADPUBLISH_HANDLER_H_
#define VIWELDHEADPUBLISH_HANDLER_H_


#include "event/viWeldHeadPublish.h"
#include "event/viWeldHeadPublish.interface.h"
#include "server/eventHandler.h"


namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TviWeldHeadPublish<EventHandler> : public Server<EventHandler>, public TviWeldHeadPublishMessageDefinition
	{
	public:
		EVENT_HANDLER( TviWeldHeadPublish );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(HeadError, headError);
			REGISTER_EVENT(HeadInfoMsg, headInfo);
			REGISTER_EVENT(HeadIsReady, headIsReady);
			REGISTER_EVENT(HeadValueReached, headValueReached);
			REGISTER_EVENT(TrackerInfoMsg, trackerInfo);

		}

		void headError(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			ErrorCode errorCode;
			receiver.deMarshal(errorCode);
			int value;
			receiver.deMarshal(value);
			getServer()->headError(axis,errorCode,value);
		}

		void headInfo(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			HeadInfo info;
			receiver.deMarshal(info);
			getServer()->headInfo(axis,info);
		}

		void headIsReady(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			MotionMode mode;
			receiver.deMarshal(mode);
			getServer()->headIsReady(axis,mode);
		}

		void headValueReached(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			MotionMode currentMode;
			receiver.deMarshal(currentMode);
			int currentValue;
			receiver.deMarshal(currentValue);
			getServer()->headValueReached(axis,currentMode,currentValue);
		}

		void trackerInfo(Receiver &receiver)
		{
			TrackerInfo oTrackerInfo;
			receiver.deMarshal(oTrackerInfo);
			getServer()->trackerInfo(oTrackerInfo);
		}


		private:
				TviWeldHeadPublish<AbstractInterface> * getServer()
				{
					return server_;
				}


	};

} // namespace interface
} // namespace precitec



#endif /*VIWELDHEADPUBLISH_HANDLER_H_*/
