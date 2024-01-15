/*
 * StateMachine.cpp
 *
 *  Created on: 08.04.2010
 *      Author: f.agrawal
 */

#include "viWeldHead/StateMachine.h"
#include "module/moduleLogger.h"

#include "viWeldHead/WeldHeadDefaults.h"

#include <sys/prctl.h>

#define DEBUG_STATEMACHINE 0

namespace precitec
{

using Poco::FastMutex;
using Poco::Event;
using namespace Poco::XML;

namespace ethercat
{

StateMachine::StateMachine(HeadAxisID p_oStateMachineInstance,
						   bool p_oSoftLimitsActive, int p_oSoftLowerLimit, int p_oSoftUpperLimit,
						   bool p_oHomingDirPos, bool p_oMountingRightTop,
						   int p_oProductCode, int p_oVendorID ,int p_oInstance, bool p_oIsHomeable,
						   VICallbackBase *cbIsReady, VICallbackBase *cbValueReached, VICallbackBase *cbHeadError,
						   TSensor<AbstractInterface>& p_rSensorProxy,
						   TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy
                          ):
		m_oStateMachineInstance(p_oStateMachineInstance),
		m_oPositionUserUnit(0),
		m_rSensorProxy (p_rSensorProxy),
		m_rEthercatOutputsProxy(p_rEthercatOutputsProxy),
		m_interval(0,0),
		m_imageNrMotionData(0),
		m_pValuesMotionData(NULL),
		m_oCycleIsOn(false),
        m_oDebugInfo_AxisController(false)
{
    wmLog(eDebug, "Start of StateMachine for Accelnet Controller\n");
	pthread_mutex_init(&m_mutex, NULL);
	sem_init(&m_sem, 0, 0);

	if (m_oStateMachineInstance == interface::eAxisX)
	    strcpy(m_oAxisLogCharacter, "X");
	else if (m_oStateMachineInstance == interface::eAxisY)
	    strcpy(m_oAxisLogCharacter, "Y");
	else if (m_oStateMachineInstance == interface::eAxisZ)
	    strcpy(m_oAxisLogCharacter, "Z");
	else
	    strcpy(m_oAxisLogCharacter, "na");

    m_oAxisInputProxyInfo.nSlaveType = 0;
    m_oAxisInputProxyInfo.nProductCode = p_oProductCode;
    m_oAxisInputProxyInfo.nVendorID = p_oVendorID;
    m_oAxisInputProxyInfo.nInstance = p_oInstance;
    m_oAxisInputProxyInfo.nStartBit = 0;
	m_oAxisInputProxyInfo.nTriggerLevel = 0;
    m_oAxisInputProxyInfo.nLength = 0;
    if (m_oAxisInputProxyInfo.nProductCode == PRODUCTCODE_ACCELNET)
    {
        m_oAxisInputProxyInfo.m_oProductIndex = eProductIndex_ACCELNET;
    }
    else if (m_oAxisInputProxyInfo.nProductCode == PRODUCTCODE_EPOS4)
    {
        m_oAxisInputProxyInfo.m_oProductIndex = eProductIndex_EPOS4;
    }
    else
    {
        // falscher ProductCode fuer Axis Input
    }
    m_oAxisInputProxyInfo.m_oInstance = static_cast<EcatInstance>(m_oAxisInputProxyInfo.nInstance);
    m_oAxisInputProxyInfo.m_oChannel = eChannel1;
    m_oAxisInputProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "Axis Input:\n");
	wmLog(eDebug, "m_oActive:   0x%x\n", m_oAxisInputProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", m_oAxisInputProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", m_oAxisInputProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", m_oAxisInputProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", m_oAxisInputProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", m_oAxisInputProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", m_oAxisInputProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oAxisInputProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", m_oAxisInputProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", m_oAxisInputProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");

    m_oAxisOutputProxyInfo.nSlaveType = 0;
    m_oAxisOutputProxyInfo.nProductCode = p_oProductCode;
    m_oAxisOutputProxyInfo.nVendorID = p_oVendorID;
    m_oAxisOutputProxyInfo.nInstance = p_oInstance;
    m_oAxisOutputProxyInfo.nStartBit = 0;
	m_oAxisOutputProxyInfo.nTriggerLevel = 0;
    m_oAxisOutputProxyInfo.nLength = 0;
    if (m_oAxisOutputProxyInfo.nProductCode == PRODUCTCODE_ACCELNET)
    {
        m_oAxisOutputProxyInfo.m_oProductIndex = eProductIndex_ACCELNET;
    }
    else if (m_oAxisOutputProxyInfo.nProductCode == PRODUCTCODE_EPOS4)
    {
        m_oAxisOutputProxyInfo.m_oProductIndex = eProductIndex_EPOS4;
    }
    else
    {
        // falscher ProductCode fuer Axis Input
    }
    m_oAxisOutputProxyInfo.m_oInstance = static_cast<EcatInstance>(m_oAxisOutputProxyInfo.nInstance);
    m_oAxisOutputProxyInfo.m_oChannel = eChannel1;
    m_oAxisOutputProxyInfo.m_oActive = true;

	wmLog(eDebug, "++++++++++++++++++++++++++++++++++++++++++++++++++\n");
	wmLog(eDebug, "Axis Output:\n");
	wmLog(eDebug, "m_oActive:   0x%x\n", m_oAxisOutputProxyInfo.m_oActive);
	wmLog(eDebug, "VendorID:    0x%x\n", m_oAxisOutputProxyInfo.nVendorID);
	wmLog(eDebug, "ProductCode: 0x%x\n", m_oAxisOutputProxyInfo.nProductCode);
	wmLog(eDebug, "Instance:    0x%x\n", m_oAxisOutputProxyInfo.nInstance);
	wmLog(eDebug, "SlaveType:   0x%x\n", m_oAxisOutputProxyInfo.nSlaveType);
	wmLog(eDebug, "StartBit:    0x%x\n", m_oAxisOutputProxyInfo.nStartBit);
	wmLog(eDebug, "Length:      0x%x\n", m_oAxisOutputProxyInfo.nLength);
	wmLog(eDebug, "m_oProductIndex: 0x%x\n", m_oAxisOutputProxyInfo.m_oProductIndex);
	wmLog(eDebug, "m_oInstance: 0x%x\n", m_oAxisOutputProxyInfo.m_oInstance);
	wmLog(eDebug, "m_oChannel:  0x%x\n", m_oAxisOutputProxyInfo.m_oChannel);
	wmLog(eDebug, "--------------------------------------------------\n");

	m_cbIsReady = cbIsReady;
	m_cbValueReached = cbValueReached;
	m_cbHeadError = cbHeadError;

	memset(&m_oStateData,0,sizeof(m_oStateData));
	m_oStateData.m_oIsHomeable = p_oIsHomeable;
	m_oStateData.m_oSoftLimitsActive = p_oSoftLimitsActive;
	m_oStateData.m_oSoftLowerLimit = p_oSoftLowerLimit;
	m_oStateData.m_oSoftUpperLimit = p_oSoftUpperLimit;
    if (p_oProductCode == PRODUCTCODE_ACCELNET)
    {
        // AxisVelocity:
        //   540000 -> ist in ACCELNET vorporgrammiert
        //   1700000 -> ist default velocity
        //   1900000 -> ist ok
        //   2100000 -> ist nok
        m_oStateData.m_oAxisVelocity = 1700000;
        // AxisAcceleration, AxisDeceleration
        //   bis aus Weiteres werden Acceleration und Deceleration vom WeldingHeadControl-Objekt auf ident. Werte gesetzt
        //   d.h. Beschleunigungs- und Bremsrampen haben identische Steigungen
        //   Das StateMachine-Objekt ist jedoch vorbereitet fuer unterschiedliche Steigungen
        //   107000 -> ist in ACCELNET vorprogrammiert
        //   600000 -> ist default Steigung beim Beschleunigen und Bremsen
        //   groessere Beschleunigungen fuehren wieder zu laengeren Fahrzeiten !
        m_oStateData.m_oAxisAcceleration = 600000;
        m_oStateData.m_oAxisDeceleration = 600000;
    }
    else if (p_oProductCode == PRODUCTCODE_EPOS4)
    {
        // AxisVelocity:
        //   540000 -> ist in EPOS4 vorporgrammiert
        //   1700000 -> ist default velocity
        //   1900000 -> ist ok
        //   2100000 -> ist nok
//        m_oStateData.m_oAxisVelocity = 1700000;
        m_oStateData.m_oAxisVelocity = 5500; // rev/min rpm
        // AxisAcceleration, AxisDeceleration
        //   bis aus Weiteres werden Acceleration und Deceleration vom WeldingHeadControl-Objekt auf ident. Werte gesetzt
        //   d.h. Beschleunigungs- und Bremsrampen haben identische Steigungen
        //   Das StateMachine-Objekt ist jedoch vorbereitet fuer unterschiedliche Steigungen
        //   107000 -> ist in EPOS4 vorprogrammiert
        //   600000 -> ist default Steigung beim Beschleunigen und Bremsen
        //   groessere Beschleunigungen fuehren wieder zu laengeren Fahrzeiten !
//        m_oStateData.m_oAxisAcceleration = 600000;
//        m_oStateData.m_oAxisDeceleration = 600000;
        m_oStateData.m_oAxisAcceleration = 85000; // (rev/min)/sec rpm/s
        m_oStateData.m_oAxisDeceleration = 85000; // (rev/min)/sec rpm/s
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.XMLFileRead", "Wrong type of axis controller\n");
    }
	// HomingDirPos, HomingMode:
	//   Bei der Achse kommt nur Homing auf einen der beiden Endschalter in Frage
	//   Einen Referenzsensor gibt es nicht !
	//   Der Encoder-Nullimpuls ist nicht angeschlossen !
	//   HomingDirPos true  -> HomingMode 18 -> Homing auf den positiven Endschalter
	//   HomingDirPos false -> HomingMode 17 -> Homing auf den negativen Endschalter
	m_oStateData.m_oAxisHomingDirPos = p_oHomingDirPos;
	if (m_oStateData.m_oAxisHomingDirPos) // Homing Richtung positiv
	{
		m_oStateData.m_oAxisHomingMode = 18; // Homing Richtung positiv
		m_oStateData.m_oAxisHomeOffset = -100; // 0.1mm zurueck bei Homing Richtung positiv
	}
	else // Homing Richtung negativ
	{
		m_oStateData.m_oAxisHomingMode = 17; // Homing Richtung negativ
		m_oStateData.m_oAxisHomeOffset = 100; // 0.1mm vorwaerts bei Homing Richtung negativ
	}
	// MountingRightTop:
	//   Ist der Kabelanschluss rechts bzw. open -> true
	//   Ist der Kabelanschluss links bzw. unten -> false;
	m_oStateData.m_oMountingRightTop = p_oMountingRightTop;

	m_oStateData.m_oAxisStatusBits = 0x00;
	if (m_oStateData.m_oIsHomeable) // Achse muss referenziert weden
	{
		m_oStateData.m_oAxisStatusBits |= eHomePosNotOK;
	}

	m_oStateData.state = STATE_OFFLINE_ACTIVE;

	m_oIsRunning = true;
	m_oThreadParam.m_pStateData = &m_oStateData;
	m_oThreadParam.m_pSem = &m_sem;
	m_oThreadParam.m_pIsRunning = &m_oIsRunning;
	m_oThreadParam.m_pCbIsReady = m_cbIsReady;
	m_oThreadParam.m_pCbValueReached = m_cbValueReached;
	m_oThreadParam.m_pSetHeadPosEvent = &m_oSetHeadPosEvent;
	m_oThreadParam.m_pSetHeadModeEvent = &m_oSetHeadModeEvent;
	m_oThreadParam.m_pStateMachineInstance = &m_oStateMachineInstance;
	m_oThreadParam.m_pAxisLogCharacter = m_oAxisLogCharacter;
	m_oThreadParam.m_pEthercatOutputsProxy = &m_rEthercatOutputsProxy;
	m_oThreadParam.m_pAxisOutputProxyInfo = &m_oAxisOutputProxyInfo;

	//start state machine
	pthread_create(&m_thread, NULL, &ThreadStateMachine, &m_oThreadParam);

	if (m_oStateMachineInstance == interface::eAxisX)
	{
		WeldHeadDefaults::instance().setBool("AxisX_SoftLimitsActive", m_oStateData.m_oSoftLimitsActive);
		WeldHeadDefaults::instance().setInt("AxisX_SoftLowerLimit", m_oStateData.m_oSoftLowerLimit);
		WeldHeadDefaults::instance().setInt("AxisX_SoftUpperLimit", m_oStateData.m_oSoftUpperLimit);
	}
	else if (m_oStateMachineInstance == interface::eAxisY)
	{
		WeldHeadDefaults::instance().setBool("AxisY_SoftLimitsActive", m_oStateData.m_oSoftLimitsActive);
		WeldHeadDefaults::instance().setInt("AxisY_SoftLowerLimit", m_oStateData.m_oSoftLowerLimit);
		WeldHeadDefaults::instance().setInt("AxisY_SoftUpperLimit", m_oStateData.m_oSoftUpperLimit);
	}
	else if (m_oStateMachineInstance == interface::eAxisZ)
	{
		WeldHeadDefaults::instance().setBool("AxisZ_SoftLimitsActive", m_oStateData.m_oSoftLimitsActive);
		WeldHeadDefaults::instance().setInt("AxisZ_SoftLowerLimit", m_oStateData.m_oSoftLowerLimit);
		WeldHeadDefaults::instance().setInt("AxisZ_SoftUpperLimit", m_oStateData.m_oSoftUpperLimit);
	}
}

StateMachine::~StateMachine()
{
	delete m_cbIsReady;
	delete m_cbSetHeadReady;
	delete m_cbSetValue;
	delete m_cbValueReached;

	pthread_mutex_destroy(&m_mutex);
	sem_destroy(&m_sem);

	m_oIsRunning = false;

	pthread_join(m_thread, NULL); //double join, safety check!
}

void StateMachine::RequestHeadInfo(HeadInfo &info)
{
	pthread_mutex_lock(&m_mutex);

	switch (m_oStateData.state)
	{
		case STATE_OFFLINE_ACTIVE:{
			info.status = Offline;
			break;
		}
		case Home:{
			info.status = Home;
			break;
		}
		case STATE_POSITION_ACTIVE:{
			info.status = Position;
			break;
		}
		case STATE_VELOCITY_ACTIVE:{
			info.status = Velocity;
			break;
		}
		case Position_Relative:{
			info.status = Position_Relative;
			break;
		}
		case Position_Absolute:{
			info.status = Position_Absolute;
			break;
		}
		default:{
			info.status = Pending;
			break;
		}
	}

	info.statusWord = m_oStateData.statusWord;
	info.modeOfOperation = m_oStateData.modesOfOpDisplay;
	info.errorCode = m_oStateData.errorReg;
	info.positionUserUnit = m_oStateData.positionUserUnit;
	info.actVelocity = m_oStateData.actVelocity;
	info.actTorque = m_oStateData.actTorque;
	info.m_oHomingDirPos = m_oStateData.m_oAxisHomingDirPos;
	info.m_oSoftLimitsActive = m_oStateData.m_oSoftLimitsActive;
	info.m_oSoftLowerLimit = m_oStateData.m_oSoftLowerLimit;
	info.m_oSoftUpperLimit = m_oStateData.m_oSoftUpperLimit;

	info.m_oAxisStatusBits = m_oStateData.m_oAxisStatusBits;

	pthread_mutex_unlock(&m_mutex);
}

void StateMachine::SetHeadMode(MotionMode mode, bool bHome, bool p_oGoToSoftLowerLimit) {

    if (m_oDebugInfo_AxisController)
    {
        char oString1[81];
        char oString2[81];
        sprintf(oString1, "SM V1 %s: SetHeadMode: mode:%d, bHome:%d, ", m_oAxisLogCharacter, mode, bHome);
        sprintf(oString2, "p_oGoToSoftLowerLimit:%d", p_oGoToSoftLowerLimit);
        wmLog(eDebug, "%s%s\n", oString1, oString2);
    }
	pthread_mutex_lock(&m_mutex);
	m_oStateData.state = STATE_TRANSITION;
	m_oStateData.requestedMode = mode;
	m_oStateData.initMode = 0;
	m_oStateData.requestHome = bHome;
	if(bHome ){
		m_oStateData.m_oAxisStatusBits |= eHomePosNotOK;

		m_oStateData.writtenTargetPos = 0;
		m_oStateData.requestTargetPosition = 0;
		if(p_oGoToSoftLowerLimit){
			if (m_oStateData.m_oSoftLimitsActive) // Ueberwachung der Software-Endschalter ist aktiv
			{
				if (m_oStateData.m_oAxisHomingDirPos)
				{
					if ((m_oStateData.m_oSoftLowerLimit > 0) ||
						(m_oStateData.m_oSoftUpperLimit > 0))
					{
						(*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,m_oStateData.requestTargetPosition);
						m_oStateData.m_oAxisStatusBits |= eOutOfRange;
						wmLogTr(eError, "QnxMsg.VI.AxisNotInRange1", "Axis %s: The actual position is out of the software position limits\n", m_oAxisLogCharacter);
						wmLogTr(eError, "QnxMsg.VI.AxisNotInRange2", "Axis %s: Please correct software position limits or do homing\n", m_oAxisLogCharacter);
					}
					else
					{
						m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
						m_oStateData.requestTargetPosition = m_oStateData.m_oSoftUpperLimit;
					}
				}
				else
				{
					if ((m_oStateData.m_oSoftLowerLimit < 0) ||
						(m_oStateData.m_oSoftUpperLimit < 0))
					{
						(*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,m_oStateData.requestTargetPosition);
						m_oStateData.m_oAxisStatusBits |= eOutOfRange;
						wmLogTr(eError, "QnxMsg.VI.AxisNotInRange1", "Axis %s: The actual position is out of the software position limits\n", m_oAxisLogCharacter);
						wmLogTr(eError, "QnxMsg.VI.AxisNotInRange2", "Axis %s: Please correct software position limits or do homing\n", m_oAxisLogCharacter);
					}
					else
					{
						m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
						m_oStateData.requestTargetPosition = m_oStateData.m_oSoftLowerLimit;
					}
				}
			}
			else
			{
				m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
			}
		}
	}

	pthread_mutex_unlock(&m_mutex);
	sem_post(&m_sem);
}
/*
 *	- Check if @mode == currentMode,
 *	- if relative positioning calc new absolute position
 *	- check if position is outer axisLength limits (set to max, and send error event)
 */

void StateMachine::SetHeadValue(int value, MotionMode mode) {

    if (m_oDebugInfo_AxisController)
    {
        char oString1[81];
        char oString2[81];
        sprintf(oString1, "SM V1 %s: SetHeadValue: value:%d, mode:%d, ", m_oAxisLogCharacter, value, mode);
        sprintf(oString2, "ModesOfOp:%d, State:%d", m_oStateData.modesOfOpDisplay, m_oStateData.state);
        wmLog(eDebug, "%s%s\n", oString1, oString2);
    }
	pthread_mutex_lock(&m_mutex);
	if ( m_oStateData.modesOfOpDisplay == Position && m_oStateData.state == STATE_POSITION_ACTIVE) {

		//check if position is in valid interval
		if (m_oStateData.m_oSoftLimitsActive) // Ueberwachung der Software-Endschalter ist aktiv
		{
			// liegt die aktuelle Position ausserhalb der Software-Endschalter ?
			if ((m_oStateData.positionUserUnit < (m_oStateData.m_oSoftLowerLimit - 2)) ||
				(m_oStateData.positionUserUnit > (m_oStateData.m_oSoftUpperLimit + 2)))
			{
				(*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,m_oStateData.requestTargetPosition);
				m_oStateData.m_oAxisStatusBits |= eOutOfRange;
		        wmLogTr(eError, "QnxMsg.VI.AxisNotInRange1", "Axis %s: The actual position is out of the software position limits\n", m_oAxisLogCharacter);
		        wmLogTr(eError, "QnxMsg.VI.AxisNotInRange2", "Axis %s: Please correct software position limits or do homing\n", m_oAxisLogCharacter);

		    	pthread_mutex_unlock(&m_mutex);
		    	sem_post(&m_sem);
		        return;
			}
			else
			{
				m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
			}
		}
		else
		{
			m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
		}

		if(mode == Position_Relative){
			m_oStateData.requestTargetPosition = m_oStateData.positionUserUnit + value;

		}
		else if ( mode == Position_Absolute){
			m_oStateData.requestTargetPosition = value;

			if (m_oStateData.requestTargetPosition == m_oStateData.writtenTargetPos)
			{
				if ((m_oStateData.statusWord & TARGET_REACHED) == TARGET_REACHED)
				{
					(*m_cbValueReached)(m_oStateMachineInstance, Position, value );
					m_oSetHeadPosEvent.set();
					m_oSetHeadModeEvent.set();
				}
				else
				{
					// Ausloesen eines Fahrbewegung simulieren
					m_oStateData.flag_blockTargetReached = false;
				}
			}
		}
		else{
			std::printf("StateMachine: HeadError(ErrorCode: %d, Value: %d)...\n",eErrorNotInRequestedMode,m_oStateData.modesOfOpDisplay);
			(*m_cbHeadError)(m_oStateMachineInstance, eErrorNotInRequestedMode,m_oStateData.modesOfOpDisplay);
		}

		//check limits
		if (m_oStateData.m_oSoftLimitsActive) // Ueberwachung der Software-Endschalter ist aktiv
		{
			// verletzt die gewuenschte Absolut-Position das untere Limit ?
			if (m_oStateData.requestTargetPosition < m_oStateData.m_oSoftLowerLimit)
			{
				// neue Absolut-Position begrenzen
				m_oStateData.requestTargetPosition = m_oStateData.m_oSoftLowerLimit;
				// Achsenfehler zurueckmelden (Ist das notwendig ? eher eine Art Warnung ausgeben ?)
				(*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,m_oStateData.requestTargetPosition);
				m_oStateData.m_oAxisStatusBits |= eSWLimitNeg;
		        wmLogTr(eError, "QnxMsg.VI.AxisSoftLowerLimit", "Axis %s: The new position is limited to the lower software position limit\n", m_oAxisLogCharacter);
			}
			else
			{
				m_oStateData.m_oAxisStatusBits &= ~eSWLimitNeg;
			}

			// verletzt die gewuenschte Absolut-Position das obere Limit ?
			if (m_oStateData.requestTargetPosition > m_oStateData.m_oSoftUpperLimit)
			{
				// neue Absolut-Position begrenzen
				m_oStateData.requestTargetPosition = m_oStateData.m_oSoftUpperLimit;
				// Achsenfehler zurueckmelden (Ist das notwendig ? eher eine Art Warnung ausgeben ?)
				(*m_cbHeadError)(m_oStateMachineInstance, eErrorUpperLimit,m_oStateData.requestTargetPosition);
				m_oStateData.m_oAxisStatusBits |= eSWLimitPos;
		        wmLogTr(eError, "QnxMsg.VI.AxisSoftUpperLimit", "Axis %s: The new position is limited to the upper software position limit\n", m_oAxisLogCharacter);
			}
			else
			{
				m_oStateData.m_oAxisStatusBits &= ~eSWLimitPos;
			}
		}
		else
		{
			m_oStateData.m_oAxisStatusBits &= ~eSWLimitNeg;
			m_oStateData.m_oAxisStatusBits &= ~eSWLimitPos;
		}

	} else if (m_oStateData.modesOfOpDisplay == Velocity && m_oStateData.state == STATE_VELOCITY_ACTIVE) {

		m_oStateData.requestedTargetVelocity = value;

	} else {
		std::printf("StateMachine: HeadError(ErrorCode: %d, Value: %d)...\n",eErrorNotInRequestedMode,m_oStateData.modesOfOpDisplay);
		(*m_cbHeadError)(m_oStateMachineInstance, eErrorNotInRequestedMode,m_oStateData.modesOfOpDisplay);
	}

	pthread_mutex_unlock(&m_mutex);
	sem_post(&m_sem);
}

void StateMachine::IncomingMotionData(void)
{
    FastMutex::ScopedLock lock(m_fastMutex);
    if(m_imageNrMotionData < int( m_interval.nbTriggers() ))
    {
        m_pValuesMotionData[0][0] = (int)m_oPositionUserUnit.load();
        m_context.setImageNumber(m_imageNrMotionData);
        if (m_oStateMachineInstance == interface::eAxisX)
        {
            m_rSensorProxy.data(eWeldHeadAxisXPos,m_context,image::Sample(m_pValuesMotionData[0], 1));
        }
        else if (m_oStateMachineInstance == interface::eAxisY)
        {
            m_rSensorProxy.data(eWeldHeadAxisYPos,m_context,image::Sample(m_pValuesMotionData[0], 1));
        }
        else if (m_oStateMachineInstance == interface::eAxisZ)
        {
            m_rSensorProxy.data(eWeldHeadAxisZPos,m_context,image::Sample(m_pValuesMotionData[0], 1));
        }
        ++m_imageNrMotionData;
    }
}

void StateMachine::ecatAxisIn(EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput) // Interface EthercatInputs
{
   if (m_oAxisInputProxyInfo.m_oActive)
   {
        if (m_oAxisInputProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oAxisInputProxyInfo.m_oInstance == instance)
            {
                pthread_mutex_lock(&m_mutex);

                auto statusWord = axisInput.statusWord;
                ConvertStatusWord(&statusWord);
#if DEBUG_STATEMACHINE
                if ((statusWord != m_oStateData.statusWord) || (axisInput.modesOfOpDisp != m_oStateData.modesOfOpDisplay) ||
                    (axisInput.errorReg != m_oStateData.errorReg) || (axisInput.manufacStatus != m_oStateData.manuStatusReg))
                {
                    struct timespec actTime;
                    if (clock_gettime(CLOCK_REALTIME, &actTime) == -1)
                    {
                        printf("Problem with clock_gettime\n");
                    }
                    // Differenz zur startTime berechnen
                    long zeit = (actTime.tv_sec) * 1000;
                    zeit += (actTime.tv_nsec / 1000000);

                    printf("Incoming: %ld: Stat: %04X, Op: %02X, Err: %02X, Digi: %08X, Pos: %d, Vel: %d, Tor: %d\n", zeit,
                            statusWord, axisInput.modesOfOpDisp, axisInput.errorReg, axisInput.manufacStatus,
                            axisInput.actualPosition, axisInput.actualVelocity, axisInput.actualTorque);
                }
#endif
                m_oStateData.statusWord = statusWord;
                m_oStateData.modesOfOpDisplay = axisInput.modesOfOpDisp;
                m_oStateData.errorReg = axisInput.errorReg;
                m_oStateData.manuStatusReg = axisInput.manufacStatus;
                switch (m_oAxisInputProxyInfo.m_oProductIndex)
                {
                    case eProductIndex_ACCELNET:
                        m_oStateData.positionUserUnit = static_cast<int>(static_cast<float>(axisInput.actualPosition) / 4.114);
                        break;
                    case eProductIndex_EPOS4:
                        m_oStateData.positionUserUnit = static_cast<int>(static_cast<float>(axisInput.actualPosition) / 4.114);
                        break;
                    default:
                        m_oStateData.positionUserUnit = axisInput.actualPosition;
                        break;
                }
                m_oStateData.actVelocity = axisInput.actualVelocity;
                m_oStateData.actTorque = axisInput.actualTorque;

                m_oPositionUserUnit.store(m_oStateData.positionUserUnit); // used for sensor interface (Incoming...-function)

                pthread_mutex_unlock(&m_mutex);
                sem_post(&m_sem);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void StateMachine::ConvertStatusWord(unsigned short *statusWord)
{
	//convert status word to Precitec_Motion_StatusWord
	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
		{
			bool bAbortedLastTrajectory = ((*statusWord) & 0x100) == 0x100;
			bool bMoving = ((*statusWord) & 0x4000) == 0x4000;

			//Bit14 = 0
			*statusWord = (*statusWord) & 0xBFFF;

			if (bMoving)
			{
				*statusWord = (*statusWord) | 0x100;
			}
			else
			{
				*statusWord = (*statusWord) & 0xFEFF;
			}

			if (bAbortedLastTrajectory)
			{
				*statusWord = (*statusWord) | 0x8000;
			}
			else
			{
				*statusWord = (*statusWord) & 0x7FFF;
			}
			break;
		}
		case eProductIndex_EPOS4:
		{
			//Bit15 = 0
			*statusWord = (*statusWord) & ~0x8000;
			break;
		}
		case eProductIndex_COMPAX:
		{
			//Toggle Bit4 (voltage enabled)
			*statusWord = (*statusWord) ^ 0x10;

			//Bit7 = 0
			*statusWord = (*statusWord) & 0xFF7F;

			//Toggle Bit8 (voltage enabled)
			*statusWord = (*statusWord) ^ 0x100;

			//Bit15 = 0
			*statusWord = (*statusWord) & 0x7FFF;
			break;
		}
		default:
			break;
	}
}

void StateMachine::startAutomaticmode(void) // Uebergibt Zyklusstart an StateMachine
{
	m_oCycleIsOn = true;
}

void StateMachine::stopAutomaticmode(void)  // Uebergibt Zyklusende an StateMachine
{
	m_oCycleIsOn = false;
}

/**
 * Passt das Senden der Sensordaten an die Bildfrequenz an.
 * @param context TriggerContext
 * @param interval Interval
 */
void StateMachine::burst(TriggerContext const& context, TriggerInterval const& interval)
{
    FastMutex::ScopedLock lock(m_fastMutex);
    m_context = context;
    m_interval = interval;

    m_imageNrMotionData = 0;

    cleanBuffer();

    unsigned int ecatSamplesPerTrigger = 1;

    m_pValuesMotionData = new TSmartArrayPtr<int>::ShArrayPtr[1];
    for (unsigned int i = 0; i < 1; ++i)
    {
        m_pValuesMotionData[i] = new int[ecatSamplesPerTrigger];
        for(unsigned int j = 0; j < ecatSamplesPerTrigger;j++) m_pValuesMotionData[i][j] = 0;
    }
} //release lock

void StateMachine::cancel(int id)
{
    FastMutex::ScopedLock lock(m_fastMutex);
    m_context = TriggerContext(0,0,0);
    m_interval = TriggerInterval(0,0);
    m_imageNrMotionData = 0;
    cleanBuffer();
}

//clean buffer
void StateMachine::cleanBuffer()
{
    if(m_pValuesMotionData != NULL)
    {
        for (unsigned int i = 0; i < 1; ++i)
        {
            m_pValuesMotionData[i] = 0; // decrement smart pointer reference counter
        }
        delete [] m_pValuesMotionData;
        m_pValuesMotionData = NULL;
    }
}

bool StateMachine::setHeadPosMsg(int p_oValue){

    if (m_oDebugInfo_AxisController)
    {
        wmLog(eDebug, "SM V1 %s: setHeadPosMsg: p_oValue:%d\n", m_oAxisLogCharacter, p_oValue);
    }
	pthread_mutex_lock(&m_mutex);
	unsigned char localModesOfOpDisplay = m_oStateData.modesOfOpDisplay;
	short localState = m_oStateData.state;
	pthread_mutex_unlock(&m_mutex);

	bool ret;
	if (localModesOfOpDisplay == Position && localState == STATE_POSITION_ACTIVE)
	{
		//Event auf null setzten => Event wuerde blockieren...
		m_oSetHeadPosEvent.reset();
		//Starte Fahrt
		SetHeadValue(p_oValue, Position_Absolute);
		//warte bis Position erreicht ist (TimeOut 2000 ms)
		ret = m_oSetHeadPosEvent.tryWait(20000);
	}
	else
	{
		wmLogTr(eError, "QnxMsg.VI.AxisNotInMode", "Axis %s: The axis is not in positioning mode\n", m_oAxisLogCharacter);
		ret = false;
	}
	return ret;
}

bool StateMachine::setHeadModeMsg(MotionMode p_oMode, bool p_oHome)
{
    if (m_oDebugInfo_AxisController)
    {
        wmLog(eDebug, "SM V1 %s: setHeadModeMsg: p_oMode:%d, p_oHome:%d\n", m_oAxisLogCharacter, p_oMode, p_oHome);
    }
	m_oSetHeadModeEvent.reset();
	SetHeadMode( p_oMode, p_oHome, true);
	bool oRet = m_oSetHeadModeEvent.tryWait(20000);
	if ((m_oStateData.statusWord & FAULT) == FAULT)
	{
		oRet = false;
	}
	return oRet;
}

int StateMachine::getHeadPositionMsg(void)
{
	pthread_mutex_lock(&m_mutex);
	int positionUserUnit = m_oStateData.positionUserUnit;
	pthread_mutex_unlock(&m_mutex);
	return positionUserUnit;
}

int StateMachine::getLowerLimitMsg(void)
{
	pthread_mutex_lock(&m_mutex);
	int lowerLimit = m_oStateData.m_oSoftLowerLimit;
	pthread_mutex_unlock(&m_mutex);
	return lowerLimit;
}

int StateMachine::getUpperLimitMsg(void)
{
	pthread_mutex_lock(&m_mutex);
	int upperLimit = m_oStateData.m_oSoftUpperLimit;
	pthread_mutex_unlock(&m_mutex);
	return upperLimit;
}

void StateMachine::SetAxisVelocity(int p_oValue)
{
	if (!m_oCycleIsOn) //Speichern nur wenn kein Zyklus aktiv ist, d.h. es handelt sich nicht um Parameter aus einem HW-Parametersatz
	{
		// Geschwindigkeit in WeldHeadDefaults-File sichern
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setInt("X_Axis_Velocity", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setInt("Y_Axis_Velocity", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setInt("Z_Axis_Velocity", p_oValue);
		}
	}

	pthread_mutex_lock(&m_mutex);

	// Geschwindigkeit fuer Uebertragung zur Achse setzen
	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
			// value parameter hat die Einheit [%], d.h. 1% -> 100%
			// Minimum velocity: 100000
			// Maximum velocity: 1700000
			// 0% -> 100% werden abgebildet auf 100000 -> 1700000
			p_oValue = (16000 * p_oValue) + 100000;
			break;
		case eProductIndex_EPOS4:
			// value parameter hat die Einheit [%], d.h. 1% -> 100%
			// Minimum velocity: 100000
			// Maximum velocity: 1700000
			// 0% -> 100% werden abgebildet auf 100000 -> 1700000
//			p_oValue = (16000 * p_oValue) + 100000;
			p_oValue = 5500 * (p_oValue / 100);
			break;
		default:
			break;
	}
	m_oStateData.m_oAxisVelocity = p_oValue;

	pthread_mutex_unlock(&m_mutex);
}

int StateMachine::GetAxisVelocity(void)
{
    int oValue;

	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
			// zurueckzugebender Wert muss in den Bereich 1% -> 100% abgebildet werden
			// siehe SetAxisVelocity
			oValue = m_oStateData.m_oAxisVelocity;
			oValue = (oValue - 100000) / 16000;
			break;
		case eProductIndex_EPOS4:
			// zurueckzugebender Wert muss in den Bereich 1% -> 100% abgebildet werden
			// siehe SetAxisVelocity
			oValue = m_oStateData.m_oAxisVelocity;
//			oValue = (oValue - 100000) / 16000;
			oValue = (oValue / 5500) * 100;
			break;
		default:
			oValue = m_oStateData.m_oAxisVelocity;
			break;
	}

	return oValue;
}

void StateMachine::SetAxisAcceleration(int p_oValue)
{
	if (!m_oCycleIsOn) //Speichern nur wenn kein Zyklus aktiv ist, d.h. es handelt sich nicht um Parameter aus einem HW-Parametersatz
	{
		// Beschleunigung in WeldHeadDefaults-File sichern
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setInt("X_Axis_Acceleration", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setInt("Y_Axis_Acceleration", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setInt("Z_Axis_Acceleration", p_oValue);
		}
	}

	pthread_mutex_lock(&m_mutex);

	// Beschleunigung fuer Uebertragung zur Achse setzen
	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
			// value parameter hat die Einheit [%], d.h. 1% -> 100%
			// Minimum acceleration: 100000
			// Maximum acceleration: 600000
			// 0% -> 100% werden abgebildet auf 100000 -> 600000
			p_oValue = (5000 * p_oValue) + 100000;
			break;
		case eProductIndex_EPOS4:
			// value parameter hat die Einheit [%], d.h. 1% -> 100%
			// Minimum acceleration: 100000
			// Maximum acceleration: 600000
			// 0% -> 100% werden abgebildet auf 100000 -> 600000
//			p_oValue = (5000 * p_oValue) + 100000;
			p_oValue = 85000 * (p_oValue / 100);
			break;
		default:
			break;
	}
	m_oStateData.m_oAxisAcceleration = p_oValue;

	pthread_mutex_unlock(&m_mutex);
}

int StateMachine::GetAxisAcceleration(void)
{
    int oValue;

	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
			// zurueckzugebender Wert muss in den Bereich 1% -> 100% abgebildet werden
			// siehe SetAxisAcceleration
			oValue = m_oStateData.m_oAxisAcceleration;
			oValue = (oValue - 100000) / 5000;
			break;
		case eProductIndex_EPOS4:
			// zurueckzugebender Wert muss in den Bereich 1% -> 100% abgebildet werden
			// siehe SetAxisAcceleration
			oValue = m_oStateData.m_oAxisAcceleration;
//			oValue = (oValue - 100000) / 5000;
			oValue = (oValue / 85000) * 100;
			break;
		default:
			oValue = m_oStateData.m_oAxisAcceleration;
			break;
	}

	return oValue;
}

void StateMachine::SetAxisDeceleration(int p_oValue)
{
	// Solange die Beschleunigung und die Verzeogerung nicht unterschiedlich gesetzt werden, muss die Verzoegerung nicht gesichert werden !
	// Das erspart unnoetige Dateizugriffe !
#if 0
	if (!m_oCycleIsOn) //Speichern nur wenn kein Zyklus aktiv ist, d.h. es handelt sich nicht um Parameter aus einem HW-Parametersatz
	{
		// Verzoegerung in WeldHeadDefaults-File sichern
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setInt("X_Axis_Acceleration", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setInt("Y_Axis_Acceleration", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setInt("Z_Axis_Acceleration", p_oValue);
		}
	}
#endif

	pthread_mutex_lock(&m_mutex);

	// Verzoegerung fuer Uebertragung zur Achse setzen
	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
			// value parameter hat die Einheit [%], d.h. 1% -> 100%
			// Minimum deceleration: 100000
			// Maximum deceleration: 600000
			// 0% -> 100% werden abgebildet auf 100000 -> 600000
			p_oValue = (5000 * p_oValue) + 100000;
			break;
		case eProductIndex_EPOS4:
			// value parameter hat die Einheit [%], d.h. 1% -> 100%
			// Minimum deceleration: 100000
			// Maximum deceleration: 600000
			// 0% -> 100% werden abgebildet auf 100000 -> 600000
//			p_oValue = (5000 * p_oValue) + 100000;
			p_oValue = 85000 * (p_oValue / 100);
			break;
		default:
			break;
	}
	m_oStateData.m_oAxisDeceleration = p_oValue;

	pthread_mutex_unlock(&m_mutex);
}

int StateMachine::GetAxisDeceleration(void)
{
    int oValue;

	switch (m_oAxisInputProxyInfo.m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
			// zurueckzugebender Wert muss in den Bereich 1% -> 100% abgebildet werden
			// siehe SetAxisDeceleration
			oValue = m_oStateData.m_oAxisDeceleration;
			oValue = (oValue - 100000) / 5000;
			break;
		case eProductIndex_EPOS4:
			// zurueckzugebender Wert muss in den Bereich 1% -> 100% abgebildet werden
			// siehe SetAxisDeceleration
			oValue = m_oStateData.m_oAxisDeceleration;
//			oValue = (oValue - 100000) / 5000;
			oValue = (oValue / 85000) * 100;
			break;
		default:
			oValue = m_oStateData.m_oAxisDeceleration;
			break;
	}

	return oValue;
}

void StateMachine::SetAxisHomingDirPos(bool p_oState)
{
	pthread_mutex_lock(&m_mutex);

	if (p_oState != m_oStateData.m_oAxisHomingDirPos)
	{
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setBool("AxisX_HomingDirPos", p_oState);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setBool("AxisY_HomingDirPos", p_oState);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setBool("AxisZ_HomingDirPos", p_oState);
		}
	}
	if (p_oState) // Homing Richtung positiv
	{
		m_oStateData.m_oAxisHomingDirPos = true;
		m_oStateData.m_oAxisHomingMode = 18;
	}
	else // Homing Richtung negativ
	{
		m_oStateData.m_oAxisHomingDirPos = false;
		m_oStateData.m_oAxisHomingMode = 17;
	}

