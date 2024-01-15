/*************************************************************************
 * Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
 * Module     :  ips2                           Version:                 *
 * Filename   :  LEDdriverHW_PP420F.cpp         Author : KIH             *
 *                                                                       *
 * Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
 *                                                                       *
 * Description:  Ansteuerung LED Beleuchtung                             *
 *                                                                       *
 ************************************************************************/

#include <math.h>
#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "viWeldHead/LEDControl/LEDdriverHW_PP420F.h"

#include "module/moduleLogger.h"
#include "common/systemConfiguration.h"

//LEDI_ParametersInternT: in_pulselength_us, in_current_mA, retriggerdelay_ms
const LEDI_ParametersInternT LEDdriverHW_PP420T::internal_parameters(80,200,1);

LEDdriverHW_PP420T::LEDdriverHW_PP420T()
{
	minChannelNumber = 1;
	maxChannelNumber = 4;
	nChannels = 1 + maxChannelNumber - minChannelNumber;

	us_wait_after_update=200;
	us_wait_after_init=200;
	us_wait_after_singlechannel_update=200;

	state = 0;
	error = 0;

    if ( precitec::interface::SystemConfiguration::instance().getInt("CameraInterfaceType", 0) == 1) // GigE camera
    {
        m_oPulseDelay = 0.02f;
    }
    else // CameraLink camera with framegrabber
    {
        //m_oPulseDelay = 0.02f; // this is used in SOUVIS5200
        m_oPulseDelay = 0.4f; // this is needed in Weldmaster !?
    }
}

LEDdriverHW_PP420T::~LEDdriverHW_PP420T()
{
}

int LEDdriverHW_PP420T::init(LEDcomBaseT * comin,LEDdriverParameterT & LEDdriverParameter, int iLEDmaxCurrSet[])
{
	printf("LEDdriverHW_PP420T::init\n");
	int i;
	char str[1000];
	double dLEDrating[8];

	error = 0; //evtl das extra
	state = 0;
	com = comin;
	if (com == NULL)
		return 0;

	com->init(LEDdriverParameter);
	error = com->errornum();
	if (error == 0)
		state = 1;

	int oRetValue = ReadVersion();
	if( oRetValue == -1 )
	{
		//printf("LED-Controller connection error\n");
		return -1;
	}
	else if( oRetValue == -2 )
	{
		//printf("wrong type of LED-Controller\n");
		return -2;
	}
	if( ReadConfiguration() == -1 )
	{
		//printf("LED-Controller connection error\n");
		return -1;
	}

	//Messung
	bool ReFlashNeededFlag=false;

	//double AmpereLimit = 1e-3 * internal_parameters.led_current_mA; //0.2A Strom fuer 100%  wobei 1000% das Maximum ist :(

	lightRatingStructureT ras[15];
	int nChannels=4;
	if (MeasureLightRating(ras,nChannels)== -1)
	{
		//printf("LED-Controller connection error\n");
		return -1;
	}

	for(i=0;i<nChannels;++i)
	{
		//printf("%d. %g    %g\n",i,ras[i].VoltageLimit,ras[i].AmpereLimit);
		// 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
		//dLEDrating[i]= iLEDmaxCurrSet[i]* 1e-4; // Umrechnung von maxCurrent in den rating-Faktor 2000mA -> 0.2 A rating
		dLEDrating[i]= iLEDmaxCurrSet[i]* 1e-3;
		//printf("Init: Umrechnung Rating: %d, %g\n",i, dLEDrating[i]);
		if(fabs(ras[i].AmpereLimit-dLEDrating[i])>=0.001)
		{
			ReFlashNeededFlag=true;
			//break;
		}
	}

	//turn off internal trigger
	sprintf(str,"TT0\r");
	com->rw(str,outdat);

	//Trigger Kanaele vergeben:
	sprintf(str,"RP1,1\r");
	com->rw(str,outdat); //channel1 use trigger 1
	sprintf(str,"RP2,1\r");
	com->rw(str,outdat);
	sprintf(str,"RP3,1\r");
	com->rw(str,outdat);
	sprintf(str,"RP4,1\r");
	com->rw(str,outdat);

	//printf("init all");

	int oIntensityArray[4];
	if (ExtractIntensityValues(oIntensityArray, nChannels) != nChannels)
	{
		printf("Error in ExtractIntensityValues()\n");
	}

	for (i = minChannelNumber; i <= maxChannelNumber; ++i)
	{
		initChannel(i, oIntensityArray[i -1]);
	}

	if(ReFlashNeededFlag)
	{
		usleep(300000);
		printf("Permanently store settings for power up setting\n");

		//lightRatingStructureT startras;
		//startras.VoltageLimit=0.0;
		//startras.AmpereLimit=AmpereLimit;
		ReFlashDriver(dLEDrating,nChannels);
	}

	if(LEDdriverParameter.useDriverInternalTrigger==false)
	{
		sprintf(str,"TT0\r");
		com->rw(str,outdat); //turn off internal trigger
	}
	else
	{
		sprintf(str,"TT1,10000\r");
		com->rw(str,outdat); //turn on internal trigger 10ms 100Hz
	}

	usleep(us_wait_after_init);
	return 0;
}

