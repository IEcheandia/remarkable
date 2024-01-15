/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDcomBase.h                   Author :    KIH          *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:  Kommunikation mit LED Ansteuerung                       *
*                                                                       *
************************************************************************/

#ifndef LED_COM_BASE_H_
#define LED_COM_BASE_H_

#include "LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#define LEDcomBaseT_BUFFERSIZE 1000

class LEDcomBaseT
{
	public:
		LEDcomBaseT();
		virtual void init(LEDdriverParameterT & LEDdriverParameter)=0;
		virtual void deinit()=0;
		virtual void rw(char *strin,char *strout,bool waitforEndFlag = true,bool p_oFastReturn=false)=0;
		int errornum() {return error;};

		long us_wait_after_send;

		int nreadbytes;
		//char buf[LEDcomBaseT_BUFFERSIZE];

		int getdebuglevel() { if(ppdebug!=0){ if(*ppdebug!=0) return **ppdebug; else return -1;} else return -1; };
		void setdebuglevelsource(int ** ppdebug_in) { ppdebug=ppdebug_in;};

	protected:
		int state;
		int error;

		const static int ERR_INIT_FAILED  =1;
		const static int ERR_COMMUNICATION_FAILED  =2;
		const static int ERR_TRY_TO_SEND_NOT_INITIALIZED  =4;

	private:
		int **ppdebug;
};

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

#endif // LED_COM_BASE_H_

