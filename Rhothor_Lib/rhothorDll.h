/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef _WIN32
  #ifdef _BUILD_DLL_
    #define RHOTHOR __declspec(dllexport)
  #else
    #define RHOTHOR __declspec(dllimport)
  #endif
  #define CALLCONV _stdcall
#else
  #define RHOTHOR __attribute__ ((visibility ("default")))
  #define CALLCONV
#endif

// error codes
#define ERR_OK -1                    /* ok                                 */
#define ERR_BUSY 2                   /* system is processing job           */
#define ERR_JOB 3                    /* error processing job/empty job     */
#define ERR_HARDWARE 5               /* hardware not responding            */
#define ERR_DATA 13                  /* can't process data                 */
#define ERR_IMPLEMENTATION 23        /* function not supported             */
#define ERR_FLASH 45                 /* flash memory not enabled           */
#define ERR_INTERLOCK 48             /* IO status aborts execution         */
#define WARNING_BIOS 49              /* incompatible bios                  */

// rtListOpen parameter Mode options
#define LIST_START 1
#define AUTO_START 2
#define BOOT_START 3
#define LOAD_START 4

/************************************************************/
/*** accessing functions                                  ***/
/************************************************************/

RHOTHOR int32_t CALLCONV rtGetFirstFreeUSBDevice(char* Name);
RHOTHOR int32_t CALLCONV rtGetID(char* Name);
RHOTHOR int32_t CALLCONV rtGetIP(char* Mac, char* IP);
RHOTHOR int32_t CALLCONV rtGetNextFreeUSBDevice(char* Name);
RHOTHOR int32_t CALLCONV rtGetQueryTarget(int32_t* Index);
RHOTHOR int32_t CALLCONV rtGetSerial(int32_t* Serial);
RHOTHOR int32_t CALLCONV rtGetTarget(int32_t* Mask);
RHOTHOR int32_t CALLCONV rtGetVersion(char* Version);
RHOTHOR int32_t CALLCONV rtSelectDevice(const char* IP);
RHOTHOR int32_t CALLCONV rtSetQueryTarget(int32_t Index);
RHOTHOR int32_t CALLCONV rtSetTarget(int32_t Mask);
RHOTHOR int32_t CALLCONV rtSystemCheck(int32_t *Target, int32_t* Powered, int32_t* Bios);

/************************************************************/
/*** control and status functions                         ***/
/************************************************************/

RHOTHOR int32_t CALLCONV rtAbort();
RHOTHOR int32_t CALLCONV rtGetStatus(int32_t* Memory);
RHOTHOR int32_t CALLCONV rtFileOpen(const char* FileName);
RHOTHOR int32_t CALLCONV rtFileClose();
RHOTHOR int32_t CALLCONV rtFileCloseAtHost();
RHOTHOR int32_t CALLCONV rtFileCloseAtIndex(int32_t Index);
RHOTHOR int32_t CALLCONV rtListClose();
RHOTHOR int32_t CALLCONV rtListOpen(int32_t Mode);
RHOTHOR int32_t CALLCONV rtReset();
RHOTHOR int32_t CALLCONV rtSystemSelectFile(int32_t Index);
RHOTHOR int32_t CALLCONV rtSystemSuspend();
RHOTHOR int32_t CALLCONV rtSystemResume();

/************************************************************/
/*** front end functions                                  ***/
/************************************************************/

// flash functions
RHOTHOR int32_t CALLCONV rtEraseFromFlash(const char* FileName);
RHOTHOR int32_t CALLCONV rtFileUpload(const char* SrcFile, const char* FileName);
RHOTHOR int32_t CALLCONV rtFileUploadAtIndex(const char* SrcFile, const char* FileName, int32_t Index);
RHOTHOR int32_t CALLCONV rtFileDownload(const char* FileName, const char* DestFile);
RHOTHOR int32_t CALLCONV rtFormatFlash();
RHOTHOR int32_t CALLCONV rtGetFileIndex(const char* FileName, int32_t* Index);
RHOTHOR int32_t CALLCONV rtGetFlashFirstFileEntry(char* Name, int32_t* Size);
RHOTHOR int32_t CALLCONV rtGetFlashNextFileEntry(char* Name, int32_t* Size);
RHOTHOR int32_t CALLCONV rtGetFlashMemorySizes(int32_t* Total, int32_t* Allocated);
RHOTHOR int32_t CALLCONV rtLoadProfile(const char* FileName, int32_t Include);
RHOTHOR int32_t CALLCONV rtStoreProfile(const char* FileName, int32_t Include);