	pthread_mutex_unlock(&m_mutex);
}

bool StateMachine::GetAxisHomingDirPos(void)
{
	return m_oStateData.m_oAxisHomingDirPos;
}

void StateMachine::SetAxisHomeOffset(int p_oValue)
{
	pthread_mutex_lock(&m_mutex);
	m_oStateData.m_oAxisHomeOffset = p_oValue;
	pthread_mutex_unlock(&m_mutex);
}

int StateMachine::GetAxisHomeOffset(void)
{
	return m_oStateData.m_oAxisHomeOffset;
}

void StateMachine::SetMountingRightTop(bool p_oState)
{
	pthread_mutex_lock(&m_mutex);

	if (p_oState != m_oStateData.m_oMountingRightTop)
	{
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setBool("AxisX_MountingRightTop", p_oState);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setBool("AxisY_MountingRightTop", p_oState);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setBool("AxisZ_MountingRightTop", p_oState);
		}
	}
	m_oStateData.m_oMountingRightTop = p_oState;

	pthread_mutex_unlock(&m_mutex);
}

bool StateMachine::GetMountingRightTop(void)
{
	return m_oStateData.m_oMountingRightTop;
}

void StateMachine::SetSoftLimitsActive(bool p_oState)
{
	pthread_mutex_lock(&m_mutex);

	if (p_oState != m_oStateData.m_oSoftLimitsActive)
	{
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setBool("AxisX_SoftLimitsActive", p_oState);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setBool("AxisY_SoftLimitsActive", p_oState);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setBool("AxisZ_SoftLimitsActive", p_oState);
		}
	}
	m_oStateData.m_oSoftLimitsActive = p_oState;

	pthread_mutex_unlock(&m_mutex);
}

