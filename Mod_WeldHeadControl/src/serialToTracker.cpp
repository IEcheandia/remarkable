/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Alexander Egger (EA)
 * 	@date		2013
 * 	@brief		Communicates via serial communication with the ScanTracker
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <iostream>
#include <iomanip>
#include <ios>
#include <string>
#include <cmath>
#include <sys/prctl.h>

#include "common/systemConfiguration.h"

#include "module/moduleLogger.h"
#include "viWeldHead/serialToTracker.h"

#include "PreciRS422.h"
#include "viWeldHead/libPreciRS422Driver.h"
#include "system/realTimeSupport.h"

namespace precitec
{

namespace hardware
{

using namespace interface;

#define DEBUG_SERIAL_INOUT   0

// Thread Funktion muss ausserhalb der Klasse sein
void* SerialBasicsThread(void *p_pArg);

// Thread Funktion muss ausserhalb der Klasse sein
void* SerialToTrackerThread(void *p_pArg);

//***************************************************************************
//***************************************************************************
//* class SerialBasics                                                      *
//***************************************************************************
//***************************************************************************

SerialBasics::SerialBasics(int p_oSerialCommType, int p_oSerialCommPort):
		m_oPreciRS422DriverFd(0),
		m_oSerialCommType(p_oSerialCommType),
		m_oSerialCommPort(p_oSerialCommPort),
		m_oSerialBase(eADR_COM1),
		m_oSerialIRQ(eIRQ_COM1)
{
	std::cout << "SerialBasics::SerialBasics(int,int)" << std::endl;

	m_oSerialInQueue.m_oWriteIndex = 0;
	m_oSerialInQueue.m_oReadIndex = 0;
	m_oSerialInQueue.m_oBufferLimit = SERIAL_QUEUE_LEN;
	m_oSerialInQueue.m_oAnzSerial = 0;

	m_oSerialOutQueue.m_oWriteIndex = 0;
	m_oSerialOutQueue.m_oReadIndex = 0;
	m_oSerialOutQueue.m_oBufferLimit = SERIAL_QUEUE_LEN;
	m_oSerialOutQueue.m_oAnzSerial = 0;

	m_oThread = 0;

	if (m_oSerialCommType == 1) // interne COM Ports
	{
		if (m_oSerialCommPort == 1)
		{
			m_oSerialBase = eADR_COM1;
			m_oSerialIRQ = eIRQ_COM1;
		}
		else if (m_oSerialCommPort == 2)
		{
			m_oSerialBase = eADR_COM2;
			m_oSerialIRQ = eIRQ_COM2;
		}
		else
		{
			// falscher SerialCommPort
		}
	}
	else if (m_oSerialCommType == 2) // PCI SerialControllerBoard
	{
	}
	else
	{
		// falscher SerialCommType
	}

	if( (m_oPreciRS422DriverFd = open("/dev/PreciRS422Driver0", O_RDWR)) < 0)
	{
        wmLog(eDebug, "open %s failed!  error is: %s\n", "/dev/PreciRS422Driver0", strerror(errno));
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "001");
	}

    m_oDataToSerialBasicsThread.m_pSerialInQueue = &m_oSerialInQueue;
    m_oDataToSerialBasicsThread.m_pPreciRS422DriverFd = &m_oPreciRS422DriverFd;

	pthread_attr_t oPthreadAttr;

    pthread_attr_init(&oPthreadAttr);
    if (pthread_create(&m_oThread, &oPthreadAttr, &SerialBasicsThread, &m_oDataToSerialBasicsThread) != 0)
    {
        wmLog(eDebug, "was not able to create thread\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "002");
    }
}

SerialBasics::~SerialBasics()
{
    if (pthread_cancel(m_oThread) != 0)
    {
        wmLog(eDebug, "was not able to abort thread\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "003");
    }

	close(m_oPreciRS422DriverFd);
}

void SerialBasics::setComPort (ComPortAddr p_oPortAddr, ComPortIRQ p_oPortIRQ)
{
	m_oSerialBase = p_oPortAddr;
	m_oSerialIRQ = p_oPortIRQ;
}

void SerialBasics::initComParameter(float p_oBaud, int p_oDBits, int p_oSBits, char p_oPar)
{
    // folgendes siehe Datenblatt EXAR XR17V352
    float oRequiredDivisor = (125000000 / 1) / (p_oBaud * 16);
    unsigned int oDLM = ((unsigned int)(oRequiredDivisor)) >> 8;
    unsigned int oDLL = ((unsigned int)(oRequiredDivisor)) & 0xFF;
    float oHelpFloat = (oRequiredDivisor - trunc(oRequiredDivisor)) * 16;
    unsigned int oDLD = round(oHelpFloat);
printf("Baudrate Register: %02X, %02X, %02X\n", oDLM, oDLL, oDLD);

	// FIFO abschalten
	PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_FIFO_CONTR_REG, 0x00);

	// Baudraten Eingabe aktivieren
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_CONTR_REG, 0x80);

