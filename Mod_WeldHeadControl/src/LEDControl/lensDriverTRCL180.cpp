#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "viWeldHead/LEDControl/lensDriverTRCL180.h"

#include "module/moduleLogger.h"
#include "common/systemConfiguration.h"

LensDriverTRCL180::LensDriverTRCL180()
{
    minChannelNumber = 1;
    maxChannelNumber = 1;
    nChannels = 1 + maxChannelNumber - minChannelNumber;

    us_wait_after_update = 200;
    us_wait_after_init = 200;
    us_wait_after_singlechannel_update = 200;

    state = 0;
    error = 0;
}

LensDriverTRCL180::~LensDriverTRCL180()
{
}

int LensDriverTRCL180::init(LEDcomBaseT* comin, LEDdriverParameterT& LEDdriverParameter, [[maybe_unused]] int iLEDmaxCurrSet[])
{
    printf("LensDriverTRCL180::init\n");
    char str[1000];

    error = 0;
    state = 0;
    com = comin;
    if (com == NULL)
    {
        return 0;
    }

    com->init(LEDdriverParameter);
    error = com->errornum();
    if (error == 0)
    {
        state = 1;
    }

    int oRetValue = ReadVersion();
    if (oRetValue == -1)
    {
        // LED-Controller connection error
        return -1;
    }
    else if (oRetValue == -2)
    {
        // wrong type of LED-Controller
        return -2;
    }
    if (ReadConfiguration() == -1)
    {
        // LED-Controller connection error
        return -1;
    }

    //disable keyboard
    sprintf(str, "KB1,8\r");
    com->rw(str, outdat);

    // set lens to 0.0 dpt
    sprintf(str, "RS1,0.0");
    com->rw(str, outdat);

    usleep(us_wait_after_init);
    return 0;
}

void LensDriverTRCL180::deinit()
{
    // set lens to 0.0 dpt
    sprintf(str, "RS1,0.0");
    com->rw(str, outdat);

    //enable keyboard
    sprintf(str, "KB0,8\r");
    com->rw(str, outdat);

    com->deinit();
    error = com->errornum();
    state = 0;
}

void LensDriverTRCL180::printstate()
{
    // override of virtual function: dummy function, not needed with lens controller
}

int LensDriverTRCL180::updateChannel([[maybe_unused]] int channelnumber, LEDI_ParametersT *mypars)
{
    double newValue = static_cast<double>(mypars->led_brightness) / 1000.0;

    char commandString[1000];
    char returnString[1000];

    sprintf(commandString, "RS1,%g\r", newValue);
    returnString[0] = 0x00;
    com->rw(commandString, returnString);
    if(returnString[0] == '\0')
    {
        return -1;
    }
    if (com->errornum() != 0)
    {
        printf("err=%d\n", com->errornum());
    }

    usleep(us_wait_after_singlechannel_update);

    return 0;
}

int LensDriverTRCL180::updateChannelAll([[maybe_unused]] LEDI_ParametersT *mypars)
{
    // override of virtual function: dummy function, not needed with lens controller
    return -1;
}

int LensDriverTRCL180::writeSetupPersistent()
{
    // override of virtual function: dummy function, not needed with lens controller
    return 0;
}

int LensDriverTRCL180::ReFlashDriver([[maybe_unused]] double ras[], [[maybe_unused]] int nCannels)
{
    // override of virtual function: dummy function, not needed with lens controller
    return 0;
}

int LensDriverTRCL180::ExtractIntensityValues([[maybe_unused]] int IntensityArray[], [[maybe_unused]] int nCannels)
{
    // override of virtual function: dummy function, not needed with lens controller
    return 0;
}

int LensDriverTRCL180::CheckConnection(void)
{
    char str1[1000];
    char str2[1000];

    sprintf(str1, "VR\r");
    com->rw(str1, str2, false);
    printf("CheckConnection: str2 (VR):\n%s\n",str2);
    if (str2[0] == '\0')
    {
        return -1;
    }

    sprintf(str1, "ST\r");
    com->rw(str1, str2, false);
    printf("CheckConnection: str2 (ST):\n%s\n", str2);
    if (str2[0] == '\0')
    {
        return -1;
    }

    return 0;
}

int LensDriverTRCL180::ReadVersion(void)
{
    char str1[1000];
    char str2[1000];

    sprintf(str1, "VR\r");
    com->rw(str1, str2, false);
    printf("ReadVersion: str2 (VR):\n%s\n",str2);
    if (strstr(str2, "TR-CL180") != nullptr)
    {
        printf("There is a TR-CL180 present -> ok !\n");
    }
    else
    {
        printf("There is a unknown lens controller present -> not ok !\n");
        return -2;
    }

    if(str2[0] == '\0')
    {
        return -1;
    }
    char helpStrg[81]{};
    strncpy(helpStrg, (str2 + 2), (strlen(str2) - 4));
    helpStrg[(strlen(str2) - 4)] = 0x00;
    precitec::wmLog(precitec::eDebug, helpStrg);
    return 0;
}

int LensDriverTRCL180::ReadConfiguration(void)
{
    char str1[1000];
    char str2[1000];

    sprintf(str1, "ST\r");
    com->rw(str1, str2, false);
    printf("ReadConfiguration: str2 (ST):\n%s\n",str2);
    if(str2[0] == '\0')
    {
        return -1;
    }
    return 0;
}