bool StateMachine::GetSoftLimitsActive(void){
	return m_oStateData.m_oSoftLimitsActive;
}

void StateMachine::SetLowerLimit(int p_oValue)
{
	pthread_mutex_lock(&m_mutex);

	if (p_oValue != m_oStateData.m_oSoftLowerLimit)
	{
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setInt("AxisX_SoftLowerLimit", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setInt("AxisY_SoftLowerLimit", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setInt("AxisZ_SoftLowerLimit", p_oValue);
		}
	}
	m_oStateData.m_oSoftLowerLimit = p_oValue;

	pthread_mutex_unlock(&m_mutex);
}

int StateMachine::GetLowerLimit(void)
{
	return m_oStateData.m_oSoftLowerLimit;
}

void StateMachine::SetUpperLimit(int p_oValue)
{
	pthread_mutex_lock(&m_mutex);

	if (p_oValue != m_oStateData.m_oSoftUpperLimit)
	{
		if (m_oStateMachineInstance == interface::eAxisX)
		{
			WeldHeadDefaults::instance().setInt("AxisX_SoftUpperLimit", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisY)
		{
			WeldHeadDefaults::instance().setInt("AxisY_SoftUpperLimit", p_oValue);
		}
		else if (m_oStateMachineInstance == interface::eAxisZ)
		{
			WeldHeadDefaults::instance().setInt("AxisZ_SoftUpperLimit", p_oValue);
		}
	}
	m_oStateData.m_oSoftUpperLimit = p_oValue;

	pthread_mutex_unlock(&m_mutex);
}

int StateMachine::GetUpperLimit(void)
{
	return m_oStateData.m_oSoftUpperLimit;
}

//******************************************************************************
//******************************************************************************
//** now the thread function and the thread-used functions                    **
//******************************************************************************
//******************************************************************************

void* StateMachine::ThreadStateMachine(void* pvThreadParam)
{
    prctl(PR_SET_NAME, "StateMachine");
    system::makeThreadRealTime(system::Priority::EtherCATDependencies);
	THREAD_PARAM* pThreadParam = (THREAD_PARAM*) pvThreadParam;
	STATE_DATA* pStateData = pThreadParam->m_pStateData;
	HeadAxisID oStateMachineInstance = *pThreadParam->m_pStateMachineInstance;
	char oAxisLogCharacter[5];
	strcpy(oAxisLogCharacter, pThreadParam->m_pAxisLogCharacter);

	unsigned int oLowerHWLimit = 0x00;
	unsigned int oUpperHWLimit = 0x00;
	unsigned int oBrakeState = 0x00;
	unsigned short oFaultState = 0x00;

	//helper
	bool isDoingHomeing = false;
	bool requestMotionStateChange = false;
	bool waitForResetTargetReached = false;
	bool oHomingIsActive = false;
	//end helper

	//debug helper
	int debugTempState = STATE_OFFLINE_ACTIVE;
	//end debug helper

struct timespec startTime;
struct timespec stopTime;

	while (pThreadParam->m_pIsRunning)
	{
		sem_wait(pThreadParam->m_pSem);
		//NEW DATA, check states...

//*****************************************************************************
// for measurement of driving time
static unsigned short oldStatusWord = 0;
if (pStateData->statusWord != 0x1637)
{
}
else
{
	if (oldStatusWord != 0x1637)
	{
        if (clock_gettime(CLOCK_REALTIME, &stopTime) == -1)
        {
            printf("Problem with clock_gettime\n");
        }
        // Differenz zur startTime berechnen
        long diff = (stopTime.tv_sec - startTime.tv_sec) * 1000;
        diff += ((stopTime.tv_nsec / 1000000) - (startTime.tv_nsec / 1000000));
        printf("Fahrzeit msec: %ld\n", (long)diff);
	}
}
oldStatusWord = pStateData->statusWord;
// for measurement of driving time
//*****************************************************************************

		if ((pStateData->manuStatusReg & 0x0400) != oLowerHWLimit)
		{
			if ((pStateData->manuStatusReg & 0x0400)&&(!oHomingIsActive)) // Sensor 1, negativer Endschalter
			{
				pStateData->m_oAxisStatusBits |= eHWLimitNeg;
		        wmLogTr(eError, "QnxMsg.VI.AxisHardLowerLimit", "Axis %s: The lower hardware limit switch is active\n", oAxisLogCharacter);
//		        wmFatal(eAxis, "QnxMsg.VI.AxisHardLowerLimit", "Axis %s: The lower hardware limit switch is active\n", oAxisLogCharacter);
			}
			else
			{
				pStateData->m_oAxisStatusBits &= ~eHWLimitNeg;
			}
		}
		oLowerHWLimit = (pStateData->manuStatusReg & 0x0400);

		if ((pStateData->manuStatusReg & 0x0200) != oUpperHWLimit)
		{
			if ((pStateData->manuStatusReg & 0x0200)&&(!oHomingIsActive)) // Sensor 2, positiver Endschalter
			{
				pStateData->m_oAxisStatusBits |= eHWLimitPos;
		        wmLogTr(eError, "QnxMsg.VI.AxisHardUpperLimit", "Axis %s: The upper hardware limit switch is active\n", oAxisLogCharacter);
//		        wmFatal(eAxis, "QnxMsg.VI.AxisHardUpperLimit", "Axis %s: The upper hardware limit switch is active\n", oAxisLogCharacter);
			}
			else
			{
				pStateData->m_oAxisStatusBits &= ~eHWLimitPos;
			}
		}
		oUpperHWLimit = (pStateData->manuStatusReg & 0x0200);

		if ((pStateData->manuStatusReg & 0x4000) != oBrakeState)
		{
			if (pStateData->manuStatusReg & 0x4000) // Zustand Motorbremse
			{
				pStateData->m_oAxisStatusBits |= eBrakeOpen;
			}
			else
			{
				pStateData->m_oAxisStatusBits &= ~eBrakeOpen;
			}
		}
		oBrakeState = (pStateData->manuStatusReg & 0x4000);

		if ((pStateData->statusWord & FAULT) != oFaultState)
		{
			// Zustand: Fehlerbit im statusWord, Bewegung abgebrochen
// TODO EPOS4 hat kein TRAJECTORY_ABORTED, auf dem Bit liegt Referenziert ja/nein !
			if ((pStateData->statusWord & FAULT) && (pStateData->statusWord & TRAJECTORY_ABORTED))
			{
				pStateData->m_oAxisStatusBits |= eGenFault;
				wmFatal(eAxis, "QnxMsg.VI.AxisMoveFault", "Axis %s: There is a movement error active, movement is stopped\n", oAxisLogCharacter);
			}
			// Zustand: Fehlerbit im statusWord, Schleppfehler angezeigt
			else if ((pStateData->statusWord & FAULT) && (pStateData->statusWord & FOLLOWING_ERROR))
			{
				pStateData->m_oAxisStatusBits |= eGenFault;
				wmFatal(eAxis, "QnxMsg.VI.AxisFollowFault", "Axis %s: There is a following error active, movement is stopped\n", oAxisLogCharacter);
			}
			// Zustand: Fehlerbit im statusWord, sonstige Ursache
			else if (pStateData->statusWord & FAULT)
			{
				pStateData->m_oAxisStatusBits |= eGenFault;
				wmFatal(eAxis, "QnxMsg.VI.AxisGenFault", "Axis %s: There is a general error active\n", oAxisLogCharacter);
			}
			// Zustand: Fehlerbit nicht gesetzt
			else
			{
				pStateData->m_oAxisStatusBits &= ~eGenFault;
			}
		}
		oFaultState = (pStateData->statusWord & FAULT);

		if (pStateData->state == STATE_TRANSITION)
		{
			if (debugTempState != STATE_TRANSITION)
			{
				std::printf("StateMachine: Entered STATE_TRANSITION...\n");
				debugTempState = STATE_TRANSITION;
			}

			if(pStateData->requestedMode == Offline)
			{
				pStateData->writtenModesOfOperation = 0;
				pStateData->writtenControlWord = 0x0;
				SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
				pStateData->state = STATE_OFFLINE_ACTIVE;
				//set event for the MsgInterface
				pThreadParam->m_pSetHeadModeEvent->set();
			}
			else if (pStateData->m_oIsHomeable && pStateData->requestHome)
			{
				if (!isDoingHomeing)
				{
					//do Homing...
					if (pStateData->initMode == 0)
					{
						oHomingIsActive = true;
						pStateData->writtenModesOfOperation = 6;
						pStateData->writtenControlWord = 0x6;
						SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

						//clear faults 0x8x
                        usleep(2 * 1000);
						pStateData->writtenControlWord = 0x86;
						SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

                        usleep(2 * 1000);
						pStateData->writtenControlWord = 0x6;
						SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

						std::printf("InitMode: %d\n", pStateData->initMode);
						pStateData->initMode = 1;
					}
					else if ((pStateData->initMode == 1 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON))
							&& ((pStateData->statusWord & SWITCHED_ON) != SWITCHED_ON))
					{
						pStateData->writtenControlWord = 0x7;
						SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
						std::printf("InitMode: %d\n", pStateData->initMode);
						pStateData->initMode = 2;
					}
					else if (pStateData->initMode == 2 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON)
							&& ((pStateData->statusWord & SWITCHED_ON) == SWITCHED_ON))
					{
						pStateData->writtenControlWord = 0xf;
						SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
						std::printf("InitMode: %d\n", pStateData->initMode);
						pStateData->initMode = 3;
					}
					else if (pStateData->initMode == 3 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON)
							&& ((pStateData->statusWord & SWITCHED_ON) == SWITCHED_ON) && ((pStateData->statusWord
							& OPERATION_ENABLED) == OPERATION_ENABLED))
					{
						pStateData->writtenControlWord = 0x1f;
						SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
						isDoingHomeing = true;
						waitForResetTargetReached = true;
						std::printf("InitMode: %d\n", pStateData->initMode);
					}
				} // !isDoingHomeing
				else if (waitForResetTargetReached)
				{
					if (!((pStateData->statusWord & TARGET_REACHED) == TARGET_REACHED))
					{
						waitForResetTargetReached = false;
						std::printf("In motion...\n");
					}
				}
				else if (!requestMotionStateChange && ((pStateData->statusWord & TARGET_REACHED) == TARGET_REACHED)
						&& ((pStateData->statusWord & HOMING_ATTAINED) == HOMING_ATTAINED) && !((pStateData->statusWord & SPEED) == SPEED))
				{
					//Target reached -> HOMEING_FINISHED

					//	send Homing finished to subscribers
					//(*pThreadParam->m_pCbValueReached)(pStateData->modesOfOpDisplay, pStateData->positionUserUnit);

					//					std::printf("StatusWOrd: 0x%x\n",pStateData->statusWord);

					//	GO TO pStateData->requestedMode
					pStateData->initMode = 0;
					requestMotionStateChange = true;
					if (pStateData->writtenTargetPos == pStateData->requestTargetPosition)
					{
						//set event for the MsgInterface
						pThreadParam->m_pSetHeadModeEvent->set();
					}
					oHomingIsActive = false;
					pStateData->m_oAxisStatusBits &= ~eHomePosNotOK;
				}
			} // requestHome
			else
			{
				//STATE_INIT && is not homeable
				if (pStateData->initMode == 0)
				{
					pStateData->writtenModesOfOperation = pStateData->requestedMode;
					pStateData->writtenControlWord = 0x6;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

					//clear faults 0x8x
                    usleep(2 * 1000);
					pStateData->writtenControlWord = 0x86;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

                    usleep(2 * 1000);
					pStateData->writtenControlWord = 0x6;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

					pStateData->initMode = 1;
					std::printf("Init Mode 0\n");
				}
				else if (pStateData->initMode == 1 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON))
				{
					pStateData->writtenControlWord = 0x7;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
					pStateData->initMode = 2;
					std::printf("Init Mode 1\n");
				}
				else if (pStateData->initMode == 2 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON)
						&& ((pStateData->statusWord & SWITCHED_ON) == SWITCHED_ON))
				{
					pStateData->writtenControlWord = 0xf;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
					pStateData->initMode = 3;
					std::printf("Init Mode 2\n");
				}
				else if (pStateData->initMode == 3 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON)
						&& ((pStateData->statusWord & SWITCHED_ON) == SWITCHED_ON) && ((pStateData->statusWord & OPERATION_ENABLED)
						== OPERATION_ENABLED))
				{
					pStateData->initMode = 4;
					requestMotionStateChange = true;
					std::printf("Init Mode 3\n");
				}
			}
			if (requestMotionStateChange)
			{
				if (pStateData->initMode == 0)
				{
					pStateData->writtenModesOfOperation = pStateData->requestedMode;
					//end last trajectory
					pStateData->writtenControlWord = 0xf;

					pStateData->writtenControlWord = 0x6;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
					pStateData->initMode = 1;
				}
				else if (pStateData->initMode == 1 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON))
				{
					pStateData->writtenControlWord = 0x7;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
					pStateData->initMode = 2;
				}
				else if (pStateData->initMode == 2 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON)
						&& ((pStateData->statusWord & SWITCHED_ON) == SWITCHED_ON))
				{
					pStateData->writtenControlWord = 0xf;
					SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
					pStateData->initMode = 3;
				}
				else if (pStateData->initMode == 3 && ((pStateData->statusWord & READY_TO_SWITCH_ON) == READY_TO_SWITCH_ON)
						&& ((pStateData->statusWord & SWITCHED_ON) == SWITCHED_ON) && ((pStateData->statusWord & OPERATION_ENABLED)
						== OPERATION_ENABLED))
				{
				}
				else if (pStateData->requestedMode == pStateData->modesOfOpDisplay)
				{
					//set new StateMachine state...
					if (pStateData->modesOfOpDisplay == Position)
					{
						pStateData->state = STATE_POSITION_ACTIVE;
						isDoingHomeing = false;
						requestMotionStateChange = false;
					}
					else if (pStateData->modesOfOpDisplay == Velocity)
					{
						pStateData->state = STATE_VELOCITY_ACTIVE;
						isDoingHomeing = false;
						requestMotionStateChange = false;
					}
					else
					{
						//TODO: ERROR HANDLING
						std::printf("ERROR_1\n");
					}
				}
				else
				{
					//State change in progress...
					//TODO: abort if not set in xxx ms...
					std::printf("StateChange in progress... StatusWord: 0x%x\n", pStateData->statusWord);
				}
			}
		}
		else if (pStateData->state == STATE_POSITION_ACTIVE && pStateData->modesOfOpDisplay == Position)
		{
			if (debugTempState != STATE_POSITION_ACTIVE)
			{
				std::printf("StateMachine: Entered STATE_POSITION_ACTIVE...\n");
				debugTempState = STATE_POSITION_ACTIVE;
				pStateData->flag_blockTargetReached = true;
				(*pThreadParam->m_pCbIsReady)(oStateMachineInstance, Position);
			}
			else if (pStateData->requestTargetPosition != pStateData->writtenTargetPos)
			{
				//do targeting
				#ifdef DEBUG_STATES
				std::printf("Init new target-transition\n");
				#endif

				pStateData->flag_blockTargetReached = true;

if (clock_gettime(CLOCK_REALTIME, &startTime) == -1)
{
	printf("Problem with clock_gettime\n");
}

				std::printf("Request new target pos.: %d,  StatusWord: 0x%x\n", pStateData->requestTargetPosition, pStateData->statusWord);
				pStateData->writtenControlWord = 0x2f;
				pStateData->writtenTargetPos = pStateData->requestTargetPosition;

				SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
				//Compax3 needs some idle time to activate transition
				SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
                usleep(2 * 1000);
				pStateData->writtenControlWord = 0x3f;
				SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);
			}
			else if ((!pStateData->flag_blockTargetReached) && ((pStateData->statusWord & TARGET_REACHED)	== TARGET_REACHED))
			{
				//target reached
				#ifdef DEBUG_STATES
				std::printf("Target Reached\n");
				#endif

				pStateData->flag_blockTargetReached = true;
				(*pThreadParam->m_pCbValueReached)(oStateMachineInstance, Position, pStateData->positionUserUnit );
				//set event for the MsgInterface
				pThreadParam->m_pSetHeadPosEvent->set();
				//set event for the MsgInterface
				pThreadParam->m_pSetHeadModeEvent->set();
			}

			//detect targetReached negative edge
			if(pStateData->flag_blockTargetReached && !((pStateData->statusWord & TARGET_REACHED)	== TARGET_REACHED))
			{
				pStateData->flag_blockTargetReached = false;
			}
		}
		else if (pStateData->state == STATE_VELOCITY_ACTIVE && pStateData->modesOfOpDisplay == Velocity)
		{
			if (debugTempState != STATE_VELOCITY_ACTIVE)
			{
				std::printf("StateMachine: Entered STATE_VELOCITY_ACTIVE...\n");
				debugTempState = STATE_VELOCITY_ACTIVE;
				pStateData->requestedTargetVelocity = 0; //reset Velocity!
				(*pThreadParam->m_pCbIsReady)(oStateMachineInstance, Velocity);
			}
			else if (pStateData->requestedTargetVelocity != pStateData->writtenTargetVelocity)
			{
				//do targeting

				pStateData->writtenTargetVelocity = pStateData->requestedTargetVelocity;
				SendMotion(pStateData, pThreadParam->m_pEthercatOutputsProxy, pThreadParam->m_pAxisOutputProxyInfo);

				(*pThreadParam->m_pCbValueReached)(oStateMachineInstance, Velocity, pStateData->requestedTargetVelocity);
			}
		}
		else if (pStateData->state == STATE_OFFLINE_ACTIVE && pStateData->modesOfOpDisplay == Offline)
		{
			if (debugTempState != STATE_OFFLINE_ACTIVE)
			{
				std::printf("StateMachine: Entered offline State...\n");
				debugTempState = STATE_OFFLINE_ACTIVE;
				(*pThreadParam->m_pCbIsReady)(oStateMachineInstance, Offline);
			}
		}
		else
		{
			//TODO: VI ERROR
			if ((pStateData->state != STATE_OFFLINE_ACTIVE) && (pStateData->state != STATE_TRANSITION) &&
				(pStateData->state != STATE_POSITION_ACTIVE) && (pStateData->state !=  STATE_VELOCITY_ACTIVE))
			{
				std::printf("StateMachine: Entered UNKNOWN_STATE...\n");
			}
		}
	}
	return NULL;
}