    // Baudraten Eingabe
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_DIVISOR_LSB_REG, oDLL);
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_DIVISOR_MSB_REG, oDLM);
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_DIVISOR_FRAC_REG, oDLD);

    unsigned char oDummy;
    switch (p_oDBits)
    {
        case 5  : oDummy = 0; break;                                 // 5 Datenbits
        case 6  : oDummy = 1; break;                                 // 6 Datenbits
        case 7  : oDummy = 2; break;                                 // 7 Datenbits
        default : oDummy = 3; break;                                 // sonst 8 Datenbits
    }
    if (p_oSBits == 2)                                               // 2 Stopbits
        oDummy = (oDummy | static_cast<unsigned char>(0x04));
    switch (p_oPar)
    {
        case 'o':
        case 'O': oDummy = (oDummy | static_cast<unsigned char>(0x08)); break; // ungerade Paritaet
        case 'e':
        case 'E': oDummy = (oDummy | static_cast<unsigned char>(0x18)); break; // gerade Paritaet
    }
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_CONTR_REG, oDummy); // inkl. Baudraten Eingabe deaktivieren

    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_INT_ENABLE_REG, 0x00);
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_MODEM_CONTR_REG, 0x00); // SENDEN NACH AUSSEN !!!!!!!!!!!!!!!!!!
//    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_MODEM_CONTR_REG, 0x10); // LOOPBACK AKTIV !!!!!!!!!!!!!!!!!!!

	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_RECEIVE_REG, &oDummy);
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_RECEIVE_REG, &oDummy);
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_RECEIVE_REG, &oDummy);
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_RECEIVE_REG, &oDummy);
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_RECEIVE_REG, &oDummy);
}

void SerialBasics::initBaudrate(float p_oBaud)
{
    // folgendes siehe Datenblatt EXAR XR17V352
    float oRequiredDivisor = (125000000 / 1) / (p_oBaud * 16);
    unsigned int oDLM = ((unsigned int)(oRequiredDivisor)) >> 8;
    unsigned int oDLL = ((unsigned int)(oRequiredDivisor)) & 0xFF;
    float oHelpFloat = (oRequiredDivisor - trunc(oRequiredDivisor)) * 16;
    unsigned int oDLD = round(oHelpFloat);
printf("Baudrate Register: %02X, %02X, %02X\n", oDLM, oDLL, oDLD);

	unsigned char oRegister;
	// Baudraten Eingabe aktivieren
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_CONTR_REG, &oRegister);
	oRegister |= 0x80;
	PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_CONTR_REG, oRegister);

    // Baudraten Eingabe
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_DIVISOR_LSB_REG, oDLL);
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_DIVISOR_MSB_REG, oDLM);
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_DIVISOR_FRAC_REG, oDLD);

	// Baudraten Eingabe deaktivieren
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_CONTR_REG, &oRegister);
	oRegister &= ~0x80;
	PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_CONTR_REG, oRegister);
}

unsigned char SerialBasics::getComStatus(void)
{
	unsigned char oStatus;
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_STATUS_REG, &oStatus);
    return static_cast<unsigned char>(oStatus);         // Leitungsstatus-Register
}

unsigned char SerialBasics::getComChar(void)
{
	unsigned char oRegister;
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_RECEIVE_REG, &oRegister);
    return static_cast<unsigned char>(oRegister);           // Empfangs-Register
}

void SerialBasics::sendCharCom(unsigned char p_oSendASCII)
{
    unsigned char oStatus;

	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_STATUS_REG, &oStatus);
    while ((oStatus & 0x40) != 0x40)
    {
    	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_LINE_STATUS_REG, &oStatus);
    }
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_TRANSMIT_REG, p_oSendASCII);
}

//***************************************************************************
//* init Interrupt for Receiving COM characters                             *
//***************************************************************************

void SerialBasics::startCOMInterrupt(void)
{
	unsigned char oRegister;
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_INT_ENABLE_REG, &oRegister);
	oRegister |= 0x01;
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_INT_ENABLE_REG, oRegister);

	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_MODEM_CONTR_REG, &oRegister);
	oRegister |= 0x08;
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_MODEM_CONTR_REG, oRegister);
}

