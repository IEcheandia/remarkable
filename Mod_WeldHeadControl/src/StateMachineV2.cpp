/**
 *   @file
 *   @copyright   Precitec Vision GmbH & Co. KG
 *   @author      Alexander Egger (EA)
 *   @date        2021
 *   @brief       Controls Maxon EPOS4 axis controller
 */

#include "viWeldHead/StateMachineV2.h"
#include "module/moduleLogger.h"

#include "viWeldHead/WeldHeadDefaults.h"

#include <sys/prctl.h>

#define DEBUG_STATEMACHINE 0

namespace precitec
{

namespace ethercat
{

// folgende CLOCK... verwenden
#define CLOCK_TO_USE CLOCK_MONOTONIC

// folgendes definiert die Anzahl ns pro Sekunde
#define NSEC_PER_SEC    (1000000000)

// Folgendes definiert eine Zykluszeit von 1ms
#define CYCLE_TIME_NS   (1000000)

void* AxisCyclicTaskThread(void *p_pArg);

StateMachineV2::StateMachineV2(HeadAxisID p_oStateMachineInstance,
                        bool p_oSoftLimitsActive, int p_oSoftLowerLimit, int p_oSoftUpperLimit,
                        bool p_oHomingDirPos, bool p_oMountingRightTop,
                        int p_oProductCode, int p_oVendorID ,int p_oInstance, bool p_oIsHomeable,
                        VICallbackBase *cbIsReady, VICallbackBase *cbValueReached, VICallbackBase *cbHeadError,
                        TSensor<AbstractInterface>& p_rSensorProxy,
                        TEthercatOutputs<EventProxy>& p_rEthercatOutputsProxy
                        ):
        m_oStateMachineInstance(p_oStateMachineInstance),
        m_oAxisCyclicTaskThread_ID(0),
        m_rSensorProxy (p_rSensorProxy),
        m_rEthercatOutputsProxy(p_rEthercatOutputsProxy),
        m_interval(0,0),
        m_imageNrMotionData(0),
        m_pValuesMotionData(NULL),
        m_oCycleIsOn(false),

        m_oRequestedControlWord(0x0000),
        m_oRequestedModesOfOperation(1),
        m_oRequestedTargetPosition(0),

        m_oStatusWord(0x0000),
        m_oModesOfOpDisplay(0),
        m_oErrorReg(0x00),
        m_oErrorCode(0x0000),
        m_oPositionUserUnit(0),

        m_oRequestOpEnabled(false),
        m_oInitDriveIsActive(false),
        m_oRequestOpDisabled(false),

        m_oDriveToPosition(false),
        m_oMovingIsActive(false),
        m_oDriveToPositionSuccessful(false),
        m_oDriveToPositionState(0),

        m_oDoHoming(false),
        m_oHomingIsActive(false),
        m_oDoHomingSuccessful(false),
        m_oDriveAfterHomingFlag(false),
        m_oDriveAfterHomingPosition(0),

