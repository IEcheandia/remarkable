#pragma once

#include "LEDI_ExportedDatatypes.hpp"

#include "LEDcomBase.h"
#include "LEDdriverHWBase.h"

class LensDriverTRCL180 : public LEDdriverHWBaseT
{
public:
    LensDriverTRCL180();
    ~LensDriverTRCL180();
    int init(LEDcomBaseT* comin, LEDdriverParameterT& LEDdriverParameter, int iLEDmaxCurrSet[]) override;
    void deinit() override;
    void printstate() override;
    int writeSetupPersistent() override;
    int errornum()
    {
        return error;
    };

private:
    int updateChannel(int channelnumber, LEDI_ParametersT* inpars) override;
    int updateChannelAll(LEDI_ParametersT* inpars) override;
    int ReFlashDriver(double ras[],int nCannels) override;
    int ExtractIntensityValues(int IntensityArray[], int nCannels) override;
    int CheckConnection(void);
    int ReadVersion(void);
    int ReadConfiguration(void) override;
};

