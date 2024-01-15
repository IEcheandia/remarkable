#ifndef VIWELDHEADSUBSCRIBE_HANDLER_H_
#define VIWELDHEADSUBSCRIBE_HANDLER_H_

#include "event/viWeldHeadSubscribe.h"
#include "event/viWeldHeadSubscribe.interface.h"
#include "server/eventHandler.h"

namespace precitec
{
namespace interface
{
	using namespace  message;

	template <>
	class TviWeldHeadSubscribe<EventHandler> : public Server<EventHandler>, public TviWeldHeadSubscribeMessageDefinition
	{
	public:
		EVENT_HANDLER( TviWeldHeadSubscribe );
	public:
		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_EVENT(SetHeadModeMsg, SetHeadMode);
			REGISTER_EVENT(SetHeadValueMsg, SetHeadValue);
			REGISTER_EVENT(SetTrackerScanWidthControlledMsg, SetTrackerScanWidthControlled);
			REGISTER_EVENT(SetTrackerScanPosControlledMsg, SetTrackerScanPosControlled);
			REGISTER_EVENT(SetTrackerDriverOnOffMsg, SetTrackerDriverOnOff);
			REGISTER_EVENT(DoZCollDriving, doZCollDriving);
			REGISTER_EVENT(RequestHeadInfoMsg, RequestHeadInfo);
			REGISTER_EVENT(SetLCPowerOffsetMsg, SetLCPowerOffset);
			REGISTER_EVENT(SetGenPurposeAnaOutMsg, SetGenPurposeAnaOut);
			REGISTER_EVENT(ScanmasterResultMsg, ScanmasterResult);
            REGISTER_EVENT(ScanmasterHeightMsg, ScanmasterHeight);
		}

		void SetHeadMode(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			MotionMode mode;
			receiver.deMarshal(mode);
			bool bHome;
			receiver.deMarshal(bHome);
			getServer()->SetHeadMode(axis,mode,bHome);
		}

		void SetHeadValue(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			int value;
			receiver.deMarshal(value);
			MotionMode mode;
			receiver.deMarshal(mode);
			getServer()->SetHeadValue(axis,value,mode);
		}

		void SetTrackerScanWidthControlled(Receiver &receiver)
		{
			int value;
			receiver.deMarshal(value);
			getServer()->SetTrackerScanWidthControlled(value);
		}
		void SetTrackerScanPosControlled(Receiver &receiver)
		{
			int value;
			receiver.deMarshal(value);
			getServer()->SetTrackerScanPosControlled(value);
		}

		void SetTrackerDriverOnOff(Receiver &receiver)
		{
			bool onOff;
			receiver.deMarshal(onOff);
			getServer()->SetTrackerDriverOnOff(onOff);
		}

		void doZCollDriving(Receiver &receiver)
		{
			DrivingType oDrivingType;
			receiver.deMarshal(oDrivingType);
			int value;
			receiver.deMarshal(value);
			getServer()->doZCollDriving(oDrivingType, value);
		}

		void RequestHeadInfo(Receiver &receiver)
		{
			HeadAxisID axis;
			receiver.deMarshal(axis);
			getServer()->RequestHeadInfo(axis);
		}

		void SetLCPowerOffset(Receiver &receiver)
		{
			int value;
			receiver.deMarshal(value);
			getServer()->SetLCPowerOffset(value);
		}

		void SetGenPurposeAnaOut(Receiver &receiver)
		{
			OutputID outputNo;
			receiver.deMarshal(outputNo);
			int value;
			receiver.deMarshal(value);
			getServer()->SetGenPurposeAnaOut(outputNo, value);
		}

		void ScanmasterResult(Receiver &receiver)
		{
			ScanmasterResultType oResultType;
			receiver.deMarshal(oResultType);
			ResultDoubleArray oResultDoubleArray;
			receiver.deMarshal(oResultDoubleArray);
			getServer()->ScanmasterResult(oResultType, oResultDoubleArray);
		}

		void ScanmasterHeight(Receiver &receiver)
        {
            double value;
            receiver.deMarshal(value);
            getServer()->ScanmasterHeight(value);
        }

		private:
				TviWeldHeadSubscribe<AbstractInterface> * getServer()
				{
					return server_;
				}
	};

} // namespace interface
} // namespace precitec

#endif /*VIWELDHEADSUBSCRIBE_HANDLER_H_*/

