/*************************************************************************
 * Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
 * Module     :  ips2                           Version:                 *
 * Filename   :  LEDdriver.cpp                  Author : KIH             *
 *                                                                       *
 * Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
 *                                                                       *
 * Description:  Ansteuerung LED Beleuchtung                             *
 *                                                                       *
 ************************************************************************/

#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "viWeldHead/LEDControl/LEDdriver.h"

#include "module/moduleLogger.h"

/*
 * LEDdriverT
 * bedient die Souvis Befehle: init update deinit
 * leistet die Umsetzung fuer alle Kanaele
 * benutzt updatechannel und initchannel des LEDdriverHWBaseT * pLEDdriverHW;
 * erst das setzt die Hardwarespezifischen Kommandos
 *
 * LEDdriverT leistet ausserdem: nur zu aktualisierende parameter werden gesetzt
 * und : enable/disable handling
 * */

LEDdriverT::LEDdriverT()
{
	seamno=-1;
	state = 0;
	error = 0;
	driver_enabled=1;
    m_oControllerTypeIsWrong = false;
	pdebug=NULL;
}

LEDdriverT::~LEDdriverT()
{
}

void LEDdriverT::init(LEDdriverHWBaseT * pLEDdriverHWin,LEDcomBaseT * pledcom,LEDdriverParameterT & LEDdriverParameter, int iLEDmaxCurrSet[])
{
	pLEDdriverHWin->setdebuglevelsource(&pdebug);
	pledcom->setdebuglevelsource(&pdebug);

	if(driver_enabled!=0)
	{
		error = 0; //evtl das extra
		state = 0;
        m_oControllerTypeIsWrong = false;
		//Uebergabe des Objekts bzw. der HW
		pLEDdriverHW = pLEDdriverHWin;
		if (pLEDdriverHW == NULL)
				return;

		int oRetValue = pLEDdriverHW->init(pledcom,LEDdriverParameter,iLEDmaxCurrSet);
		if (oRetValue == -1) // LED-Controller connection error
		{
			printf("driver_enabled =0 !!!!\n");
			driver_enabled=0;
			//sprintf(statusStrg,"%s (001)", getTextPtr(HOST_ERR,105));
			//statusMsg(STATUS_ERR,statusStrg);
			printf("LED: Timeout beim initialisieren! KOMMUNIKATION ABGEBROCHEN! (001)\n");
			precitec::wmLog(precitec::eDebug, "Timeout while initializing LED controller, no communication (001)\n");
			precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNotOk", "Timeout while initializing LED controller: No Communication !\n" );
			return;
		}
		else if (oRetValue == -2) // wrong type of LED-Controller
        {
            seamno=-1;
            state = 0;
            error = 0;
            driver_enabled=1;
            m_oControllerTypeIsWrong = true;
            pdebug=NULL;
            return;
        }

		error = pLEDdriverHW->errornum();
		printf("LEDdriverT init error %d\n",error);
		if (error == 0)  state = 1;

#ifdef LED_DRIVER_CHATTY
printf("LEDdriverT getdebuglevel()=%d\n",getdebuglevel());
printf("pLEDdriverHWin getdebuglevel()=%d\n",pLEDdriverHWin->getdebuglevel());
printf("pledcom getdebuglevel()=%d\n",pledcom->getdebuglevel());
#endif

		usleep(pLEDdriverHW->us_wait_after_init);
	}
}

void LEDdriverT::deinit()
{
	if(driver_enabled!=0)
	{
		pLEDdriverHW->deinit();
		error = pLEDdriverHW->errornum();
		state = 0;
	}
}

void LEDdriverT::updateall(LEDI_ParametersT *inpars)
{
	if(driver_enabled!=0)
	{
		if (state < 1)
		{
			printf("LEDdriverT not initialized\n");
			return;
		}

		if(getdebuglevel()>0) printf("update all\n");

		int i,index;
		LEDI_ParametersT *mypars;

		for (i = pLEDdriverHW->minChannelNumber; i <= pLEDdriverHW->maxChannelNumber; ++i)
		{
			index=(i -  pLEDdriverHW->minChannelNumber);
			mypars = inpars + index;

			if (mypars->led_enable > 0)
			{
				if ((mypars->led_brightness != currentparams[index].led_brightness) ||
				    (mypars->led_pulse_width != currentparams[index].led_pulse_width))
				{
					if (pLEDdriverHW->updateChannel(i,mypars)== -1)
					{
						//state = 0;
						printf("LED: Timeout beim Senden von Parametern! KOMMUNIKATION ABGEBROCHEN! (001)\n");
						precitec::wmLog(precitec::eDebug, "Timeout while sending data to LED controller, no communication (001)\n");
						precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNoData", "Timeout while sending data to LED controller: No Communication !\n" );
						return;
					}
					currentparams[index].led_brightness=mypars->led_brightness;
					currentparams[index].led_pulse_width=mypars->led_pulse_width;
				}
				else
				{
					//printf("channel %d   brightness=%d is up-to-date\n", i,mypars->led_brightness);
				}
			}
			else
			{
				if (currentparams[index].led_brightness != 0)
				{
					pLEDdriverHW->disableChannel(i,mypars->led_pulse_width); // calls updateChannel
					currentparams[index].led_brightness=0;
					currentparams[index].led_pulse_width=mypars->led_pulse_width;
				}
			}
		}
		//pLEDdriverHW->writeSetupPersistent();
		//usleep(pLEDdriverHW->us_wait_after_update);
	}
}