void LEDdriverHW_PP420T::deinit()
{
	int channelnumber;
	int percentage = 0;
	double PulseOnTime_ms= 1e-3*internal_parameters.led_pulselength_us;

	for (channelnumber = minChannelNumber; channelnumber <= maxChannelNumber; ++channelnumber)
	{
		sprintf(str, "RT%d,%g,%g,%d,%d\r", channelnumber, PulseOnTime_ms, m_oPulseDelay, percentage,internal_parameters.retriggerdelay_ms);
		com->rw(str,outdat);
        //printf("err=%d\n", com->errornum());
   	}

	com->deinit();
	error = com->errornum();
	state = 0;
}

void LEDdriverHW_PP420T::printstate() // wird derzeit nicht aufgerufen
{
	printf("err=%d\n", error);
	printf("state=%d\n", state);
}

void LEDdriverHW_PP420T::initChannel(int channelnumber, int brightness)
{
	//LEDI_ParametersT *mypars = &tmppars;
	//int brightness=0;
	//printf("init channel %d  brightness=%d\n", channelnumber, brightness);

	double PulseOnTime_ms = 1e-3 * internal_parameters.led_pulselength_us;

	// 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
	//int percentage = 10* brightness;
	int percentage = brightness;
	sprintf(str, "RT%d,%g,%g,%d,%d\r", channelnumber, PulseOnTime_ms, m_oPulseDelay, percentage,internal_parameters.retriggerdelay_ms);
	//printf("initChannel_str: Ch:%d %s\n",channelnumber,str);
	com->rw(str,outdat);
}

int LEDdriverHW_PP420T::updateChannel(int channelnumber,LEDI_ParametersT *mypars)
{
	int PulseOnTime_us = internal_parameters.led_pulselength_us;
	double PulseOnTime_ms = internal_parameters.led_pulselength_us*1e-3;
	int percentage; // 0..1000
	char oCommandString[1000];
	char oReturnString[1000];

	//printf("update channel %d   brightness=%d  pulsewidth=%d\n", channelnumber, mypars->led_brightness, mypars->led_pulse_width);
	// 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
	//percentage = 10* mypars->led_brightness;
	percentage = mypars->led_brightness;
	if (percentage < 0) percentage = 0;
	if (percentage > 100) percentage = 100;

	PulseOnTime_us = mypars->led_pulse_width;
	if (PulseOnTime_us < 40) PulseOnTime_us = 40;
	if (PulseOnTime_us > 400) PulseOnTime_us = 400;
	PulseOnTime_ms = PulseOnTime_us*1e-3;

	sprintf(oCommandString, "RT%d,%g,%g,%d,%d\r", channelnumber, PulseOnTime_ms, m_oPulseDelay, percentage,internal_parameters.retriggerdelay_ms);
    //printf("oCommandString: %s\n", oCommandString);
	oReturnString[0] = 0x00;
	com->rw(oCommandString,oReturnString);
	if(oReturnString[0] == '\0')
	{
		return -1;
	}
	if (com->errornum() != 0) printf("err=%d\n", com->errornum());

	usleep(us_wait_after_singlechannel_update);

	return 0;
}

