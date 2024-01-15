#ifndef VIWELDHEADPUBLISH_PROXY_H_
#define VIWELDHEADPUBLISH_PROXY_H_


#include "server/eventProxy.h"
#include "event/viWeldHeadPublish.interface.h"
#include "Poco/UUID.h"


namespace precitec
{
namespace interface
{

	template <>
	class TviWeldHeadPublish<EventProxy> : public Server<EventProxy>, public TviWeldHeadPublish<AbstractInterface>, public TviWeldHeadPublishMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TviWeldHeadPublish() : EVENT_PROXY_CTOR(TviWeldHeadPublish), TviWeldHeadPublish<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TviWeldHeadPublish() {}

	public:
		virtual void headError(HeadAxisID axis, ErrorCode errorCode, int value){
			INIT_EVENT(HeadError);
			//signaler().initMessage(Msg::index);
			signaler().marshal(axis);
			signaler().marshal(errorCode);
			signaler().marshal(value);
			signaler().send();
		}

		virtual void headInfo(HeadAxisID axis, HeadInfo info){
			INIT_EVENT(HeadInfoMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(axis);
			signaler().marshal(info);
			signaler().send();
		}

		virtual void headIsReady(HeadAxisID axis, MotionMode currentMode){
			INIT_EVENT(HeadIsReady);
			//signaler().initMessage(Msg::index);
			signaler().marshal(axis);
			signaler().marshal(currentMode);
			signaler().send();
		}

		virtual void headValueReached(HeadAxisID axis, MotionMode currentMode, int currentValue){
			INIT_EVENT(HeadValueReached);
			//signaler().initMessage(Msg::index);
			signaler().marshal(axis);
			signaler().marshal(currentMode);
			signaler().marshal(currentValue);
			signaler().send();
		}

		virtual void trackerInfo(TrackerInfo p_oTrackerInfo){
			INIT_EVENT(TrackerInfoMsg);
			//signaler().initMessage(Msg::index);
			signaler().marshal(p_oTrackerInfo);
			signaler().send();
		}
	};

} // interface
} // precitec


#endif /*VIWELDHEADPUBLISH_PROXY_H_*/