        m_oDriveTimeStart {},
        m_oDriveTimeStop {},
        m_oDebugInfo_AxisController(false)
{
    wmLog(eDebug, "Start of StateMachineV2 for EPOS4 Controller\n");
    pthread_mutex_init(&m_mutex, NULL);

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
        m_oStateData.m_oAxisVelocity = 5400; // rev/min rpm
        // AxisAcceleration, AxisDeceleration
        //   bis aus Weiteres werden Acceleration und Deceleration vom WeldingHeadControl-Objekt auf ident. Werte gesetzt
        //   d.h. Beschleunigungs- und Bremsrampen haben identische Steigungen
        //   Das StateMachine-Objekt ist jedoch vorbereitet fuer unterschiedliche Steigungen
        m_oStateData.m_oAxisAcceleration = 180000; // (rev/min)/sec rpm/s
        m_oStateData.m_oAxisDeceleration = 180000; // (rev/min)/sec rpm/s
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
    if (p_oIsHomeable) // Achse muss referenziert weden
    {
        m_oStateData.m_oAxisStatusBits |= eHomePosNotOK;
    }

    m_oStateData.state = STATE_OFFLINE_ACTIVE;

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

    StartAxisCyclicTaskThread();
}

StateMachineV2::~StateMachineV2()
{
    if (m_oAxisCyclicTaskThread_ID != 0)
    {
        if (pthread_cancel(m_oAxisCyclicTaskThread_ID) != 0)
        {
            wmLog(eDebug, "was not able to abort thread\n");
        }
    }

    delete m_cbIsReady;
    delete m_cbValueReached;
    delete m_cbHeadError;

    pthread_mutex_destroy(&m_mutex);
}

void StateMachineV2::RequestHeadInfo(HeadInfo &info)
{
    pthread_mutex_lock(&m_mutex);

    switch (m_oStateData.state)
    {
        case STATE_OFFLINE_ACTIVE:
        {
            info.status = Offline;
            break;
        }
        case Home:
        {
            info.status = Home;
            break;
        }
        case STATE_POSITION_ACTIVE:
        {
            info.status = Position;
            break;
        }
        case Position_Relative:
        {
            info.status = Position_Relative;
            break;
        }
        case Position_Absolute:
        {
            info.status = Position_Absolute;
            break;
        }
        default:
        {
            info.status = Pending;
            break;
        }
    }

    info.statusWord = m_oStatusWord.load();
    info.modeOfOperation = m_oModesOfOpDisplay.load();
    info.errorCode = m_oErrorReg.load();
    info.positionUserUnit = m_oPositionUserUnit.load();
    info.actVelocity = m_oStateData.actVelocity;
    info.actTorque = m_oStateData.actTorque;
    info.m_oHomingDirPos = m_oStateData.m_oAxisHomingDirPos;
    info.m_oSoftLimitsActive = m_oStateData.m_oSoftLimitsActive;
    info.m_oSoftLowerLimit = m_oStateData.m_oSoftLowerLimit;
    info.m_oSoftUpperLimit = m_oStateData.m_oSoftUpperLimit;

    info.m_oAxisStatusBits = m_oStateData.m_oAxisStatusBits;

    pthread_mutex_unlock(&m_mutex);
}

void StateMachineV2::SetHeadMode(MotionMode mode, bool bHome, bool p_oGoToSoftLowerLimit)
{
    if (m_oDebugInfo_AxisController)
    {
        char oString1[81];
        char oString2[81];
        sprintf(oString1, "SM V2 %s: SetHeadMode: mode:%d, bHome:%d, ", m_oAxisLogCharacter, mode, bHome);
        sprintf(oString2, "p_oGoToSoftLowerLimit:%d", p_oGoToSoftLowerLimit);
        wmLog(eDebug, "%s%s\n", oString1, oString2);
    }
    if (clock_gettime(CLOCK_REALTIME, &m_oDriveTimeStart) == -1)
    {
        wmLog(eError, "Problem with clock_gettime\n");
    }

    pthread_mutex_lock(&m_mutex);

    if (mode == Offline)
    {
        m_oRequestOpDisabled.store(true);
        pthread_mutex_unlock(&m_mutex);
        return;
    }

    if(bHome)
    {
        m_oStateData.m_oAxisStatusBits |= eHomePosNotOK;
        if(p_oGoToSoftLowerLimit)
        {
            if (m_oStateData.m_oSoftLimitsActive) // Ueberwachung der Software-Endschalter ist aktiv
            {
                if (m_oStateData.m_oAxisHomingDirPos)
                {
                    if ((m_oStateData.m_oSoftLowerLimit > 0) ||
                        (m_oStateData.m_oSoftUpperLimit > 0))
                    {
                        (*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,0);
                        m_oStateData.m_oAxisStatusBits |= eOutOfRange;
                        wmLogTr(eError, "QnxMsg.VI.AxisNotInRange1", "Axis %s: The actual position is out of the software position limits\n", m_oAxisLogCharacter);
                        wmLogTr(eError, "QnxMsg.VI.AxisNotInRange2", "Axis %s: Please correct software position limits or do homing\n", m_oAxisLogCharacter);
                    }
                    else
                    {
                        m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
                        m_oDriveAfterHomingPosition.store(m_oStateData.m_oSoftUpperLimit);
                        m_oDriveAfterHomingFlag.store(true);
                    }
                }
                else
                {
                    if ((m_oStateData.m_oSoftLowerLimit < 0) ||
                        (m_oStateData.m_oSoftUpperLimit < 0))
                    {
                        (*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,0);
                        m_oStateData.m_oAxisStatusBits |= eOutOfRange;
                        wmLogTr(eError, "QnxMsg.VI.AxisNotInRange1", "Axis %s: The actual position is out of the software position limits\n", m_oAxisLogCharacter);
                        wmLogTr(eError, "QnxMsg.VI.AxisNotInRange2", "Axis %s: Please correct software position limits or do homing\n", m_oAxisLogCharacter);
                    }
                    else
                    {
                        m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
                        m_oDriveAfterHomingPosition.store(m_oStateData.m_oSoftLowerLimit);
                        m_oDriveAfterHomingFlag.store(true);
                    }
                }
            }
            else
            {
                m_oStateData.m_oAxisStatusBits &= ~eOutOfRange;
            }
        }
        m_oDoHoming.store(true);
    }
    pthread_mutex_unlock(&m_mutex);
}

void StateMachineV2::SetHeadValue(int value, MotionMode mode)
{
    if (m_oDebugInfo_AxisController)
    {
        char oString1[81];
        char oString2[81];
        sprintf(oString1, "SM V2 %s: SetHeadValue: value:%d, mode:%d, ", m_oAxisLogCharacter, value, mode);
        sprintf(oString2, "ModesOfOp:%d, State:%d", m_oModesOfOpDisplay.load(), m_oStateData.state);
        wmLog(eDebug, "%s%s\n", oString1, oString2);
    }

    pthread_mutex_lock(&m_mutex);
    //check if position is in valid interval
    if (m_oStateData.m_oSoftLimitsActive) // Ueberwachung der Software-Endschalter ist aktiv
    {
        // liegt die aktuelle Position ausserhalb der Software-Endschalter ?
        if ((m_oPositionUserUnit.load() < (m_oStateData.m_oSoftLowerLimit - 3)) ||
            (m_oPositionUserUnit.load() > (m_oStateData.m_oSoftUpperLimit + 3)))
        {
            (*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,m_oPositionUserUnit.load());
            m_oStateData.m_oAxisStatusBits |= eOutOfRange;
            wmLogTr(eError, "QnxMsg.VI.AxisNotInRange1", "Axis %s: The actual position is out of the software position limits\n", m_oAxisLogCharacter);
            wmLogTr(eError, "QnxMsg.VI.AxisNotInRange2", "Axis %s: Please correct software position limits or do homing\n", m_oAxisLogCharacter);

            pthread_mutex_unlock(&m_mutex);
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

    if(mode == Position_Relative)
    {
        m_oRequestedTargetPosition.store(m_oPositionUserUnit.load() + value);
        SendDataToAxis();
    }
    else if ( mode == Position_Absolute)
    {
        m_oRequestedTargetPosition.store(value);
        SendDataToAxis();
    }
    else
    {
        (*m_cbHeadError)(m_oStateMachineInstance, eErrorNotInRequestedMode,m_oModesOfOpDisplay.load());
    }

    int32_t oDiff = std::abs(m_oRequestedTargetPosition.load() - m_oPositionUserUnit.load());
    if (oDiff < 3)
    {
        // no driving necessary
        (*m_cbValueReached)(m_oStateMachineInstance, Position, value );
        m_oDriveToPositionSuccessful.store(true);
        m_oSetHeadPosEvent.set();
        pthread_mutex_unlock(&m_mutex);
        return;
    }

    //check limits
    if (m_oStateData.m_oSoftLimitsActive) // Ueberwachung der Software-Endschalter ist aktiv
    {
        // verletzt die gewuenschte Absolut-Position das untere Limit ?
        if (m_oRequestedTargetPosition.load() < m_oStateData.m_oSoftLowerLimit)
        {
            // neue Absolut-Position begrenzen
            m_oRequestedTargetPosition.store(m_oStateData.m_oSoftLowerLimit);
            // Achsenfehler zurueckmelden (Ist das notwendig ? eher eine Art Warnung ausgeben ?)
            (*m_cbHeadError)(m_oStateMachineInstance, eErrorLowerLimit,m_oRequestedTargetPosition.load());
            m_oStateData.m_oAxisStatusBits |= eSWLimitNeg;
            wmLogTr(eError, "QnxMsg.VI.AxisSoftLowerLimit", "Axis %s: The new position is limited to the lower software position limit\n", m_oAxisLogCharacter);
        }
        else
        {
            m_oStateData.m_oAxisStatusBits &= ~eSWLimitNeg;
        }
        // verletzt die gewuenschte Absolut-Position das obere Limit ?
        if (m_oRequestedTargetPosition.load() > m_oStateData.m_oSoftUpperLimit)
        {
            // neue Absolut-Position begrenzen
            m_oRequestedTargetPosition.store(m_oStateData.m_oSoftUpperLimit);
            // Achsenfehler zurueckmelden (Ist das notwendig ? eher eine Art Warnung ausgeben ?)
            (*m_cbHeadError)(m_oStateMachineInstance, eErrorUpperLimit,m_oRequestedTargetPosition.load());
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

    pthread_mutex_unlock(&m_mutex);

    if (!m_oMovingIsActive.load()) // no moving is active
    {
        if (clock_gettime(CLOCK_REALTIME, &m_oDriveTimeStart) == -1)
        {
            wmLog(eError, "Problem with clock_gettime\n");
        }

        m_oDriveToPosition.store(true);
    }
    else // moving is already active
    {
        m_oDriveToPositionState.store(0); // abort the DriveToPosition state machine
        usleep(2*1000);
        m_oDriveToPosition.store(true); // restart the DriveToPosition state machine with new target position
    }
}

void StateMachineV2::IncomingMotionData(void)
{
    Poco::FastMutex::ScopedLock lock(m_fastMutex);
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

void StateMachineV2::ecatAxisIn(EcatProductIndex productIndex, EcatInstance instance, const EcatAxisInput &axisInput) // Interface EthercatInputs
{
   if (m_oAxisInputProxyInfo.m_oActive)
   {
        if (m_oAxisInputProxyInfo.m_oProductIndex == productIndex)
        {
            if (m_oAxisInputProxyInfo.m_oInstance == instance)
            {
#if 0
                static int oLoop = 0;
                oLoop++;
                if (oLoop >= 5000)
                {
                    printf("statusWord: %04X, modesOfOpDisp: %02X, errorReg: %02X, manufacStatus: %04X\n",
                            axisInput.statusWord, axisInput.modesOfOpDisp, axisInput.errorReg, axisInput.manufacStatus);
                    oLoop = 0;
                }
#endif
                static uint16_t oDigitalInputs = 0xff;
                if (axisInput.m_oDigitalInputs != oDigitalInputs)
                {
                    oDigitalInputs = axisInput.m_oDigitalInputs;
                }
                static uint16_t oDigitalOutputs = 0xff;
                if (axisInput.m_oDigitalOutputs != oDigitalOutputs)
                {
                    if (axisInput.m_oDigitalOutputs & 0x0001)
                    {
                        m_oStateData.m_oAxisStatusBits &= ~eBrakeOpen;
                    }
                    else
                    {
                        m_oStateData.m_oAxisStatusBits |= eBrakeOpen;
                    }
                    oDigitalOutputs = axisInput.m_oDigitalOutputs;
                }
#if 0
                static int32_t oFollowingError = 0;
                if (std::abs(axisInput.m_oFollowingError - oFollowingError) > 100)
                {
                    printf("new FollowingError: %d\n", axisInput.m_oFollowingError);
                    oFollowingError = axisInput.m_oFollowingError;
                }
#endif
                pthread_mutex_lock(&m_mutex);

                auto statusWord = axisInput.statusWord;
                ConvertStatusWord(&statusWord);
#if DEBUG_STATEMACHINE
                if ((statusWord != m_oStatusWord.load()) || (axisInput.modesOfOpDisp != m_oModesOfOpDisplay.load()) ||
                    (axisInput.errorReg != m_oErrorReg.load()) || (axisInput.manufacStatus != m_oErrorCode.load()))
                {
                    struct timespec actTime;
                    if (clock_gettime(CLOCK_REALTIME, &actTime) == -1)
                    {
                        wmLog(eError, "Problem with clock_gettime\n");
                    }
                    long zeit = (actTime.tv_sec) * 1000;
                    zeit += (actTime.tv_nsec / 1000000);

                    printf("Incoming: %ld: Stat: %04X, Op: %02X, Err: %02X, manu: %04X, Pos: %d, Vel: %d, Tor: %d\n", zeit,
                            statusWord, axisInput.modesOfOpDisp, axisInput.errorReg, axisInput.manufacStatus,
                            axisInput.actualPosition, axisInput.actualVelocity, axisInput.actualTorque);
                }
#endif
                m_oStatusWord.store(statusWord);
                m_oModesOfOpDisplay.store(axisInput.modesOfOpDisp);
                m_oErrorReg.store(axisInput.errorReg);
                m_oErrorCode.store(axisInput.manufacStatus);
                switch (m_oAxisInputProxyInfo.m_oProductIndex)
                {
                    case eProductIndex_ACCELNET:
                        m_oPositionUserUnit.store(static_cast<int>(static_cast<float>(axisInput.actualPosition) / 4.114));
                        break;
                    case eProductIndex_EPOS4:
                        m_oPositionUserUnit.store(static_cast<int>(static_cast<float>(axisInput.actualPosition) / 4.114));
                        break;
                    default:
                        m_oPositionUserUnit.store(axisInput.actualPosition);
                        break;
                }
                m_oStateData.actVelocity = axisInput.actualVelocity;
                m_oStateData.actTorque = axisInput.actualTorque;

                pthread_mutex_unlock(&m_mutex);
            }
        }
        else
        {
            // falscher productIndex !
        }
    }
}

void StateMachineV2::ConvertStatusWord(unsigned short *statusWord)
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
        default:
            break;
    }
}

void StateMachineV2::startAutomaticmode(void) // Uebergibt Zyklusstart an StateMachine
{
    m_oCycleIsOn = true;
}

void StateMachineV2::stopAutomaticmode(void)  // Uebergibt Zyklusende an StateMachine
{
    m_oCycleIsOn = false;
}

/**
 * Passt das Senden der Sensordaten an die Bildfrequenz an.
 * @param context TriggerContext
 * @param interval Interval
 */
void StateMachineV2::burst(TriggerContext const& context, TriggerInterval const& interval)
{
    Poco::FastMutex::ScopedLock lock(m_fastMutex);
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

void StateMachineV2::cancel(int id)
{
    Poco::FastMutex::ScopedLock lock(m_fastMutex);
    m_context = TriggerContext(0,0,0);
    m_interval = TriggerInterval(0,0);
    m_imageNrMotionData = 0;
    cleanBuffer();
}

//clean buffer
void StateMachineV2::cleanBuffer()
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

bool StateMachineV2::setHeadPosMsg(int p_oValue)
{
    if (m_oDebugInfo_AxisController)
    {
        wmLog(eDebug, "SM V2 %s: setHeadPosMsg: p_oValue:%d\n", m_oAxisLogCharacter, p_oValue);
    }
    //Event auf null setzten => Event wuerde blockieren...
    m_oSetHeadPosEvent.reset();
    //Starte Fahrt
    SetHeadValue(p_oValue, Position_Absolute);
    //warte bis Position erreicht ist (TimeOut 2000 ms)
    bool oRet = m_oSetHeadPosEvent.tryWait(20000);
    if ((m_oStatusWord.load() & STATUS_FAULT) == STATUS_FAULT)
    {
        oRet = false;
    }
    if (m_oDriveToPositionSuccessful.load() == false)
    {
        oRet = false;
    }
    return oRet;
}

bool StateMachineV2::setHeadModeMsg(MotionMode p_oMode, bool p_oHome)
{
    if (m_oDebugInfo_AxisController)
    {
        wmLog(eDebug, "SM V2 %s: setHeadModeMsg: p_oMode:%d, p_oHome:%d\n", m_oAxisLogCharacter, p_oMode, p_oHome);
    }
    m_oSetHeadModeEvent.reset();
    SetHeadMode( p_oMode, p_oHome, true);
    bool oRet = m_oSetHeadModeEvent.tryWait(20000);
    if ((m_oStatusWord.load() & STATUS_FAULT) == STATUS_FAULT)
    {
        oRet = false;
    }
    if (m_oDoHomingSuccessful.load() == false)
    {
        oRet = false;
    }
    return oRet;
}

int StateMachineV2::getHeadPositionMsg(void)
{
    return m_oPositionUserUnit.load();
}

int StateMachineV2::getLowerLimitMsg(void)
{
    pthread_mutex_lock(&m_mutex);
    int lowerLimit = m_oStateData.m_oSoftLowerLimit;
    pthread_mutex_unlock(&m_mutex);
    return lowerLimit;
}

int StateMachineV2::getUpperLimitMsg(void)
{
    pthread_mutex_lock(&m_mutex);
    int upperLimit = m_oStateData.m_oSoftUpperLimit;
    pthread_mutex_unlock(&m_mutex);
    return upperLimit;
}

void StateMachineV2::SetAxisVelocity(int p_oValue)
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
            // Minimum velocity: 200
            // Maximum velocity: 5400
            // 0% -> 100% werden abgebildet auf 200 -> 5400
            p_oValue = (52 * p_oValue) + 200;
            break;
        default:
            break;
    }
    m_oStateData.m_oAxisVelocity = p_oValue;

    pthread_mutex_unlock(&m_mutex);
}

int StateMachineV2::GetAxisVelocity(void)
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
            oValue = (oValue - 200) / 52;
            break;
        default:
            oValue = m_oStateData.m_oAxisVelocity;
            break;
    }

    return oValue;
}

void StateMachineV2::SetAxisAcceleration(int p_oValue)
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
            // Minimum acceleration: 10000
            // Maximum acceleration: 180000
            // 0% -> 100% werden abgebildet auf 10000 -> 180000
            p_oValue = (1700 * p_oValue) + 10000;
            break;
        default:
            break;
    }
    m_oStateData.m_oAxisAcceleration = p_oValue;

