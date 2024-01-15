#ifndef WELDHEAD_HANDLER_H_
#define WELDHEAD_HANDLER_H_

#include  "Poco/NamedMutex.h"
#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"

#include  "server/handler.h"
#include  "message/weldHead.interface.h"

namespace precitec
{
namespace interface
{
	template <>
	class TWeldHeadMsg<MsgHandler> : public Server<MsgHandler>, public TWeldHeadMsgDefinition
	{
	public:
		MSG_HANDLER(TWeldHeadMsg );
	public:

		void registerCallbacks()
		{
			// die Message-Callbacks eintragen, kein Returntyp in Macros!!!
			REGISTER_MESSAGE(SetHeadPos, setHeadPos);
			REGISTER_MESSAGE(SetHeadMode, setHeadMode);
			REGISTER_MESSAGE(GetHeadPosition, getHeadPosition);
			REGISTER_MESSAGE(GetLowerLimit, getLowerLimit);
			REGISTER_MESSAGE(GetUpperLimit, getUpperLimit);
			REGISTER_MESSAGE(DoZCollHoming, doZCollHoming);
			REGISTER_MESSAGE(GetLEDEnable, getLEDEnable);
			REGISTER_MESSAGE(SetLEDEnable, setLEDEnable);
			REGISTER_MESSAGE(ReloadFiberSwitchCalibration, reloadFiberSwitchCalibration);
            REGISTER_MESSAGE(WeldForScannerCalibration, weldForScannerCalibration);
			REGISTER_MESSAGE(DoZCollDrivingRelative, doZCollDrivingRelative);
		}

		void setHeadPos(Receiver &receiver)
		{
			HeadAxisID axis; receiver.deMarshal(axis);
			int value; receiver.deMarshal(value);
			receiver.marshal(server_->setHeadPos(axis,value));
			receiver.reply();
		}

		void setHeadMode(Receiver &receiver)
		{

			HeadAxisID axis;
			receiver.deMarshal(axis);
			MotionMode mode;
			receiver.deMarshal(mode);
			bool bHome;
			receiver.deMarshal(bHome);
			receiver.marshal( server_->setHeadMode(axis,mode,bHome) );
			receiver.reply();
		}

		void getHeadPosition(Receiver &receiver)
		{

			HeadAxisID axis;
			receiver.deMarshal(axis);
			receiver.marshal( server_->getHeadPosition(axis) );
			receiver.reply();
		}

		void getLowerLimit(Receiver &receiver)
		{

			HeadAxisID axis;
			receiver.deMarshal(axis);
			receiver.marshal( server_->getLowerLimit(axis) );
			receiver.reply();
		}

		void getUpperLimit(Receiver &receiver)
		{

			HeadAxisID axis;
			receiver.deMarshal(axis);
			receiver.marshal( server_->getUpperLimit(axis) );
			receiver.reply();
		}

		void doZCollHoming(Receiver &receiver)
		{
			receiver.marshal( server_->doZCollHoming() );
			receiver.reply();
		}

        void getLEDEnable(Receiver &receiver)
        {
            LEDPanelNo ledPanel;
            receiver.deMarshal(ledPanel);
            receiver.marshal( server_->getLEDEnable(ledPanel) );
            receiver.reply();
        }

        void setLEDEnable(Receiver &receiver)
        {
            LEDPanelNo ledPanel;
            receiver.deMarshal(ledPanel);
            bool enabled;
            receiver.deMarshal(enabled);
            receiver.marshal( server_->setLEDEnable(ledPanel, enabled) );
            receiver.reply();
        }

        void reloadFiberSwitchCalibration(Receiver &receiver)
        {
            receiver.marshal(server_->reloadFiberSwitchCalibration());
            receiver.reply();
        }

        void weldForScannerCalibration(Receiver &receiver)
        {
            std::vector<geo2d::DPoint> points;
            double durationInMs;
            double laserPowerInPct;
            double jumpSpeedInMmPerSec;
            receiver.deMarshal(jumpSpeedInMmPerSec);
            receiver.deMarshal(durationInMs);
            receiver.deMarshal(laserPowerInPct);
            receiver.deMarshal(points);
            receiver.marshal(server_->weldForScannerCalibration(points, laserPowerInPct, durationInMs, jumpSpeedInMmPerSec));
            receiver.reply();
        }

        void doZCollDrivingRelative(Receiver &receiver)
        {
            int value;
            receiver.deMarshal(value);
            receiver.marshal(server_->doZCollDrivingRelative(value));
            receiver.reply();
        }
	};

} // namespace interface
} // namespace precitec

#endif /*WELDHEAD_HANDLER_H_*/
