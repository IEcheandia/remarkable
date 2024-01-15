/**
 *   @file
 *   @copyright   Precitec Vision GmbH & Co. KG
 *   @author      Alexander Egger (EA)
 *   @date        2021
 *   @brief       Controls Maxon EPOS4 axis controller
 */

#pragma once

#include <iostream>
#include <string>
#include <atomic>
#include <semaphore.h>

#include "Poco/Mutex.h"
#include "Poco/Event.h"

#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"

#include "event/viWeldHeadSubscribe.h"
#include "event/ethercatInputs.h"
#include "VIDefs.h"
#include "common/triggerInterval.h"

#include "event/ethercatOutputs.proxy.h"
#include "event/sensor.proxy.h"

#include "viWeldHead/VICallbackBase.h"

#include "viWeldHead/StateMachineInterface.h"

#define CTRL_NEW_SETPOINT       0x0010
#define CTRL_START_HOMING       0x0010
#define CTRL_CHANGE_IMMEDIATELY 0x0020
#define CTRL_RELATIVE_MOVE      0x0040

#define STATUS_FAULT            0x0008
#define STATUS_TARGET_REACHED   0x0400
#define STATUS_INTERNAL_LIMIT   0x0800
#define STATUS_ACKN_SETPOINT    0x1000
#define STATUS_HOMING_ATTAINED  0x1000
#define STATUS_FOLLOWING_ERROR  0x2000
#define STATUS_HOMING_ERROR     0x2000

struct STATE_DATAV2
{
    int actVelocity;
    short actTorque;

    short state;
    unsigned int m_oAxisStatusBits;

    bool m_oSoftLimitsActive;
    int m_oSoftLowerLimit;
    int m_oSoftUpperLimit;

    int m_oAxisVelocity;
    int m_oAxisAcceleration;
    int m_oAxisDeceleration;
    bool m_oAxisHomingDirPos;
    int m_oAxisHomingMode;
    int m_oAxisHomeOffset;
    bool m_oMountingRightTop;
};

namespace precitec
{

namespace ethercat
{

enum DriveStateType {eNotReadyToSwitchOn, eSwitchOnDisabled, eReadyToSwitchOn, eSwitchedOn, eOperationEnabled, eQuickStopActive, eFaultReactionActive, eFault};
enum DriveOperationModeType {eNoValidMode, eProfilePositionMode, eProfileVelocityMode, eHomingMode, eCyclicSyncPositionMode, eCyclicSyncVelocityMode, eCyclicSyncTorqueMode};
 
class StateMachineV2;

struct DataToAxisCyclicTaskThread
{
    StateMachineV2* m_pStateMachineV2;
};

class StateMachineV2 : public StateMachineInterface
{
public:
    StateMachineV2(HeadAxisID p_oStateMachineInstance,
                bool p_oSoftLimitsActive, int p_oSoftLowerLimit, int p_oSoftUpperLimit,
                bool p_oHomingDirPos, bool p_oMountingRightTop,
                int p_oProductCode, int p_oVendorID ,int p_oInstance, bool p_oIsHomeable,
                VICallbackBase *cbIsReady, VICallbackBase *cbValueReached, VICallbackBase *cbHeadError,
                TSensor<AbstractInterface>& p_rSensorProxy,
                TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy
                );

    virtual ~StateMachineV2();

    void SetHeadMode(MotionMode mode, bool bHome, bool p_oGoToSoftLowerLimit);
    void SetHeadValue(int value, MotionMode mode);
    void RequestHeadInfo(HeadInfo &info);

    void SetSoftLimitsActive(bool p_oState);
    bool GetSoftLimitsActive(void);
    void SetLowerLimit(int p_oValue);
    int GetLowerLimit(void);
    void SetUpperLimit(int p_oValue);
    int GetUpperLimit(void);

    void SetAxisVelocity(int p_oValue);
    int GetAxisVelocity(void);
    void SetAxisAcceleration(int p_oValue);
    int GetAxisAcceleration(void);
    void SetAxisDeceleration(int p_oValue);
    int GetAxisDeceleration(void);
    void SetAxisHomingDirPos(bool p_oState);
    bool GetAxisHomingDirPos(void);
    void SetAxisHomeOffset(int p_oValue);
    int GetAxisHomeOffset(void);
    void SetMountingRightTop(bool p_oState);
    bool GetMountingRightTop(void);

    //MsgInterface
    bool setHeadPosMsg(int value);
    bool setHeadModeMsg(MotionMode p_oMode, bool p_oHome);
    int getHeadPositionMsg(void);
    int getLowerLimitMsg(void);
    int getUpperLimitMsg(void);