    pthread_mutex_unlock(&m_mutex);
}

int StateMachineV2::GetAxisAcceleration(void)
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
            oValue = (oValue - 10000) / 1700;
            break;
        default:
            oValue = m_oStateData.m_oAxisAcceleration;
            break;
    }

    return oValue;
}

void StateMachineV2::SetAxisDeceleration(int p_oValue)
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
            // Minimum acceleration: 10000
            // Maximum acceleration: 180000
            // 0% -> 100% werden abgebildet auf 10000 -> 180000
            p_oValue = (1700 * p_oValue) + 10000;
            break;
        default:
            break;
    }
    m_oStateData.m_oAxisDeceleration = p_oValue;

    pthread_mutex_unlock(&m_mutex);
}

int StateMachineV2::GetAxisDeceleration(void)
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
            oValue = (oValue - 10000) / 1700;
            break;
        default:
            oValue = m_oStateData.m_oAxisDeceleration;
            break;
    }

    return oValue;
}

void StateMachineV2::SetAxisHomingDirPos(bool p_oState)
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

bool StateMachineV2::GetAxisHomingDirPos(void)
{
    return m_oStateData.m_oAxisHomingDirPos;
}

void StateMachineV2::SetAxisHomeOffset(int p_oValue)
{
    pthread_mutex_lock(&m_mutex);
    m_oStateData.m_oAxisHomeOffset = p_oValue;
    pthread_mutex_unlock(&m_mutex);
}