void SerialBasics::stopCOMInterrupt(void)
{
	unsigned char oRegister;
	PreciRS422_read_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_INT_ENABLE_REG, &oRegister);
	oRegister &= ~0x01;
    PreciRS422_write_byte(m_oPreciRS422DriverFd, PRECIRS422_CH1_INT_ENABLE_REG, oRegister);
}

//***************************************************************************
//*       Interrupt-Service-Routine                                         *
//***************************************************************************

// Thread Funktion muss ausserhalb der Klasse sein
void *SerialBasicsThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "SerialBasics");
    system::makeThreadRealTime(system::Priority::Sensors);
	struct DataToSerialBasicsThread *pDataToSerialBasicsThread;
	SerialQueueType *pSerialInQueue;
	int *pPreciRS422DriverFd;
	unsigned char oCharacter;

    pDataToSerialBasicsThread = static_cast<struct DataToSerialBasicsThread *>(p_pArg);
	pSerialInQueue = pDataToSerialBasicsThread->m_pSerialInQueue;
	pPreciRS422DriverFd = pDataToSerialBasicsThread->m_pPreciRS422DriverFd;

	while(true)
	{
		// Neu empfangenes Zeichen lesen und in Eingangspuffer ablegen
		int oRetVal;
		oRetVal = read(*pPreciRS422DriverFd, &oCharacter, sizeof(oCharacter));
		if (oRetVal == 0)
		{
			printf("RetValue of read is 0 ! \n");
		}
		else if (oRetVal < 0)
		{
			printf("RetValue of read shows an error ! \n");
			perror("");
		}
		else
		{
			//printf("<%02X>%c\n", oCharacter, oCharacter);
			pSerialInQueue->m_oBuffer[pSerialInQueue->m_oWriteIndex] = static_cast<unsigned char>(oCharacter);
		}

		unsigned short oIndexCopy = pSerialInQueue->m_oWriteIndex;
		oIndexCopy++;
		if (oIndexCopy >= SERIAL_QUEUE_LEN)
		{
			oIndexCopy = 0;
		}
		if (pSerialInQueue->m_oReadIndex == oIndexCopy)
		{
			// TODO: buffer overrun melden
		}
		pSerialInQueue->m_oWriteIndex = oIndexCopy;
	}

    return NULL;
}

int SerialBasics::readSerialProtocolData (void)
{
    unsigned short oIndexCopy;
    unsigned char oActSerialData;
    int oRetValue;

    oRetValue = -1;
    if (m_oSerialInQueue.m_oReadIndex != m_oSerialInQueue.m_oWriteIndex)
    {
        oActSerialData = m_oSerialInQueue.m_oBuffer[m_oSerialInQueue.m_oReadIndex];

		oRetValue = (int)oActSerialData;

        m_oSerialInQueue.m_oAnzSerial++;

        oIndexCopy = m_oSerialInQueue.m_oReadIndex;
        oIndexCopy++;
        if (oIndexCopy >= SERIAL_QUEUE_LEN)
        {
            oIndexCopy = 0;
        }
        m_oSerialInQueue.m_oReadIndex = oIndexCopy;
    }
    return oRetValue;
}

//***************************************************************************
//* Send Data to Device                                                     *
//***************************************************************************

// collect characters for sending via serial Port

void SerialBasics::collectSerialChar (unsigned char p_oSendChar)
{
    unsigned short oIndexCopy;

    m_oSerialOutQueue.m_oBuffer[m_oSerialOutQueue.m_oWriteIndex] = p_oSendChar;

    oIndexCopy = m_oSerialOutQueue.m_oWriteIndex;
    oIndexCopy++;
    if (oIndexCopy >= SERIAL_QUEUE_LEN)
    {
        oIndexCopy = 0;
    }
    if (m_oSerialOutQueue.m_oReadIndex == oIndexCopy)
    {
    	// TODO: Bufferueberlauf behandeln
        if (oIndexCopy != 0)
        {
            oIndexCopy--;
        }
        else
        {
            oIndexCopy = SERIAL_QUEUE_LEN - 1;
        }
    }
    m_oSerialOutQueue.m_oWriteIndex = oIndexCopy;
}

// send characters via serial Port in case of time to do this

