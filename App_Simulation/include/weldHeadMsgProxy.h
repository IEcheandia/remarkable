#pragma once

#include "message/weldHead.interface.h"


namespace precitec
{
using namespace precitec::interface;

namespace Simulation
{

class WeldHeadMsgServer : public TWeldHeadMsg<AbstractInterface>
{
public:
    WeldHeadMsgServer(){}
    ~WeldHeadMsgServer(){}

    bool setHeadPos(HeadAxisID, int) override
    {
        return true;
    }
    bool setHeadMode(HeadAxisID, MotionMode, bool) override
    {
        return true;
    }
    int getHeadPosition(HeadAxisID) override
    {
        return 0;
    }
    int getLowerLimit(HeadAxisID) override
    {
        return 0;
    }
    int getUpperLimit(HeadAxisID) override
    {
        return 0;
    }
    bool doZCollHoming() override
    {
        return 0;
    }
    bool getLEDEnable(LEDPanelNo) override
    {
        return false;
    }
    bool setLEDEnable(LEDPanelNo, bool) override
    {
        return false;
    }
    bool reloadFiberSwitchCalibration(void) override
    {
        return false;
    }
    bool weldForScannerCalibration(const std::vector<geo2d::DPoint>& points, double laserPowerInPct, double durationInMs, double jumpSpeedInMmPerSec) override
    {
        return false;
    }
    bool doZCollDrivingRelative(int) override
    {
        return false;
    }
};

}
}