void StateMachine::SendMotion(STATE_DATA* p_pStateData, TEthercatOutputs<EventProxy> *p_pEthercatOutputsProxy, SLAVE_PROXY_INFORMATION *p_pAxisOutputProxyInfo)
{
#if DEBUG_STATEMACHINE
	static unsigned short oWrittenModesOfOperation = 0;
	static unsigned short oWrittenControlWord = 0;
	static int oWrittenTargetPos = 0;

	if ((oWrittenModesOfOperation != p_pStateData->writtenModesOfOperation) || (oWrittenControlWord != p_pStateData->writtenControlWord) ||
		(oWrittenTargetPos != p_pStateData->writtenTargetPos))
	{
		struct timespec actTime;
        if (clock_gettime(CLOCK_REALTIME, &actTime) == -1)
        {
            printf("Problem with clock_gettime\n");
        }
        // Differenz zur startTime berechnen
        long zeit = (actTime.tv_sec) * 1000;
        zeit += (actTime.tv_nsec / 1000000);

		printf("SendMotion: %ld: Op: %04X, Contr: %04X, Targ: %d, Vel: %d, Acc: %d, Dec: %d\n",
				zeit,
				p_pStateData->writtenModesOfOperation,
				p_pStateData->writtenControlWord,
				p_pStateData->writtenTargetPos,
				p_pStateData->m_oAxisVelocity,
				p_pStateData->m_oAxisAcceleration,
				p_pStateData->m_oAxisDeceleration);
	}
	oWrittenModesOfOperation = p_pStateData->writtenModesOfOperation;
	oWrittenControlWord = p_pStateData->writtenControlWord;
	oWrittenTargetPos = p_pStateData->writtenTargetPos;
#endif

    // TODO Unterscheidung zwischen ACCELNET und Parker-Achse
    EcatAxisOutput axisOutput;
    axisOutput.controlWord = p_pStateData->writtenControlWord;
    axisOutput.modesOfOp = p_pStateData->writtenModesOfOperation;
	switch (p_pAxisOutputProxyInfo->m_oProductIndex)
	{
		case eProductIndex_ACCELNET:
            axisOutput.profileTargetPosition = static_cast<int>(static_cast<float>(p_pStateData->writtenTargetPos) * 4.114);
            axisOutput.homingOffset = static_cast<int>(static_cast<float>(p_pStateData->m_oAxisHomeOffset) * 4.114);
            axisOutput.homingVelocityFast = 200000;
            axisOutput.homingVelocitySlow = 83333;
			break;
		case eProductIndex_EPOS4:
            axisOutput.profileTargetPosition = static_cast<int>(static_cast<float>(p_pStateData->writtenTargetPos) * 4.114);
            axisOutput.homingOffset = static_cast<int>(static_cast<float>(p_pStateData->m_oAxisHomeOffset) * 4.114);
            axisOutput.homingVelocityFast = 500;
            axisOutput.homingVelocitySlow = 100;
			break;
		default:
            axisOutput.profileTargetPosition = p_pStateData->writtenTargetPos;
            axisOutput.homingOffset = p_pStateData->m_oAxisHomeOffset;
            axisOutput.homingVelocityFast = 200000;
            axisOutput.homingVelocitySlow = 83333;
			break;
	}
    axisOutput.profileVelocity = p_pStateData->m_oAxisVelocity;
    axisOutput.profileAcceleration = p_pStateData->m_oAxisAcceleration;
    axisOutput.profileDeceleration = p_pStateData->m_oAxisDeceleration;
    axisOutput.homingMethod = p_pStateData->m_oAxisHomingMode;

    if (p_pAxisOutputProxyInfo->m_oActive)
    {
        p_pEthercatOutputsProxy->ecatAxisOut(p_pAxisOutputProxyInfo->m_oProductIndex,
                                            p_pAxisOutputProxyInfo->m_oInstance,
                                            axisOutput);
    }
}

} // namespace ethercat
} // namespace precitec

