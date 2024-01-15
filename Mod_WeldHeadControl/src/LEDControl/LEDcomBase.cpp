/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDcomBase.cpp                 Author : KIH             *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:                                                          *
*                                                                       *
************************************************************************/

#include "viWeldHead/LEDControl/LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "viWeldHead/LEDControl/LEDcomBase.h"

LEDcomBaseT::LEDcomBaseT()
{
	state=0;
	error=0;
	us_wait_after_send=1000;
}

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

