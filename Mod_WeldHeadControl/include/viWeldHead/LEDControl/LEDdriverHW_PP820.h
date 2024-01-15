#pragma once

#include "LEDI_ExportedDatatypes.hpp"

#ifndef LED_DRIVER_DISABLE_COMPLETE_CODE

#include "LEDcomBase.h"
#include "LEDdriverHWBase.h"

class LEDdriverHW_PP820T : public LEDdriverHWBaseT
{
	public:
		LEDdriverHW_PP820T();
		~LEDdriverHW_PP820T();
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
		int ReFlashDriver(double ras[],int nCannels);
		int ExtractIntensityValues(int IntensityArray[], int nCannels);
		int CheckConnection(void);
		int ReadVersion(void);
		int ReadConfiguration(void);

		static const LEDI_ParametersInternT internal_parameters;

    private:
        double m_oPulseDelay;
        int m_oLEDchannelMaxCurrent[ANZ_LEDI_PARAMETERS];
};

#endif // LED_DRIVER_DISABLE_COMPLETE_CODE

