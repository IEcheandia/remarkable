/**
 *  @file
 *  @copyright  Precitec Vision GmbH & Co. KG
 *  @author     Alexander Egger (EA)
 *  @date       2021
 *  @brief      Interface class for axis state machines
 */

#pragma once

#include "event/viWeldHeadPublish.h"
#include "event/ethercatDefines.h"
#include "common/triggerInterval.h"

namespace precitec
{

namespace ethercat
{

class StateMachineInterface
{
public:
    StateMachineInterface() {}

    virtual ~StateMachineInterface() {}

    virtual void SetHeadMode(MotionMode mode, bool bHome, bool p_oGoToSoftLowerLimit) = 0;
    virtual void SetHeadValue(int value, MotionMode mode) = 0;
    virtual void RequestHeadInfo(HeadInfo &info) = 0;

    virtual void SetSoftLimitsActive(bool p_oState) = 0;
    virtual bool GetSoftLimitsActive(void) = 0;
    virtual void SetLowerLimit(int p_oValue) = 0;
    virtual int GetLowerLimit(void) = 0;
    virtual void SetUpperLimit(int p_oValue) = 0;
    virtual int GetUpperLimit(void) = 0;

    virtual void SetAxisVelocity(int p_oValue) = 0;
    virtual int GetAxisVelocity(void) = 0;
    virtual void SetAxisAcceleration(int p_oValue) = 0;
    virtual int GetAxisAcceleration(void) = 0;
    virtual void SetAxisDeceleration(int p_oValue) = 0;
    virtual int GetAxisDeceleration(void) = 0;
    virtual void SetAxisHomingDirPos(bool p_oState) = 0;
    virtual bool GetAxisHomingDirPos(void) = 0;
    virtual void SetAxisHomeOffset(int p_oValue) = 0;
    virtual int GetAxisHomeOffset(void) = 0;
    virtual void SetMountingRightTop(bool p_oState) = 0;
    virtual bool GetMountingRightTop(void) = 0;

    //MsgInterface
    virtual bool setHeadPosMsg(int value) = 0;
    virtual bool setHeadModeMsg(MotionMode p_oMode, bool p_oHome) = 0;
    virtual int getHeadPositionMsg(void) = 0;
    virtual int getLowerLimitMsg(void) = 0;
    virtual int getUpperLimitMsg(void) = 0;

    virtual void burst(TriggerContext const& context, TriggerInterval const& interval) = 0;
    virtual void cancel(int id) = 0;

    virtual void startAutomaticmode(void) = 0; // Uebergibt Zyklusstart an StateMachine
    virtual void stopAutomaticmode(void) = 0;  // Uebergibt Zyklusende an StateMachine

    virtual void ecatAxisIn(EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput) = 0; // Interface EthercatInputs
    virtual void IncomingMotionData(void) = 0;

    virtual void setDebugInfo_AxisController(bool p_oDebugInfo_AxisController) = 0;

    virtual void TestFunctionAxis_1(void) = 0;
    virtual void TestFunctionAxis_2(void) = 0;
    virtual void TestFunctionAxis_3(void) = 0;
    virtual void TestFunctionAxis_4(void) = 0;
    virtual void TestFunctionAxis_5(void) = 0;
};

} // namespace ethercat
} // namespace precitec