void SerialBasics::xmtSerialChar (void)
{
    unsigned short oIndexCopy;
    unsigned char oSendChar;
    int oLoopCounter;

    oLoopCounter = 0;
    while ((m_oSerialOutQueue.m_oReadIndex != m_oSerialOutQueue.m_oWriteIndex) &&
           (oLoopCounter < 20))
    {
        oLoopCounter++;
        oSendChar = m_oSerialOutQueue.m_oBuffer[m_oSerialOutQueue.m_oReadIndex];
#if DEBUG_SERIAL_INOUT
        if ((oSendChar >= 0x00) && (oSendChar <= 0x20))
        {
            std::printf("[%02X]", oSendChar);
        }
        else if ((oSendChar >= 0x7F) && (oSendChar <= 0xFF))
        {
            std::printf("[%02X]", oSendChar);
        }
        else
        {
            std::printf("[%c]", oSendChar);
        }
#endif
        sendCharCom(oSendChar);
        m_oSerialOutQueue.m_oAnzSerial++;

        oIndexCopy = m_oSerialOutQueue.m_oReadIndex;                    // Zeiger auf naechstes Zeichen
        oIndexCopy++;
        if (oIndexCopy >= SERIAL_QUEUE_LEN)
        {
            oIndexCopy = 0;
        }
        m_oSerialOutQueue.m_oReadIndex = oIndexCopy;
    }
}