int StateMachineV2::GetAxisHomeOffset(void)
{
    return m_oStateData.m_oAxisHomeOffset;
}

void StateMachineV2::SetMountingRightTop(bool p_oState)
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

bool StateMachineV2::GetMountingRightTop(void)
{
    return m_oStateData.m_oMountingRightTop;
}

void StateMachineV2::SetSoftLimitsActive(bool p_oState)
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

bool StateMachineV2::GetSoftLimitsActive(void)
{
    return m_oStateData.m_oSoftLimitsActive;
}

void StateMachineV2::SetLowerLimit(int p_oValue)
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

int StateMachineV2::GetLowerLimit(void)
{
    return m_oStateData.m_oSoftLowerLimit;
}

void StateMachineV2::SetUpperLimit(int p_oValue)
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

int StateMachineV2::GetUpperLimit(void)
{
    return m_oStateData.m_oSoftUpperLimit;
}

//*********************************************************************************************************************
//*********************************************************************************************************************
//*********************************************************************************************************************

DriveStateType StateMachineV2::StateOfTheDrive(void)
{
    DriveStateType oStateOfTheDrive = eNotReadyToSwitchOn;

    uint16_t oStatusWord = m_oStatusWord.load();
    if ((oStatusWord & 0x006F) == 0x0000)
    {
        oStateOfTheDrive = eNotReadyToSwitchOn;
    }
    if ((oStatusWord & 0x006F) == 0x0040)
    {
        oStateOfTheDrive = eSwitchOnDisabled;
    }
    if ((oStatusWord & 0x006F) == 0x0021)
    {
        oStateOfTheDrive = eReadyToSwitchOn;
    }
    if ((oStatusWord & 0x006F) == 0x0023)
    {
        oStateOfTheDrive = eSwitchedOn;
    }
    if ((oStatusWord & 0x006F) == 0x0027)
    {
        oStateOfTheDrive = eOperationEnabled;
    }
    if ((oStatusWord & 0x006F) == 0x0007)
    {
        oStateOfTheDrive = eQuickStopActive;
    }
    if ((oStatusWord & 0x006F) == 0x000F)
    {
        oStateOfTheDrive = eFaultReactionActive;
    }
    if ((oStatusWord & 0x006F) == 0x0008)
    {
        oStateOfTheDrive = eFault;
    }

    if (m_oDebugInfo_AxisController)
    {
        static DriveStateType oOldStateOfTheDrive = eNotReadyToSwitchOn;
        if (oStateOfTheDrive != oOldStateOfTheDrive)
        {
            char oLogStrg1[61] {};
            char oLogStrg2[61] {};
            sprintf(oLogStrg1, "--> DriveState axis %s: ", m_oAxisLogCharacter);
            switch (oStateOfTheDrive)
            {
                case eNotReadyToSwitchOn:
                    sprintf(oLogStrg2, "eNotReadyToSwitchOn");
                    break;
                case eSwitchOnDisabled:
                    sprintf(oLogStrg2, "eSwitchOnDisabled");
                    break;
                case eReadyToSwitchOn:
                    sprintf(oLogStrg2, "eReadyToSwitchOn");
                    break;
                case eSwitchedOn:
                    sprintf(oLogStrg2, "eSwitchedOn");
                    break;
                case eOperationEnabled:
                    sprintf(oLogStrg2, "eOperationEnabled");
                    break;
                case eQuickStopActive:
                    sprintf(oLogStrg2, "eQuickStopActive");
                    break;
                case eFaultReactionActive:
                    sprintf(oLogStrg2, "eFaultReactionActive");
                    break;
                case eFault:
                    sprintf(oLogStrg2, "eFault");
                    break;
                default:
                    sprintf(oLogStrg2, "unknown state");
                    break;
            }
            wmLog(eDebug, "%s%s\n", oLogStrg1, oLogStrg2);
        }
        oOldStateOfTheDrive = oStateOfTheDrive;
    }

    return oStateOfTheDrive;
}

DriveOperationModeType StateMachineV2::OperationModeOfTheDrive(void)
{
    DriveOperationModeType oOperationModeOfTheDrive = eNoValidMode;

    int8_t oModesOfOpDisplay = m_oModesOfOpDisplay.load();
    if (oModesOfOpDisplay == 1)
    {
        oOperationModeOfTheDrive = eProfilePositionMode;
    }
    if (oModesOfOpDisplay == 3)
    {
        oOperationModeOfTheDrive = eProfileVelocityMode;
    }
    if (oModesOfOpDisplay == 6)
    {
        oOperationModeOfTheDrive = eHomingMode;
    }
    if (oModesOfOpDisplay == 8)
    {
        oOperationModeOfTheDrive = eCyclicSyncPositionMode;
    }
    if (oModesOfOpDisplay == 9)
    {
        oOperationModeOfTheDrive = eCyclicSyncVelocityMode;
    }
    if (oModesOfOpDisplay == 10)
    {
        oOperationModeOfTheDrive = eCyclicSyncTorqueMode;
    }

    if (m_oDebugInfo_AxisController)
    {
        static DriveOperationModeType oOldOperationModeOfTheDrive = eNoValidMode;
        if (oOperationModeOfTheDrive != oOldOperationModeOfTheDrive)
        {
            char oLogStrg1[61] {};
            char oLogStrg2[61] {};
            sprintf(oLogStrg1, "--> DriveOperationMode axis %s: ", m_oAxisLogCharacter);
            switch (oOperationModeOfTheDrive)
            {
                case eProfilePositionMode:
                    sprintf(oLogStrg2, "eProfilePositionMode");
                    break;
                case eProfileVelocityMode:
                    sprintf(oLogStrg2, "eProfileVelocityMode");
                    break;
                case eHomingMode:
                    sprintf(oLogStrg2, "eHomingMode");
                    break;
                case eCyclicSyncPositionMode:
                    sprintf(oLogStrg2, "eCyclicSyncPositionMode");
                    break;
                case eCyclicSyncVelocityMode:
                    sprintf(oLogStrg2, "eCyclicSyncVelocityMode");
                    break;
                case eCyclicSyncTorqueMode:
                    sprintf(oLogStrg2, "eCyclicSyncTorqueMode");
                    break;
                default:
                    sprintf(oLogStrg2, "unknown operation mode");
                    break;
            }
            wmLog(eDebug, "%s%s\n", oLogStrg1, oLogStrg2);
        }
        oOldOperationModeOfTheDrive = oOperationModeOfTheDrive;
    }

    return oOperationModeOfTheDrive;
}

void StateMachineV2::DriveCommandShutdown(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord &= ~0x0087;
    oControlWord |= 0x0006;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
}

void StateMachineV2::DriveCommandSwitchOn(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord &= ~0x0087;
    oControlWord |= 0x0007;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
}

void StateMachineV2::DriveCommandEnableOperation(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord &= ~0x008F;
    oControlWord |= 0x000F;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
}

void StateMachineV2::DriveCommandDisableVoltage(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord &= ~0x0082;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
}

void StateMachineV2::DriveCommandQuickStop(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord &= ~0x0086;
    oControlWord |= 0x0002;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
}

void StateMachineV2::DriveCommandDisableOperation(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord &= ~0x008F;
    oControlWord |= 0x0007;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
}

void StateMachineV2::DriveCommandFaultReset(void)
{
    uint16_t oControlWord = m_oRequestedControlWord.load();
    oControlWord |= 0x0080;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
    usleep(5*1000);
    oControlWord &= ~0x0080;
    m_oRequestedControlWord.store(oControlWord);
    SendDataToAxis();
    m_oStateData.m_oAxisStatusBits &= ~eHWLimitNeg;
    m_oStateData.m_oAxisStatusBits &= ~eHWLimitPos;
    m_oStateData.m_oAxisStatusBits &= ~eGenFault;
}

