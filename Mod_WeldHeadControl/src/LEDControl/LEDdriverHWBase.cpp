/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDdriverHWBase.cpp            Author : KIH             *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:                                                          *
*                                                                       *
************************************************************************/

#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "viWeldHead/LEDControl/LEDdriverHWBase.h"
#include "stdio.h"

LEDdriverHWBaseT::LEDdriverHWBaseT()
{
	printf("LEDdriverHWBaseT::LEDdriverHWBaseT()\n");
	state=0;
	error=0;
	us_wait_after_update=10000;
	us_wait_after_init=10000;
	us_wait_after_singlechannel_update=50000;
}

LEDdriverHWBaseT::~LEDdriverHWBaseT()
{
}

void LEDdriverHWBaseT::disableChannel(int channelnumber,int pulsewidth)
{
	LEDI_ParametersT mypars;
	mypars.led_brightness=0;
	mypars.led_enable=0;
	mypars.led_pulse_width=pulsewidth;
	updateChannel(channelnumber,&mypars);
}
      	
int LEDdriverHWBaseT::errornum(void) 
{
	return error;
}

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