void LEDdriverT::updateall_in_once(LEDI_ParametersT *inpars)
{
    if(driver_enabled!=0)
    {
        if (state < 1)
        {
            printf("LEDdriverT not initialized\n");
            return;
        }

        if(getdebuglevel()>0) printf("updateall_in_once\n");

        bool oNeedToSend = false;
        for (int i = pLEDdriverHW->minChannelNumber; i <= pLEDdriverHW->maxChannelNumber; ++i)
        {
            LEDI_ParametersT *mypars;
            int index=(i - pLEDdriverHW->minChannelNumber);
            mypars = inpars + index;

            if (mypars->led_enable > 0)
            {
                if ((mypars->led_brightness != currentparams[index].led_brightness) ||
                    (mypars->led_pulse_width != currentparams[index].led_pulse_width))
                {
                    oNeedToSend = true;
                    currentparams[index].led_brightness=mypars->led_brightness;
                    currentparams[index].led_pulse_width=mypars->led_pulse_width;
                }
            }
            else
            {
                if (currentparams[index].led_brightness != 0)
                {
                    oNeedToSend = true;
                    currentparams[index].led_brightness=0;
                    currentparams[index].led_pulse_width=mypars->led_pulse_width;
                }
            }
        }

        if (oNeedToSend)
        {
            if (pLEDdriverHW->updateChannelAll(inpars)== -1)
            {
                //state = 0;
                printf("LED: Timeout beim Senden von Parametern! KOMMUNIKATION ABGEBROCHEN! (001)\n");
                precitec::wmLog(precitec::eDebug, "Timeout while sending data to LED controller, no communication (001)\n");
                precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNoData", "Timeout while sending data to LED controller: No Communication !\n" );
                return;
            }
        }
#if 0
        else
        {
            usleep(10*1000); // constant delay, also in case of oNeedToSend==false
        }
#endif
    }
}

void LEDdriverT::writeSetupPersistent()
{
	if(driver_enabled!=0)
	{
		if (pLEDdriverHW->writeSetupPersistent()== -1)
		{
			//state = 0;
			printf("LED: Timeout beim Senden von Parametern! KOMMUNIKATION ABGEBROCHEN! (002)\n");
			precitec::wmLog(precitec::eDebug, "Timeout while sending data to LED controller, no communication (002)\n");
			precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNoData", "Timeout while sending data to LED controller: No Communication !\n" );
			return;
		}
	}
}

void LEDdriverT::updateMaxCurrent(int LEDmaxCurrSet[])
{
	//printf("updateMaxCurrent\n");
	if(driver_enabled!=0)
	{
		int i;
		double dLEDrating[8];
		for(i=0;i<pLEDdriverHW->nChannels;++i)
		{
			// 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
			//dLEDrating[i]= LEDmaxCurrSet[i]* 1e-4; // Umrechnung von maxCurrent in den rating-Faktor (2000mA -> 0.2 A rating)
			dLEDrating[i]= LEDmaxCurrSet[i]* 1e-3;
			//printf("Manuell: Umrechnung Rating: %d, %g\n",i, dLEDrating[i]);
		}
		//alle Channels mit dem Rating-Faktor setzten, soll nur in LEVEL9 moeglich sein
		if (pLEDdriverHW->ReFlashDriver(dLEDrating, pLEDdriverHW->nChannels)== -1)
		{
			//sprintf(statusStrg,"%s (002)", getTextPtr(HOST_ERR,105));
		    //statusMsg(STATUS_ERR,statusStrg);
			printf("LED: Timeout beim initialisieren! KOMMUNIKATION ABGEBROCHEN! (002)\n");
			precitec::wmLog(precitec::eDebug, "Timeout while initializing LED controller, no communication (002)\n");
			precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNotOk", "Timeout while initializing LED controller: No Communication !\n" );
		}
		//printf("LED:updateMaxCurrent\n");
	}
}

void LEDdriverT::ExtractIntensityValues(int p_pIntensityArray[], int nCannels)
{
	if(driver_enabled!=0)
	{
		if (pLEDdriverHW->ExtractIntensityValues(p_pIntensityArray, pLEDdriverHW->nChannels) != pLEDdriverHW->nChannels)
		{
			//state = 0;
			printf("LED: Timeout beim Senden von Parametern! KOMMUNIKATION ABGEBROCHEN! (003)\n");
			precitec::wmLog(precitec::eDebug, "Timeout while sending data to LED controller, no communication (003)\n");
			precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNoData", "Timeout while sending data to LED controller: No Communication !\n" );
			return;
		}
	}
}

void LEDdriverT::ReadConfiguration()
{
	if(driver_enabled!=0)
	{
		if (pLEDdriverHW->ReadConfiguration()== -1)
		{
			//state = 0;
			printf("LED: Timeout beim Senden von Parametern! KOMMUNIKATION ABGEBROCHEN! (002)\n");
			precitec::wmLog(precitec::eDebug, "Timeout while sending data to LED controller, no communication (002)\n");
			precitec::wmFatal( precitec::eLighting, "QnxMsg.VI.LEDcontrNoData", "Timeout while sending data to LED controller: No Communication !\n" );
			return;
		}
	}
}

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

