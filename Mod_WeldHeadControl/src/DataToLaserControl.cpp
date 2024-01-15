/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Alexander Egger (EA)
 * 	@date		2014
 * 	@brief		Communicates via TCP/IP communication with LaserControl device
 */

#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <fcntl.h>
#include <system/timer.h>

#include "module/moduleLogger.h"
#include "viWeldHead/DataToLaserControl.h"

namespace precitec
{

namespace hardware
{

DataToLaserControl::DataToLaserControl()
{
	std::cout << "DataToLaserControl::DataToLaserControl()" << std::endl;

	SERVER_NAME_LASERCONTROL = "LASERCONTROL";
	m_oTcpConnIsOpen = false;
	m_oTcpOpeningIsActive = false;
	m_oErrorState = false;
	m_oInspectCycleIsOn = false;

	m_oLCDataBlock.variable1 = 0;
    m_oLCDataBlock.variable2 = 0;
    m_oLCDataBlock.variable3 = 0;
    m_oLCDataBlock.variable4 = 0;
    m_oLCDataBlock.variable5 = 0;
    m_oLCDataBlock.variable6 = 0;
    m_oLCDataBlock.variable7 = 0;
    m_oLCDataBlock.variable8 = 0;
    m_oLCDataBlock.variable9 = 0;
    m_oLCDataBlock.variable10 = 0;
    m_oLCDataBlock.variable11 = 0;
    m_oLCDataBlock.variable12 = 0;
    m_oLCDataBlock.variable13 = 0;
    m_oLCDataBlock.variable14 = 0;
    m_oLCDataBlock.variable15 = 0;
    m_oLCDataBlock.variable16 = 0;
    m_oLCDataBlock.variable17 = 0;
    m_oLCDataBlock.variable18 = 0;
    m_oLCDataBlock.variable19 = 0;
    m_oLCDataBlock.variable20 = 0;
    m_oLCDataBlock.variable21 = 0;
    m_oLCDataBlock.variable22 = 0;
    m_oLCDataBlock.variable23 = 0;
    m_oLCDataBlock.variable24 = 0;
    m_oLCDataBlock.variable25 = 0;
    m_oLCDataBlock.variable26 = 0;
    m_oLCDataBlock.variable27 = 0;
    m_oLCDataBlock.variable28 = 0;
}

DataToLaserControl::~DataToLaserControl()
{
}

void DataToLaserControl::setDataVariable(LCDataVarIdx p_oVarIdx, signed long p_oVarValue)
{
	switch(p_oVarIdx)
	{
		case eLCVarIdx1:
			m_oLCDataBlock.variable1 = p_oVarValue;
			break;
		case eLCVarIdx2:
			m_oLCDataBlock.variable2 = p_oVarValue;
			break;
		case eLCVarIdx3:
			m_oLCDataBlock.variable3 = p_oVarValue;
			break;
		case eLCVarIdx4:
			m_oLCDataBlock.variable4 = p_oVarValue;
			break;
		case eLCVarIdx5:
			m_oLCDataBlock.variable5 = p_oVarValue;
			break;
		case eLCVarIdx6:
			m_oLCDataBlock.variable6 = p_oVarValue;
			break;
		case eLCVarIdx7:
			m_oLCDataBlock.variable7 = p_oVarValue;
			break;
		case eLCVarIdx8:
			m_oLCDataBlock.variable8 = p_oVarValue;
			break;
		case eLCVarIdx9:
			m_oLCDataBlock.variable9 = p_oVarValue;
			break;
		case eLCVarIdx10:
			m_oLCDataBlock.variable10 = p_oVarValue;
			break;
		case eLCVarIdx11:
			m_oLCDataBlock.variable11 = p_oVarValue;
			break;
		case eLCVarIdx12:
			m_oLCDataBlock.variable12 = p_oVarValue;
			break;
		case eLCVarIdx13:
			m_oLCDataBlock.variable13 = p_oVarValue;
			break;
		case eLCVarIdx14:
			m_oLCDataBlock.variable14 = p_oVarValue;
			break;
		case eLCVarIdx15:
			m_oLCDataBlock.variable15 = p_oVarValue;
			break;
		case eLCVarIdx16:
			m_oLCDataBlock.variable16 = p_oVarValue;
			break;
		case eLCVarIdx17:
			m_oLCDataBlock.variable17 = p_oVarValue;
			break;
		case eLCVarIdx18:
			m_oLCDataBlock.variable18 = p_oVarValue;
			break;
		case eLCVarIdx19:
			m_oLCDataBlock.variable19 = p_oVarValue;
			break;
		case eLCVarIdx20:
			m_oLCDataBlock.variable20 = p_oVarValue;
			break;
		case eLCVarIdx21:
			m_oLCDataBlock.variable21 = p_oVarValue;
			break;
		case eLCVarIdx22:
			m_oLCDataBlock.variable22 = p_oVarValue;
			break;
		case eLCVarIdx23:
			m_oLCDataBlock.variable23 = p_oVarValue;
			break;
		case eLCVarIdx24:
			m_oLCDataBlock.variable24 = p_oVarValue;
			break;
		case eLCVarIdx25:
			m_oLCDataBlock.variable25 = p_oVarValue;
			break;
		case eLCVarIdx26:
			m_oLCDataBlock.variable26 = p_oVarValue;
			break;
		case eLCVarIdx27:
			m_oLCDataBlock.variable27 = p_oVarValue;
			break;
		case eLCVarIdx28:
			m_oLCDataBlock.variable28 = p_oVarValue;
			break;
		default:
			break;
	}
}

signed long DataToLaserControl::getDataVariable(LCDataVarIdx p_oVarIdx)
{
	signed long retValue = 0;

	switch(p_oVarIdx)
	{
		case eLCVarIdx1:
			retValue = m_oLCDataBlock.variable1;
			break;
		case eLCVarIdx2:
			retValue = m_oLCDataBlock.variable2;
			break;
		case eLCVarIdx3:
			retValue = m_oLCDataBlock.variable3;
			break;
		case eLCVarIdx4:
			retValue = m_oLCDataBlock.variable4;
			break;
		case eLCVarIdx5:
			retValue = m_oLCDataBlock.variable5;
			break;
		case eLCVarIdx6:
			retValue = m_oLCDataBlock.variable6;
			break;
		case eLCVarIdx7:
			retValue = m_oLCDataBlock.variable7;
			break;
		case eLCVarIdx8:
			retValue = m_oLCDataBlock.variable8;
			break;
		case eLCVarIdx9:
			retValue = m_oLCDataBlock.variable9;
			break;
		case eLCVarIdx10:
			retValue = m_oLCDataBlock.variable10;
			break;
		case eLCVarIdx11:
			retValue = m_oLCDataBlock.variable11;
			break;
		case eLCVarIdx12:
			retValue = m_oLCDataBlock.variable12;
			break;
		case eLCVarIdx13:
			retValue = m_oLCDataBlock.variable13;
			break;
		case eLCVarIdx14:
			retValue = m_oLCDataBlock.variable14;
			break;
		case eLCVarIdx15:
			retValue = m_oLCDataBlock.variable15;
			break;
		case eLCVarIdx16:
			retValue = m_oLCDataBlock.variable16;
			break;
		case eLCVarIdx17:
			retValue = m_oLCDataBlock.variable17;
			break;
		case eLCVarIdx18:
			retValue = m_oLCDataBlock.variable18;
			break;
		case eLCVarIdx19:
			retValue = m_oLCDataBlock.variable19;
			break;
		case eLCVarIdx20:
			retValue = m_oLCDataBlock.variable20;
			break;
		case eLCVarIdx21:
			retValue = m_oLCDataBlock.variable21;
			break;
		case eLCVarIdx22:
			retValue = m_oLCDataBlock.variable22;
			break;
		case eLCVarIdx23:
			retValue = m_oLCDataBlock.variable23;
			break;
		case eLCVarIdx24:
			retValue = m_oLCDataBlock.variable24;
			break;
		case eLCVarIdx25:
			retValue = m_oLCDataBlock.variable25;
			break;
		case eLCVarIdx26:
			retValue = m_oLCDataBlock.variable26;
			break;
		case eLCVarIdx27:
			retValue = m_oLCDataBlock.variable27;
			break;
		case eLCVarIdx28:
			retValue = m_oLCDataBlock.variable28;
			break;
		default:
			break;
	}

	return retValue;
}

void DataToLaserControl::setInspectCycleIsOn(bool p_oState)
{
	m_oInspectCycleIsOn = p_oState;
}

int DataToLaserControl::openTcpConnectionToLC (void)
{
	char oServerName[81];
	struct hostent *pServerEntry;
	struct sockaddr_in oServerAddr;

	m_oTCP_to_LC_Mutex.lock();
	system::Timer oTimer; oTimer.start();
	wmLog(eDebug, "openTcpConnectionToLC Start\n");

	m_oTcpOpeningIsActive = true;
	m_oErrorState = false;

	// create socket
	m_oSockDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (m_oSockDesc == -1)
	{
		wmLog(eDebug, "openTcpConnectionToLC: Error (001): creating socket\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(001)");
		m_oTcpConnIsOpen = false;
		m_oTcpOpeningIsActive = false;
		m_oErrorState = true;
		m_oTCP_to_LC_Mutex.unlock();
		return -1;
	}

	// determine Server address
	strcpy(oServerName, SERVER_NAME_LASERCONTROL.c_str());
	pServerEntry = gethostbyname(oServerName);
	if (pServerEntry == NULL)
	{
		wmLog(eDebug, "openTcpConnectionToLC: Error (002): getting hostname\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(002)");
		closeTcpConnectionToLC();
		m_oTcpOpeningIsActive = false;
		m_oErrorState = true;
		m_oTCP_to_LC_Mutex.unlock();
		return -1;
	}
	memset((char *)&oServerAddr, 0, sizeof(oServerAddr));
	oServerAddr.sin_family = AF_INET;
	memcpy(&oServerAddr.sin_addr, pServerEntry->h_addr, pServerEntry->h_length);
	oServerAddr.sin_port = htons(TCPSERVER_THREAD_PORT);

	// Set socket to non-blocking mode
	if (setSocketMode (m_oSockDesc, eSetNonBlocking) == -1)
	{
		wmLog(eDebug, "openTcpConnectionToLC: Error (003): setting socket mode\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(003)");
		closeTcpConnectionToLC();
		m_oTcpOpeningIsActive = false;
		m_oErrorState = true;
		m_oTCP_to_LC_Mutex.unlock();
		return -1;
	}

	// connect to Server
	int oConnectResult = connect(m_oSockDesc, (struct sockaddr *)&oServerAddr, sizeof(oServerAddr));
	if (oConnectResult == -1) // Error in connect
	{
		if (errno == EINPROGRESS) // connect is not ready yet
		{
			fd_set oFdset;
			FD_ZERO(&oFdset);
			FD_SET(m_oSockDesc, &oFdset);
			struct timeval oTimeout;
			oTimeout.tv_sec = 1; // Timeout is 1 seconds
			oTimeout.tv_usec = 0;

			int oSelectResult = select(m_oSockDesc + 1, NULL, &oFdset, NULL, &oTimeout);
			switch(oSelectResult)
			{
				case 1: // select successful terminated
					{
						int oSockError;
						socklen_t oLength = sizeof(oSockError);
						if (getsockopt(m_oSockDesc, SOL_SOCKET, SO_ERROR, &oSockError, &oLength) == -1)
						{
							// Set socket to blocking mode
							if (setSocketMode (m_oSockDesc, eSetBlocking) == -1)
							{
								wmLog(eDebug, "openTcpConnectionToLC: Error (004): setting socket mode\n");
								wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(004)");
								closeTcpConnectionToLC();
								m_oTcpOpeningIsActive = false;
								m_oErrorState = true;
								m_oTCP_to_LC_Mutex.unlock();
								return -1;
							}
							wmLog(eDebug, "openTcpConnectionToLC: Error (005): getting socket options\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(005)");
							closeTcpConnectionToLC();
							m_oTcpOpeningIsActive = false;
							m_oErrorState = true;
							m_oTCP_to_LC_Mutex.unlock();
							return -1;
						}
						if (oSockError == 0)
						{
							// successful connected within timeout
						}
						else
						{
							// error, not connected
							// Set socket to blocking mode
							if (setSocketMode (m_oSockDesc, eSetBlocking) == -1)
							{
								wmLog(eDebug, "openTcpConnectionToLC: Error (006): setting socket mode\n");
								wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(006)");
								closeTcpConnectionToLC();
								m_oTcpOpeningIsActive = false;
								m_oErrorState = true;
								m_oTCP_to_LC_Mutex.unlock();
								return -1;
							}
							wmLog(eDebug, "openTcpConnectionToLC: Error (007): after select\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(007)");
							closeTcpConnectionToLC();
							m_oTcpOpeningIsActive = false;
							m_oErrorState = true;
							m_oTCP_to_LC_Mutex.unlock();
							return -1;
						}
					}
					break;
				case 0: // timeout expired
					{
						// Set socket to blocking mode
						if (setSocketMode (m_oSockDesc, eSetBlocking) == -1)
						{
							wmLog(eDebug, "openTcpConnectionToLC: Error (008): setting socket mode\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(008)");
							closeTcpConnectionToLC();
							m_oTcpOpeningIsActive = false;
							m_oErrorState = true;
							m_oTCP_to_LC_Mutex.unlock();
							return -1;
						}
						wmLog(eDebug, "openTcpConnectionToLC: Error (009): timeout while connecting\n");
						wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(009)");
						closeTcpConnectionToLC();
						m_oTcpOpeningIsActive = false;
						m_oErrorState = true;
						m_oTCP_to_LC_Mutex.unlock();
						return -1;
					}
					break;
				case -1: // error in select
					{
						// Set socket to blocking mode
						if (setSocketMode (m_oSockDesc, eSetBlocking) == -1)
						{
							wmLog(eDebug, "openTcpConnectionToLC: Error (010): setting socket mode\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(010)");
							closeTcpConnectionToLC();
							m_oTcpOpeningIsActive = false;
							m_oErrorState = true;
							m_oTCP_to_LC_Mutex.unlock();
							return -1;
						}
						wmLog(eDebug, "openTcpConnectionToLC: Error (011): error select\n");
						wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(011)");
						closeTcpConnectionToLC();
						m_oTcpOpeningIsActive = false;
						m_oErrorState = true;
						m_oTCP_to_LC_Mutex.unlock();
						return -1;
					}
					break;
			}
		}
		else // There is another error
		{
			// cannot connect to server
			// Set socket to blocking mode
			if (setSocketMode (m_oSockDesc, eSetBlocking) == -1)
			{
				wmLog(eDebug, "openTcpConnectionToLC: Error (012): setting socket mode\n");
				wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(012)");
				closeTcpConnectionToLC();
				m_oTcpOpeningIsActive = false;
				m_oErrorState = true;
				m_oTCP_to_LC_Mutex.unlock();
				return -1;
			}
			wmLog(eDebug, "openTcpConnectionToLC: Error (013): error connect\n");
			wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(013)");
			closeTcpConnectionToLC();
			m_oTcpOpeningIsActive = false;
			m_oErrorState = true;
			m_oTCP_to_LC_Mutex.unlock();
			return -1;
		}
	}
	else // no Error in connect
	{
		// successful connected
	}

	// Set socket to blocking mode
	if (setSocketMode (m_oSockDesc, eSetBlocking) == -1)
	{
		wmLog(eDebug, "openTcpConnectionToLC: Error (014): setting socket mode\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(014)");
		closeTcpConnectionToLC();
		m_oTcpOpeningIsActive = false;
		m_oErrorState = true;
		m_oTCP_to_LC_Mutex.unlock();
		return -1;
	}

	m_oTcpConnIsOpen = true;
	m_oTcpOpeningIsActive = false;

	oTimer.elapsed(); wmLog(eDebug, "openTcpConnectionToLC End - %d ms\n", oTimer.ms() );
	m_oTCP_to_LC_Mutex.unlock();
	return 0;
}

int DataToLaserControl::sendDataBlockToLC (void)
{
	bool oTcpConnIsLocal = false;
	char oLCDataBuffer[1500];
	int oLengthToSend;
	char oRecvBuffer[1500];

	m_oTCP_to_LC_Mutex.lock();
	wmLog(eDebug, "sendDataBlockToLC Start\n");

	if ((m_oErrorState) && (m_oInspectCycleIsOn))
	{
		// vorhergehender Kommunikationsaufbau mit LaserControl ist gescheitert
		wmLog(eDebug, "sendDataBlockToLC Abort due to m_oErrorState\n");
		m_oTCP_to_LC_Mutex.unlock();
		return 0;
	}

	if (!m_oTcpConnIsOpen)
	{
		int oLoopCount = 0;
		while ((m_oTcpOpeningIsActive) && (oLoopCount < 60)) // 60 * 20ms = 1200ms >> 1 sec timeout
		{
			usleep(20*1000);
			oLoopCount++;
		}
		m_oTCP_to_LC_Mutex.unlock();
		int oOpenResult = openTcpConnectionToLC();
		m_oTCP_to_LC_Mutex.lock();
		if (oOpenResult == -1)
		{
			wmLog(eDebug, "sendDataBlockToLC: Error (001): error open connection\n");
			wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPSend", "Unable to send data to the LaserControl device %s\n", "(001)");
			closeTcpConnectionToLC();
			m_oErrorState = true;
			m_oTCP_to_LC_Mutex.unlock();
			return -1;
		}
		oTcpConnIsLocal = true;
	}

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 1;
    oTcpTimeout.tv_usec = 0;
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

	packLCData(m_oLCDataBlock, oLCDataBuffer, oLengthToSend);
	std::cout << "sendDataBlockToLC: oLengthToSend = " << oLengthToSend << std::endl;

	if (send(m_oSockDesc, oLCDataBuffer, oLengthToSend, 0) == -1)
	{
		//wmLogTr(eError, "QnxMsg.VI.LCErrTCPSend", "unable to send data to LaserControl via TCP/IP\n");
		wmLog(eDebug, "sendDataBlockToLC: Error (005): error send\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPSend", "Unable to send data to the LaserControl device %s\n", "(005)");
		closeTcpConnectionToLC();
		m_oErrorState = true;
		m_oTCP_to_LC_Mutex.unlock();
		return -1;
	}

	if (recv(m_oSockDesc, oRecvBuffer, sizeof(oRecvBuffer), 0) <= 0)
	{
		//wmLogTr(eError, "QnxMsg.VI.LCErrTCPReceive", "unable to receive data from LaserControl via TCP/IP\n");
		wmLog(eDebug, "sendDataBlockToLC: Error (007): error rcv\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPSend", "Unable to send data to the LaserControl device %s\n", "(007)");
		closeTcpConnectionToLC();
		m_oErrorState = true;
		m_oTCP_to_LC_Mutex.unlock();
		return -1;
	}

	unpackAcknBlock(oRecvBuffer, m_oAcknBlock);
	std::cout << "sendDataBlockToLC: acknCode = " << m_oAcknBlock.acknCode << " blockType = " << m_oAcknBlock.blockType << " reserve1 = " << m_oAcknBlock.reserve1 << std::endl;

	if (oTcpConnIsLocal)
	{
		closeTcpConnectionToLC();
	}

	wmLog(eDebug, "sendDataBlockToLC End\n");
	m_oTCP_to_LC_Mutex.unlock();
	return 0;
}

int DataToLaserControl::closeTcpConnectionToLC (void)
{
	wmLog(eDebug, "closeTcpConnectionToLC Start\n");

	// close connection and socket
	m_oTcpConnIsOpen = false;
	close (m_oSockDesc);

	wmLog(eDebug, "closeTcpConnectionToLC End\n");
	return 0;
}

int DataToLaserControl::sendDataToLC (void)
{
	char oServerName[81];
	struct hostent *pServerEntry;
	struct sockaddr_in oServerAddr;
	int oSockDesc;

	char oLCDataBuffer[1500];
	int oLengthToSend;
	char oRecvBuffer[1500];

	//*************************************************************************

	wmLog(eDebug, "sendDataToLC Start\n");

	// create socket
	oSockDesc = socket(AF_INET, SOCK_STREAM, 0);
	if (oSockDesc == -1)
	{
		wmLog(eDebug, "sendDataToLC: Error (101): creating socket\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(101)");
		m_oErrorState = true;
		return -1;
	}

	// determine Server address
	strcpy(oServerName, SERVER_NAME_LASERCONTROL.c_str());
	pServerEntry = gethostbyname(oServerName);
	if (pServerEntry == NULL)
	{
		wmLog(eDebug, "sendDataToLC: Error (102): getting hostname\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(102)");
		close (oSockDesc);
		m_oErrorState = true;
		return -1;
	}
	memset((char *)&oServerAddr, 0, sizeof(oServerAddr));
	oServerAddr.sin_family = AF_INET;
	memcpy(&oServerAddr.sin_addr, pServerEntry->h_addr, pServerEntry->h_length);
	oServerAddr.sin_port = htons(TCPSERVER_THREAD_PORT);

	// Set socket to non-blocking mode
	if (setSocketMode (oSockDesc, eSetNonBlocking) == -1)
	{
		wmLog(eDebug, "sendDataToLC: Error (103): setting socket mode\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(103)");
		close (oSockDesc);
		m_oErrorState = true;
		return -1;
	}

	// connect to Server
	int oConnectResult = connect(oSockDesc, (struct sockaddr *)&oServerAddr, sizeof(oServerAddr));
	if (oConnectResult == -1) // Error in connect
	{
		if (errno == EINPROGRESS) // connect is not ready yet
		{
			fd_set oFdset;
			FD_ZERO(&oFdset);
			FD_SET(oSockDesc, &oFdset);
			struct timeval oTimeout;
			oTimeout.tv_sec = 1; // Timeout is 1 seconds
			oTimeout.tv_usec = 0;

			int oSelectResult = select(oSockDesc + 1, NULL, &oFdset, NULL, &oTimeout);
			switch(oSelectResult)
			{
				case 1: // select successful terminated
					{
						int oSockError;
						socklen_t oLength = sizeof(oSockError);
						if (getsockopt(oSockDesc, SOL_SOCKET, SO_ERROR, &oSockError, &oLength) == -1)
						{
							// Set socket to blocking mode
							if (setSocketMode (oSockDesc, eSetBlocking) == -1)
							{
								wmLog(eDebug, "sendDataToLC: Error (104): setting socket mode\n");
								wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(104)");
								close (oSockDesc);
								m_oErrorState = true;
								return -1;
							}
							wmLog(eDebug, "sendDataToLC: Error (105): getting socket options\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(105)");
							close (oSockDesc);
							m_oErrorState = true;
							return -1;
						}
						if (oSockError == 0)
						{
							// successful connected within timeout
						}
						else
						{
							// error, not connected
							// Set socket to blocking mode
							if (setSocketMode (oSockDesc, eSetBlocking) == -1)
							{
								wmLog(eDebug, "sendDataToLC: Error (106): setting socket mode\n");
								wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(106)");
								close (oSockDesc);
								m_oErrorState = true;
								return -1;
							}
							wmLog(eDebug, "sendDataToLC: Error (107): after select\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(107)");
							close (oSockDesc);
							m_oErrorState = true;
							return -1;
						}
					}
					break;
				case 0: // timeout expired
					{
						// Set socket to blocking mode
						if (setSocketMode (oSockDesc, eSetBlocking) == -1)
						{
							wmLog(eDebug, "sendDataToLC: Error (108): setting socket mode\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(108)");
							close (oSockDesc);
							m_oErrorState = true;
							return -1;
						}
						wmLog(eDebug, "sendDataToLC: Error (109): timeout while connecting\n");
						wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(109)");
						close (oSockDesc);
						m_oErrorState = true;
						return -1;
					}
					break;
				case -1: // error in select
					{
						// Set socket to blocking mode
						if (setSocketMode (oSockDesc, eSetBlocking) == -1)
						{
							wmLog(eDebug, "sendDataToLC: Error (110): setting socket mode\n");
							wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(110)");
							close (oSockDesc);
							m_oErrorState = true;
							return -1;
						}
						wmLog(eDebug, "sendDataToLC: Error (111): error select\n");
						wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(111)");
						close (oSockDesc);
						m_oErrorState = true;
						return -1;
					}
					break;
			}
		}
		else // There is another error
		{
			// cannot connect to server
			// Set socket to blocking mode
			if (setSocketMode (oSockDesc, eSetBlocking) == -1)
			{
				wmLog(eDebug, "sendDataToLC: Error (112): setting socket mode\n");
				wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(112)");
				close (oSockDesc);
				m_oErrorState = true;
				return -1;
			}
			wmLog(eDebug, "sendDataToLC: Error (113): error connect\n");
			wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(113)");
			close (oSockDesc);
			m_oErrorState = true;
			return -1;
		}
	}
	else // no Error in connect
	{
		// successful connected
	}

	// Set socket to blocking mode
	if (setSocketMode (oSockDesc, eSetBlocking) == -1)
	{
		wmLog(eDebug, "sendDataToLC: Error (114): setting socket mode\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPOpen", "Unable to open connection to the LaserControl device %s\n", "(114)");
		close (oSockDesc);
		m_oErrorState = true;
		return -1;
	}

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 1;
    oTcpTimeout.tv_usec = 0;
    setsockopt(oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

	packLCData(m_oLCDataBlock, oLCDataBuffer, oLengthToSend);
	std::cout << "sendDataToLC: oLengthToSend = " << oLengthToSend << std::endl;

	if (send(oSockDesc, oLCDataBuffer, oLengthToSend, 0) == -1)
	{
		//wmLogTr(eError, "QnxMsg.VI.LCErrTCPSend", "unable to send data to LaserControl via TCP/IP\n");
		wmLog(eDebug, "sendDataToLC: Error (118): error send\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPSend", "Unable to send data to the LaserControl device %s\n", "(118)");
		close (oSockDesc);
		m_oErrorState = true;
		return -1;
	}

	if (recv(oSockDesc, oRecvBuffer, sizeof(oRecvBuffer), 0) <= 0)
	{
		//wmLogTr(eError, "QnxMsg.VI.LCErrTCPReceive", "unable to receive data from LaserControl via TCP/IP\n");
		wmLog(eDebug, "sendDataToLC: Error (120): error rcv\n");
		wmFatal(eExtEquipment, "QnxMsg.VI.LCErrTCPSend", "Unable to send data to the LaserControl device %s\n", "(120)");
		close (oSockDesc);
		m_oErrorState = true;
		return -1;
	}

	unpackAcknBlock(oRecvBuffer, m_oAcknBlock);
	std::cout << "sendDataToLC: acknCode = " << m_oAcknBlock.acknCode << " blockType = " << m_oAcknBlock.blockType << " reserve1 = " << m_oAcknBlock.reserve1 << std::endl;

	// close connection and socket
	close (oSockDesc);

	wmLog(eDebug, "sendDataToLC End\n");
	return 0;
}

void DataToLaserControl::packLCData(LCDataDef & p_rLCDataBlock, char * p_pSendBuffer, int & p_rBufLength)
{
    short int *pword;
    int *pdword;
    int blockLength;

    // Write DATABLOCK-Header
    pword = (short int *) p_pSendBuffer;

    *pword = (short int) DBLOCK_LASERCONTROL;
    pword++;
    *pword = (short int) DBLOCK_LASERCONTROL_VERSION;
    pword++;
    *pword = 0;           // place for blockLength
    pword++;
    *pword = 0;           // reserve
    pword++;

    // Write DATABLOCK-Information
    // change datatype
    pdword = (int *) pword;

    // Variable 1
    *pdword = p_rLCDataBlock.variable1;
    pdword++;

    // Variable 2
    *pdword = p_rLCDataBlock.variable2;
    pdword++;

    // Variable 3
    *pdword = p_rLCDataBlock.variable3;
    pdword++;

    // Variable 4
    *pdword = p_rLCDataBlock.variable4;
    pdword++;

    // Variable 5
    *pdword = p_rLCDataBlock.variable5;
    pdword++;

    // Variable 6
    *pdword = p_rLCDataBlock.variable6;
    pdword++;

    // Variable 7
    *pdword = p_rLCDataBlock.variable7;
    pdword++;

    // Variable 8
    *pdword = p_rLCDataBlock.variable8;
    pdword++;

    // Variable 9
    *pdword = p_rLCDataBlock.variable9;
    pdword++;

    // Variable 10
    *pdword = p_rLCDataBlock.variable10;
    pdword++;

    // Variable 11
    *pdword = p_rLCDataBlock.variable11;
    pdword++;

    // Variable 12
    *pdword = p_rLCDataBlock.variable12;
    pdword++;

    // Variable 13
    *pdword = p_rLCDataBlock.variable13;
    pdword++;

    // Variable 14
    *pdword = p_rLCDataBlock.variable14;
    pdword++;

    // Variable 15
    *pdword = p_rLCDataBlock.variable15;
    pdword++;

    // Variable 16
    *pdword = p_rLCDataBlock.variable16;
    pdword++;

    // Variable 17
    *pdword = p_rLCDataBlock.variable17;
    pdword++;

    // Variable 18
    *pdword = p_rLCDataBlock.variable18;
    pdword++;

    // Variable 19
    *pdword = p_rLCDataBlock.variable19;
    pdword++;

    // Variable 20
    *pdword = p_rLCDataBlock.variable20;
    pdword++;

    // Variable 21
    *pdword = p_rLCDataBlock.variable21;
    pdword++;

    // Variable 22
    *pdword = p_rLCDataBlock.variable22;
    pdword++;

    // Variable 23
    *pdword = p_rLCDataBlock.variable23;
    pdword++;

    // Variable 24
    *pdword = p_rLCDataBlock.variable24;
    pdword++;

    // Variable 25
    *pdword = p_rLCDataBlock.variable25;
    pdword++;

    // Variable 26
    *pdword = p_rLCDataBlock.variable26;
    pdword++;

    // Variable 27
    *pdword = p_rLCDataBlock.variable27;
    pdword++;

    // Variable 28
    *pdword = p_rLCDataBlock.variable28;
    pdword++;

    blockLength = (char *)pdword - (char *)p_pSendBuffer;
    pword = (short int *) &p_pSendBuffer[4];
    *pword = (short int) blockLength;
    p_rBufLength = blockLength;
}

void DataToLaserControl::unpackAcknBlock(char * p_pRecvBuffer, acknDef & p_rAcknBlock)
{
	int *pdword;
	short int *pword;
	int blockLength1;
	int blockLength2;

	// Read DATABLOCK-Header
	pword = (short int *) p_pRecvBuffer;

	//xyz = *pword;              // block type
	pword++;
	//xyz = *pword;              // block version
	pword++;
	blockLength1 = *pword;       // block length
	pword++;
	//xyz = *pword;              // reserve
	pword++;

	// Read DATABLOCK-Information
	// change datatype
	pdword = (int *) pword;

	p_rAcknBlock.acknCode = *pdword;
	pdword++;
	p_rAcknBlock.blockType = *pdword;
	pdword++;
	p_rAcknBlock.reserve1 = *pdword;
	pdword++;
	p_rAcknBlock.reserve2 = *pdword;
	pdword++;
	p_rAcknBlock.reserve3 = *pdword;
	pdword++;
	p_rAcknBlock.reserve4 = *pdword;
	pdword++;

	blockLength2 = (char *)pdword - (char *)p_pRecvBuffer;
	if (blockLength1 != blockLength2)
	{
		wmLogTr(eError, "QnxMsg.VI.LCErrAckBlock", "Length of acknowledge block is not identical\n");
	}
}

int DataToLaserControl::setSocketMode (int p_oSockDesc, SocketMode p_oMode)
{
	long oSockFlags;

	oSockFlags = fcntl(p_oSockDesc, F_GETFL, NULL);
	if (oSockFlags == -1)
	{
		wmLog(eDebug, "openTcpConnectionToLC: Error (100): getting socket flags\n");
		return -1;
	}

	if (p_oMode == eSetNonBlocking)
		oSockFlags |= O_NONBLOCK;
	else if (p_oMode == eSetBlocking)
		oSockFlags &= (~O_NONBLOCK);

	if (fcntl(p_oSockDesc, F_SETFL, oSockFlags) == -1)
	{
		wmLog(eDebug, "openTcpConnectionToLC: Error (101): setting socket flags\n");
		return -1;
	}

	return 0;
}

} // namespace hardware

} // namespace precitec