void StateMachineV2::SetOperationMode(DriveOperationModeType p_oNewMode)
{
    int8_t oModesOfOperation = 1;

    switch (p_oNewMode)
    {
        case eProfilePositionMode:
            oModesOfOperation = 1;
            break;
        case eProfileVelocityMode:
            oModesOfOperation = 3;
            break;
        case eHomingMode:
            oModesOfOperation = 6;
            break;
        case eCyclicSyncPositionMode:
            oModesOfOperation = 8;
            break;
        case eCyclicSyncVelocityMode:
            oModesOfOperation = 9;
            break;
        case eCyclicSyncTorqueMode:
            oModesOfOperation = 10;
            break;
        default:
            oModesOfOperation = 1;
            break;
    }

    m_oRequestedModesOfOperation.store(oModesOfOperation);
    SendDataToAxis();
}

void StateMachineV2::SendDataToAxis(void)
{
    EcatAxisOutput oDataToAxis;

    oDataToAxis.controlWord = m_oRequestedControlWord.load();
    oDataToAxis.modesOfOp = m_oRequestedModesOfOperation.load();
    switch (m_oAxisOutputProxyInfo.m_oProductIndex)
    {
        case eProductIndex_ACCELNET:
            oDataToAxis.profileTargetPosition = static_cast<int>(static_cast<float>(m_oRequestedTargetPosition.load()) * 4.114);
            oDataToAxis.homingOffset = static_cast<int>(static_cast<float>(m_oStateData.m_oAxisHomeOffset) * 4.114);
            oDataToAxis.homingVelocityFast = 200000;
            oDataToAxis.homingVelocitySlow = 83333;
            break;
        case eProductIndex_EPOS4:
            oDataToAxis.profileTargetPosition = static_cast<int>(static_cast<float>(m_oRequestedTargetPosition.load()) * 4.114);
            oDataToAxis.homingOffset = static_cast<int>(static_cast<float>(m_oStateData.m_oAxisHomeOffset) * 4.114);
            oDataToAxis.homingVelocityFast = 500;
            oDataToAxis.homingVelocitySlow = 100;
            break;
        default:
            break;
    }
    oDataToAxis.profileVelocity = m_oStateData.m_oAxisVelocity;
    oDataToAxis.profileAcceleration = m_oStateData.m_oAxisAcceleration;
    oDataToAxis.profileDeceleration = m_oStateData.m_oAxisDeceleration;
    oDataToAxis.homingMethod = m_oStateData.m_oAxisHomingMode;

    if (m_oAxisOutputProxyInfo.m_oActive)
    {
        m_rEthercatOutputsProxy.ecatAxisOut(m_oAxisOutputProxyInfo.m_oProductIndex,
                                            m_oAxisOutputProxyInfo.m_oInstance,
                                            oDataToAxis);
    }
}

void StateMachineV2::TestFunctionAxis_1(void)
{
    DriveCommandDisableOperation();
}

void StateMachineV2::TestFunctionAxis_2(void)
{
    DriveCommandEnableOperation();
}

void StateMachineV2::TestFunctionAxis_3(void)
{
}

void StateMachineV2::TestFunctionAxis_4(void)
{
}

void StateMachineV2::TestFunctionAxis_5(void)
{
    DriveCommandFaultReset();
}

/***************************************************************************/
/* StartAxisCyclicTaskThread                                               */
/***************************************************************************/

void StateMachineV2::StartAxisCyclicTaskThread(void)
{
    ///////////////////////////////////////////////////////
    // Thread fuer zyklischen Ablauf starten
    ///////////////////////////////////////////////////////
    pthread_attr_t oPthreadAttr;
    pthread_attr_init(&oPthreadAttr);

    m_oDataToAxisCyclicTaskThread.m_pStateMachineV2 = this;

    if (pthread_create(&m_oAxisCyclicTaskThread_ID, &oPthreadAttr, &AxisCyclicTaskThread, &m_oDataToAxisCyclicTaskThread) != 0)
    {
        char oStrErrorBuffer[256];

        wmLog(eDebug, "pthread_create failed (%s)(%s)\n", "001", strerror_r(errno, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
        wmFatal(eInternalError, "QnxMsg.VI.IDMCreateThreadFail", "Cannot start thread (%s)\n", "001");
    }
}

/***************************************************************************/
/* AxisCyclicTaskFunction                                                  */
/***************************************************************************/

void StateMachineV2::AxisCyclicTaskFunction(void)
{
    /////////////////////////////////////////////////////////////
    // check for device errors
    /////////////////////////////////////////////////////////////
    static DriveStateType oOldDriveState = eSwitchOnDisabled;
    if (StateOfTheDrive() != oOldDriveState)
    {
        if (StateOfTheDrive() == eFault)
        {
            //if (m_oStatusWord.load() & STATUS_FOLLOWING_ERROR)
            //{
            //    wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis reports an following error (1)\n");
            //}
            //else
            //{
            //    wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis reports an unspecified error (1)\n");
            //}
            m_oStateData.state = STATE_OFFLINE_ACTIVE;
            m_oStateData.m_oAxisStatusBits |= eGenFault;
            wmFatal(eAxis, "QnxMsg.VI.ABCDEF", "Axis %s is in fault state !\n", m_oAxisLogCharacter);
        }
        oOldDriveState = StateOfTheDrive();
    }

    static uint16_t oOldErrorCode = 0x0000;
    if (m_oErrorCode.load() != oOldErrorCode)
    {
        if (m_oErrorCode.load() == 0x8611)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s: A following error is active: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x8A80)
        {
            m_oStateData.m_oAxisStatusBits |= eHWLimitNeg;
            //wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s: The lower hardware limit switch is active: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
            wmLogTr(eError, "QnxMsg.VI.AxisHardLowerLimit", "Axis %s: The lower hardware limit switch is active\n", m_oAxisLogCharacter);
        }
        else if (m_oErrorCode.load() == 0x8A81)
        {
            m_oStateData.m_oAxisStatusBits |= eHWLimitPos;
            //wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s: The upper hardware limit switch is active: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
            wmLogTr(eError, "QnxMsg.VI.AxisHardUpperLimit", "Axis %s: The upper hardware limit switch is active\n", m_oAxisLogCharacter);
        }
        else if (m_oErrorCode.load() == 0x8A82)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports a software position limit error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x8A88)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports a save torque off error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x2310)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports an overcurrent error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x3210)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports an overvoltage error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x3220)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports an undervoltage error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x4210)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports a thermal overload error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() == 0x5113)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports a logic supply voltage too low error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if ((m_oErrorCode.load() >= 0x7300) && (m_oErrorCode.load() <= 0x73FF))
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports an encoder error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if ((m_oErrorCode.load() >= 0x8100) && (m_oErrorCode.load() <= 0x82FF))
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports an communication error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        else if (m_oErrorCode.load() != 0x0000)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s reports an unspecified error: %x\n", m_oAxisLogCharacter, m_oErrorCode.load());
        }
        oOldErrorCode = m_oErrorCode.load();
    }

    static uint8_t oOldErrorReg = 0x00;
    if (m_oErrorReg.load() != oOldErrorReg)
    {
        if (m_oErrorReg.load() & 0x80)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Motion error\n", m_oAxisLogCharacter);
        }
        if (m_oErrorReg.load() & 0x40)
        {
            // reserved, always 0;
        }
        if (m_oErrorReg.load() & 0x20)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Device profile specific error\n", m_oAxisLogCharacter);
        }
        if (m_oErrorReg.load() & 0x10)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Communication error\n", m_oAxisLogCharacter);
        }
        if (m_oErrorReg.load() & 0x08)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Temperature error\n", m_oAxisLogCharacter);
        }
        if (m_oErrorReg.load() & 0x04)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Voltage error\n", m_oAxisLogCharacter);
        }
        if (m_oErrorReg.load() & 0x02)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Current error\n", m_oAxisLogCharacter);
        }
        if (m_oErrorReg.load() & 0x01)
        {
            wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Axis %s Error Register: Generic error\n", m_oAxisLogCharacter);
        }
        oOldErrorReg = m_oErrorReg.load();
    }

    GotoOpEnabledProcedure();
    GotoOpDisabledProcedure();
    DriveToPositionProcedure();
    DoHomingProcedure();
}