// system functions
RHOTHOR int32_t CALLCONV rtGetCanLink(int32_t Address, int32_t* Value);
RHOTHOR int32_t CALLCONV rtSystemUartOpen(int32_t baudrate, char parity, char stopbits);
RHOTHOR int32_t CALLCONV rtSystemUartWrite(int32_t bytes, char* data);
RHOTHOR int32_t CALLCONV rtSystemUDPsend(char* IP, int16_t port, char* data);
RHOTHOR int32_t CALLCONV rtUartRead(int32_t* bytes, char* data);

// rtListOpen(1,2,3 or 4) or rtFileOpen()
RHOTHOR int32_t CALLCONV rtOpenCanLink(int32_t Baudrate);
RHOTHOR int32_t CALLCONV rtScanCanLink(int32_t Address, int32_t Node, int32_t Index, int32_t SubIndex);
RHOTHOR int32_t CALLCONV rtSetCanLink(int32_t Node, int32_t Index, int32_t SubIndex, const char* Data);
RHOTHOR int32_t CALLCONV rtUDPsend(char* IP, int16_t port, char* data);
RHOTHOR int32_t CALLCONV rtWaitCanLink(int32_t Address, int32_t Value, int32_t Mask);

/************************************************************/
/*** single target functions  (rtSetTarget-lowest bit)    ***/
/*** Those functions query or control only the least      ***/
/*** significant selected target                          ***/
/*** rtSetTaget(0b*******1)-> Target 1                    ***/ 
/*** rtSetTaget(0b******10)-> Target 2                    ***/ 
/*** rtSetTaget(0b10000000)-> Target 8                    ***/ 
/************************************************************/

// calibration functions
RHOTHOR int32_t CALLCONV rtAddCalibrationData(const char* FileName);
RHOTHOR int32_t CALLCONV rtLoadCalibrationFile(const char* FileName);
RHOTHOR int32_t CALLCONV rtResetCalibration();
RHOTHOR int32_t CALLCONV rtStoreCalibrationFile(const char* FileName);

// rtListOpen(1,2,3 or 4) or rtFileOpen()
RHOTHOR int32_t CALLCONV rtWaitIdle();
RHOTHOR int32_t CALLCONV rtWaitIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtWaitPosition(double Window);
RHOTHOR int32_t CALLCONV rtWaitResolver(int32_t Nr, double TriggerPos, int32_t TriggerMode);
RHOTHOR int32_t CALLCONV rtWaitStall();

// rtListOpen(3 or 4) or rtFileOpen()
RHOTHOR int32_t CALLCONV rtElseIfIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtIfIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtWhileIO(int32_t Value, int32_t Mask);

/************************************************************/
/*** broadcast target functions (rtSetTarget)             ***/
/*** Those functions control all selected targets         ***/
/*** rtSetTarget(0b01000001)-> Target 1 and 7             ***/
/************************************************************/

