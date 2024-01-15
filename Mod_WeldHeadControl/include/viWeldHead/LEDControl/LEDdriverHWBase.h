/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDdriverHWBase.h              Author :    KIH          *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* LEDdriverHWBaseT                                                      *
* wird von LEDdriverT verwendet                                         *
* leistet die Befehle updatechannel und initchannel                     *
* setzt die jeweiligen Kommandos der Treiberhardware ein                *
* zur Kommunikation mit der treiberhardware wird LEDcomBaseT * com; benutzt *
************************************************************************/

#ifndef LED_DRIVER_HW_BASE_H_
#define LED_DRIVER_HW_BASE_H_

#include "LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "LEDcomBase.h"

class LEDdriverHWBaseT
{
	public:
		LEDdriverHWBaseT();
		virtual int init(LEDcomBaseT * comin,LEDdriverParameterT & LEDdriverParameter, int iLEDmaxCurrSet[])=0;
		virtual void deinit()=0;
		virtual ~LEDdriverHWBaseT();

		virtual void printstate()=0;
		int errornum(void);

		virtual int updateChannel(int channelnumber,LEDI_ParametersT *inpars)=0;
		virtual int updateChannelAll(LEDI_ParametersT *inpars)=0;
		virtual int ReFlashDriver(double ras[],int nCannels)=0;
		virtual int ExtractIntensityValues(int IntensityArray[], int nCannels)=0;
		virtual int ReadConfiguration(void)=0;
		void disableChannel(int channelnummer,int pulsewidth);
		virtual int writeSetupPersistent()=0;

		int getdebuglevel() { if(ppdebug!=0){ if(*ppdebug!=0) return **ppdebug; else return -1;} else return -1; };
		void setdebuglevelsource(int ** ppdebug_in) { ppdebug=ppdebug_in;};

		char outdat[1000];

		int minChannelNumber;
		int maxChannelNumber;
		int nChannels;
		long us_wait_after_update;
		long us_wait_after_singlechannel_update;
		long us_wait_after_init;

	protected:
		int state;
		int error;

		LEDcomBaseT * com;

		char str[300];

		const static int ERR_INIT_FAILED  =1;
		const static int ERR_COMMUNICATION_FAILED  =2;
		const static int ERR_TRY_TO_SEND_NOT_INITIALIZED  =4;

	private:
		int **ppdebug;
};

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

#endif // LED_DRIVER_HW_BASE_H_

