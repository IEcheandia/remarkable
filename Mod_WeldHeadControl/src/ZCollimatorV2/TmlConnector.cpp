#include <unistd.h>

#include <cstring>
#include <iostream>

#include "viWeldHead/ZCollimatorV2/TmlConnector.h"
#include "module/moduleLogger.h"

#define COMMUNICATION_DEBUG 0

/// BEGIN TML_Lib_light callback definition
bool SendMessage(RS232_MSG* RS232_TX_message)
{
    return precitec::tml::TmlConnector::instance().SendMessage(RS232_TX_message);
}
bool ReceiveMessage(RS232_MSG* RS232_RX_message)
{
    return precitec::tml::TmlConnector::instance().ReceiveMessage(RS232_RX_message);
}
/// END ML_Lib_light callbacks definition

namespace precitec
{

namespace tml
{

TmlConnector::TmlConnector():
    m_oTmlControllerIpAddress("192.168.170.3"),
    m_oEtcConfigurationPort(30689),
    m_oEtcCommunicationPort(1700),
    m_oSockDesc(-1)
{
}

TmlConnector::~TmlConnector()
{
    if (m_oSockDesc != -1)
    {
        close(m_oSockDesc);
        m_oSockDesc = -1;
    }
}

TmlConnector& TmlConnector::instance()
{
    static TmlConnector _instance;
    return _instance;
}

bool TmlConnector::SendMessage(RS232_MSG* RS232_TX_message)
{
#if COMMUNICATION_DEBUG
wmLog(eDebug, "------------------------------\n");
wmLog(eDebug,"SendMessage\n");
#endif

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 0;
    oTcpTimeout.tv_usec = 100000; // 100ms
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    int oSentByte = sendto(m_oSockDesc, RS232_TX_message->RS232_data, RS232_TX_message->length, 0,
                            (struct sockaddr *)&m_oServerAddrCommunication, sizeof(struct sockaddr_in));
    if (oSentByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to sendto: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(104)");
        return false;
    }
    else
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%2d", oSentByte);
        char oHelpStrg2[21];
        sprintf(oHelpStrg1, "%02X", RS232_TX_message->RS232_data[0]);
        wmLog(eDebug, "oSentByte: %s, RS232_data    : %s\n", oHelpStrg1, oHelpStrg2);
#endif
    }

    char oSendAnswer[1500];
    int oRecvByte = recvfrom(m_oSockDesc, oSendAnswer, sizeof(oSendAnswer), 0, NULL, NULL);
    if (oRecvByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(105)");
        return false;
    }
    else if (oRecvByte == 1)
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%2d", oRecvByte);
        char oHelpStrg2[21];
        sprintf(oHelpStrg1, "%02X", oSendAnswer[0]);
        wmLog(eDebug, "oRecvByte: %s, oSendAnswer: %s\n", oHelpStrg1, oHelpStrg2);
#endif
        if (oSendAnswer[0] != 0x4F)
        {
            wmLog(eDebug, "ZColl: was not able to recvfrom: wrong send answer\n");
            wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(106)");
            return false;
        }
    }
    else if (oRecvByte == 0)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: connection shutdown\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(107)");
        return false;
    }
    else
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: wrong number of sent bytes\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(108)");
        return false;
    }

    return true;
}

bool TmlConnector::ReceiveMessage(RS232_MSG* RS232_RX_message)
{
#if COMMUNICATION_DEBUG
wmLog(eDebug, "------------------------------\n");
wmLog(eDebug, "ReceiveMessage\n");
#endif

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 0;
    oTcpTimeout.tv_usec = 100000; // 100ms
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    char oReceiveMessage[1500];
    int oRecvByte = recvfrom(m_oSockDesc, oReceiveMessage, sizeof(oReceiveMessage), 0, NULL, NULL);
    if (oRecvByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(113)");
        return false;
    }
    else if (oRecvByte == 0)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: connection shutdown\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(114)");
        return false;
    }
    else
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%2d", oRecvByte);
        char oHelpStrg2[81];
        sprintf(oHelpStrg1, "%02X,%02X,%02X,%02X,%02X,%02X",
               (uint8_t)oReceiveMessage[0], (uint8_t)oReceiveMessage[1], (uint8_t)oReceiveMessage[2],
               (uint8_t)oReceiveMessage[3], (uint8_t)oReceiveMessage[4], (uint8_t)oReceiveMessage[5]);
        wmLog(eDebug, "oRecvByte: %s, oReceiveMessage: %s\n", oHelpStrg1, oHelpStrg2);