void StateMachineV2::GotoOpEnabledProcedure(void)
{
    /////////////////////////////////////////////////////////////
    // dialog for switching controller to operation enabled state
    /////////////////////////////////////////////////////////////
    static int oGotoOpEnabledState = 0;
    static int oGotoOpEnabledDelayCounter = 0;

    switch(oGotoOpEnabledState)
    {
        case 0:
            // waiting for request to switch to operation enabled state
            if (m_oRequestOpEnabled.load())
            {
                m_oRequestOpEnabled.store(false);
                m_oInitDriveIsActive.store(true);
                DriveStateType oStateOfTheDrive = StateOfTheDrive();
                if (oStateOfTheDrive == eSwitchOnDisabled)
                {
                    oGotoOpEnabledDelayCounter = 0;
                    oGotoOpEnabledState = 10;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oGotoOpEnabledState: goto 10\n");
                    }
                }
                else if (oStateOfTheDrive == eReadyToSwitchOn)
                {
                    oGotoOpEnabledDelayCounter = 0;
                    oGotoOpEnabledState = 12;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oGotoOpEnabledState: goto 12\n");
                    }
                }
                else if (oStateOfTheDrive == eSwitchedOn)
                {
                    oGotoOpEnabledDelayCounter = 0;
                    oGotoOpEnabledState = 14;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oGotoOpEnabledState: goto 14\n");
                    }
                }
                else if (oStateOfTheDrive == eOperationEnabled)
                {
                    m_oInitDriveIsActive.store(false);
                    m_oStateData.state = STATE_POSITION_ACTIVE;
                    (*m_cbIsReady)(m_oStateMachineInstance, Position);
                    oGotoOpEnabledDelayCounter = 0;
                    oGotoOpEnabledState = 0;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oGotoOpEnabledState: goto 0\n");
                    }
                }
                else if (oStateOfTheDrive == eQuickStopActive)
                {
                }
                else if (oStateOfTheDrive == eFault)
                {
                    oGotoOpEnabledDelayCounter = 0;
                    oGotoOpEnabledState = 4;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oGotoOpEnabledState: goto 4\n");
                    }
                }
            }
            break;
        case 4:
            DriveCommandFaultReset();
            oGotoOpEnabledDelayCounter = 0;
            oGotoOpEnabledState = 5;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpEnabledState: goto 5\n");
            }
            break;
        case 5:
            oGotoOpEnabledDelayCounter++;
            if (oGotoOpEnabledDelayCounter >= 1000) // Timeout 1 second
            {
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for reset fault (goto 100)\n");
                }
            }
            if (StateOfTheDrive() != eFault)
            {
                m_oRequestOpEnabled.store(true);
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 0;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oGotoOpEnabledState: goto 0\n");
                }
            }
            break;
        case 10:
            DriveCommandShutdown();
            oGotoOpEnabledDelayCounter = 0;
            oGotoOpEnabledState = 11;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpEnabledState: goto 11\n");
            }
            break;
        case 11:
            oGotoOpEnabledDelayCounter++;
            if (oGotoOpEnabledDelayCounter >= 1000) // Timeout 1 second
            {
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for command shutdown (goto 100)\n");
                }
            }
            if (StateOfTheDrive() == eReadyToSwitchOn)
            {
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 12;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oGotoOpEnabledState: goto 12\n");
                }
            }
            break;
        case 12:
            DriveCommandSwitchOn();
            oGotoOpEnabledDelayCounter = 0;
            oGotoOpEnabledState = 13;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpEnabledState: goto 13\n");
            }
            break;
        case 13:
            oGotoOpEnabledDelayCounter++;
            if (oGotoOpEnabledDelayCounter >= 1000) // Timeout 1 second
            {
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for command switch on (goto 100)\n");
                }
            }
            if (StateOfTheDrive() == eSwitchedOn)
            {
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 14;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oGotoOpEnabledState: goto 14\n");
                }
            }
            break;
        case 14:
            DriveCommandEnableOperation();
            oGotoOpEnabledDelayCounter = 0;
            oGotoOpEnabledState = 15;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpEnabledState: goto 15\n");
            }
            break;
        case 15:
            oGotoOpEnabledDelayCounter++;
            if (oGotoOpEnabledDelayCounter >= 1000) // Timeout 1 second
            {
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for command enable operation (goto 100)\n");
                }
            }
            if (StateOfTheDrive() == eOperationEnabled)
            {
                m_oInitDriveIsActive.store(false);
                m_oStateData.state = STATE_POSITION_ACTIVE;
                (*m_cbIsReady)(m_oStateMachineInstance, Position);
                oGotoOpEnabledDelayCounter = 0;
                oGotoOpEnabledState = 0;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oGotoOpEnabledState: goto 0\n");
                }
            }
            break;
        case 100:
            wmLogTr(eError, "QnxMsg.VI.DriveEnableTimeout", "There is a timeout while initializing the axis\n");
            oGotoOpEnabledDelayCounter = 0;
            oGotoOpEnabledState = 0;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpEnabledState: goto 0\n");
            }
            break;        
        default:
            break;
    }
}

void StateMachineV2::GotoOpDisabledProcedure(void)
{
    //////////////////////////////////////////////////////////////
    // dialog for switching controller to operation disabled state
    //////////////////////////////////////////////////////////////
    static int oGotoOpDisabledState = 0;
    static int oGotoOpDisabledDelayCounter = 0;

    switch(oGotoOpDisabledState)
    {
        case 0:
            // waiting for request to switch to operation disabled state
            if (m_oRequestOpDisabled.load())
            {
                m_oRequestOpDisabled.store(false);
                DriveStateType oStateOfTheDrive = StateOfTheDrive();
                if (oStateOfTheDrive == eOperationEnabled)
                {
                    oGotoOpDisabledDelayCounter = 0;
                    oGotoOpDisabledState = 10;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oGotoOpDisabledState: goto 10\n");
                    }
                }
                else
                {
                    m_oSetHeadModeEvent.set();
                }
            }
            break;
        case 10:
            DriveCommandShutdown();
            oGotoOpDisabledDelayCounter = 0;
            oGotoOpDisabledState = 11;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpDisabledState: goto 11\n");
            }
            break;
        case 11:
            oGotoOpDisabledDelayCounter++;
            if (oGotoOpDisabledDelayCounter >= 1000) // Timeout 1 second
            {
                oGotoOpDisabledDelayCounter = 0;
                oGotoOpDisabledState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for command disable operation (goto 100)\n");
                }
            }
            if (StateOfTheDrive() != eOperationEnabled)
            {
                m_oStateData.state = STATE_OFFLINE_ACTIVE;
                (*m_cbIsReady)(m_oStateMachineInstance, Offline);
                m_oSetHeadModeEvent.set();
                oGotoOpDisabledDelayCounter = 0;
                oGotoOpDisabledState = 0;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oGotoOpDisabledState: goto 0\n");
                }
            }
            break;
        case 100:
            wmLogTr(eError, "QnxMsg.VI.DriveDisableTimeout", "There is a timeout while switching off axis\n");
            m_oSetHeadModeEvent.set();
            oGotoOpDisabledDelayCounter = 0;
            oGotoOpDisabledState = 0;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oGotoOpDisabledState: goto 0\n");
            }
            break;        
        default:
            break;
    }
}