int LEDdriverHW_PP420T::updateChannelAll(LEDI_ParametersT *mypars)
{
    char oCommandString[1000] {};
    char oReturnString[1000] {};

    int PulseOnTime_us = internal_parameters.led_pulselength_us;
    double PulseOnTime_ms = internal_parameters.led_pulselength_us*1e-3;
    int percentage; // 0..100

    for(int i = 0;i < nChannels;++i)
    {
        if (mypars[i].led_enable > 0)
        {
            // 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
            //percentage = 10* mypars->led_brightness;
            percentage = mypars[i].led_brightness;
            if (percentage < 0) percentage = 0;
            if (percentage > 100) percentage = 100;
        }
        else
        {
            percentage = 0;
        }

        PulseOnTime_us = mypars[i].led_pulse_width;
        if (PulseOnTime_us < 40) PulseOnTime_us = 40;
        if (PulseOnTime_us > 400) PulseOnTime_us = 400;
        PulseOnTime_ms = PulseOnTime_us*1e-3;

        char oHelpStrg[1000];
        sprintf(oHelpStrg, "RT%d,%g,%g,%d,%d", i+1, PulseOnTime_ms, m_oPulseDelay, percentage,internal_parameters.retriggerdelay_ms);
        strcat(oCommandString, oHelpStrg);
        if (i < (nChannels -1)) strcat(oCommandString, ";");
    }
    strcat(oCommandString, "\r");

//    com->rw(oCommandString,oReturnString,false,true);
    com->rw(oCommandString,oReturnString,false,false);
    if(oReturnString[0] == '\0')
    {
        return -1;
    }
    if (com->errornum() != 0) printf("err=%d\n", com->errornum());

    return 0;
}

void LEDdriverHW_PP420T::deinitchannel(int channelnumber)
{
}

int LEDdriverHW_PP420T::writeSetupPersistent()
{
	char oCommandString[1000];
	char oReturnString[1000];

	sprintf(oCommandString,"AW\r");
	oReturnString[0] = 0x00;
	com->rw(oCommandString,oReturnString);
	if(oReturnString[0] == '\0')
	{
		return -1;
	}
	printf("writeSetupPersistent done\n");
	if( ReadConfiguration() == -1 )
	{
		return -1;
	}

	return 0;
}

int LEDdriverHW_PP420T::ReFlashDriver(double ras[],int nCannels)
{
	char str1[1000];
	char str2[1000];
	int i,channelnumber;
	lightRatingStructureT rasCheck[15];

	if( CheckConnection() == -1 )
	{
		//printf("LED-Controller connection error\n");
		return -1;
	}

	for(i=0;i<nCannels;++i)
	{
		channelnumber=i+1;

		// 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
		//if (ras[i] < 0.0) ras[i] = 0.0;
		//if (ras[i] > 0.3) ras[i] = 0.3;
		if (ras[i] < 0.0) ras[i] = 0.0;
		if (ras[i] > 5.0) ras[i] = 5.0;

		//Maximalwert fuer Strom setzen
		//printf("ReFlash: vor Setzen Ch: %d, %g\n", i, ras[i]);
		sprintf(str1,"RR%d,%gA\r",channelnumber,ras[i]);
		com->rw(str1,str2);
		usleep(500*1000); //box erstmal einstellen lassen

		//LED ausschalten
		//sprintf(str1, "RS%d,%d\r",channelnumber,0);
		double PulseOnTime_ms= 1e-3*internal_parameters.led_pulselength_us;
		sprintf(str1, "RT%d,%g,%g,%d,%d\r", channelnumber, PulseOnTime_ms, m_oPulseDelay, 0,internal_parameters.retriggerdelay_ms);

		com->rw(str1,str2);
		usleep(500*1000);
	}

	//Settings nichtfluechtig schreiben
	sprintf(str1,"AW\r");
	com->rw(str1,str2);

	printf("Permanently store settings for power up setting\n");

	// Pruefen des maxCurrent auf dem LED-Controller
	usleep(500*1000);
	if( MeasureLightRating(rasCheck,nCannels) == -1 )
	{
		//printf("LED-Controller connection error\n");
		return -1;
	}

	if( ReadConfiguration() == -1 )
	{
		//printf("LED-Controller connection error\n");
		return -1;
	}

	return 0;
}

int LEDdriverHW_PP420T::MeasureLightRating(lightRatingStructureT *ras,int nCannels)
{
	char str1[1000];
	char str2[1000];
	int i;
	sprintf(str1,"ST\r");
	char *tp1,*tp2;

	com->rw(str1,str2,false);
	//printf("str2: %s\n",str2);
	if(str2[0] == '\0')
	{
		return -1;
	}

	tp1=strtok(str2," ,");

	for(i=0;i<nCannels;++i)
	{
		tp1=strtok(NULL,"S");
		tp1=strtok(NULL,"A");

		ras[i].AmpereLimit=strtod(tp1,&tp2);
		printf("MaxCurrent on LED-Controller: Ch: %d %f\n",i,ras[i].AmpereLimit);

		tp1=strtok(NULL,"F");
		tp1=strtok(NULL," ,");
		tp1=strtok(NULL," ,");
	}
	return i;
}

