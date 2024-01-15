#include <math.h>
#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/time.h>
#include "viWeldHead/LEDControl/LEDdriverHW_PP820.h"

#include "module/moduleLogger.h"
#include "common/systemConfiguration.h"

//LEDI_ParametersInternT: in_pulselength_us, in_current_mA, retriggerdelay_ms
const LEDI_ParametersInternT LEDdriverHW_PP820T::internal_parameters(80,200,1);

LEDdriverHW_PP820T::LEDdriverHW_PP820T()
{
	minChannelNumber = 1;
	maxChannelNumber = 8;
	nChannels = 1 + maxChannelNumber - minChannelNumber;

	us_wait_after_update=200;
	us_wait_after_init=200;
	us_wait_after_singlechannel_update=200;

	state = 0;
	error = 0;

    if(precitec::interface::SystemConfiguration::instance().getInt("CameraInterfaceType", 0) == 1) // GigE camera
    {
        m_oPulseDelay = 0.02f;
    }
    else // CameraLink camera with framegrabber
    {
        //m_oPulseDelay = 0.02f; // this is used in SOUVIS5200
        m_oPulseDelay = 0.4f; // this is needed in Weldmaster !?
    }

    for(int i = 0; i < ANZ_LEDI_PARAMETERS; i++)
    {
        m_oLEDchannelMaxCurrent[i] = 0;
    }
}

LEDdriverHW_PP820T::~LEDdriverHW_PP820T()
{
}