#endif
    }

    if (oRecvByte != RS232_RX_message->length)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: wrong length\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(115)");
        return false;
    }
    for(int i = 0;i < oRecvByte;++i)
    {
        RS232_RX_message->RS232_data[i] = oReceiveMessage[i];
    }

    return true;
}

bool TmlConnector::connect()
{
#if COMMUNICATION_DEBUG
wmLog(eDebug, "------------------------------\n");
wmLog(eDebug, "connect\n");
#endif

    // IP address of TML controller
    uint32_t oIPAddressOfTml; // in network byte order
    int oRetValue = inet_pton(AF_INET, m_oTmlControllerIpAddress.c_str(), &oIPAddressOfTml);
    if (oRetValue != 1)
    {
        wmLogTr(eError, "QnxMsg.VI.TCPWrongIPFormat", "wrong format of IP address string\n");
        oIPAddressOfTml = 0x0100007F; // 127.0.0.1 in network byte order
    }
    char oHelpStrg[21];
    sprintf(oHelpStrg, "%08X", oIPAddressOfTml);
    wmLog(eDebug, "oIPAddressOfTml: %s, oRetValue: %d\n", oHelpStrg, oRetValue); // in network byte order

    // define server address Configuration
    memset((char *)&m_oServerAddrConfiguration, 0, sizeof(m_oServerAddrConfiguration));
    m_oServerAddrConfiguration.sin_family = AF_INET;
    memcpy(&m_oServerAddrConfiguration.sin_addr, &oIPAddressOfTml, 4); // in network byte order
    m_oServerAddrConfiguration.sin_port = htons(m_oEtcConfigurationPort);

    // define server address Communication
    memset((char *)&m_oServerAddrCommunication, 0, sizeof(m_oServerAddrCommunication));
    m_oServerAddrCommunication.sin_family = AF_INET;
    memcpy(&m_oServerAddrCommunication.sin_addr, &oIPAddressOfTml, 4); // in network byte order
    m_oServerAddrCommunication.sin_port = htons(m_oEtcCommunicationPort);

    // define server address Receiving
    memset((char *)&m_oServerAddrReceiving, 0, sizeof(m_oServerAddrReceiving));
    m_oServerAddrReceiving.sin_family = AF_INET;
    m_oServerAddrReceiving.sin_addr.s_addr = INADDR_ANY;
    m_oServerAddrReceiving.sin_port = htons(m_oEtcConfigurationPort);

    // create socket
    m_oSockDesc = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_oSockDesc == -1)
    {
        wmLog(eDebug, "ZColl: Error while creating socket: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(001)");
        return 1;
    }

    // bind for receiving
    if (bind(m_oSockDesc, (struct sockaddr *)&m_oServerAddrReceiving, sizeof(m_oServerAddrReceiving)) == -1)
    {
        wmLog(eDebug, "ZColl: Error while binding socket: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(002)");
        close(m_oSockDesc);
        m_oSockDesc = -1;
        return 2;
    }

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 4;
    oTcpTimeout.tv_usec = 0;
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    char oConnectMsg[1];
    oConnectMsg[0] = 0x01;
    int oSentByte = sendto(m_oSockDesc, oConnectMsg, sizeof(oConnectMsg), 0,
                           (struct sockaddr *)&m_oServerAddrConfiguration, sizeof(struct sockaddr_in));
    if (oSentByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to sendto: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(006)");
        return false;
    }
    else if (oSentByte == 1)
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%d", oSentByte);
        char oHelpStrg2[21];
        sprintf(oHelpStrg1, "%02X", oConnectMsg[0]);
        wmLog(eDebug, "oSentByte: %s, oConnectMsg:    %s\n", oHelpStrg1, oHelpStrg2);
