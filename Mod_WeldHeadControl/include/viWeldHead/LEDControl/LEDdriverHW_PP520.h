/************************************************************************
* Project    :  SOUVIS 5000                    Changed: 08.01.2007      *
* Module     :  ips2                           Version:                 *
* Filename   :  LEDdriver.h                    Author :    KIH          *
*                                                                       *
* Copyright  :  (c) 2007  Soudronic AG Neftenbach                       *
*                                                                       *
* Description:  Ansteuerung LED Beleuchtung                             *
*                                                                       *
************************************************************************/

#ifndef LED_DRIVER_HW_PP520_H_
#define LED_DRIVER_HW_PP520_H_

#include "LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "LEDcomBase.h"
#include "LEDdriverHWBase.h"

class LEDdriverHW_PP520T : public LEDdriverHWBaseT
{
	public:
		LEDdriverHW_PP520T();
		~LEDdriverHW_PP520T();
		int init(LEDcomBaseT * comin,LEDdriverParameterT & LEDdriverParameter, int iLEDmaxCurrSet[]);
		void deinit();
		void printstate();
		int writeSetupPersistent();
		int errornum() { return error; };

	//private:

		void initChannel(int channelnumber, int brightness);
		void deinitchannel(int channelnumber);
		int updateChannel(int channelnumber, LEDI_ParametersT *inpars);
		int updateChannelAll(LEDI_ParametersT *inpars);
		int MeasureLightRating(lightRatingStructureT *ras,int nCannels);
		int ReFlashDriver(double ras[],int nCannels);
		int ExtractIntensityValues(int IntensityArray[], int nCannels);
		int CheckConnection(void);
		int ReadVersion(void);
		int ReadConfiguration(void);

		static const LEDI_ParametersInternT internal_parameters;

    private:
        double m_oPulseDelay;
};

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

#endif // LED_DRIVER_HW_PP520_H_