int LEDdriverHW_PP820T::init(LEDcomBaseT * comin,LEDdriverParameterT & LEDdriverParameter, int iLEDmaxCurrSet[])
{
	printf("LEDdriverHW_PP820T::init\n");
	int i;
	char str[1000];

	error = 0; //evtl das extra
	state = 0;
	com = comin;
	if(com == NULL)
	{
		return 0;
	}

	com->init(LEDdriverParameter);
	error = com->errornum();
	if(error == 0)
	{
		state = 1;
	}

	int oRetValue = ReadVersion();
	if(oRetValue == -1)
	{
		return -1;
	}
	else if(oRetValue == -2)
	{
		return -2;
	}
	if(ReadConfiguration() == -1)
	{
		return -1;
	}

	//double AmpereLimit = 1e-3 * internal_parameters.led_current_mA; //0.2A Strom fuer 100%  wobei 1000% das Maximum ist :(

	int nChannels = 8;
	for(i = 0; i < nChannels; ++i)
	{
		m_oLEDchannelMaxCurrent[i] = iLEDmaxCurrSet[i];
	}

	//turn off internal trigger
	sprintf(str,"TT0\r");
	com->rw(str,outdat);

	// all channel have own trigger signals (email Maurizio 8.4.2022)
	sprintf(str,"FP0\r");
	com->rw(str,outdat);

	int oIntensityArray[8];
	if(ExtractIntensityValues(oIntensityArray, nChannels) != nChannels)
	{
		printf("Error in ExtractIntensityValues()\n");
	}

	for(i = minChannelNumber; i <= maxChannelNumber; ++i)
	{
		initChannel(i, oIntensityArray[i -1]);
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

void LEDdriverHW_PP820T::deinit()
{
	int channelnumber;

	for(channelnumber = minChannelNumber; channelnumber <= maxChannelNumber; ++channelnumber)
	{
        int PulseOnTime_us = internal_parameters.led_pulselength_us;
        if(PulseOnTime_us < 40)
        {
            PulseOnTime_us = 40;
        }
        if(PulseOnTime_us > 400)
        {
            PulseOnTime_us = 400;
        }
        int PulseDelay_us = std::lround(m_oPulseDelay * 1000.0);
        double PulseCurrent_A = 0.0;

		sprintf(str, "RT%d,%d,%d,%g,%d\r", (channelnumber - 1), PulseOnTime_us, PulseDelay_us, PulseCurrent_A, (internal_parameters.retriggerdelay_ms * 1000));
		com->rw(str,outdat);
   	}

	com->deinit();
	error = com->errornum();
	state = 0;
}

void LEDdriverHW_PP820T::printstate() // wird derzeit nicht aufgerufen
{
	printf("err=%d\n", error);
	printf("state=%d\n", state);
}

void LEDdriverHW_PP820T::initChannel(int channelnumber, int brightness)
{
    int PulseOnTime_us = internal_parameters.led_pulselength_us;
    if(PulseOnTime_us < 40)
    {
        PulseOnTime_us = 40;
    }
    if(PulseOnTime_us > 400)
    {
        PulseOnTime_us = 400;
    }
    int PulseDelay_us = std::lround(m_oPulseDelay * 1000.0);
    int PulseCurrent_mA = (m_oLEDchannelMaxCurrent[channelnumber - 1] * brightness) / 100;
    PulseCurrent_mA = std::lround(static_cast<float>(PulseCurrent_mA) / 100.0) * 100;
    double PulseCurrent_A = static_cast<double>(PulseCurrent_mA) / 1000.0;

	sprintf(str, "RT%d,%d,%d,%g,%d\r", (channelnumber - 1), PulseOnTime_us, PulseDelay_us, PulseCurrent_A, (internal_parameters.retriggerdelay_ms * 1000));
	com->rw(str,outdat);
}

int LEDdriverHW_PP820T::updateChannel(int channelnumber,LEDI_ParametersT *mypars)
{
	int PulseOnTime_us = internal_parameters.led_pulselength_us;
	int percentage; // 0..1000
	char oCommandString[1000];
	char oReturnString[1000];

	percentage = mypars->led_brightness;
	if(percentage < 0)
	{
		percentage = 0;
	}
	if(percentage > 100)
	{
		percentage = 100;
	}
	PulseOnTime_us = mypars->led_pulse_width;
	if(PulseOnTime_us < 40)
	{
		PulseOnTime_us = 40;
	}
	if(PulseOnTime_us > 400)
	{
		PulseOnTime_us = 400;
	}
    int PulseDelay_us = std::lround(m_oPulseDelay * 1000.0);
    int PulseCurrent_mA = (m_oLEDchannelMaxCurrent[channelnumber - 1] * percentage) / 100;
    PulseCurrent_mA = std::lround(static_cast<float>(PulseCurrent_mA) / 100.0) * 100;
    double PulseCurrent_A = static_cast<double>(PulseCurrent_mA) / 1000.0;

	sprintf(oCommandString, "RT%d,%d,%d,%g,%d\r", (channelnumber - 1), PulseOnTime_us, PulseDelay_us, PulseCurrent_A, (internal_parameters.retriggerdelay_ms * 1000));
	oReturnString[0] = 0x00;
	com->rw(oCommandString,oReturnString);
	if(oReturnString[0] == '\0')
	{
		return -1;
	}
	if(com->errornum() != 0)
	{
		printf("err=%d\n", com->errornum());
	}

	usleep(us_wait_after_singlechannel_update);

	return 0;
}

int LEDdriverHW_PP820T::updateChannelAll(LEDI_ParametersT *mypars)
{
    char oCommandString[1000]{};
    char oReturnString[1000]{};

    int PulseOnTime_us = internal_parameters.led_pulselength_us;
    int percentage; // 0..100

    for(int i = 0;i < nChannels; ++i)
    {
        if(mypars[i].led_enable > 0)
        {
            percentage = mypars[i].led_brightness;
            if(percentage < 0)
            {
                percentage = 0;
            }
            if(percentage > 100)
            {
                percentage = 100;
            }
        }
        else
        {
            percentage = 0;
        }
        PulseOnTime_us = mypars[i].led_pulse_width;
        if(PulseOnTime_us < 40)
        {
            PulseOnTime_us = 40;
        }
        if(PulseOnTime_us > 400)
        {
            PulseOnTime_us = 400;
        }
        int PulseDelay_us = std::lround(m_oPulseDelay * 1000.0);
        int PulseCurrent_mA = (m_oLEDchannelMaxCurrent[i] * percentage) / 100;
        PulseCurrent_mA = std::lround(static_cast<float>(PulseCurrent_mA) / 100.0) * 100;
        double PulseCurrent_A = static_cast<double>(PulseCurrent_mA) / 1000.0;

        char oHelpStrg[1000];
        sprintf(oHelpStrg, "RT%d,%d,%d,%g,%d\r", i, PulseOnTime_us, PulseDelay_us, PulseCurrent_A, (internal_parameters.retriggerdelay_ms * 1000));
        strcat(oCommandString, oHelpStrg);
        if(i < (nChannels -1))
        {
            strcat(oCommandString, ";");
        }
    }
    strcat(oCommandString, "\r");

//    com->rw(oCommandString,oReturnString,false,true);
    com->rw(oCommandString,oReturnString,false,false);
    if(oReturnString[0] == '\0')
    {
        return -1;
    }
    if(com->errornum() != 0)
    {
        printf("err=%d\n", com->errornum());
    }

    return 0;
}

void LEDdriverHW_PP820T::deinitchannel(int channelnumber)
{
}

int LEDdriverHW_PP820T::writeSetupPersistent()
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
	if(ReadConfiguration() == -1)
	{
		return -1;
	}

	return 0;
}

// function is needed because of base class
int LEDdriverHW_PP820T::ReFlashDriver([[maybe_unused]] double ras[],[[maybe_unused]] int nCannels)
{
    return 0;
}

int LEDdriverHW_PP820T::ExtractIntensityValues(int IntensityArray[], int nCannels)
{
	char oCommandString[1000];
	char oReturnString[1000];
	int i;
	char *tp1,*tp2,*tp3;
	char oStatusChan[8][200];
	double oHelpFloat;

    for(int i = 0; i < nChannels; i++)
    {
        sprintf(oCommandString,"ST%d\r", i);
        com->rw(oCommandString,oReturnString,false);
        printf("ReadConfiguration: oReturnString (ST%d):\n%s\n",i,oReturnString);
        if(oReturnString[0] == '\0')
        {
            return -1;
        }
        // Trennen in einzelne Strings
        tp1=strtok(oReturnString,"\n"); // Wiederholung Befehl
        tp1=strtok(NULL,"\n"); // Status Kanal
        strcpy(oStatusChan[i], tp1);
    }
	//printf("#Chan1#\n%s\n",oStatusChan[0]);
	//printf("#Chan2#\n%s\n",oStatusChan[1]);
	//printf("#Chan3#\n%s\n",oStatusChan[2]);
	//printf("#Chan4#\n%s\n",oStatusChan[3]);
	//printf("#Chan5#\n%s\n",oStatusChan[4]);
	//printf("#Chan6#\n%s\n",oStatusChan[5]);
	//printf("#Chan7#\n%s\n",oStatusChan[6]);
	//printf("#Chan8#\n%s\n",oStatusChan[7]);

	for(i = 0; i < nCannels; ++i)
	{
		tp2 = strstr(oStatusChan[i], "V ");
		tp3 = strstr(tp2, "A ");

		int charCnt = tp3 - tp2 - 2;
		char oTempStrg[20];
		strncpy(oTempStrg, (tp2 + 2), charCnt);
		oTempStrg[charCnt] = 0x00;
		oHelpFloat = atof(oTempStrg);
		IntensityArray[i] = (int)(oHelpFloat * 1000.0 * 100.0) / m_oLEDchannelMaxCurrent[i];
	}
	return i;
}

int LEDdriverHW_PP820T::CheckConnection(void)
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

	sprintf(str1,"ST0\r");
	com->rw(str1,str2,false);
printf("CheckConnection: str2 (ST0):\n%s\n",str2);
	if(str2[0] == '\0')
	{
		return -1;
	}

	return 0;
}

