/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDdriver.h                    Author :    KIH          *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:  Ansteuerung LED Beleuchtung :                           *
*               Oberste Schicht, kennt nur fuer Souvis relevante Steuerbefehle *
*                                                                       *
************************************************************************/

#ifndef LED_DRIVER_H_
#define LED_DRIVER_H_

#include "LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "LEDcomBase.h"
#include "LEDdriverHWBase.h"

class LEDdriverT
{
	public:
		LEDdriverT();
		~LEDdriverT();
		void init(LEDdriverHWBaseT * pLEDdriverHWin,LEDcomBaseT * pledcom,LEDdriverParameterT & LEDdriverParameter, int iLEDmaxCurrSet[]);
		void deinit();
		void updateall(LEDI_ParametersT *inpars);
		void updateall_in_once(LEDI_ParametersT *inpars);
		void updateMaxCurrent(int LEDmaxCurrSet[]);

		void writeSetupPersistent();
		void ExtractIntensityValues(int p_pIntensityArray[], int nCannels);
		void ReadConfiguration();
		int errornum() { return error; };
		int seamno;

		int getdebuglevel() { if(pdebug!=NULL) return *pdebug; else return -1; };
		void setdebuglevelsource(int * pdebug_in) { pdebug=pdebug_in;};

	//private:
		int driver_enabled;
		int state;
		int error;
		LEDI_ParametersT currentparams[20];
		LEDdriverHWBaseT * pLEDdriverHW;

        bool m_oControllerTypeIsWrong;

	private:
		int *pdebug;
};

extern LEDdriverT led_driver;

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

#endif // LED_DRIVER_H_