    /**
    * Passt das Senden der Sensordaten an die Bildfrequenz an.
    * @param context TriggerContext
    * @param interval Interval
    */
    void burst(TriggerContext const& context, TriggerInterval const& interval);
    void cancel(int id);

    void startAutomaticmode(void); // Uebergibt Zyklusstart an StateMachine
    void stopAutomaticmode(void);  // Uebergibt Zyklusende an StateMachine

    void ecatAxisIn(EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput) override; // Interface EthercatInputs
    void IncomingMotionData(void);

    void setDebugInfo_AxisController(bool p_oDebugInfo_AxisController) { m_oDebugInfo_AxisController = p_oDebugInfo_AxisController; };

    void TestFunctionAxis_1(void);
    void TestFunctionAxis_2(void);
    void TestFunctionAxis_3(void);
    void TestFunctionAxis_4(void);
    void TestFunctionAxis_5(void);

    // new public member ///////////////////////////////

    void AxisCyclicTaskFunction(void);

private:
    void ConvertStatusWord(unsigned short *statusWord);
    void cleanBuffer();

    static const unsigned short STATE_POSITION_ACTIVE = 40;
    static const unsigned short STATE_OFFLINE_ACTIVE = 50;

    HeadAxisID m_oStateMachineInstance;
    char m_oAxisLogCharacter[5];

    pthread_mutex_t m_mutex;
    STATE_DATAV2 m_oStateData;

    pthread_t m_oAxisCyclicTaskThread_ID;
    struct DataToAxisCyclicTaskThread m_oDataToAxisCyclicTaskThread;
    void StartAxisCyclicTaskThread(void);

    SLAVE_PROXY_INFORMATION m_oAxisInputProxyInfo;
    SLAVE_PROXY_INFORMATION m_oAxisOutputProxyInfo;

    VICallbackBase *m_cbIsReady;
    VICallbackBase *m_cbValueReached;
    VICallbackBase *m_cbHeadError;

    TSensor<AbstractInterface> &m_rSensorProxy;
    TEthercatOutputs<EventProxy>& m_rEthercatOutputsProxy;

    TriggerContext m_context;
    Poco::FastMutex m_fastMutex;
    Poco::Event m_oSetHeadPosEvent;
    Poco::Event m_oSetHeadModeEvent;

    TriggerInterval m_interval;
    int m_imageNrMotionData;
    TSmartArrayPtr<int>::ShArrayPtr* m_pValuesMotionData;

    bool m_oCycleIsOn;

    // new private member ///////////////////////////////

    DriveStateType StateOfTheDrive(void);
    DriveOperationModeType OperationModeOfTheDrive(void);
    void DriveCommandShutdown(void);
    void DriveCommandSwitchOn(void);
    void DriveCommandEnableOperation(void);
    void DriveCommandDisableVoltage(void);
    void DriveCommandQuickStop(void);
    void DriveCommandDisableOperation(void);
    void DriveCommandFaultReset(void);
    void SetOperationMode(DriveOperationModeType p_oNewMode);

    void SendDataToAxis(void);

    void GotoOpEnabledProcedure(void);
    void GotoOpDisabledProcedure(void);
    void DriveToPositionProcedure(void);
    void DoHomingProcedure(void);

    std::atomic<uint16_t> m_oRequestedControlWord;
    std::atomic<int8_t> m_oRequestedModesOfOperation;
    std::atomic<int32_t> m_oRequestedTargetPosition;

    std::atomic<uint16_t> m_oStatusWord;
    std::atomic<int8_t> m_oModesOfOpDisplay;
    std::atomic<uint8_t> m_oErrorReg;
    std::atomic<uint16_t> m_oErrorCode;
    std::atomic<int32_t> m_oPositionUserUnit;

    std::atomic<bool> m_oRequestOpEnabled;
    std::atomic<bool> m_oInitDriveIsActive;
    std::atomic<bool> m_oRequestOpDisabled;

    std::atomic<bool> m_oDriveToPosition;
    std::atomic<bool> m_oMovingIsActive;
    std::atomic<bool> m_oDriveToPositionSuccessful;
    std::atomic<int32_t> m_oDriveToPositionState;

    std::atomic<bool> m_oDoHoming;
    std::atomic<bool> m_oHomingIsActive;
    std::atomic<bool> m_oDoHomingSuccessful;
    std::atomic<bool> m_oDriveAfterHomingFlag;
    std::atomic<int32_t> m_oDriveAfterHomingPosition;

    struct timespec m_oDriveTimeStart;
    struct timespec m_oDriveTimeStop;

    bool m_oDebugInfo_AxisController;
};

} // namespace ethercat
} // namespace precitec

