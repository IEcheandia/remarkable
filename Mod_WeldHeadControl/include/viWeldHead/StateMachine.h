/*
 * StateMachine.h
 *
 *  Created on: 08.04.2010
 *      Author: f.agrawal
 */

#ifndef STATEMACHINE_H_
#define STATEMACHINE_H_

#include <iostream>
#include <fstream>
#include <string>
#include <atomic>

#include "viWeldHead/VICallbackBase.h"

#include "event/ethercatInputs.h"
#include "event/ethercatOutputs.proxy.h"

#include "VIDefs.h"

#include <Tools.h>
#include <DefsHW.h>
#include <semaphore.h>

#include "message/module.h"
// die Basisklasse
#include "module/baseModule.h"
#include "event/viWeldHeadSubscribe.h"
//#include "ParallelPort.h"

#include "event/sensor.proxy.h"

#include "Poco/Mutex.h"
#include "Poco/Event.h"
#include "Poco/DOM/DOMParser.h"
#include "Poco/DOM/DOMWriter.h"
#include "Poco/DOM/Document.h"
#include "Poco/DOM/Element.h"
#include "Poco/DOM/Text.h"
#include "Poco/DOM/NodeList.h"
#include "Poco/DOM/AutoPtr.h"
#include "Poco/XML/XMLWriter.h"
#include "Poco/SAX/InputSource.h"

#include "common/triggerContext.h"
#include "common/triggerInterval.h"

#include "viWeldHead/StateMachineInterface.h"

//#define MODE_POSITION 1
//#define MODE_VELOCITY 3
//#define MODE_HOME 6

#define READY_TO_SWITCH_ON	0x0001
#define SWITCHED_ON			0x0002
#define OPERATION_ENABLED	0x0004
#define FAULT               0x0008
#define SPEED				0x0100 // is moving
#define TARGET_REACHED		0x0400
#define HOMING_ATTAINED		0x1000
#define FOLLOWING_ERROR     0x2000
#define TRAJECTORY_ABORTED	0x8000

struct STATE_DATA {
	unsigned short statusWord;
	unsigned char modesOfOpDisplay;
	unsigned char errorReg;
	unsigned int manuStatusReg;
	int positionUserUnit;
	int actVelocity;
	short actTorque;

	unsigned short writtenModesOfOperation;
	unsigned short writtenControlWord;
	int writtenTargetPos;
	int writtenTargetVelocity;

	unsigned short requestedMode;
	bool m_oIsHomeable;
	int requestTargetPosition;
	int requestedTargetVelocity;
	//bool requestSetHeadReady;
	short initMode;
	short state;
	bool m_oSoftLimitsActive;
	int m_oSoftLowerLimit;
	int m_oSoftUpperLimit;

	unsigned int m_oAxisStatusBits;

	bool requestHome;
	bool flag_blockTargetReached;

	int m_oAxisVelocity;
	int m_oAxisAcceleration;
	int m_oAxisDeceleration;
	bool m_oAxisHomingDirPos;
	int m_oAxisHomingMode;
	int m_oAxisHomeOffset;
	bool m_oMountingRightTop;
};

struct THREAD_PARAM{
	STATE_DATA* m_pStateData;
	sem_t* m_pSem;
	TEthercatOutputs<EventProxy> *m_pEthercatOutputsProxy;
    SLAVE_PROXY_INFORMATION *m_pAxisOutputProxyInfo;
	VICallbackBase *m_pCbIsReady;
	VICallbackBase *m_pCbValueReached;
	bool* m_pIsRunning;
	Poco::Event* m_pSetHeadPosEvent;
	Poco::Event* m_pSetHeadModeEvent;
	HeadAxisID *m_pStateMachineInstance;
	char *m_pAxisLogCharacter;
};


namespace precitec
{

using Poco::FastMutex;
using Poco::Event;
using namespace Poco::XML;

namespace ethercat
{

class StateMachine : public StateMachineInterface
{
public:
	StateMachine(HeadAxisID p_oStateMachineInstance,
				 bool p_oSoftLimitsActive, int p_oSoftLowerLimit, int p_oSoftUpperLimit,
				 bool p_oHomingDirPos, bool p_oMountingRightTop,
				 int p_oProductCode, int p_oVendorID ,int p_oInstance, bool p_oIsHomeable,
				 VICallbackBase *cbIsReady, VICallbackBase *cbValueReached, VICallbackBase *cbHeadError,
				 TSensor<AbstractInterface>& p_rSensorProxy,
				 TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy
                );

	virtual ~StateMachine();

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

    void TestFunctionAxis_1(void) {};
    void TestFunctionAxis_2(void) {};
    void TestFunctionAxis_3(void) {};
    void TestFunctionAxis_4(void) {};
    void TestFunctionAxis_5(void) {};

private:
	void ConvertStatusWord(unsigned short *statusWord);
	void cleanBuffer();
	static void* ThreadStateMachine(void* pvThreadParam);
    static void SendMotion(STATE_DATA* p_pStateData, TEthercatOutputs<EventProxy> *p_pEthercatOutputsProxy, SLAVE_PROXY_INFORMATION *p_pAxisOutputProxyInfo);

    static const unsigned short STATE_VELOCITY_ACTIVE = 30;
    static const unsigned short STATE_POSITION_ACTIVE = 40;
    static const unsigned short STATE_OFFLINE_ACTIVE = 50;
    static const unsigned short STATE_TRANSITION= 99;

	HeadAxisID m_oStateMachineInstance;
	char m_oAxisLogCharacter[5];

	std::string m_sRegOn;
	pthread_mutex_t m_mutex;
	sem_t m_sem;
	pthread_t m_thread;
	THREAD_PARAM m_oThreadParam;
	STATE_DATA m_oStateData;

	bool m_oIsRunning;

	SLAVE_PROXY_INFORMATION m_oAxisInputProxyInfo;
    std::atomic<int> m_oPositionUserUnit;
	SLAVE_PROXY_INFORMATION m_oAxisOutputProxyInfo;

	VICallbackBase *m_cbIsReady;
	VICallbackBase *m_cbValueReached;
	VICallbackBase *m_cbHeadError;

	VICallbackBase *m_cbSetHeadReady;
	VICallbackBase *m_cbSetValue;

	TSensor<AbstractInterface> &m_rSensorProxy;

	TEthercatOutputs<EventProxy>& m_rEthercatOutputsProxy;

	TriggerContext m_context;
	FastMutex m_fastMutex;
	Event m_oSetHeadPosEvent;
	Event m_oSetHeadModeEvent;

	TriggerInterval m_interval;
	int m_imageNrMotionData;
	TSmartArrayPtr<int>::ShArrayPtr* m_pValuesMotionData;

	bool m_oCycleIsOn;

    bool m_oDebugInfo_AxisController;
};

} // namespace ethercat
} // namespace precitec

#endif /* STATEMACHINE_H_ */

