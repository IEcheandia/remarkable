#ifndef VIWELDHEADSUBSCRIBE_SERVER_H_
#define VIWELDHEADSUBSCRIBE_SERVER_H_



#include "event/viWeldHeadSubscribe.interface.h"


namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <>
	class TviWeldHeadSubscribe<EventServer> : public TviWeldHeadSubscribe<AbstractInterface>
	{
	public:
		TviWeldHeadSubscribe(){}
		virtual ~TviWeldHeadSubscribe() {}
	public:


		virtual void SetHeadMode(HeadAxisID axis, MotionMode mode, bool bHome){std::printf("EventServer -> SetHeadMode()...\n");}

		virtual void SetHeadValue(HeadAxisID axis, int value, MotionMode mode){std::printf("EventServer -> SetHeadValue()...\n");}

		//16 Bit resolution
		virtual void SetTrackerScanWidthControlled(int value){std::printf("EventServer -> SetTrackerScanWidthControlled()...\n");}
		virtual void SetTrackerScanPosControlled(int value){std::printf("EventServer -> SetTrackerScanPosControlled()...\n");}
		virtual void SetTrackerDriverOnOff(bool onOff){std::printf("EventServer -> SetTrackerDriverOnOff()...\n");}

		virtual void doZCollDriving(DrivingType p_oDrivingType, int value){std::printf("EventServer -> doZCollDriving()...\n");}

		virtual void RequestHeadInfo(HeadAxisID axis){std::printf("EventServer -> RequestHeadInfo()...\n");}

		virtual void SetLCPowerOffset(int value){std::printf("EventServer -> SetLCPowerOffset()...\n");}

		virtual void SetGenPurposeAnaOut(OutputID outputNo, int value){std::printf("EventServer -> SetGenPurposeAnaOut()...\n");}

		virtual void ScanmasterResult(ScanmasterResultType p_oResultType, ResultDoubleArray p_rResultDoubleArray){std::printf("EventServer -> ScanmasterResult()...\n");}

		void ScanmasterHeight(double value) override
        {
            std::printf("EventServer -> ScanmasterHeight()...\n");
        }
	};


} // interface
} // precitec



#endif /*VIWELDHEADSUBSCRIBE_SERVER_H_*/