int LEDdriverHW_PP820T::ReadVersion(void)
{
	char str1[1000];
	char str2[1000];

	sprintf(str1,"VR\r");
	com->rw(str1,str2,false);
printf("ReadVersion: str2 (VR):\n%s\n",str2);
    if(strstr(str2, "PP420F") != nullptr)
    {
        printf("There is a PP420F present -> not ok !\n");
        return -2;
    }
    else if(strstr(str2, "PP520") != nullptr)
    {
        printf("There is a PP520 present -> not ok !\n");
        return -2;
    }
    else if(strstr(str2, "PP820") != nullptr)
    {
        printf("There is a PP820 present -> ok !\n");
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

int LEDdriverHW_PP820T::ReadConfiguration(void)
{
	char str1[1000];
	char str2[1000];

    for(int i = 0; i < nChannels; i++)
    {
        sprintf(str1,"ST%d\r", i);
        com->rw(str1,str2,false);
printf("ReadConfiguration: str2 (ST%d):\n%s\n",i,str2);
        if(str2[0] == '\0')
        {
            return -1;
        }
    }
    // read general configuration
	sprintf(str1,"ST8\r");
	com->rw(str1,str2,false);
printf("ReadConfiguration: str2 (ST8):\n%s\n",str2);
	if(str2[0] == '\0')
	{
		return -1;
	}
	return 0;
}

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

