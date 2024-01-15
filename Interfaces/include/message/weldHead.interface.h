#ifndef WELDHEAD_INTERFACE_H_
#define WELDHEAD_INTERFACE_H_

#include  "server/interface.h"
#include  "module/interfaces.h" // wg appId
#include  "protocol/protocol.info.h"
#include  "event/viWeldHeadPublish.h" // MotionMode

namespace precitec
{
	using namespace  system;
	using namespace  message;

namespace interface
{

	template <int CallType>
	class TWeldHeadMsg;

	/**
	 *	AbstrakteBasisklasse des Module Servers
	 * 	Der Modul-Server kommuniziert mit dem Module-Manager,
	 * 		d.h. er wartet auf Befehler dessselben
	 */
	template <>
	class TWeldHeadMsg<AbstractInterface>
	{
	public:
		TWeldHeadMsg() {}
		virtual ~TWeldHeadMsg() {}
	public:
		// aehnlich dem PoCo-Subsystem-Interface
		virtual bool setHeadPos(HeadAxisID axis, int value) = 0;
		virtual bool setHeadMode(HeadAxisID axis, MotionMode mode, bool bHome) = 0 ;
		virtual int getHeadPosition(HeadAxisID axis) = 0 ;
		virtual int getLowerLimit(HeadAxisID axis) = 0 ;
		virtual int getUpperLimit(HeadAxisID axis) = 0 ;
		virtual bool doZCollHoming(void) = 0 ;
        virtual bool getLEDEnable(LEDPanelNo ledPanel) = 0;
        virtual bool setLEDEnable(LEDPanelNo ledPanel, bool enabled) = 0;
        virtual bool reloadFiberSwitchCalibration(void) = 0;
        virtual bool weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserPowerInPct, double durationInMs, double jumpSpeedInMmPerSec) = 0;
        virtual bool doZCollDrivingRelative(int value) = 0;
	};

    struct TWeldHeadMsgDefinition
    {
		MESSAGE(bool, SetHeadPos, HeadAxisID, int);
		MESSAGE(bool, SetHeadMode, HeadAxisID, MotionMode, bool);
		MESSAGE(int, GetHeadPosition, HeadAxisID);
		MESSAGE(int, GetLowerLimit, HeadAxisID);
		MESSAGE(int, GetUpperLimit, HeadAxisID);
		MESSAGE(bool, DoZCollHoming, void);
        MESSAGE(bool, GetLEDEnable, LEDPanelNo);
        MESSAGE(bool, SetLEDEnable, LEDPanelNo, bool);
        MESSAGE(bool, ReloadFiberSwitchCalibration, void);
        MESSAGE(bool, WeldForScannerCalibration, void);
        MESSAGE(bool, DoZCollDrivingRelative, int);

		MESSAGE_LIST(
			SetHeadPos,
			SetHeadMode,
			GetHeadPosition,
			GetLowerLimit,
			GetUpperLimit,
			DoZCollHoming,
            GetLEDEnable,
            SetLEDEnable,
            ReloadFiberSwitchCalibration,
            WeldForScannerCalibration,
            DoZCollDrivingRelative
		);
    };

	template <>
	class TWeldHeadMsg<Messages> : public Server<Messages>, public TWeldHeadMsgDefinition
	{
	public:
		TWeldHeadMsg() : info(module::WeldHeadMsg, sendBufLen, replyBufLen, MessageList::NumMessages) {}
		MessageInfo info;
	private:
		/// Konstanten wg Lesbarkeit, diese koennten auch in der Basisklasse stehen, wuerden dann aber wohl kaum verwendet
		enum { Bytes=1, KBytes=1024, MBytes=1024*1024};
		enum { sendBufLen  = 50*KBytes, replyBufLen = 10000*Bytes };
	};

} // namespace interface
} // namespace precitec


#endif /*WELDHEAD_INTERFACE_H_*/
