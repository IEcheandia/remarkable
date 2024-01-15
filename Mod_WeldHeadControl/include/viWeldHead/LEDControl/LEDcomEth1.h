/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDcomEth1.h                   Author :    KIH          *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:  Kommunikation mit LED Ansteuerung                       *
*                                                                       *
************************************************************************/

#ifndef LED_COM_ETHERNET_1_H_
#define LED_COM_ETHERNET_1_H_

#include "LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include <sys/socket.h>
#include <arpa/inet.h>
#include "LEDcomBase.h"
#include <string>

class LEDcomEthernet1TTest;

class LEDcomEthernet1T : public LEDcomBaseT
{
	public:
		LEDcomEthernet1T();
		void init(LEDdriverParameterT & LEDdriverParameter);
		void deinit();
		void rw(char *strin,char *strout,bool waitforEndFlag = true,bool p_oFastReturn=false);

    const std::string &ledIp() const
    {
        return m_ledIp;
    }
    int ledPort() const
    {
        return m_ledPort;
    }

    const std::string &localIp() const
    {
        return m_localIp;
    }
    int localPort() const
    {
        return m_localPort;
    }

    void setLocalAddress(const std::string &ip, int port)
    {
        m_localIp = ip;
        m_localPort = port;
    }
    void setLedAddress(const std::string &ip, int port)
    {
        m_ledIp = ip;
        m_ledPort = port;
    }

	private:
		int sock ;
		struct sockaddr_in  led_driver_address;
		struct sockaddr_in  my_address;
        std::string m_localIp;
        int m_localPort;
        std::string m_ledIp;
        int m_ledPort;

        friend LEDcomEthernet1TTest;
};

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

#endif // LED_COM_ETHERNET_1_H_