void StateMachineV2::DriveToPositionProcedure(void)
{
    ///////////////////////////////////
    // dialog for drive to new position
    ///////////////////////////////////
    static int oDriveToPositionDelayCounter = 0;

    switch(m_oDriveToPositionState.load())
    {
        case 0:
            // waiting for request to switch to operation enabled state
            if (m_oDriveToPosition.load())
            {
                m_oDriveToPosition.store(false);
                m_oDriveToPositionSuccessful.store(false);
                if (StateOfTheDrive() != eFault)
                {
                    m_oMovingIsActive.store(true);
                    oDriveToPositionDelayCounter = 0;
                    m_oDriveToPositionState.store(1);
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oDriveToPositionState: goto 1\n");
                    }
                }
                else
                {
                    wmLogTr(eError, "QnxMsg.VI.ABCDEF", "Cannot drive axis, axis is in error state\n");
                }
            }
            break;
        case 1:
            if (OperationModeOfTheDrive() != eProfilePositionMode)
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(2);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 2\n");
                }
            }
            else
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(5);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 5\n");
                }
            }
            break;
        case 2:
            SetOperationMode(eProfilePositionMode);
            oDriveToPositionDelayCounter = 0;
            m_oDriveToPositionState.store(3);
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDriveToPositionState: goto 3\n");
            }
            break;
        case 3:
            oDriveToPositionDelayCounter++;
            if (oDriveToPositionDelayCounter >= 1000) // Timeout 1 second
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(100);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for Profile Position Mode (goto 100)\n");
                }
            }
            if (OperationModeOfTheDrive() == eProfilePositionMode)
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(5);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 5\n");
                }
            }
            break;
        case 5:
            if (StateOfTheDrive() != eOperationEnabled) // Controller ist not in state operation enabled
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(6);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 6\n");
                }
            }
            else // Controller is already in state operation enabled
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(10);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 10\n");
                }
            }
            break;
        case 6:
            // start to switch controller in state operation enabled
            m_oRequestOpEnabled.store(true);
            oDriveToPositionDelayCounter = 0;
            m_oDriveToPositionState.store(7);
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDriveToPositionState: goto 7\n");
            }
            break;
        case 7:
            // wait for controller signals state operation enabled
            oDriveToPositionDelayCounter++;
            if (oDriveToPositionDelayCounter >= 1000) // Timeout 1 second
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(100);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for operation enabled state (goto 100)\n");
                }
            }
            if (StateOfTheDrive() == eOperationEnabled)
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(10);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 10\n");
                }
            }
            break;
        case 10:
            // New setpoint in ControlWord setzen
            m_oRequestedControlWord &= ~CTRL_RELATIVE_MOVE;
            m_oRequestedControlWord |= CTRL_NEW_SETPOINT;
            m_oRequestedControlWord |= CTRL_CHANGE_IMMEDIATELY;
            SendDataToAxis();
            oDriveToPositionDelayCounter = 0;
            m_oDriveToPositionState.store(11);
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDriveToPositionState: goto 11\n");
            }
            break;
        case 11:
            // warten auf setpoint acknowledge high in StatusWord (und auf Target Reached low ?)
            oDriveToPositionDelayCounter++;
            if (oDriveToPositionDelayCounter >= 1000) // Timeout 1 second
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(100);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for setpoint acknowledge high (goto 100)\n");
                }
            }
            if ((m_oStatusWord.load() & STATUS_ACKN_SETPOINT) == STATUS_ACKN_SETPOINT)
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(12);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 12\n");
                }
            }
            break;
        case 12:
            // New setpoint in ControlWord zuruecksetzen
            m_oRequestedControlWord &= ~CTRL_CHANGE_IMMEDIATELY;
            m_oRequestedControlWord &= ~CTRL_NEW_SETPOINT;
            SendDataToAxis();
            oDriveToPositionDelayCounter = 0;
            m_oDriveToPositionState.store(13);
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDriveToPositionState: goto 13\n");
            }
            break;
        case 13:
            // warten auf setpoint acknowledge low in StatusWord
            oDriveToPositionDelayCounter++;
            if (oDriveToPositionDelayCounter >= 1000) // Timeout 1 second
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(100);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for setpoint acknowledge low (goto 100)\n");
                }
            }
            if ((m_oStatusWord.load() & STATUS_ACKN_SETPOINT) == 0x0000)
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(14);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 14\n");
                }
            }
            break;
        case 14:
            // warten auf Target Reached in StatusWord
            oDriveToPositionDelayCounter++;
            if (oDriveToPositionDelayCounter >= 20000) // Timeout 20 second
            {
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(100);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for target reached (goto 100)\n");
                }
            }
            if ((m_oStatusWord.load() & STATUS_TARGET_REACHED) == STATUS_TARGET_REACHED)
            {
                (*m_cbValueReached)(m_oStateMachineInstance, Position, m_oPositionUserUnit.load() );
                m_oDriveToPositionSuccessful.store(true);
                m_oSetHeadPosEvent.set();

                if (clock_gettime(CLOCK_REALTIME, &m_oDriveTimeStop) == -1)
                {
                    wmLog(eError, "Problem with clock_gettime\n");
                }
                long diff = (m_oDriveTimeStop.tv_sec - m_oDriveTimeStart.tv_sec) * 1000;
                diff += ((m_oDriveTimeStop.tv_nsec / 1000000) - (m_oDriveTimeStart.tv_nsec / 1000000));
                wmLog(eDebug, "Fahrzeit msec: %d\n", diff);

                m_oMovingIsActive.store(false);
                oDriveToPositionDelayCounter = 0;
                m_oDriveToPositionState.store(0);
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDriveToPositionState: goto 0\n");
                }
            }
            break;
        case 100:
            wmLogTr(eError, "QnxMsg.VI.DriveMoveTimeout", "There is a timeout while driving the axis\n");

            m_oMovingIsActive.store(false);
            m_oDriveToPositionSuccessful.store(false);
            m_oSetHeadPosEvent.set();

            oDriveToPositionDelayCounter = 0;
            m_oDriveToPositionState.store(0);
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDriveToPositionState: goto 0\n");
            }
            break;        
        default:
            break;
    }
}

