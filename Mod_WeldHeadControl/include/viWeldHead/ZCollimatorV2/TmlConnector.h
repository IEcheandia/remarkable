#pragma once

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <string>

#include "viWeldHead/ZCollimatorV2/TML_lib_light.h"

/// BEGIN TML_Lib_light callback declaration
bool SendMessage(RS232_MSG* RS232_TX_message);
bool ReceiveMessage(RS232_MSG* RS232_RX_message);
/// END TML_Lib_light callback declaration

namespace precitec
{

namespace tml
{

class TmlConnector
{
public:
    virtual ~TmlConnector();
    static TmlConnector& instance();
    TmlConnector (const TmlConnector&) = delete;
    void operator=(const TmlConnector&) = delete;

    bool SendMessage(RS232_MSG* RS232_TX_message);
    bool ReceiveMessage(RS232_MSG* RS232_RX_message);

    bool connect();
    bool disconnect();

    void SetTmlControllerIpAddress(std::string p_oIpAddress);

private:
    explicit TmlConnector ();

    std::string m_oTmlControllerIpAddress;
    const uint16_t m_oEtcConfigurationPort;
    const uint16_t m_oEtcCommunicationPort;
    int m_oSockDesc;
    struct sockaddr_in m_oServerAddrConfiguration;
    struct sockaddr_in m_oServerAddrCommunication;
    struct sockaddr_in m_oServerAddrReceiving;
};

} // namespace tml
} // namespace precitec

