#ifndef VIWELDHEADSUBSCRIBE_PROXY_H_
#define VIWELDHEADSUBSCRIBE_PROXY_H_


#include "server/eventProxy.h"
#include "event/viWeldHeadSubscribe.interface.h"
#include "Poco/UUID.h"


namespace precitec
{
namespace interface
{

	template <>
	class TviWeldHeadSubscribe<EventProxy> : public Server<EventProxy>, public TviWeldHeadSubscribe<AbstractInterface>, public TviWeldHeadSubscribeMessageDefinition
	{
	public:
		/// Der CTor braucht die Klasse, die die Arbeit erledigt, also den eigentlichen Server
		TviWeldHeadSubscribe() : EVENT_PROXY_CTOR(TviWeldHeadSubscribe), TviWeldHeadSubscribe<AbstractInterface>()
		{
			//std::cout << "remote CTor::TRegistrar<Proxy> ohne Protokoll" << std::endl;
		}

		virtual ~TviWeldHeadSubscribe() {}

	public:
		virtual void SetHeadMode(HeadAxisID axis, MotionMode mode, bool bHome){
			INIT_EVENT(SetHeadModeMsg);
			signaler().marshal(axis);
			signaler().marshal(mode);
			signaler().marshal(bHome);
			signaler().send();
		}

		virtual void SetHeadValue(HeadAxisID axis, int value, MotionMode mode){
			INIT_EVENT(SetHeadValueMsg);
			signaler().marshal(axis);
			signaler().marshal(value);
			signaler().marshal(mode);
			signaler().send();
		}

		virtual void SetTrackerScanWidthControlled(int value){
			INIT_EVENT(SetTrackerScanWidthControlledMsg);
			signaler().marshal(value);
			signaler().send();
		}
		virtual void SetTrackerScanPosControlled(int value){
			INIT_EVENT(SetTrackerScanPosControlledMsg);
			signaler().marshal(value);
			signaler().send();
		}
		virtual void SetTrackerDriverOnOff(bool onOff){
			INIT_EVENT(SetTrackerDriverOnOffMsg);
			signaler().marshal(onOff);
			signaler().send();
		}

		virtual void doZCollDriving(DrivingType p_oDrivingType, int value){
			INIT_EVENT(DoZCollDriving);
			signaler().marshal(p_oDrivingType);
			signaler().marshal(value);
			signaler().send();
		}

		virtual void RequestHeadInfo(HeadAxisID axis){
			INIT_EVENT(RequestHeadInfoMsg);
			signaler().marshal(axis);

			signaler().send();
		}

		virtual void SetLCPowerOffset(int value){
			INIT_EVENT(SetLCPowerOffsetMsg);
			signaler().marshal(value);
			signaler().send();
		}

		virtual void SetGenPurposeAnaOut(OutputID outputNo, int value){
			INIT_EVENT(SetGenPurposeAnaOutMsg);
			signaler().marshal(outputNo);
			signaler().marshal(value);
			signaler().send();
		}

		virtual void ScanmasterResult(ScanmasterResultType p_oResultType, ResultDoubleArray const& p_rResultDoubleArray){
			INIT_EVENT(ScanmasterResultMsg);
			signaler().marshal(p_oResultType);
			signaler().marshal(p_rResultDoubleArray);
			signaler().send();
		}

		void ScanmasterHeight(double value) override
        {
            INIT_EVENT(ScanmasterHeightMsg);
            signaler().marshal(value);
            signaler().send();
        }
	};

} // interface
} // precitec


#endif /*VIWELDHEADSUBSCRIBE_PROXY_H_*/