void StateMachineV2::DoHomingProcedure(void)
{
    //////////////////////////////
    // dialog for homing procedure
    //////////////////////////////
    static int oDoHomingState = 0;
    static int oDoHomingCounter = 0;

    switch(oDoHomingState)
    {
        case 0:
            // waiting for request to switch to operation enabled state
            if (m_oDoHoming.load())
            {
                m_oDoHoming.store(false);
                m_oDoHomingSuccessful.store(false);
                m_oHomingIsActive.store(true);
                if (OperationModeOfTheDrive() != eHomingMode) // Controller ist not in homing mode
                {
                    oDoHomingCounter = 0;
                    oDoHomingState = 1;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oDoHomingState: goto 1\n");
                    }
                }
                else // Controller is already in homing mode
                {
                    oDoHomingCounter = 0;
                    oDoHomingState = 5;
                    if (m_oDebugInfo_AxisController)
                    {
                        wmLog(eDebug, "oDoHomingState: goto 5\n");
                    }
                }
            }
            break;
        case 1:
            // switch controller in homing mode
            SetOperationMode(eHomingMode);
            oDoHomingCounter = 0;
            oDoHomingState = 2;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDoHomingState: goto 2\n");
            }
            break;
        case 2:
            // wait for controller signals homing mode
            oDoHomingCounter++;
            if (oDoHomingCounter >= 1000) // Timeout 1 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for homing mode (goto 100)\n");
                }
            }
            if (OperationModeOfTheDrive() == eHomingMode)
            {
                oDoHomingCounter = 0;
                oDoHomingState = 5;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 5\n");
                }
            }
            break;
        case 5:
            if (StateOfTheDrive() != eOperationEnabled) // Controller ist not in state operation enabled
            {
                oDoHomingCounter = 0;
                oDoHomingState = 6;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 6\n");
                }
            }
            else // Controller is already in state operation enabled
            {
                oDoHomingCounter = 0;
                oDoHomingState = 10;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 10\n");
                }
            }
            break;
        case 6:
            // start to switch controller in state operation enabled
            m_oRequestOpEnabled.store(true);
            oDoHomingCounter = 0;
            oDoHomingState = 7;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDoHomingState: goto 7\n");
            }
            break;
        case 7:
            // wait for controller signals state operation enabled
            oDoHomingCounter++;
            if (oDoHomingCounter >= 1000) // Timeout 1 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for operation enabled (goto 100)\n");
                }
            }
            if (StateOfTheDrive() == eOperationEnabled)
            {
                oDoHomingCounter = 0;
                oDoHomingState = 10;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 10\n");
                }
            }
            break;
        case 10:
            // set homing operation start in ControlWord
            m_oRequestedControlWord |= CTRL_START_HOMING;
            SendDataToAxis();
            oDoHomingCounter = 0;
            oDoHomingState = 11;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDoHomingState: goto 11\n");
            }
            break;
        case 11:
            // wait for homing procedure is in progress
            oDoHomingCounter++;
            if (oDoHomingCounter >= 2000) // Timeout 2 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for homing procedure startet (goto 100)\n");
                }
            }
            if ((m_oStatusWord.load() & (STATUS_HOMING_ATTAINED | STATUS_HOMING_ERROR | STATUS_TARGET_REACHED)) == 0x0000)
            {
                oDoHomingCounter = 0;
                oDoHomingState = 12;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 12\n");
                }
            }
            break;
        case 12:
            // wait for homing procedure has finished
            oDoHomingCounter++;
            if (oDoHomingCounter >= 20000) // Timeout 20 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for homing procedure finished (goto 100)\n");
                }
            }
            if ((m_oStatusWord.load() & (STATUS_HOMING_ATTAINED | STATUS_HOMING_ERROR)) == STATUS_HOMING_ATTAINED)
            {
                m_oRequestedControlWord &= ~CTRL_START_HOMING;
                SendDataToAxis();
                oDoHomingCounter = 0;
                oDoHomingState = 13;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 13\n");
                }
            }
            break;
        case 13:
            // switch controller in profile position mode
            SetOperationMode(eProfilePositionMode);
            oDoHomingCounter = 0;
            oDoHomingState = 14;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDoHomingState: goto 14\n");
            }
            break;
        case 14:
            // wait for controller signals profile position mode
            oDoHomingCounter++;
            if (oDoHomingCounter >= 1000) // Timeout 1 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for profile position mode (goto 100)\n");
                }
            }
            if (OperationModeOfTheDrive() == eProfilePositionMode)
            {
                oDoHomingCounter = 0;
                oDoHomingState = 15;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 15\n");
                }
            }
            break;
        case 15:
            // is a movement after homing requested ?
            if (m_oDriveAfterHomingFlag.load())
            {
                m_oDriveAfterHomingFlag.store(false);
                m_oRequestedTargetPosition.store(m_oDriveAfterHomingPosition.load());
                SendDataToAxis();
                m_oDriveToPosition.store(true);

                oDoHomingCounter = 0;
                oDoHomingState = 16;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 16\n");
                }
            }
            else // no movement requested, go back to state 0
            {
                m_oStateData.m_oAxisStatusBits &= ~eHomePosNotOK;
                m_oDoHomingSuccessful.store(true);
                m_oSetHeadModeEvent.set();

                if (clock_gettime(CLOCK_REALTIME, &m_oDriveTimeStop) == -1)
                {
                    wmLog(eError, "Problem with clock_gettime\n");
                }
                long diff = (m_oDriveTimeStop.tv_sec - m_oDriveTimeStart.tv_sec) * 1000;
                diff += ((m_oDriveTimeStop.tv_nsec / 1000000) - (m_oDriveTimeStart.tv_nsec / 1000000));
                wmLog(eDebug, "Fahrzeit msec: %d\n", diff);

                m_oHomingIsActive.store(false);
                oDoHomingCounter = 0;
                oDoHomingState = 0;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 0\n");
                }
            }
            break;
        case 16:
            // wait for movement has started
            oDoHomingCounter++;
            if (oDoHomingCounter >= 1000) // Timeout 1 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for movement started (goto 100)\n");
                }
            }
            if (m_oDriveToPositionState.load() > 0)
            {
                oDoHomingCounter = 0;
                oDoHomingState = 17;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 17\n");
                }
            }
            break;
        case 17:
            // wait for movement has finished
            oDoHomingCounter++;
            if (oDoHomingCounter >= 20000) // Timeout 20 second
            {
                oDoHomingCounter = 0;
                oDoHomingState = 100;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "Timeout while waiting for movement finished (goto 100)\n");
                }
            }
            if (m_oDriveToPositionState.load() == 0)
            {
                m_oStateData.m_oAxisStatusBits &= ~eHomePosNotOK;
                m_oDoHomingSuccessful.store(true);
                m_oSetHeadModeEvent.set();

                if (clock_gettime(CLOCK_REALTIME, &m_oDriveTimeStop) == -1)
                {
                    wmLog(eError, "Problem with clock_gettime\n");
                }
                long diff = (m_oDriveTimeStop.tv_sec - m_oDriveTimeStart.tv_sec) * 1000;
                diff += ((m_oDriveTimeStop.tv_nsec / 1000000) - (m_oDriveTimeStart.tv_nsec / 1000000));
                wmLog(eDebug, "Fahrzeit msec: %d\n", diff);

                m_oHomingIsActive.store(false);
                oDoHomingCounter = 0;
                oDoHomingState = 0;
                if (m_oDebugInfo_AxisController)
                {
                    wmLog(eDebug, "oDoHomingState: goto 0\n");
                }
            }
            break;
        case 100:
            wmLogTr(eError, "QnxMsg.VI.DriveHomeTimeout", "There is a timeout while homing the axis\n");

            m_oStateData.m_oAxisStatusBits |= eHomePosNotOK;
            m_oDoHomingSuccessful.store(false);
            m_oSetHeadModeEvent.set();

            oDoHomingCounter = 0;
            oDoHomingState = 0;
            if (m_oDebugInfo_AxisController)
            {
                wmLog(eDebug, "oDoHomingState: goto 0\n");
            }
            break;        
        default:
            break;
    }
}

/***************************************************************************/
/* AxisCyclicTaskThread                                                    */
/***************************************************************************/

// Thread Funktion muss ausserhalb der Klasse sein
void *AxisCyclicTaskThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "AxisCyclicTask");
    system::makeThreadRealTime(system::Priority::EtherCATDependencies);
    struct timespec oWakeupTime;
    int retValue;

    auto pDataToAxisCyclicTaskThread = static_cast<struct DataToAxisCyclicTaskThread *>(p_pArg);
    auto pStateMachineV2 = pDataToAxisCyclicTaskThread->m_pStateMachineV2;

    wmLog(eDebug, "AxisCyclicTaskThread is started\n");

    usleep(2000 * 1000); // 2 seconds delay before starting to feed interfaces

    wmLog(eDebug, "AxisCyclicTaskThread is active\n");

    clock_gettime(CLOCK_TO_USE, &oWakeupTime);
    oWakeupTime.tv_sec += 1; // start in future
    oWakeupTime.tv_nsec = 0;

    while(true)
    {
        retValue = clock_nanosleep(CLOCK_TO_USE, TIMER_ABSTIME, &oWakeupTime, NULL);
        if (retValue)
        {
            char oStrErrorBuffer[256];

            wmLog(eDebug, "clock_nanosleep failed (%s)\n", strerror_r(retValue, oStrErrorBuffer, sizeof(oStrErrorBuffer)));
            wmLogTr(eError, "QnxMsg.VI.IDMCyclicSleepFail", "Sleeping time for cycle loop failed\n");
            break;
        }

        pStateMachineV2->AxisCyclicTaskFunction();

        // calculate time for next send
        oWakeupTime.tv_nsec += CYCLE_TIME_NS;
        while(oWakeupTime.tv_nsec >= NSEC_PER_SEC)
        {
            oWakeupTime.tv_nsec -= NSEC_PER_SEC;
            oWakeupTime.tv_sec++;
        }
    }

    return NULL;
}

} // namespace ethercat
} // namespace precitec