void SerialBasics::test (void)
{
	std::cout << "SerialBasics::test" << std::endl;

	struct PreciRS422_info_struct oBoardInfo;
	if (PreciRS422_read_board_info  (m_oPreciRS422DriverFd, &oBoardInfo))
	{
        wmLog(eDebug, "SerialBasics::test: Failed to read board info\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "004");
	}
    char oHelpStrg[81];
    sprintf(oHelpStrg, "SerialBasics::test: VendorID: %04X", oBoardInfo.vendor_id);
    wmLog(eDebug, "%s\n", oHelpStrg);
    sprintf(oHelpStrg, "SerialBasics::test: DeviceID: %04X", oBoardInfo.device_id);
    wmLog(eDebug, "%s\n", oHelpStrg);
    sprintf(oHelpStrg, "SerialBasics::test: irq_ctr:  %d",   oBoardInfo.intStatus.PreciRS422_irq_ctr);
    wmLog(eDebug, "%s\n", oHelpStrg);
}

//***************************************************************************
//***************************************************************************
//* class SerialBasicsDirectOS                                              *
//***************************************************************************
//***************************************************************************

SerialBasicsDirectOS::SerialBasicsDirectOS():
        m_oPreciRS422DriverFd(0)
{
    std::cout << "SerialBasicsDirectOS::SerialBasicsDirectOS()" << std::endl;

    m_oSerialInQueue.m_oWriteIndex = 0;
    m_oSerialInQueue.m_oReadIndex = 0;
    m_oSerialInQueue.m_oBufferLimit = SERIAL_QUEUE_LEN;
    m_oSerialInQueue.m_oAnzSerial = 0;

    m_oSerialOutQueue.m_oWriteIndex = 0;
    m_oSerialOutQueue.m_oReadIndex = 0;
    m_oSerialOutQueue.m_oBufferLimit = SERIAL_QUEUE_LEN;
    m_oSerialOutQueue.m_oAnzSerial = 0;

    m_oThread = 0;

    // if serial port is onboard, which port should be used ?
    std::string oScanTrackerSerialOnboardPort = SystemConfiguration::instance().getString("ScanTrackerSerialOnboardPort", "ttyS2");
    wmLog(eDebug, "oScanTrackerSerialOnboardPort (string): %s\n", oScanTrackerSerialOnboardPort.c_str());

    std::string oDeviceName("/dev/");
    oDeviceName += oScanTrackerSerialOnboardPort;

    wmLog(eDebug, "now open port: %s\n", oDeviceName.c_str());
    if( (m_oPreciRS422DriverFd = open(oDeviceName.c_str(), (O_RDWR | O_NOCTTY))) < 0)
    {
        wmLog(eDebug, "open %s failed!  error is: %s\n", oDeviceName.c_str(), strerror(errno));
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "005");
    }

    m_oDataToSerialBasicsThread.m_pSerialInQueue = &m_oSerialInQueue;
    m_oDataToSerialBasicsThread.m_pPreciRS422DriverFd = &m_oPreciRS422DriverFd;

    pthread_attr_t oPthreadAttr;

    pthread_attr_init(&oPthreadAttr);
    if (pthread_create(&m_oThread, &oPthreadAttr, &SerialBasicsThread, &m_oDataToSerialBasicsThread) != 0)
    {
        wmLog(eDebug, "was not able to create thread\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "006");
    }
}

SerialBasicsDirectOS::~SerialBasicsDirectOS()
{
    if (pthread_cancel(m_oThread) != 0)
    {
        wmLog(eDebug, "was not able to abort thread\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "007");
    }

    int retVal = tcsetattr(m_oPreciRS422DriverFd, TCSANOW, &m_oOldAttributes);
    if (retVal < 0)
    {
        wmLog(eDebug, "SerialBasicsDirectOS: can't set old attributes\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "008");
    }

    close(m_oPreciRS422DriverFd);
}

void SerialBasicsDirectOS::initComConfiguration(void)
{
    int retVal;
    retVal = tcgetattr(m_oPreciRS422DriverFd, &m_oOldAttributes);
    if (retVal < 0)
    {
        wmLog(eDebug, "SerialBasicsDirectOS: can't get port attributes\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "009");
    }

    memcpy(&m_oNewAttributes, &m_oOldAttributes, sizeof(m_oOldAttributes));
    retVal = tcflush(m_oPreciRS422DriverFd, TCIOFLUSH);
    if (retVal < 0)
    {
        wmLog(eDebug, "SerialBasicsDirectOS: can't flush port\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "010");
    }
    m_oNewAttributes.c_iflag = 0;
    m_oNewAttributes.c_oflag = 0;
    m_oNewAttributes.c_cflag = 0;
    m_oNewAttributes.c_cflag |= (CLOCAL | CREAD);
    m_oNewAttributes.c_cflag &= ~PARENB;
    m_oNewAttributes.c_cflag &= ~CSTOPB;
    m_oNewAttributes.c_cflag &= ~CSIZE;
    m_oNewAttributes.c_cflag |= CS8;
    m_oNewAttributes.c_cflag &= ~CRTSCTS;
    retVal = cfsetispeed(&m_oNewAttributes, B57600);
    if (retVal < 0)
    {
        wmLog(eDebug, "SerialBasicsDirectOS: can't set input baudrate\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "011");
    }
    retVal = cfsetospeed(&m_oNewAttributes, B57600);
    if (retVal < 0)
    {
        wmLog(eDebug, "SerialBasicsDirectOS: can't set output baudrate\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "012");
    }
    m_oNewAttributes.c_lflag = 0;
    m_oNewAttributes.c_lflag &= ~(ICANON);
    m_oNewAttributes.c_cc[VMIN] = 1;
    m_oNewAttributes.c_cc[VTIME] = 0;
    cfmakeraw(&m_oNewAttributes);

    retVal = tcsetattr(m_oPreciRS422DriverFd, TCSANOW, &m_oNewAttributes);
    if (retVal < 0)
    {
        wmLog(eDebug, "SerialBasicsDirectOS: can't set new attributes\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "013");
    }

    wmLog(eDebug, "SerialBasicsDirectOS: Init ok\n");
}

int SerialBasicsDirectOS::readSerialProtocolData (void)
{
    unsigned short oIndexCopy;
    unsigned char oActSerialData;
    int oRetValue;

    oRetValue = -1;
    if (m_oSerialInQueue.m_oReadIndex != m_oSerialInQueue.m_oWriteIndex)
    {
        oActSerialData = m_oSerialInQueue.m_oBuffer[m_oSerialInQueue.m_oReadIndex];

        oRetValue = (int)oActSerialData;

        m_oSerialInQueue.m_oAnzSerial++;

        oIndexCopy = m_oSerialInQueue.m_oReadIndex;
        oIndexCopy++;
        if (oIndexCopy >= SERIAL_QUEUE_LEN)
        {
            oIndexCopy = 0;
        }
        m_oSerialInQueue.m_oReadIndex = oIndexCopy;
    }
    return oRetValue;
}

//***************************************************************************
//* Send Data to Device                                                     *
//***************************************************************************

// collect characters for sending via serial Port

void SerialBasicsDirectOS::collectSerialChar (unsigned char p_oSendChar)
{
    unsigned short oIndexCopy;

    m_oSerialOutQueue.m_oBuffer[m_oSerialOutQueue.m_oWriteIndex] = p_oSendChar;

    oIndexCopy = m_oSerialOutQueue.m_oWriteIndex;
    oIndexCopy++;
    if (oIndexCopy >= SERIAL_QUEUE_LEN)
    {
        oIndexCopy = 0;
    }
    if (m_oSerialOutQueue.m_oReadIndex == oIndexCopy)
    {
        // TODO: Bufferueberlauf behandeln
        if (oIndexCopy != 0)
        {
            oIndexCopy--;
        }
        else
        {
            oIndexCopy = SERIAL_QUEUE_LEN - 1;
        }
    }
    m_oSerialOutQueue.m_oWriteIndex = oIndexCopy;
}

// send characters via serial Port in case of time to do this

void SerialBasicsDirectOS::xmtSerialChar (void)
{
    unsigned short oIndexCopy;
    unsigned char oSendChar;
    int oLoopCounter;

    oLoopCounter = 0;
    while ((m_oSerialOutQueue.m_oReadIndex != m_oSerialOutQueue.m_oWriteIndex) &&
           (oLoopCounter < 20))
    {
        oLoopCounter++;
        oSendChar = m_oSerialOutQueue.m_oBuffer[m_oSerialOutQueue.m_oReadIndex];
#if DEBUG_SERIAL_INOUT
        if ((oSendChar >= 0x00) && (oSendChar <= 0x20))
        {
            std::printf("[%02X]", oSendChar);
        }
        else if ((oSendChar >= 0x7F) && (oSendChar <= 0xFF))
        {
            std::printf("[%02X]", oSendChar);
        }
        else
        {
            std::printf("[%c]", oSendChar);
        }
#endif
        int retVal = write(m_oPreciRS422DriverFd, &oSendChar, sizeof(oSendChar));
        if (retVal != 1)
        {
            wmLog(eDebug, "SerialBasicsDirectOS: error in write: %d (%s)\n", retVal, strerror(errno));
            wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "014");
        }
        m_oSerialOutQueue.m_oAnzSerial++;

        oIndexCopy = m_oSerialOutQueue.m_oReadIndex;                    // Zeiger auf naechstes Zeichen
        oIndexCopy++;
        if (oIndexCopy >= SERIAL_QUEUE_LEN)
        {
            oIndexCopy = 0;
        }
        m_oSerialOutQueue.m_oReadIndex = oIndexCopy;
    }
}

void SerialBasicsDirectOS::test (void)
{
    std::cout << "SerialBasicsDirectOS::test" << std::endl;
}

//*****************************************************************************
//*****************************************************************************
//*****************************************************************************

SerialToTracker::SerialToTracker(int p_oSerialCommType, int p_oSerialCommPort):
        m_oSerialCommType(p_oSerialCommType),
        m_oSerialCommPort(p_oSerialCommPort),
        m_pSerialBasicsInterface(nullptr),
        m_pSerialBasics(nullptr),
        m_pSerialBasicsDirectOS(nullptr),
        m_oIsWaitingForAnswer(false),
        m_oAnswerTimeoutCounter(0)
{
    std::cout << "SerialToTracker::SerialToTracker(int,int)" << std::endl;

    // is serial port on external board or onboard ?
    m_oScanTrackerSerialViaOnboardPort = SystemConfiguration::instance().getBool("ScanTrackerSerialViaOnboardPort", false);
    wmLog(eDebug, "m_oScanTrackerSerialViaOnboardPort (bool): %d\n", m_oScanTrackerSerialViaOnboardPort);

    if(!m_oScanTrackerSerialViaOnboardPort)
    {
        m_pSerialBasics = new SerialBasics(m_oSerialCommType, m_oSerialCommPort);

        if (m_pSerialBasics != nullptr)
        {
            m_pSerialBasics->initComParameter(57600.0, 8, 1, 'N');
            m_pSerialBasics->startCOMInterrupt();
            m_pSerialBasicsInterface = m_pSerialBasics;
        }
    }
    else
    {
        m_pSerialBasicsDirectOS = new SerialBasicsDirectOS();

        if (m_pSerialBasicsDirectOS != nullptr)
        {
            m_pSerialBasicsDirectOS->initComConfiguration();
            m_pSerialBasicsInterface = m_pSerialBasicsDirectOS;
        }
    }
    m_oThread = 0;
}

SerialToTracker::~SerialToTracker()
{
    if(!m_oScanTrackerSerialViaOnboardPort)
    {
        if (m_pSerialBasics != nullptr)
        {
            m_pSerialBasics->stopCOMInterrupt();
        }
    }

    if(m_pSerialBasicsInterface != nullptr)
    {
        delete m_pSerialBasicsInterface;
    }
}

void SerialToTracker::run (void)
{
    m_oDataToSerialToTrackerThread.m_pSerialBasicsInterface = m_pSerialBasicsInterface;
    m_oDataToSerialToTrackerThread.m_pSerialToTracker = this;

    pthread_attr_t oPthreadAttr;

    pthread_attr_init(&oPthreadAttr);
    if (pthread_create(&m_oThread, &oPthreadAttr, &SerialToTrackerThread, &m_oDataToSerialToTrackerThread) != 0)
    {
        wmLog(eDebug, "was not able to create thread\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "015");
    }
}

void SerialToTracker::stop (void)
{
    if (pthread_cancel(m_oThread) != 0)
    {
        wmLog(eDebug, "was not able to abort thread\n");
        wmFatal(eAxis, "QnxMsg.VI.TrackerSerialFault", "Problem while initializing serial communication (%s)\n", "016");
    }
}

void SerialToTracker::setTrackerFrequencyStep(TrackerFrequencyStep p_oFreq)
{
	std::string oSendStrg("set");

	switch (p_oFreq)
	{
		case eFreq30:
			oSendStrg += " /0";
			break;
		case eFreq40:
			oSendStrg += " /1";
			break;
		case eFreq50:
			oSendStrg += " /2";
			break;
		case eFreq100:
			oSendStrg += " /3";
			break;
		case eFreq150:
			oSendStrg += " /4";
			break;
		case eFreq200:
			oSendStrg += " /5";
			break;
		case eFreq250:
			oSendStrg += " /6";
			break;
		case eFreq300:
			oSendStrg += " /7";
			break;
		case eFreq350:
			oSendStrg += " /8";
			break;
		case eFreq400:
			oSendStrg += " /9";
			break;
		case eFreq450:
			oSendStrg += " /a";
			break;
		case eFreq500:
			oSendStrg += " /b";
			break;
		case eFreq550:
			oSendStrg += " /c";
			break;
		case eFreq600:
			oSendStrg += " /d";
			break;
		case eFreq650:
			oSendStrg += " /e";
			break;
		case eFreq700:
			oSendStrg += " /f";
			break;
		case eFreq750:
			oSendStrg += " /g";
			break;
		default:
			break;
	}

	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::setTrackerFrequencyCont(unsigned short p_oFreq)
{
	std::string oSendStrg("setf ");

	char oDummyStrg[10];
	sprintf(oDummyStrg, "%03d", p_oFreq);
	oSendStrg += std::string(oDummyStrg);

	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::enableTracker(TrackerMode p_oMode)
{
	std::string oSendStrg("en");

	switch(p_oMode)
	{
		case eOff:
			oSendStrg += " /0";
			break;
		case eOn:
			oSendStrg += " /1";
			break;
		default:
			break;
	}

	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::trackerCmdShow(void)
{
	std::string oSendStrg("show");
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::trackerCmdRevisions()
{
	std::string oSendStrg("mod");
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
	usleep(20 * 1000);

	oSendStrg = "hw";
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
	usleep(20 * 1000);

	oSendStrg = "fw";
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
	usleep(20 * 1000);

	oSendStrg = "buno";
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::trackerCmdShowSerial()
{
	std::string oSendStrg("show /s");
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::trackerCmdLimitTable()
{
	std::string oSendStrg("ll");
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::trackerCmdHelp(void)
{
	std::string oSendStrg("help");
	oSendStrg += 0x0D;

	m_oAnswerTimeoutCounter = 0;
	m_oIsWaitingForAnswer = true;
	for(unsigned int i = 0;i < oSendStrg.length();i++)
	{
        if (m_pSerialBasicsInterface != nullptr)
        {
            m_pSerialBasicsInterface->collectSerialChar(static_cast<unsigned char>(oSendStrg[i]));
        }
	}
}

void SerialToTracker::test (void)
{
	std::cout << "SerialToTracker::test" << std::endl;
    if (m_pSerialBasicsInterface != nullptr)
    {
        m_pSerialBasicsInterface->test();
    }
}

#define DEBUG_TRACKER_INPUTS 0

// Thread Funktion muss ausserhalb der Klasse sein
void *SerialToTrackerThread(void *p_pArg)
{
    prctl(PR_SET_NAME, "SerialToTracker");
    system::makeThreadRealTime(system::Priority::Sensors);
	struct DataToSerialToTrackerThread* pDataToSerialToTrackerThread;
	SerialBasicsInterface* pSerialBasicsInterface;
	SerialToTracker* pSerialToTracker;
	std::string oLoggerStrg("");

    pDataToSerialToTrackerThread = static_cast<struct DataToSerialToTrackerThread *>(p_pArg);
	pSerialBasicsInterface = pDataToSerialToTrackerThread->m_pSerialBasicsInterface;
	pSerialToTracker = pDataToSerialToTrackerThread->m_pSerialToTracker;

	int oInputValue = 0x00;;
	while(true)
	{
		usleep(3 * 1000);

		// empfangene Zeichen verarbeiten
        if (pSerialBasicsInterface != nullptr)
        {
            oInputValue = pSerialBasicsInterface->readSerialProtocolData();
        }
		if (oInputValue != -1)
		{
#if DEBUG_SERIAL_INOUT
			if (oInputValue <= 0x20)
			{
				std::printf("<%02X>", oInputValue);
			}
			else if (oInputValue >= 0x7F)
			{
				std::printf("<%02X>", oInputValue);
			}
			else
			{
				std::printf("<%c>", (oInputValue & 0xFF));
			}
#endif
			// Steuerzeichen vom Tracker
			if (oInputValue < 0x20)
			{
				// Zeilenende-Zeichen vom Tracker
				if (oInputValue == 0x0A)
				{
					oLoggerStrg += '\n';
					if ((oLoggerStrg.find("mod") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos)
																	   && (oLoggerStrg.find("model") == std::string::npos))
					{
						oLoggerStrg="Product model no.:  ";
					}
					else if ((oLoggerStrg.find("hw") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos))
					{
						oLoggerStrg="Hardware revision:  ";
					}
					else if ((oLoggerStrg.find("fw") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos))
					{
						oLoggerStrg="Firmware revision:  ";
					}
					else if ((oLoggerStrg.find("buno") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos))
					{
						oLoggerStrg="Firmware build no.: ";
					}
					else if ((oLoggerStrg.find("show") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos))
					{
						oLoggerStrg.clear();
					}
					else if ((oLoggerStrg.find("set /") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos))
					{
						oLoggerStrg.clear();
					}
					else if ((oLoggerStrg.find("ll") != std::string::npos) && (oLoggerStrg.find("..") == std::string::npos))
					{
						oLoggerStrg.clear();
					}
					else if (oLoggerStrg.find("help") != std::string::npos)
					{
						oLoggerStrg.clear();
					}
					else if (oLoggerStrg.find("Wobbler Settings") != std::string::npos)
					{
						oLoggerStrg.clear();
					}
					else if (oLoggerStrg.find("Terminal") != std::string::npos)
					{
						oLoggerStrg.clear();
					}
					else if (oLoggerStrg.find("Galvo drive") != std::string::npos) // Nach dieser Meldung Leerzeile einfuegen
					{
						wmLog(eTracker, oLoggerStrg.c_str());
						wmLog(eTracker, " \n");
#if DEBUG_TRACKER_INPUTS
						std::cout << std::endl;
						std::cout << std::endl;
#endif
						oLoggerStrg.clear();
					}
					else if (oLoggerStrg.find("Firmware build no") != std::string::npos) // nach dieser Meldung Leerzeile einfuegen
					{
						wmLog(eTracker, oLoggerStrg.c_str());
						wmLog(eTracker, " \n");
#if DEBUG_TRACKER_INPUTS
						std::cout << std::endl;
						std::cout << std::endl;
#endif
						oLoggerStrg.clear();
					}
					else
					{
						wmLog(eTracker, oLoggerStrg.c_str());
#if DEBUG_TRACKER_INPUTS
						std::cout << std::endl;
#endif
						oLoggerStrg.clear();
					}

					pSerialToTracker->setIsWaitingForAnswer(false);
					pSerialToTracker->setAnswerTimeoutCounter(0);
				}
			}
			else
			{
				oLoggerStrg += static_cast<char>(oInputValue);
				if (static_cast<char>(oInputValue) == '%') oLoggerStrg += '%';
#if DEBUG_TRACKER_INPUTS
			    std::cout << static_cast<char>(oInputValue);
#endif
			}
		}

		if (pSerialToTracker->getIsWaitingForAnswer())
		{
			pSerialToTracker->incAnswerTimeoutCounter();
			if (pSerialToTracker->getAnswerTimeoutCounter() >= 400)
			{
				wmFatal(eAxis, "QnxMsg.VI.STRS422Timeout", "Timeout in communication with scantracker via serial interface !\n");
				pSerialToTracker->setIsWaitingForAnswer(false);
				pSerialToTracker->setAnswerTimeoutCounter(0);
			}
		}

		// zu sendende Zeichen auf Port schreiben
        if (pSerialBasicsInterface != nullptr)
        {
            pSerialBasicsInterface->xmtSerialChar();
        }
	}

    return NULL;
}


} // namespace hardware

} // namespace precitec

