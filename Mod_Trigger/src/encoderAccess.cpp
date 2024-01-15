/**
 *   @file
 *   @copyright   Precitec Vision GmbH & Co. KG
 *   @author      Alexander Egger (EA)
 *   @date        2019
 *   @brief       Accesses encoder board for generating position dependend image trigger
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <ios>
#include <string>
#include <cmath>

#include "module/moduleLogger.h"
#include "common/systemConfiguration.h"
#include "system/realTimeSupport.h"

#include "Trigger/encoderAccess.h"

#include "Trigger/libPreciEncoderDriver.h"
#include "Trigger/Trigger.h"

namespace precitec
{

namespace trigger
{

// Thread Funktion muss ausserhalb der Klasse sein
void* EncoderAccessThread(void *p_pArg);

EncoderAccess::EncoderAccess():
        m_oIsImageTriggerViaEncoderSignals(false),
        m_oPreciEncoderDriverFd(-1),
        IRQCounter(0),
        m_oVendorId(0),
        m_oDeviceId(0),
        m_oSubVendorId(0),
        m_oSubDeviceId(0),
        m_oEncoderDevice(eNoDevice)
{
    m_oThread = 0;

    // SystemConfig Switch for ImageTriggerViaEncoderSignals
    m_oIsImageTriggerViaEncoderSignals = SystemConfiguration::instance().getBool("ImageTriggerViaEncoderSignals", false);
    wmLog(eDebug, "ImageTriggerViaEncoderSignals (bool): %d\n", m_oIsImageTriggerViaEncoderSignals);

    if (m_oIsImageTriggerViaEncoderSignals)
    {
        if( (m_oPreciEncoderDriverFd = open("/dev/PreciEncoderDriver0", O_RDWR)) < 0)
        {
            wmLog(eDebug, "open %s failed!  error is: %s\n", "/dev/PreciEncoderDriver0", strerror(errno));
            wmFatal(eAxis, "QnxMsg.VI.InitEncoderFault", "Problem while initializing encoder access ! %s\n", "(010)");
        }

        if (m_oPreciEncoderDriverFd >= 0)
        {
            struct PreciEncoder_info_struct oBoardInfo;
            if (PreciEncoder_read_board_info  (m_oPreciEncoderDriverFd, &oBoardInfo))
            {
                wmLog(eDebug, "EncoderAccess: Failed to read board info: %s\n", strerror(errno));
                wmFatal(eAxis, "QnxMsg.VI.EncoderAccFault", "Problem with access to encoder device ! %s\n", "(001)");
            }
            m_oVendorId = oBoardInfo.vendor_id;
            m_oDeviceId = oBoardInfo.device_id;
            m_oSubVendorId = oBoardInfo.sub_vendor_id;
            m_oSubDeviceId = oBoardInfo.sub_device_id;

            char oHelpStrg[81];
            sprintf(oHelpStrg, "VendorId: %04X, DeviceId: %04X", oBoardInfo.vendor_id, oBoardInfo.device_id);
            wmLog(eDebug, "%s\n", oHelpStrg);
            sprintf(oHelpStrg, "SubVendorId: %04X, SubDeviceId: %04X", oBoardInfo.sub_vendor_id, oBoardInfo.sub_device_id);
            wmLog(eDebug, "%s\n", oHelpStrg);

            if ((m_oVendorId == PCI_VENDOR_ID_JANZTEC) && (m_oDeviceId == PCI_DEVICE_ID_PCIINC) &&
                (m_oSubVendorId == PCI_VENDOR_ID_SUB_JANZTEC) && (m_oSubDeviceId == PCI_DEVICE_ID_SUB_PCIINC))
            {
                m_oEncoderDevice = eJanztecPCIINC;
            }
            if ((m_oVendorId == PCI_VENDOR_ID_ADVANTECH) && (m_oDeviceId == PCI_DEVICE_ID_PCI1784) &&
                (m_oSubVendorId == PCI_VENDOR_ID_SUB_ADVANTECH) && (m_oSubDeviceId == PCI_DEVICE_ID_SUB_PCI1784))
            {
                m_oEncoderDevice = eAdvantechPCI1784;
            }
        }
    }

    m_oDataToEncoderAccessThread.m_pEncoderAccess = this;
    m_oDataToEncoderAccessThread.m_pPreciEncoderDriverFd = &m_oPreciEncoderDriverFd;

    pthread_attr_t oPthreadAttr;

    pthread_attr_init(&oPthreadAttr);
    if (pthread_create(&m_oThread, &oPthreadAttr, &EncoderAccessThread, &m_oDataToEncoderAccessThread) != 0)
    {
        wmLog(eDebug, "was not able to create thread: %s\n", strerror(errno));
        wmFatal(eAxis, "QnxMsg.VI.InitEncoderFault", "Problem while initializing encoder access ! %s\n", "(011)");
    }
}

EncoderAccess::~EncoderAccess()
{
    if (pthread_cancel(m_oThread) != 0)
    {
        wmLog(eDebug, "was not able to abort thread: %s\n", strerror(errno));
        wmFatal(eAxis, "QnxMsg.VI.InitEncoderFault", "Problem while initializing encoder access ! %s\n", "(012)");
    }

    if (m_oPreciEncoderDriverFd >= 0)
    {
        close(m_oPreciEncoderDriverFd);
    }
}

void EncoderAccess::initEncoder(void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    if (m_oEncoderDevice == eJanztecPCIINC)
    {
        // reset counter preset register
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT_CTRL_REG, (PCIINC_ENC_CTRL_RESP | PCIINC_ENC_CTRL_ENI | PCIINC_ENC_CTRL_QUAT_X4));
        // reset counter register
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT_CTRL_REG, (PCIINC_ENC_CTRL_RESC | PCIINC_ENC_CTRL_ENI | PCIINC_ENC_CTRL_QUAT_X4));

        // folgendes ist <doppelt gemoppelt>
        writeEncCompare(0); // set counter preset regster to 0
        setEncoder(0); // set counter register to 0

        uint8_t oRegister0;
        uint8_t oRegister1;
        uint8_t oRegister2;
        uint8_t oRegister3;
        uint32_t oHelpRegister;
        char oHelpString[81];

        // read test register
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_TEST0_REG, &oRegister0);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_TEST1_REG, &oRegister1);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_TEST2_REG, &oRegister2);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_TEST3_REG, &oRegister3);
        oHelpRegister = (uint32_t)oRegister0
                    | (uint32_t)(oRegister1 << 8)
                    | (uint32_t)(oRegister2 << 16)
                    | (uint32_t)(oRegister3 << 24);
        sprintf(oHelpString, "Test Register:    %08X", oHelpRegister);
        wmLog(eDebug, "%s\n", oHelpString);

        // read release register
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_RELEASE0_REG, &oRegister0);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_RELEASE1_REG, &oRegister1);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_RELEASE2_REG, &oRegister2);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_RELEASE3_REG, &oRegister3);
        oHelpRegister = (uint32_t)oRegister0
                    | (uint32_t)(oRegister1 << 8)
                    | (uint32_t)(oRegister2 << 16)
                    | (uint32_t)(oRegister3 << 24);
        sprintf(oHelpString, "Release Register: %08X", oHelpRegister);
        wmLog(eDebug, "%s\n", oHelpString);

        // read counter value
        // low byte must be read first
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT0_REG, &oRegister0);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT1_REG, &oRegister1);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT2_REG, &oRegister2);
        oHelpRegister = (uint32_t)oRegister0
                    | (uint32_t)(oRegister1 << 8)
                    | (uint32_t)(oRegister2 << 16);
        sprintf(oHelpString, "oCounterValue: %d", oHelpRegister);
        wmLog(eDebug, "%s\n", oHelpString);
    }
    else if (m_oEncoderDevice == eAdvantechPCI1784)
    {
        uint32_t oRegister32 = 0;

        // set Counter0 Mode: X4, with input filter, reset value is 0, software latch is enabled
        oRegister32 = 0x00000193;
        PreciEncoder_write_dword(m_oPreciEncoderDriverFd, PCI1784_COUNTER0_MODE, oRegister32);
        // set Interrupt Control: no interrupts
        oRegister32 = 0x00000000;
        PreciEncoder_write_dword(m_oPreciEncoderDriverFd, PCI1784_INTERUPT_CTRL_STATUS, oRegister32);
        // set Clock Control: 1 MHz filter frequency, timer divider 1, 50 Hz timer frequency
        oRegister32 = 0x03010003;
        PreciEncoder_write_dword(m_oPreciEncoderDriverFd, PCI1784_CLOCK_CONTROL, oRegister32);
        // reset Counter0
        oRegister32 = 0x00000001;
        PreciEncoder_write_dword(m_oPreciEncoderDriverFd, PCI1784_RESET_COUNTER, oRegister32);
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.NoSuitableEncoder", "No suitable encoder board found\n");
    }
}

uint32_t EncoderAccess::readEncoder (void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return 0;
    }

    uint32_t oCounter = 0;

    if (m_oEncoderDevice == eJanztecPCIINC)
    {
        uint8_t oRegister0;
        uint8_t oRegister1;
        uint8_t oRegister2;

        // low byte must be read first
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT0_REG, &oRegister0);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT1_REG, &oRegister1);
        PreciEncoder_read_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT2_REG, &oRegister2);
        oCounter = (uint32_t)oRegister0
                | (uint32_t)(oRegister1 << 8)
                | (uint32_t)(oRegister2 << 16);
    }
    else if (m_oEncoderDevice == eAdvantechPCI1784)
    {
        // counter to latch of Counter0
        uint32_t oRegister32 = 0x00000001;
        PreciEncoder_write_dword(m_oPreciEncoderDriverFd, PCI1784_SOFTWARE_LATCH, oRegister32);
        // read latch register of Counter0
        PreciEncoder_read_dword(m_oPreciEncoderDriverFd, PCI1784_COUNTER0_MODE, &oCounter);
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.NoSuitableEncoder", "No suitable encoder board found\n");
    }

    return oCounter;
}

void EncoderAccess::setEncoder (uint32_t p_oValue)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    if (m_oEncoderDevice == eJanztecPCIINC)
    {
        writeEncCompare(p_oValue);
        // X4 mode, Input enable, Preset to Counter
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT_CTRL_REG, (PCIINC_ENC_CTRL_TRANS | PCIINC_ENC_CTRL_ENI | PCIINC_ENC_CTRL_QUAT_X4));
    }
    else if (m_oEncoderDevice == eAdvantechPCI1784)
    {
        // function is missing at Advantech PCI1784 ?
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.NoSuitableEncoder", "No suitable encoder board found\n");
    }
}

void EncoderAccess::writeEncCompare (uint32_t p_oValue)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    if (m_oEncoderDevice == eJanztecPCIINC)
    {
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT0_PRE_REG, (unsigned char)(p_oValue & 0xFF));
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT1_PRE_REG, (unsigned char)((p_oValue & 0xFF00) >> 8));
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_COUNT2_PRE_REG, (unsigned char)((p_oValue & 0xFF0000) >> 16));
    }
    else if (m_oEncoderDevice == eAdvantechPCI1784)
    {
        PreciEncoder_write_dword(m_oPreciEncoderDriverFd, PCI1784_COUNTER0_COMPARE, p_oValue);
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.NoSuitableEncoder", "No suitable encoder board found\n");
    }
}

void EncoderAccess::writeEncDivider (uint32_t p_oValue)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    PreciEncoder_write_encoder_divider(m_oPreciEncoderDriverFd, p_oValue);
}

//***************************************************************************
//* init Interrupt for Receiving Encoder Events                             *
//***************************************************************************

void EncoderAccess::startEncInterrupt(void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    // enable IRQ
    IRQCounter = 0;

    if (m_oEncoderDevice == eJanztecPCIINC)
    {
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_SETIRQ_MASK_REG, PCIINC_ENC_IRQ_EIM);
    }
    else if (m_oEncoderDevice == eAdvantechPCI1784)
    {
        // TODO, not used in Trigger.cpp
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.NoSuitableEncoder", "No suitable encoder board found\n");
    }
}

void EncoderAccess::start_encoder_interrupt(void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    IRQCounter = 0;
    PreciEncoder_start_encoder_interrupt(m_oPreciEncoderDriverFd);
}

void EncoderAccess::stopEncInterrupt(void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    // clear IRQ
    if (m_oEncoderDevice == eJanztecPCIINC)
    {
        PreciEncoder_write_byte(m_oPreciEncoderDriverFd, PCIINC_ENC_CLEARMASK_STAT_REG, PCIINC_ENC_IRQ_EIS);
    }
    else if (m_oEncoderDevice == eAdvantechPCI1784)
    {
        // TODO, not used in Trigger.cpp
    }
    else
    {
        wmLogTr(eError, "QnxMsg.VI.NoSuitableEncoder", "No suitable encoder board found\n");
    }

    wmLog(eDebug, "stopEncInterrupt: IRQCounter: %d\n", IRQCounter);
}

void EncoderAccess::stop_encoder_interrupt(void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    PreciEncoder_stop_encoder_interrupt(m_oPreciEncoderDriverFd);

    wmLog(eDebug, "stop_encoder_interrupt: IRQCounter: %d\n", IRQCounter);
}

void EncoderAccess::test (void)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    struct PreciEncoder_info_struct oBoardInfo;
    if (PreciEncoder_read_board_info  (m_oPreciEncoderDriverFd, &oBoardInfo))
    {
        wmLog(eDebug, "EncoderAccess: Failed to read board info: %s\n", strerror(errno));
        wmFatal(eAxis, "QnxMsg.VI.EncoderAccFault", "Problem with access to encoder device ! %s\n", "(001)");
    }
    char oHelpStrg[81];
    sprintf(oHelpStrg, "VendorID: %04X", oBoardInfo.vendor_id);
    wmLog(eDebug, "%s\n", oHelpStrg);
    sprintf(oHelpStrg, "DeviceID: %04X", oBoardInfo.device_id);
    wmLog(eDebug, "%s\n", oHelpStrg);
    sprintf(oHelpStrg, "irq_ctr:  %d",   oBoardInfo.intStatus.PreciEncoder_irq_ctr);
    wmLog(eDebug, "%s\n", oHelpStrg);
}

void EncoderAccess::IsrCallback (uint32_t p_oEncoderCounter)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    if (m_pIRQCallbackFunction)
    {
        m_pIRQCallbackFunction(p_oEncoderCounter);
    }
    IRQCounter++;
}

void EncoderAccess::connectCallback(IRQCallbackFunction p_pIRQCallbackFunction)
{
    if (m_oPreciEncoderDriverFd < 0)
    {
        return;
    }

    m_pIRQCallbackFunction = p_pIRQCallbackFunction;
}


//***************************************************************************
//*       Interrupt-Service-Routine                                         *
//***************************************************************************

// Thread Funktion muss ausserhalb der Klasse sein
void *EncoderAccessThread(void *p_pArg)
{
    struct DataToEncoderAccessThread *pDataToEncoderAccessThread;
    EncoderAccess *pEncoderAccess;
    int *pPreciEncoderDriverFd;
    uint32_t oEncoderCounter;

    pDataToEncoderAccessThread = static_cast<struct DataToEncoderAccessThread *>(p_pArg);
    pEncoderAccess = pDataToEncoderAccessThread->m_pEncoderAccess;
    pPreciEncoderDriverFd = pDataToEncoderAccessThread->m_pPreciEncoderDriverFd;

    wmLog(eDebug, "EncoderAccessThread is started\n");

    pthread_t oMyPthread_ID = pthread_self();

    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(2, &cpuset);
    if (pthread_setaffinity_np(oMyPthread_ID, sizeof(cpuset), &cpuset) != 0)
    {
        wmLog(eDebug, "EncoderAccessThread: cannot set cpu affinity: %s\n", strerror(errno));
        wmFatal(eAxis, "QnxMsg.VI.InitEncoderFault", "Problem while initializing encoder access ! %s\n", "(013)");
    }

    // same priority as ethercat cyclic task
    system::makeThreadRealTime(system::Priority::FieldBusCyclicTask);

    while(true)
    {
        if (*pPreciEncoderDriverFd >= 0)
        {
            int oRetVal;
            oRetVal = read(*pPreciEncoderDriverFd, &oEncoderCounter, sizeof(oEncoderCounter));
            if (oRetVal == 0)
            {
                wmLog(eDebug, "-----> RetValue of read is 0 ! \n");
            }
            else if (oRetVal < 0)
            {
                wmLog(eDebug, "EncoderAccess: read failed: %s\n", strerror(errno));
                wmFatal(eAxis, "QnxMsg.VI.EncoderAccFault", "Problem with access to encoder device ! %s\n", "(002)");
            }
            else
            {
                pEncoderAccess->IsrCallback(oEncoderCounter);
            }
        }
        else
        {
            usleep(100*1000);
        }
    }

    return NULL;
}

} // namespace trigger

} // namespace precitec

