#ifndef WELDHEAD_SERVER_H_
#define WELDHEAD_SERVER_H_

#include  <map> // wg HandlerList
#include  "Poco/NamedMutex.h"
#include  "Poco/ScopedLock.h"
#include  "Poco/Process.h"
#include  "Poco/Path.h"

#include  "message/weldHead.interface.h"


namespace precitec
{
namespace interface
{

	template <>
	class TWeldHeadMsg<MsgServer> : public TWeldHeadMsg<AbstractInterface>
	{
	public:
		// aehnlich dem PoCo-Subsystem-Interface
		// 1. Ini- wird nur einmal durchgefuehrt
		virtual bool setHeadPos(HeadAxisID axis, int pos) {return false;}
		virtual bool setHeadMode(HeadAxisID axis, MotionMode mode, bool bHome){ return false; }
		virtual int getHeadPosition(HeadAxisID axis){ return 0; }
		virtual int getLowerLimit(HeadAxisID axis){ return 0; }
		virtual int getUpperLimit(HeadAxisID axis){ return 0; }
		virtual bool doZCollHoming(void){ return false; }
		virtual bool getLEDEnable(LEDPanelNo ledPanel){ return false; }
		virtual bool setLEDEnable(LEDPanelNo ledPanel, bool enabled){ return false; }
		bool reloadFiberSwitchCalibration(void) override { return false; }
        bool doZCollDrivingRelative(int value) override { return false; }
	};


} // namespace interface
} // namespace precitec

#endif /*WELDHEAD_SERVER_H_*/