#endif
    }
    else
    {
        wmLog(eDebug, "ZColl: was not able to sendto: wrong number of sent bytes\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(007)");
        return false;
    }

    char oConnectAnswer[1000];
    int oRecvByte = recvfrom(m_oSockDesc, oConnectAnswer, sizeof(oConnectAnswer), 0, NULL, NULL);
    if (oRecvByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(008)");
        return false;
    }
    else if (oRecvByte == 1)
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%d", oRecvByte);
        char oHelpStrg2[21];
        sprintf(oHelpStrg1, "%02X", oConnectAnswer[0]);
        wmLog(eDebug, "oRecvByte: %s, oConnectAnswer: %s\n", oHelpStrg1, oHelpStrg2);
#endif
        if (oConnectAnswer[0] != 0x02)
        {
            wmLog(eDebug, "ZColl: was not able to recvfrom: wrong connect answer\n");
            wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(009)");
            return false;
        }
    }
    else if (oRecvByte == 0)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: connection shutdown\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(010)");
        return false;
    }
    else
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: wrong number of sent bytes\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(011)");
        return false;
    }

    return true;
}

bool TmlConnector::disconnect()
{
#if COMMUNICATION_DEBUG
wmLog(eDebug, "------------------------------\n");
wmLog(eDebug, "disconnect\n");
#endif

    struct timeval oTcpTimeout;
    oTcpTimeout.tv_sec = 4;
    oTcpTimeout.tv_usec = 0;
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_SNDTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);
    setsockopt(m_oSockDesc, SOL_SOCKET, SO_RCVTIMEO, (const char*)&oTcpTimeout, sizeof oTcpTimeout);

    char oDisconnectMsg[1];
    oDisconnectMsg[0] = 0x05;
    int oSentByte = sendto(m_oSockDesc, oDisconnectMsg, sizeof(oDisconnectMsg), 0,
                            (struct sockaddr *)&m_oServerAddrConfiguration, sizeof(struct sockaddr_in));
    if (oSentByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to sendto: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(016)");
        return false;
    }
    else if (oSentByte == 1)
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%d", oSentByte);
        char oHelpStrg2[21];
        sprintf(oHelpStrg1, "%02X", oDisconnectMsg[0]);
        wmLog(eDebug, "oSentByte: %s, oDisconnectMsg:    %s\n", oHelpStrg1, oHelpStrg2);
#endif
    }
    else
    {
        wmLog(eDebug, "ZColl: was not able to sendto: wrong number of sent bytes\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(017)");
        return false;
    }

    char oDisconnectAnswer[1000];
    int oRecvByte = recvfrom(m_oSockDesc, oDisconnectAnswer, sizeof(oDisconnectAnswer), 0, NULL, NULL);
    if (oRecvByte == -1)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: %s\n", strerror(errno));
        wmFatal(eExtEquipment, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(018)");
        return false;
    }
    else if (oRecvByte == 1)
    {
#if COMMUNICATION_DEBUG
        char oHelpStrg1[21];
        sprintf(oHelpStrg1, "%d", oRecvByte);
        char oHelpStrg2[21];
        sprintf(oHelpStrg1, "%02X", oDisconnectAnswer[0]);
        wmLog(eDebug, "oRecvByte: %s, oDisconnectAnswer: %s\n", oHelpStrg1, oHelpStrg2);
#endif
        if (oDisconnectAnswer[0] != 0x06)
        {
            wmLog(eDebug, "ZColl: was not able to recvfrom: wrong disconnect answer\n");
            wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(019)");
            return false;
        }
    }
    else if (oRecvByte == 0)
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: connection shutdown\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(020)");
        return false;
    }
    else
    {
        wmLog(eDebug, "ZColl: was not able to recvfrom: wrong number of sent bytes\n");
        wmLogTr(eError, "QnxMsg.VI.TCPCommFault", "Problem while establishing TCP/IP communication %s\n", "(021)");
        return false;
    }

    if (m_oSockDesc != -1)
    {
        close(m_oSockDesc);
        m_oSockDesc = -1;
    }

    return true;
}

void TmlConnector::SetTmlControllerIpAddress(std::string p_oIpAddress)
{
    m_oTmlControllerIpAddress = p_oIpAddress;
}

} // namespace tml
} // namespace precitec