int LEDdriverHW_PP420T::ExtractIntensityValues(int IntensityArray[], int nCannels)
{
	char oCommandString[1000];
	char oReturnString[1000];
	int i;
	char *tp1,*tp2,*tp3;
	char oStatusChan[4][200];
	double oHelpFloat;

	sprintf(oCommandString,"ST\r");
	oReturnString[0] = 0x00;
	com->rw(oCommandString,oReturnString);
	if(oReturnString[0] == '\0')
	{
		return -1;
	}
	printf("ExtractIntensityValues: oReturnString:\n%s\n",oReturnString);

	// Trennen in 4 einzelne Strings, fuer jeden Kanal einen
	tp1=strtok(oReturnString,"\n"); // Wiederholung Befehl
	tp1=strtok(NULL,"\n"); // Status Kanal 1
	strcpy(oStatusChan[0], tp1);
	tp1=strtok(NULL,"\n"); // Status Kanal 2
	strcpy(oStatusChan[1], tp1);
	tp1=strtok(NULL,"\n"); // Status Kanal 3
	strcpy(oStatusChan[2], tp1);
	tp1=strtok(NULL,"\n"); // Status Kanal 4
	strcpy(oStatusChan[3], tp1);
	//printf("#Chan1#\n%s\n",oStatusChan[0]);
	//printf("#Chan2#\n%s\n",oStatusChan[1]);
	//printf("#Chan3#\n%s\n",oStatusChan[2]);
	//printf("#Chan4#\n%s\n",oStatusChan[3]);

	for(i=0;i<nCannels;++i)
	{
		tp2 = strstr(oStatusChan[i], "SE");
		//printf("#%s#\n", tp2);
		tp3 = strstr(tp2, ",");
		//printf("#%s#\n", tp3);

		int charCnt = tp3 - tp2 - 2;
		char oTempStrg[20];
		strncpy(oTempStrg, (tp2 + 2), charCnt);
		oTempStrg[charCnt] = 0x00;
		//printf("#%s#\n", oTempStrg);
		oHelpFloat = atof(oTempStrg);
		// 10.8.2016 EA: Umstellung von 0.2A und 1000% auf 2.0A und 100%
		//IntensityArray[i] = (int)(oHelpFloat) / 10;
		IntensityArray[i] = (int)(oHelpFloat);
		//printf("#%d#\n", IntensityArray[i]);
	}
	return i;
}

int LEDdriverHW_PP420T::CheckConnection(void)
{
	char str1[1000];
	char str2[1000];

	sprintf(str1,"VR\r");
	com->rw(str1,str2,false);
printf("CheckConnection: str2 (VR):\n%s\n",str2);
	if(str2[0] == '\0')
	{
		return -1;
	}

	sprintf(str1,"ST\r");
	com->rw(str1,str2,false);
printf("CheckConnection: str2 (ST):\n%s\n",str2);
	if(str2[0] == '\0')
	{
		return -1;
	}

	return 0;
}

int LEDdriverHW_PP420T::ReadVersion(void)
{
	char str1[1000];
	char str2[1000];

	sprintf(str1,"VR\r");
	com->rw(str1,str2,false);
printf("ReadVersion: str2 (VR):\n%s\n",str2);
    if (strstr(str2, "PP420F") != nullptr)
    {
        printf("There is a PP420F present -> ok !\n");
    }
    else if (strstr(str2, "PP520") != nullptr)
    {
        printf("There is a PP520 present -> not ok !\n");
        return -2;
    }
    else if (strstr(str2, "PP820") != nullptr)
    {
        printf("There is a PP820 present -> not ok !\n");
        return -2;
    }
    else
    {
        printf("There is a unknown controller present -> not ok !\n");
        return -2;
    }

	if(str2[0] == '\0')
	{
		return -1;
	}
	char helpStrg[81];
	strncpy(helpStrg, (str2 + 2), (strlen(str2)-4));
	precitec::wmLog(precitec::eDebug, helpStrg);
	return 0;
}

int LEDdriverHW_PP420T::ReadConfiguration(void)
{
	char str1[1000];
	char str2[1000];

	sprintf(str1,"ST\r");
	com->rw(str1,str2,false);
printf("ReadConfiguration: str2 (ST):\n%s\n",str2);
	if(str2[0] == '\0')
	{
		return -1;
	}
	return 0;
}

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

