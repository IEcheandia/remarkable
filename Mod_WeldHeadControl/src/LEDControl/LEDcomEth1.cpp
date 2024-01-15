/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDcomETH1.cpp                 Author : KIH             *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:  Ansteuerung LED Beleuchtung                             *
*                                                                       *
************************************************************************/

#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "viWeldHead/LEDControl/LEDcomEth1.h"

#include <unistd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#define LED_DRIVER_PORT      30313
#define LED_DRIVER_IP        "128.1.1.2"

#define LED_MY_PORT          30312
#define LED_MY_IP            "128.1.1.1"

LEDcomEthernet1T::LEDcomEthernet1T()
{
	state=0;
	error=0;
	us_wait_after_send=2000;

    m_localIp = LED_MY_IP;
    m_localPort = LED_MY_PORT;

    m_ledIp = LED_DRIVER_IP;
    m_ledPort = LED_DRIVER_PORT;
}

void LEDcomEthernet1T::init(LEDdriverParameterT & LEDdriverParameter)
{
	state =0;

	struct timeval timeout;

	/* Create the socket. */
	sock = socket ( PF_INET, SOCK_DGRAM, 0 );
	if (sock < 0)
	{
		//printf("Error Socket failed\n");
		error|=ERR_INIT_FAILED;
		return;
	}

	/* Give the socket a name. */
	memset(&led_driver_address, 0, sizeof(led_driver_address));
	led_driver_address.sin_family = AF_INET;
	led_driver_address.sin_port = htons ( m_ledPort );
	led_driver_address.sin_addr.s_addr = inet_addr(m_ledIp.c_str());

	//timeout.tv_sec=2; // this is SOUVIS5200
	//timeout.tv_usec = 0; // this is SOUVIS5200
	timeout.tv_sec=1; // WM cannot spend so much time for waiting for LED controller
	timeout.tv_usec = 500000; // WM cannot spend so much time for waiting for LED controller
	setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)  );
	setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout)  );

/*
	// war gut:
	if (connect(sock, (struct sockaddr *) &led_driver_address, sizeof(led_driver_address)) < 0)
	{
		printf("connect() failed\n");
		error|=ERR_INIT_FAILED;
		return;
	}
*/

	memset(&my_address, 0, sizeof(my_address));
	my_address.sin_family = AF_INET;
	my_address.sin_port = htons ( m_localPort );
	my_address.sin_addr.s_addr = inet_addr(m_localIp.c_str());
	bind(sock,(struct sockaddr *) &my_address, sizeof(my_address));

	state=1;
}

void LEDcomEthernet1T::deinit()
{
	// close aus unistd.h
	::close (sock);
	state=0;
}

void LEDcomEthernet1T::rw(char * datain, char * dataout, bool waitforEndFlag, bool p_oFastReturn)
{
	int i;

#if 0
    {
        struct timespec timeStamp;
        clock_gettime(CLOCK_REALTIME, &timeStamp);
        struct tm *ptmVar;
        ptmVar = localtime(&timeStamp.tv_sec);
        printf("%02d:%02d:%02d:%03ld ", ptmVar->tm_hour, ptmVar->tm_min, ptmVar->tm_sec, (timeStamp.tv_nsec / 1000000));
        printf("LEDcomEthernet1T::rw Start\n");
    }
#endif

	if(error!=0) return;
	if(state<1) { error |= ERR_TRY_TO_SEND_NOT_INITIALIZED; return; }

    char oDummy[1000] {};
    recv(sock, oDummy, sizeof(oDummy), MSG_DONTWAIT);

	int sent_bytes, all_sent_bytes;
	int sendstrlen;

	sendstrlen = strlen ( datain );
	all_sent_bytes = 0;

	//verwende das send der sockets:
	//sent_bytes =   ::send(sock,  (void *)data, sendstrlen, 0);
	sent_bytes =   ::sendto(sock,  (void *)datain, sendstrlen, 0,(struct sockaddr *) &led_driver_address, sizeof(led_driver_address));
#if 0
    printf ("sent data:[");
    for(int j = 0;j < strlen(datain);j++)
    {
        if (datain[j] > 0x20)
        {
            printf("%c", datain[j]);
        }
        else
        {
            printf("{%02X}", datain[j]);
        }
    }
    printf ("]\n");
#endif

	all_sent_bytes = all_sent_bytes + sent_bytes;
	// printf ("Sent data:%s\n", datain);

	usleep(us_wait_after_send); // 2 ms
    if(strstr(datain, "RT")!=0)
    {
        if(p_oFastReturn)
        {
            usleep(8 * 1000); // 8 ms
            dataout[0] = '>';
            dataout[1] = 0x00;
#if 0
            {
                struct timespec timeStamp;
                clock_gettime(CLOCK_REALTIME, &timeStamp);
                struct tm *ptmVar;
                ptmVar = localtime(&timeStamp.tv_sec);
                printf("%02d:%02d:%02d:%03ld ", ptmVar->tm_hour, ptmVar->tm_min, ptmVar->tm_sec, (timeStamp.tv_nsec / 1000000));
                printf("LEDcomEthernet1T::rw nach sendto, ohne recv\n");
            }
#endif
            return;
        }
    }

	char *tp;
	int wp=0;

	int rec_bytes;
	//printf("testi xxx\n");

	dataout[0]=0;
	tp=dataout;
	//for(i=0;i<5;++i) // this is SOUVIS5200
	for(i=0;i<4;++i) // WM cannot spend so much time for waiting for LED controller
	{
		rec_bytes=recv(sock, tp, 1000, 0);
		//recvfrom(sock, receivebuffer, 1000, 0, (struct sockaddr *) &led_driver_address, sizeof(led_driver_address));
#if 0
        {
            struct timespec timeStamp;
            clock_gettime(CLOCK_REALTIME, &timeStamp);
            struct tm *ptmVar;
            ptmVar = localtime(&timeStamp.tv_sec);
            printf("%02d:%02d:%02d:%03ld ", ptmVar->tm_hour, ptmVar->tm_min, ptmVar->tm_sec, (timeStamp.tv_nsec / 1000000));
            printf("LEDcomEthernet1T::rw nach recv\n");
        }
#endif
		//printf("rec_bytes=%d\n",rec_bytes);

		if(rec_bytes>=0)
		{
			wp+=rec_bytes;
			dataout[wp]=0;
			//printf ("%d received data:%s\n",i, dataout);
#if 0
            printf ("trial:%d received data:[",i);
            for(int j = 0;j < strlen(dataout);j++)
            {
                if (dataout[j] > 0x20)
                {
                    printf("%c", dataout[j]);
                }
                else
                {
                    printf("{%02X}", dataout[j]);
                }
            }
            printf ("]\n");
#endif

			if(!waitforEndFlag) return;
			if(strchr(dataout,'>')!=0) return;
			tp=&(dataout[wp]);
		}
	}
}

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