// rtListOpen(1,2,3 or 4) or rtFileOpen()
RHOTHOR int32_t CALLCONV rtArcTo(double X, double Y, double BF);
RHOTHOR int32_t CALLCONV rtArcMoveTo(double X, double Y, double BF);
RHOTHOR int32_t CALLCONV rtBurst(int32_t Time);
RHOTHOR int32_t CALLCONV rtBurstEx(int32_t Time1, int32_t Time2, double PulseWidth2);
RHOTHOR int32_t CALLCONV rtCircle(double X, double Y, double Angle);
RHOTHOR int32_t CALLCONV rtCircleMove(double X, double Y, double Angle);
RHOTHOR int32_t CALLCONV rtFileFetch(const char* FileName);
RHOTHOR int32_t CALLCONV rtIncrementCounter();
RHOTHOR int32_t CALLCONV rtIndexFetch(int32_t Index);
RHOTHOR int32_t CALLCONV rtJumpTo(double X, double Y);
RHOTHOR int32_t CALLCONV rtLineTo(double X, double Y);
RHOTHOR int32_t CALLCONV rtMoveTo(double X, double Y);
RHOTHOR int32_t CALLCONV rtPrint(char* data);
RHOTHOR int32_t CALLCONV rtPulse(double X, double Y);
RHOTHOR int32_t CALLCONV rtResetCounter();
RHOTHOR int32_t CALLCONV rtResetEventCounter();
RHOTHOR int32_t CALLCONV rtResetResolver(int32_t Nr);
RHOTHOR int32_t CALLCONV rtSetAnalog(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtSetCfgIO(int32_t Nr, int32_t Value);
RHOTHOR int32_t CALLCONV rtSetCounter(int32_t Value);
RHOTHOR int32_t CALLCONV rtSetFieldSize(double Size);
RHOTHOR int32_t CALLCONV rtSetFont(const char* FileName);
RHOTHOR int32_t CALLCONV rtSetImageMatrix(double a11, double a12, double a21, double a22);
RHOTHOR int32_t CALLCONV rtSetImageMatrix3D(double a11, double a12, double a21, double a22, double a31, double a32);
RHOTHOR int32_t CALLCONV rtSetImageOffsXY(double X, double Y);
RHOTHOR int32_t CALLCONV rtSetImageOffsRelXY(double X, double Y);
RHOTHOR int32_t CALLCONV rtSetImageOffsZ(double Z);
RHOTHOR int32_t CALLCONV rtSetImageRotation(double Angle);
RHOTHOR int32_t CALLCONV rtSetIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtSetLaser(bool OnOff);
RHOTHOR int32_t CALLCONV rtSetLaserFirstPulse(double Time);
RHOTHOR int32_t CALLCONV rtSetLaserLink(int32_t Address, int32_t Value);
RHOTHOR int32_t CALLCONV rtSetLaserTimes(int32_t GateOnDelay, int32_t GateOffDelay);
RHOTHOR int32_t CALLCONV rtSetMatrix(double a11, double a12, double a21, double a22);
RHOTHOR int32_t CALLCONV rtSetMinGatePeriod(int32_t Time);
RHOTHOR int32_t CALLCONV rtSetOTF(int32_t Nr, bool On);
RHOTHOR int32_t CALLCONV rtSetOffsIndex(int32_t Index);
RHOTHOR int32_t CALLCONV rtSetOffsXY(double X, double Y);
RHOTHOR int32_t CALLCONV rtSetOffsZ(double Z);
RHOTHOR int32_t CALLCONV rtSetOscillator(int32_t Nr, double Period, double PulseWidth);
RHOTHOR int32_t CALLCONV rtSetPulseBulge(double Factor);
RHOTHOR int32_t CALLCONV rtSetResolver(int32_t Nr, double StepSize, double Range);
RHOTHOR int32_t CALLCONV rtSetResolverPosition(int32_t Nr, double Position);
RHOTHOR int32_t CALLCONV rtSetResolverRange(int32_t Nr, double Range);
RHOTHOR int32_t CALLCONV rtSetResolverTrigger(int32_t Nr, double Position, int32_t IO);
RHOTHOR int32_t CALLCONV rtSetRotation(double Angle);
RHOTHOR int32_t CALLCONV rtSetSpeed(double Speed);
RHOTHOR int32_t CALLCONV rtSetTable(int32_t Nr, double Position);
RHOTHOR int32_t CALLCONV rtSetTableDelay(int32_t Nr, int32_t Delay);
RHOTHOR int32_t CALLCONV rtSetTableSnapSize(int32_t Nr, double SnapSize);
RHOTHOR int32_t CALLCONV rtSetTableStepSize(int32_t Nr, double StepSize);
RHOTHOR int32_t CALLCONV rtSetTableWhileIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtSetVarBlock(int32_t i, char data);
RHOTHOR int32_t CALLCONV rtSetWhileIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtSetWobble(double Diam, int32_t Freq);
RHOTHOR int32_t CALLCONV rtSetWobbleEx(int32_t nType, double nAmpl, int32_t nFreq, int32_t tType, double tAmpl, int32_t tHarm, int32_t tPhase);
RHOTHOR int32_t CALLCONV rtSetJumpSpeed(double Speed);
RHOTHOR int32_t CALLCONV rtSleep(int32_t Time);
RHOTHOR int32_t CALLCONV rtSynchronise();
RHOTHOR int32_t CALLCONV rtTableArcTo(double X, double Y, double BF);
RHOTHOR int32_t CALLCONV rtTableJog(int32_t Nr, double Speed, int32_t WhileIO);
RHOTHOR int32_t CALLCONV rtTableJumpTo3D(double X, double Y, double Z, double SpeedX, double SpeedY, double SpeedZ);
RHOTHOR int32_t CALLCONV rtTableMove(int32_t Nr, double Target);
RHOTHOR int32_t CALLCONV rtTableLineTo(double X, double Y);
RHOTHOR int32_t CALLCONV rtTableMoveTo(double X, double Y);
RHOTHOR int32_t CALLCONV rtMoveTo3D(double X, double Y, double Z);
RHOTHOR int32_t CALLCONV rtLineTo3D(double X, double Y, double Z);
RHOTHOR int32_t CALLCONV rtJumpTo3D(double X, double Y, double Z);
RHOTHOR int32_t CALLCONV rtPulse3D(double X, double Y, double Z);

// rtListOpen(3 or 4) or rtFileOpen()
RHOTHOR int32_t CALLCONV rtCharDef(int32_t Ascii);
RHOTHOR int32_t CALLCONV rtDoLoop();
RHOTHOR int32_t CALLCONV rtDoWhile();
RHOTHOR int32_t CALLCONV rtElse();
RHOTHOR int32_t CALLCONV rtEndIf();
RHOTHOR int32_t CALLCONV rtFontDef(const char* Name);
RHOTHOR int32_t CALLCONV rtFontDefEnd();
RHOTHOR int32_t CALLCONV rtSetLoop(int32_t LoopCtr);
RHOTHOR int32_t CALLCONV rtSuspend();
RHOTHOR int32_t CALLCONV rtVarBlockFetch(int32_t Start, int32_t Size, const char* FontName);

/************************************************************/
/*** query target functions (rtSetQueryIndex)             ***/
/*** Those functions query or control the target          ***/
/*** selected by rtSetQueryIndex(1 to 8)                  ***/ 
/************************************************************/

RHOTHOR int32_t CALLCONV rtGetAnalog(int32_t Nr, int32_t* Value);
RHOTHOR int32_t CALLCONV rtGetCfgIO(int32_t Nr, int32_t *Value);
RHOTHOR int32_t CALLCONV rtGetCounter(int32_t* Value);
RHOTHOR int32_t CALLCONV rtGetDeflReplies(int32_t* CH1, int32_t* CH2, int32_t* CH3);
RHOTHOR int32_t CALLCONV rtGetFieldSize(double* Size);
RHOTHOR int32_t CALLCONV rtGetFieldSizeZ(double* Size);
RHOTHOR int32_t CALLCONV rtGetIO(int32_t* Value);
RHOTHOR int32_t CALLCONV rtGetLaserLink(int32_t Address, int32_t* Value);
RHOTHOR int32_t CALLCONV rtGetMaxSpeed(double* Speed);
RHOTHOR int32_t CALLCONV rtGetResolvers(double* X, double* Y);
RHOTHOR int32_t CALLCONV rtGetScannerDelay(int32_t* Delay);
RHOTHOR int32_t CALLCONV rtGetSetpointFilter(int32_t* TimeConst);
RHOTHOR int32_t CALLCONV rtGetTablePositions(double* X, double* Y, double* Z);
RHOTHOR int32_t CALLCONV rtSystemSetIO(int32_t Value, int32_t Mask);
RHOTHOR int32_t CALLCONV rtSystemSetResolverSpeed(int32_t Nr, double Speed);

/************************************************************/
/*** reserved rhothor functions                           ***/
/************************************************************/

RHOTHOR int32_t CALLCONV rtParse(const char* Cmd);
RHOTHOR int32_t CALLCONV rtRunServer(int32_t Id, void* Params1, void* Params2);

/************************************************************/
/*** not supported functions, kept for compatibility      ***/
/************************************************************/

RHOTHOR int32_t CALLCONV rtSetLead(int32_t Time);
RHOTHOR int32_t CALLCONV rtSetMaxSpeed(double Speed);
RHOTHOR int32_t CALLCONV rtSetHover(int32_t Time);
RHOTHOR int32_t CALLCONV rtSetTableOffsXY(double X, double Y);
RHOTHOR int32_t CALLCONV rtSetTableSnap(double Distance);
RHOTHOR int32_t CALLCONV rtSendUartLink(const char* Data);
RHOTHOR int32_t CALLCONV bcSamplePoint(double X, double Y, int32_t Row, int32_t Col, double Sweep, double* OffsetX, double* OffsetY);
RHOTHOR int32_t CALLCONV bcSelectDevice(const char* CommPort);
RHOTHOR int32_t CALLCONV rtLoadCalibration();
RHOTHOR int32_t CALLCONV rtStoreCalibration();
RHOTHOR int32_t CALLCONV rtAddCalibrationDataZ(const char* FileName);
RHOTHOR int32_t CALLCONV rtResetCalibrationZ();
RHOTHOR int32_t CALLCONV rtStoreCalibrationFileZ(const char* FileName);
RHOTHOR int32_t CALLCONV rtLoadCalibrationFileZ(const char* FileName);
RHOTHOR int32_t CALLCONV rtWaitEventCounter(int32_t Count);

#ifdef __cplusplus
}
#endif
