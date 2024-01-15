/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

//#include "stdafx.h"
#include "All.h"

#define DLLAPP 0

#define LOGFILE 0

#if LOGFILE
  FILE* Log;
#endif

CrtCtrl32 g_Ctrl32;

#define ACTION_LIST 1
#define MAX_USB_DEVICES 16

char USBlist[MAX_USB_DEVICES][64];
int32_t USBdevice;


/*** private functions *************************************************************/

//RHOTHOR int32_t CALLCONV Connect(const char* IP, bool StartPollerThread)
int32_t Connect(const char* IP, bool StartPollerThread)
{
  //int32_t i;
  
  g_Ctrl32.Disconnect();
  if (IP[0]==0) { return ERR_OK; } // request to disconnect

  if (strncmp(IP,"USB",3)==0)
  {
    //char s[64];
    //if (sscanf(IP,"USB \"%[^\"]",s)!=1) s[0]=0;
    //char list[MAX_USB_DEVICES][64];
    //
    //CHardwareUSB32 cua_usb32;
    //for (i=0;i<MAX_USB_DEVICES;i++) list[i][0]=0;
    //cua_usb32.m_Pic.GetDevList(list,MAX_USB_DEVICES);
    //for (i=0; list[i][0] && strncmp(list[i],s,strlen(s)); i++);
    //if (list[i][0])
    //{
    //  g_Ctrl32.p_Hw=new(CHardwareUSB32);
    //  return g_Ctrl32.Connect(s,StartPollerThread);
    //}
  }
  else if (strncmp(IP,"UDP",3)==0)
  {
    char s[64];
    sscanf(IP,"UDP \"%[^\"]",s);
    g_Ctrl32.p_Hw=new(CHardwareUDP32);
    return g_Ctrl32.Connect(s,StartPollerThread);
  }
  else if (strncmp(IP,"TCP",3)==0)
  {
    char s[64];
    sscanf(IP,"TCP \"%[^\"]",s);
    g_Ctrl32.p_Hw=new(CHardwareTCP32);
    return g_Ctrl32.Connect(s,StartPollerThread);
  }
  return ERR_HARDWARE;
}

#ifdef _WIN32
BOOL WINAPI DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved)
{
  switch(dwReason)
  {
  case DLL_PROCESS_ATTACH:
    g_Ctrl32.h_Dll= HINSTANCE(hModule);
#if LOGFILE
    Log=fopen("c:\\rhothorlog.txt","w+b");
#endif
    break;

  case DLL_PROCESS_DETACH:
#if LOGFILE
    fclose(Log);
#endif
    break;
  }
  return TRUE;
}

void GetVersionInfo(HMODULE hLib, char* pPath, char* pVersion)
{
  pVersion[0]=0;
  if (GetModuleFileName(hLib, pPath, 512-1))
  {
    DWORD dwDummy= 0; 
    DWORD dwLen= ::GetFileVersionInfoSize(pPath, &dwDummy); 
    LPBYTE pVerInfo= new BYTE[dwLen+1]; 
    if (::GetFileVersionInfo(pPath, 0, dwLen, pVerInfo)) 
    { 
      LPBYTE pFileVerInfo= NULL; 
      UINT Len; 
      ::VerQueryValue(pVerInfo, "\\",
           reinterpret_cast<LPVOID*>(&pFileVerInfo), &Len);
      VS_FIXEDFILEINFO* lpFixedFileInfo=
           reinterpret_cast<VS_FIXEDFILEINFO*>(pFileVerInfo); 
      sprintf(pVersion,"%d.%d.%d.%d",  
               HIWORD(lpFixedFileInfo->dwProductVersionMS), 
               LOWORD(lpFixedFileInfo->dwProductVersionMS), 
               HIWORD(lpFixedFileInfo->dwProductVersionLS),
               LOWORD(lpFixedFileInfo->dwProductVersionLS)); 
    } 
    delete[] pVerInfo; 
  }
}
#endif

/*** accessing functions (alphabetically) ******************************************/

RHOTHOR int32_t CALLCONV rtAbort()
{
  int32_t err=g_Ctrl32.Abort();
#if LOGFILE
  fprintf(Log,"rtAbort()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtAddCalibrationData(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.AddCalibrationData(FileName,false);
#if LOGFILE
  fprintf(Log,"rtAddCalibrationData(%s)->%d;\r\n",FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtArcMoveTo(double X, double Y, double BF)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.ArcMoveXY(ClfPoint(X,Y),BF);
#if LOGFILE
  fprintf(Log,"rtArcMoveTo(%lf,%lf,%lf)->%d;\r\n",X,Y,BF,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtArcTo(double X, double Y, double BF)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.ArcXY(ClfPoint(X,Y),BF);
#if LOGFILE
  fprintf(Log,"rtArcTo(%lf,%lf,%lf)->%d;\r\n",X,Y,BF,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtBurst(int32_t Time)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.Burst(Time);
#if LOGFILE
  fprintf(Log,"rtBurst(%d)->%d;\r\n",Time,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtBurstEx(int32_t Time1, int32_t Time2, double PulseWidth2)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.BurstEx(Time1,Time2,PulseWidth2);
#if LOGFILE
  fprintf(Log,"rtBurstEx(%d,%d,%lf)->%d;\r\n",Time1,Time2,PulseWidth2,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtCharDef(int32_t Ascii)
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.CharDef(Ascii);
#if LOGFILE
  fprintf(Log,"rtCharDef(%d)->%d;\r\n",Ascii,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtCircle(double X, double Y, double Angle)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.Circle(ClfPoint(X,Y),Angle,true);
#if LOGFILE
  fprintf(Log,"rtCircle(%lf,%lf,%lf)->%d;\r\n",X,Y,Angle,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtCircleMove(double X, double Y, double Angle)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.Circle(ClfPoint(X,Y),Angle,false);
#if LOGFILE
  fprintf(Log,"rtCircleMove(%lf,%lf,%lf)->%d;\r\n",X,Y,Angle,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtDoLoop()
{
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.DoLoop();
#if LOGFILE
  fprintf(Log,"rtDoLoop()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtDoWhile()
{
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.While_End();
#if LOGFILE
  fprintf(Log,"rtDoWhile()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtElse()
{ 
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.If_Else();
#if LOGFILE
  fprintf(Log,"rtElse()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtElseIfIO(int32_t Value, int32_t Mask)
{
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.If_ElseIO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtElseIfIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtEndIf()
{ 
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.If_End();
#if LOGFILE
  fprintf(Log,"rtEndIf()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtEraseFromFlash(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode!=0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.EraseFromFlash((char*)FileName);
#if LOGFILE
  fprintf(Log,"rtEraseFromFlash(%s)->%d;\r\n",FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileClose()
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileClose();
#if LOGFILE
  fprintf(Log,"rtFileClose()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileCloseAtHost()
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileCloseHost();
#if LOGFILE
  fprintf(Log,"rtFileCloseAtHost()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileCloseAtIndex(int32_t Index)
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileClose(Index);
#if LOGFILE
  fprintf(Log,"rtFileCloseAtIndex(%d)->%d;\r\n",Index,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileDownload(const char* FileName, const char* DestFile)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileDownload((char*)FileName,(char*)DestFile);
#if LOGFILE
  fprintf(Log,"rtFileDownload(%s,%s)->%d;\r\n",FileName,DestFile,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileFetch(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileFetch((char*)FileName);
#if LOGFILE
  fprintf(Log,"rtFileFetch(%s)->%d;\r\n",FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileOpen(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK && strcmp(FileName,"bootstart")==0) err=ERR_DATA; // reserved filename
  err=g_Ctrl32.FileOpen((char*)FileName);
#if LOGFILE
  fprintf(Log,"rtFileOpen(%s)->%d;\r\n",FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileUpload(const char* SrcFile, const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileUpload((char*)SrcFile,(char*)FileName);
#if LOGFILE
  fprintf(Log,"rtFileUpload(%s,%s)->%d;\r\n",SrcFile,FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFileUploadAtIndex(const char* SrcFile, const char* FileName, int32_t Index)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FileUpload((char*)SrcFile, (char*)FileName, Index);
#if LOGFILE
  fprintf(Log,"rtFileUploadAtIndex(%s,%s,%d)->%d;\r\n",SrcFile,FileName,Index,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFontDef(const char* Name)
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.FontDef((char*)Name);
#if LOGFILE
  fprintf(Log,"rtFontDef(%s)->%d;\r\n",Name,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFontDefEnd()
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.FontDefEnd();
#if LOGFILE
  fprintf(Log,"rtFontDefEnd()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtFormatFlash()
{
  int32_t err=(g_Ctrl32.ld_ListMode!=0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.FormatFlash();
#if LOGFILE
  fprintf(Log,"rtFormatFlash()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetAnalog(int32_t Nr, int32_t* Value)
{
  *Value=g_Ctrl32.GetAnalog(Nr);
#if LOGFILE
  fprintf(Log,"rtGetAnalog(%d,->%d)->%d;\r\n",Nr,*Value,ERR_OK);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetCanLink(int32_t Address, int32_t* Value)
{
  int32_t err=g_Ctrl32.GetCanLink(Address,Value);
#if LOGFILE
  fprintf(Log,"rtGetCanLink(%d,->%d)->%d;\r\n",Address,*Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetCfgIO(int32_t Nr, int32_t* Value)
{
  int32_t err=g_Ctrl32.GetCfgIO(Nr,Value);
#if LOGFILE
  fprintf(Log,"rtGetCfgIO(%d,->%d)->%d;\r\n",Nr,*Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetCounter(int32_t* Value)
{
  int32_t err=g_Ctrl32.GetCounter(Value);
#if LOGFILE
  fprintf(Log,"rtGetCounter(->%d)->%d;\r\n",*Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetDeflReplies(int32_t* CH1, int32_t* CH2, int32_t* CH3)
{
  int32_t err=g_Ctrl32.GetDeflReplies(CH1,CH2,CH3);
#if LOGFILE
  fprintf(Log,"rtGetDeflReplies(->0x%.8x,->0x%.8x,->0x%.8x)->%d;\r\n",*CH1,*CH2,*CH3,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetFieldSize(double* Size)
{
  int32_t err=g_Ctrl32.GetFieldSize(Size);
#if LOGFILE
  fprintf(Log,"rtGetFieldSize(->%lf)->%d;\r\n",*Size,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetFieldSizeZ(double* Size)
{
  int32_t err=g_Ctrl32.GetFieldSizeZ(Size);
#if LOGFILE
  fprintf(Log,"rtGetFieldSizeZ(->%lf)->%d;\r\n",*Size,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetFileIndex(const char* FileName, int32_t* Index)
{
  *Index=g_Ctrl32.FileSearch((char*)FileName);
#if LOGFILE
  fprintf(Log,"rtGetFileIndex(%s,->%d)->%d;\r\n",FileName,*Index,ERR_OK);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetFirstFreeUSBDevice(char* Name)
{
  for (int32_t i=0; i<MAX_USB_DEVICES; i++) USBlist[i][0]=0;
//  
//  CHardwareUSB32 cua_usb32;
//  cua_usb32.m_Pic.GetDevList(USBlist,MAX_USB_DEVICES);
//
//  Name[0]=0;
//  USBdevice=0;
//  strncpy(Name,USBlist[USBdevice],64);
//#if LOGFILE
//  fprintf(Log,"rtGetFirstFreeUSBDevice(%s)->%d;\r\n",Name,ERR_OK);
//#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetFlashFirstFileEntry(char* Name, int32_t* Size)
{
  int32_t err=g_Ctrl32.GetFlashFirstFileEntry(Name,Size);
#if LOGFILE
  fprintf(Log,"rtGetFlashFirstFileEntry(->%s,->%d)->%d;\r\n",Name,*Size,ERR_OK);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetFlashMemorySizes(int32_t* Total, int32_t* Allocated)
{
  int32_t err=g_Ctrl32.GetFlashMemorySizes(Total,Allocated);
#if LOGFILE
  fprintf(Log,"rtGetFlashMemorySizes(->%d,->%d)->%d;\r\n",*Total,*Allocated,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetFlashNextFileEntry(char* Name, int32_t* Size)
{
  int32_t err=g_Ctrl32.GetFlashNextFileEntry(Name,Size);
#if LOGFILE
  fprintf(Log,"rtGetFlashNextFileEntry(->%s,->%d)->%d;\r\n",Name,*Size,ERR_OK);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetID(char* Name)
{
  strncpy(Name,g_Ctrl32.GetID(),64);
#if LOGFILE
  fprintf(Log,"rtGetID(->%s)->%d;\r\n",Name,ERR_OK);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetIO(int32_t* Value)
{
  *Value=g_Ctrl32.GetIO();
#if LOGFILE
  fprintf(Log,"rtGetIO(->%d)->%d;\r\n",*Value,ERR_OK);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetIP(char* Mac, char* IP)
{
  int32_t err=g_Ctrl32.GetIP(Mac,IP);
#if LOGFILE
  fprintf(Log,"rtGetIP(%s,->%s)->%d;\r\n",Mac,IP,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetLaserLink(int32_t Address, int32_t* Value)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_BUSY:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.GetLaserLink(Address,Value);
#if LOGFILE
  fprintf(Log,"rtGetLaserLink(%d,->%d)->%d;\r\n",Address,*Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetMaxSpeed(double* Speed)
{
  *Speed=g_Ctrl32.GetMaxSpeed();
#if LOGFILE
  fprintf(Log,"rtGetMaxSpeed(->%lf)->%d;\r\n",*Speed,ERR_OK);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetNextFreeUSBDevice(char* Name)
{
  Name[0]=0;
  USBdevice++;
  if (USBdevice>=MAX_USB_DEVICES) Name[0]=0;
  strncpy(Name,USBlist[USBdevice],64);
#if LOGFILE
  fprintf(Log,"rtGetNextFreeUSBDevice(%s)->%d;\r\n",Name,ERR_OK);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtGetQueryTarget(int32_t* Index)
{
  int32_t err=g_Ctrl32.GetQueryTarget(Index);
#if LOGFILE
  fprintf(Log,"rtGetQueryTarget(->%d)->%d;\r\n",*Index,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetResolvers(double* X, double* Y)
{
  int32_t err=g_Ctrl32.GetResolvers(X,Y);
#if LOGFILE
  fprintf(Log,"rtGetResolvers(->%lf,->%lf)->%d;\r\n",*X,*Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetScannerDelay(int32_t* Delay)
{
  int32_t err=g_Ctrl32.GetScannerDelay(Delay);
#if LOGFILE
  fprintf(Log,"rtGetScannerDelay(->%d)->%d;\r\n",*Delay,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetSerial(int32_t* Serial)
{
  int32_t err=g_Ctrl32.GetSerial(Serial);
#if LOGFILE
  fprintf(Log,"rtGetSerial(->%d)->%d;\r\n",*Serial,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetSetpointFilter(int32_t* TimeConst)
{
  int32_t err=g_Ctrl32.GetSetpointFilter(TimeConst);
#if LOGFILE
  fprintf(Log,"rtGetSetpointFilter(->%d)->%d;\r\n",*TimeConst,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetStatus(int32_t* Memory)
{
  int32_t err=g_Ctrl32.GetStatus(Memory);
#if LOGFILE
  fprintf(Log,"rtGetStatus(->%d)->%d;\r\n",*Memory,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetTablePositions(double* X, double* Y, double* Z)
{
  int32_t err=g_Ctrl32.GetTablePositions(X,Y,Z);
#if LOGFILE
  fprintf(Log,"rtGetTablePositions(->%lf,->%lf,->%lf)->%d;\r\n",*X,*Y,*Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetTarget(int32_t* Mask)
{
  int32_t err=g_Ctrl32.GetTarget(Mask);
#if LOGFILE
  fprintf(Log,"rtGetTarget(->%d)->%d;\r\n",*Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtGetVersion(char* Version)
{
#ifdef _WIN32
  char s[512];
  char s2[512];
  GetVersionInfo(g_Ctrl32.h_Dll, s,s2);
  strcpy(Version,s2);
#if LOGFILE
  fprintf(Log,"rtGetVersion(->%s)->%d;\r\n",Version,ERR_OK);
#endif
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtIncrementCounter()
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.IncrementCounter();
#if LOGFILE
  fprintf(Log,"rtIncrementCounter()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtIndexFetch(int32_t Index)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SectorFetch(Index);
#if LOGFILE
  fprintf(Log,"rtIndexFetch(%d)->%d;\r\n",Index,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtIfIO(int32_t Value, int32_t Mask)
{ 
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.If_IO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtIfIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtJumpTo(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.JumpXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtJumpTo(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtJumpTo3D(double X, double Y, double Z)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.JumpXYZ(ClfPoint(X,Y),Z);
#if LOGFILE
  fprintf(Log,"rtJumpTo3D(%lf,%lf,%lf)->%d;\r\n",X,Y,Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtLineTo(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.LineXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtLineTo(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtLineTo3D(double X, double Y, double Z)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.LineXYZ(ClfPoint(X,Y),Z);
#if LOGFILE
  fprintf(Log,"rtLineTo3D(%lf,%lf,%lf)->%d;\r\n",X,Y,Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtListClose()
{
  int32_t err=ERR_OK;
  if (g_Ctrl32.ld_ListMode<1 && g_Ctrl32.ld_ListMode>5) err=ERR_JOB;
  if (err==ERR_OK) err=g_Ctrl32.ListClose();
#if LOGFILE
  fprintf(Log,"rtListClose()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtListOpen(int32_t Mode)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.ListOpen(Mode);
#if LOGFILE
  fprintf(Log,"rtListOpen(%d)->%d;\r\n",Mode,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtLoadCalibrationFile(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.LoadCalibrationFile(FileName);
#if LOGFILE
  fprintf(Log,"rtLoadCalibrationFile(%s)->%d;\r\n",FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtLoadProfile(const char* FileName, int32_t Include)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.LoadProfile(FileName, Include);
#if LOGFILE
  fprintf(Log,"rtLoadProfile(%s,%d)->%d;\r\n",FileName,Include,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtMoveTo(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.MoveXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtMoveTo(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtMoveTo3D(double X, double Y, double Z)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.MoveXYZ(ClfPoint(X,Y),Z);
#if LOGFILE
  fprintf(Log,"rtMoveTo3D(%lf,%lf,%lf)->%d;\r\n",X,Y,Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtOpenCanLink(int32_t Baudrate)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.OpenCanLink(Baudrate);
#if LOGFILE
  fprintf(Log,"rtOpenCanLink(%d)->%d;\r\n",Baudrate,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtPrint(char* data)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.Print(data);
#if LOGFILE
  fprintf(Log,"rtPrint(%s)->%d;\r\n",data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtPulse(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.PulseXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtPulse(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtPulse3D(double X, double Y, double Z)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.PulseXYZ(ClfPoint(X,Y),Z);
#if LOGFILE
  fprintf(Log,"rtPuls3D(%lf,%lf,%lf)->%d;\r\n",X,Y,Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtReset()
{
  int32_t err=g_Ctrl32.SystemReset();
#if LOGFILE
  fprintf(Log,"rtReset()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtResetCalibration()
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) g_Ctrl32.AddCalibrationData("",true);
#if LOGFILE
  fprintf(Log,"rtResetCalibration()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtResetCounter()
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.ResetCounter();
#if LOGFILE
  fprintf(Log,"rtResetCounter()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtResetResolver(int32_t Nr)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.ResetResolver(Nr);
#if LOGFILE
  fprintf(Log,"rtResetResolver(%d)->%d;\r\n",Nr,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtScanCanLink(int32_t Address, int32_t Node, int32_t Index, int32_t SubIndex)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.ScanCanLink(Address,Node,Index,SubIndex);
#if LOGFILE
  fprintf(Log,"rtScanCanLink(%d,%d,%d,%d)->%d;\r\n",Address,Node,Index,SubIndex,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSelectDevice(const char* IP)
{
  int32_t err=Connect(IP,true);
#if LOGFILE
  fprintf(Log,"rtSelectDevice(%s)->%d;\r\n",IP,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetAnalog(int32_t Value, int32_t Mask)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetAnalog(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtSetAnalog(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetCanLink(int32_t Node, int32_t Index, int32_t SubIndex, const char* Data)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetCanLink(Node,Index,SubIndex,(char*)Data);
#if LOGFILE
  fprintf(Log,"rtSetCanLink(%d,%d,%d,%s)->%d;\r\n",Node,Index,SubIndex,Data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetCfgIO(int32_t Nr, int32_t Value)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetCfgIO(Nr,Value);
#if LOGFILE
  fprintf(Log,"rtSetCfgIO(%d,%d)->%d;\r\n",Nr,Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetCounter(int32_t Value)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetCounter(Value);
#if LOGFILE
  fprintf(Log,"rtSetCounter(%d)->%d;\r\n",Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetFieldSize(double Size)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetFieldSize(Size);
#if LOGFILE
  fprintf(Log,"rtSetFieldSize(%lf)->%d;\r\n",Size,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetFont(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetFont((char*)FileName);
  return err;
}

RHOTHOR int32_t CALLCONV rtSetImageMatrix(double a11, double a12, double a21, double a22)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetImageAij(a11,a12,a21,a22,0,0);
#if LOGFILE
  fprintf(Log,"rtSetImageMatrix(%lf,%lf,%lf,%lf)->%d;\r\n",a11,a12,a21,a22,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetImageMatrix3D(double a11, double a12, double a21, double a22, double a31, double a32)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetImageAij(a11,a12,a21,a22,a31,a32);
#if LOGFILE
  fprintf(Log,"rtSetImageMatrix3D(%lf,%lf,%lf,%lf,%lf,%lf)->%d;\r\n",a11,a12,a21,a22,a31,a32,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetImageOffsRelXY(double X, double Y)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetImageOffsRelXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtSetImageOffsRelXY(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetImageOffsXY(double X, double Y)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetImageOffsXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtSetImageOffsXY(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetImageOffsZ(double Z)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetImageOffsZ(Z);
#if LOGFILE
  fprintf(Log,"rtSetImageOffsZ(%lf)->%d;\r\n",Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetImageRotation(double Angle)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetImageRotation(Angle);
#if LOGFILE
  fprintf(Log,"rtSetImageRotation(%lf)->%d;\r\n",Angle,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetIO(int32_t Value, int32_t Mask)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetIO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtSetIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetJumpSpeed(double Speed)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK && Speed>g_Ctrl32.GetMaxSpeed()) err=ERR_DATA;
  if (err==ERR_OK) err=g_Ctrl32.SetJumpSpeed(Speed);
#if LOGFILE
  fprintf(Log,"rtSetJumpSpeed(%lf)->%d;\r\n",Speed,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetLaser(bool OnOff)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetLaser(OnOff);
#if LOGFILE
  fprintf(Log,"rtSetLaser(%d)->%d;\r\n",OnOff,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetLaserLink(int32_t Command, int32_t Value)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.LaserLink(Command,Value);
#if LOGFILE
  fprintf(Log,"rtSetLaserlink(%d,%d)->%d;\r\n",Command,Value,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetLaserTimes(int32_t GateOnDelay, int32_t GateOffDelay)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetLaserTimes(GateOnDelay,GateOffDelay);
#if LOGFILE
  fprintf(Log,"rtSetLaserTimes(%d,%d)->%d;\r\n",GateOnDelay,GateOffDelay,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetLaserFirstPulse(double Time)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetLaserFirstPulse(Time);
#if LOGFILE
  fprintf(Log,"rtSetLaserFirstPulse(%lf)->%d;\r\n",Time,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetLoop(int32_t LoopCtr)
{
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.SetLoop(LoopCtr);
#if LOGFILE
  fprintf(Log,"rtSetLoop(%d)->%d;\r\n",LoopCtr,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetMatrix(double a11, double a12, double a21, double a22)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetAij(a11,a12,a21,a22);
#if LOGFILE
  fprintf(Log,"rtSetMatrix(%lf,%lf,%lf,%lf)->%d;\r\n",a11,a12,a21,a22,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetMinGatePeriod(int32_t Time)
{
  int32_t err=g_Ctrl32.SetMinGatePeriod(Time);
#if LOGFILE
  fprintf(Log,"rtSetMinGatePeriod(%d)->%d;\r\n",Time,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetOffsIndex(int32_t Index)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetOffsIndex(Index);
#if LOGFILE
  fprintf(Log,"rtSetOffsIndex(%d)->%d;\r\n",Index,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetOffsXY(double X, double Y)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetOffsXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtSetOffsXY(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetOffsZ(double Z)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetOffsZ(Z);
#if LOGFILE
  fprintf(Log,"rtSetOffsZ(%lf)->%d;\r\n",Z,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetOscillator(int32_t Nr, double Period, double PulseWidth)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetPWM(Nr, Period, PulseWidth);
#if LOGFILE
  fprintf(Log,"rtSetOscillator(%d,%lf,%lf)->%d;\r\n",Nr,Period,PulseWidth,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetOTF(int32_t Nr, bool On)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetOTF(Nr,On);
#if LOGFILE
  fprintf(Log,"rtSetOTF(%d,%d)->%d;\r\n",Nr,On,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetPulseBulge(double Factor)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetBulge(Factor);
#if LOGFILE
  fprintf(Log,"rtSetPulseBulge(%lf)->%d;\r\n",Factor,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetQueryTarget(int32_t Index)
{
  int32_t err=g_Ctrl32.SetQueryTarget(Index);
#if LOGFILE
  fprintf(Log,"rtSetQueryTarget(%d)->%d;\r\n",Index,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetResolver(int32_t Nr, double StepSize, double Range)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetResolver(Nr, StepSize,Range);
#if LOGFILE
  fprintf(Log,"rtSetResolver(%d,%lf,%lf)->%d;\r\n",Nr,StepSize,Range,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetResolverPosition(int32_t Nr, double Position)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetResolverPosition(Nr,Position);
#if LOGFILE
  fprintf(Log,"rtSetResolverPosition(%d,%lf)->%d;\r\n",Nr,Position,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetResolverRange(int32_t Nr, double Range)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetResolverRange(Nr, Range);
#if LOGFILE
  fprintf(Log,"rtSetResolverRange(%d,%lf)->%d;\r\n",Nr,Range,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemSetResolverSpeed(int32_t Nr, double Speed)
{
  int32_t err=g_Ctrl32.SystemSetResolverSpeed(Nr,Speed);
#if LOGFILE
  fprintf(Log,"rtSystemSetResolverSpeed(%d,%lf)->%d;\r\n",Nr,Speed,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetResolverTrigger(int32_t Nr, double Position, int32_t IO)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetResolverTrigger(Nr, Position,IO);
#if LOGFILE
  fprintf(Log,"rtSetResolverTrigger(%d,%lf,%d)->%d;\r\n",Nr,Position,IO,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetRotation(double Angle)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetRotation(Angle);
#if LOGFILE
  fprintf(Log,"rtSetRotation(%lf)->%d;\r\n",Angle,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetSpeed(double Speed)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK && Speed>g_Ctrl32.GetMaxSpeed()) err=ERR_DATA;
  if (err==ERR_OK) err=g_Ctrl32.SetSpeed(Speed);
#if LOGFILE
  fprintf(Log,"rtSetSpeed(%lf)->%d;\r\n",Speed,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetTable(int32_t Nr, double Position)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetTable(Nr,Position);
#if LOGFILE
  fprintf(Log,"rtSetTable(%d,%lf)->%d;\r\n",Nr,Position,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetTableDelay(int32_t Nr, int32_t Delay)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetTableDelay(Nr,Delay);
#if LOGFILE
  fprintf(Log,"rtSetTableDelay(%d,%d)->%d;\r\n",Nr,Delay,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetTableSnapSize(int32_t Nr, double SnapSize)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetTableSnapSize(Nr,SnapSize);
#if LOGFILE
  fprintf(Log,"rtSetTableSnapSize(%d,%lf)->%d;\r\n",Nr,SnapSize,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetTableStepSize(int32_t Nr, double StepSize)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetTableStepSize(Nr,StepSize);
#if LOGFILE
  fprintf(Log,"rtSetTableStepSize(%d,%lf)->%d;\r\n",Nr,StepSize,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetTableWhileIO(int32_t Value, int32_t Mask)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetTableWhileIO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtSetTableWhileIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetTarget(int32_t Mask)
{
  int32_t err=g_Ctrl32.SetTarget(Mask);
#if LOGFILE
  fprintf(Log,"rtSetTarget(%d)->%d;\r\n",Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetVarBlock(int32_t i, char data)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetVarBlock(i,data);
#if LOGFILE
  fprintf(Log,"rtSetVarBlock(%d,%c)->%d;\r\n",i,data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetWhileIO(int32_t Value, int32_t Mask)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetWhileIO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtSetWhileIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetWobble(double Diam, int32_t Freq)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetWobble(Diam,Freq);
#if LOGFILE
  fprintf(Log,"rtSetWobble(%lf,%d)->%d;\r\n",Diam,Freq,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSetWobbleEx(int32_t nType, double nAmpl, int32_t nFreq, int32_t tType, double tAmpl, int32_t tHarm, int32_t tPhase)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.SetWobbleEx(nType,nAmpl,nFreq,tType,tAmpl,tHarm,tPhase);
#if LOGFILE
  fprintf(Log,"rtSetWobbleEx(%d,%lf,%d,%d,%lf,%d,%d)->%d;\r\n",nType,nAmpl,nFreq,tType,tAmpl,tHarm,tPhase,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSleep(int32_t Time)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.Sleep(Time);
#if LOGFILE
  fprintf(Log,"rtSleep(%d)->%d;\r\n",Time,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtStoreCalibrationFile(const char* FileName)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) g_Ctrl32.StoreCalibrationFile(FileName);
#if LOGFILE
  fprintf(Log,"rtStoreCalibrationFile(%s)->%d;\r\n",FileName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtStoreProfile(const char* FileName, int32_t Include)
{
  int32_t err=(g_Ctrl32.ld_ListMode != 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.StoreProfile(FileName, Include);
#if LOGFILE
  fprintf(Log,"rtStoreProfile(%s,%d)->%d;\r\n",FileName,Include,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSuspend()
{
  int32_t err=g_Ctrl32.Suspend();
#if LOGFILE
  fprintf(Log,"rtSuspend()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSynchronise()
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.Synchronise();
#if LOGFILE
  fprintf(Log,"rtSynchronise()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemCheck(int32_t *Target, int32_t* Powered, int32_t* Bios)
{
  return g_Ctrl32.SystemCheck(Target, Powered, Bios);
}

RHOTHOR int32_t CALLCONV rtSystemResume()
{
  int32_t err=g_Ctrl32.SystemResume();
#if LOGFILE
  fprintf(Log,"rtSystemResume()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemSelectFile(int32_t Index)
{
  return g_Ctrl32.SystemSelectFile(Index);
}

RHOTHOR int32_t CALLCONV rtSystemSetIO(int32_t Value, int32_t Mask)
{
  int32_t err=g_Ctrl32.SystemSetIO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtSystemSetIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemSuspend()
{
  int32_t err=g_Ctrl32.SystemSuspend();
#if LOGFILE
  fprintf(Log,"rtSystemSuspend()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemUartOpen(int32_t baudrate, char parity, char stopbits)
{
  int32_t err=g_Ctrl32.UARTopen(baudrate,parity,stopbits);
#if LOGFILE
  fprintf(Log,"rtUartOpen(%d,%c,%c)->%d;\r\n",baudrate,parity,stopbits,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemUartWrite(int32_t bytes, char* data)
{
  int32_t err=g_Ctrl32.UARTwrite(bytes,data);
#if LOGFILE
  fprintf(Log,"rtUartWrite(%d,%s)->%d;\r\n",bytes,data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtSystemUDPsend(char* IP, short port, char* data)
{
  int32_t err=g_Ctrl32.SystemUDPsend(IP,port,data);
#if LOGFILE
  fprintf(Log,"rtSystemUDPsend(%s,%d,%s)->%d;\r\n",IP,port,data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableArcTo(double X, double Y, double BF)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableArcXY(ClfPoint(X,Y),BF);
#if LOGFILE
  fprintf(Log,"rtTableArcTo(%lf,%lf,%lf)->%d;\r\n",X,Y,BF,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableJog(int32_t Nr, double Speed, int32_t WhileIO)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableJog(Nr,Speed,WhileIO);
#if LOGFILE
  fprintf(Log,"rtTableJog(%d,%lf,%d)->%d;\r\n",Nr,Speed,WhileIO,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableJumpTo(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableJumpXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtTableJumpTo(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableJumpTo3D(double X, double Y, double Z, double SpeedX, double SpeedY, double SpeedZ)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableJump3D(X,Y,Z,SpeedX,SpeedY,SpeedZ);
#if LOGFILE
  fprintf(Log,"rtTableJumpTo3D(%lf,%lf,%lf,%lf,%lf,%lf)->%d;\r\n",X,Y,Z,SpeedX,SpeedY,SpeedZ,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableLineTo(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableLineXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtTableLineTo(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableMove(int32_t Nr, double Target)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableMove(Nr,Target);
#if LOGFILE
  fprintf(Log,"rtTableMove(%d,%lf)->%d;\r\n",Nr,Target,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtTableMoveTo(double X, double Y)
{ 
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.TableMoveXY(ClfPoint(X,Y));
#if LOGFILE
  fprintf(Log,"rtTableMoveTo(%lf,%lf)->%d;\r\n",X,Y,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtUartRead(int32_t* bytes, char* data)
{
  int32_t err=g_Ctrl32.UARTread(bytes,data);
#if LOGFILE
  fprintf(Log,"rtUartRead(%d,%s)->%d;\r\n",*bytes,data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtUDPsend(char* IP, short port, char* data)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.UDPsend(IP,port,data);
#if LOGFILE
  fprintf(Log,"rtUDPsend(%s,%d,%s)->%d;\r\n",IP,port,data,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtVarBlockFetch(int32_t Start, int32_t Size, const char* FontName)
{
  int32_t err=(g_Ctrl32.ld_ListMode != FILE_LOAD)? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.VarBlockFetch(Start,Size,(char*)FontName);
#if LOGFILE
  fprintf(Log,"rtVarBlockFetch(%d,%d,%s)->%d;\r\n",Start,Size,FontName,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtWaitCanLink(int32_t ByteNr, int32_t Value, int32_t Mask)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.WaitCanLink(ByteNr,Value,Mask);
#if LOGFILE
  fprintf(Log,"rtWaitcanLink(%d,%d,%d)->%d;\r\n",ByteNr,Value,Mask,err);
#endif
  return ERR_OK;
}

RHOTHOR int32_t CALLCONV rtWaitIdle()
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.WaitIdle(g_Ctrl32.c_tgtidx);
#if LOGFILE
  fprintf(Log,"rtWaitIdle()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtWaitIO(int32_t Value, int32_t Mask)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.WaitIO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtWaitIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtWaitPosition(double Window)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.WaitPosition(Window);
#if LOGFILE
  fprintf(Log,"rtWaitPosition(%lf)->%d;\r\n",Window,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtWaitResolver(int32_t Nr, double TriggerPos, int32_t TriggerMode)
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.WaitResolver(Nr,TriggerPos,TriggerMode);
#if LOGFILE
  fprintf(Log,"rtWaitResolver(%d,%lf,%d)->%d;\r\n",Nr,TriggerPos,TriggerMode,err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtWaitStall()
{
  int32_t err=(g_Ctrl32.ld_ListMode == 0)? ERR_JOB:ERR_OK;
  if (err==ERR_OK) err=g_Ctrl32.WaitStall(g_Ctrl32.c_tgtidx);
#if LOGFILE
  fprintf(Log,"rtWaitStall()->%d;\r\n",err);
#endif
  return err;
}

RHOTHOR int32_t CALLCONV rtWhileIO(int32_t Value, int32_t Mask)
{
  int32_t err=(!g_Ctrl32.FlowCommands())? ERR_JOB:ERR_OK; 
  if (err==ERR_OK) err=g_Ctrl32.While_IO(Value,Mask);
#if LOGFILE
  fprintf(Log,"rtWhileIO(%d,%d)->%d;\r\n",Value,Mask,err);
#endif
  return err;
}


/*** string style function call ****************************************************/

#define GET_D (sscanf(Cmd,"(%ld)",&ld1)==1)
#define GET_F (sscanf(Cmd,"(%lf)",&lf1)==1)
#define GET_S  (sscanf(Cmd,"(%[^)])",&hc1)==1)
#define GET_DD (sscanf(Cmd,"(%ld,%ld)",&ld1,&ld2)==2)
#define GET_DF (sscanf(Cmd,"(%ld,%lf)",&ld1,&lf1)==2)
#define GET_DS  (sscanf(Cmd,"(%ld,%s)",&ld1,hc1)==2)
#define GET_FD (sscanf(Cmd,"(%lf,%ld)",&lf1,&ld1)==2)
#define GET_FF (sscanf(Cmd,"(%lf,%lf)",&lf1,&lf2)==2)
#define GET_SS      (sscanf(Cmd,"(%[^,],%[^)])",&hc1,&hc2)==1)
#define GET_DDD (sscanf(Cmd,"(%ld,%ld,%ld)",&ld1,&ld2,&ld3)==3)
#define GET_DCD  (sscanf(Cmd,"(%ld,%c,%ld)",&ld1,&hc,&ld2)==3)
#define GET_DDF (sscanf(Cmd,"(%ld,%ld,%lf)",&ld1,&ld2,&lf1)==3)
#define GET_DFD (sscanf(Cmd,"(%ld,%lf,%ld)",&ld1,&lf1,&ld2)==3)
#define GET_DFF (sscanf(Cmd,"(%ld,%lf,%lf)",&ld1,&lf1,&lf2)==3)
#define GET_FFF (sscanf(Cmd,"(%lf,%lf,%lf)",&lf1,&lf2,&lf3)==3)
#define GET_DDS  (sscanf(Cmd,"(%ld,%ld,%[^)])",&ld1,&ld2,hc1)==3)
#define GET_SDS   (sscanf(Cmd,"(%[^,],%ld,%[^)])",&hc1,&ld1,&hc2)==3)
#define GET_DDDD (sscanf(Cmd,"(%ld,%ld,%ld,%ld)",&ld1,&ld2,&ld3,&ld4)==4)
#define GET_FFFF (sscanf(Cmd,"(%lf,%lf,%lf,%lf)",&lf1,&lf2,&lf3,&lf4)==4)
#define GET_DDDS  (sscanf(Cmd,"(%ld,%ld,%ld,%[^)])",&ld1,&ld2,&ld3,hc2)==4)
#define GET_FFDDF (sscanf(Cmd,"(%lf,%lf,%ld,%ld,%lf",&lf1,&lf2,&ld1,&ld2,&lf3)==5)
#define GET_FFFFF (sscanf(Cmd,"(%lf,%lf,%lf,%lf,%lf,%lf)",&lf1,&lf2,&lf3,&lf4,&lf5,&lf6)==6)
#define GET_DFDDFDD (sscanf(Cmd,"(%ld,%lf,%ld,%ld,%lf,%ld,%ld)",&ld1,&lf1,&ld2,&ld3,&lf2,&ld4,&ld5)==7)

RHOTHOR int32_t CALLCONV rtParse(const char* Cmd)
{
#ifdef _WIN32
  double lf1, lf2, lf3, lf4, lf5, lf6;
  long Err, ld1, ld2, ld3, ld4, ld5;
  char hc1[256];
  char hc2[256];
  char hc;
  long i,j;

  while (Cmd[0]==' ') Cmd++; // remove leading blanks
  if (!strlen(Cmd)) return ERR_OK; // empty line
  if (Cmd[0]=='/') return ERR_OK; // comment line

  hc1[0]=0;
  switch (HashVal(&Cmd))
  {
  case _bcSamplePoint:            if (!GET_FFDDF || bcSamplePoint(lf1,lf2,ld1,ld2,lf3,&lf4,&lf5)!=ERR_OK) break;
                                  sprintf(hc2,"x:%lf,y:%lf",lf4,lf5);
                                  MessageBox(GetActiveWindow(),hc2,"bcSamplePoint",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _bcSelectDevice:           if (GET_S) return(bcSelectDevice(hc1)); break;
  case _rtAddCalibrationData:     if (GET_S) return(rtAddCalibrationData(hc1)); break;
  case _rtAddCalibrationDataZ:    if (GET_S) return(rtAddCalibrationDataZ(hc1)); break;
  case _rtArcMoveTo:              if (GET_FFF) return(rtArcMoveTo(lf1,lf2,lf3)); break;
  case _rtArcTo:                  if (GET_FFF) return(rtArcTo(lf1,lf2,lf3)); break;
  case _rtBurst:                  if (GET_D) return(rtBurst(ld1)); break;
  case _rtBurstEx:                if (GET_DDF) return(rtBurstEx(ld1,ld2,lf1)); break;
  case _rtCharDef:                if (GET_D) return(rtCharDef(ld1)); break;
  case _rtCircle:                 if (GET_FFF) return(rtCircle(lf1,lf2,lf3)); break;
  case _rtCircleMove:             if (GET_FFF) return(rtCircleMove(lf1,lf2,lf3)); break;
  case _rtDoLoop:                 return(rtDoLoop());
  case _rtDoWhile:                return(rtDoWhile());
  case _rtElse:                   return(rtElse());
  case _rtElseIfIO:               if (GET_DD) return(rtElseIfIO(ld1,ld2)); break;
  case _rtEndIf:                  return(rtEndIf());
  case _rtFileClose:              return(rtFileClose());
  case _rtFileCloseAtHost:        return(rtFileCloseAtHost());
  case _rtFileCloseAtIndex:       if (GET_D) return(rtFileCloseAtIndex(ld1)); break;
  case _rtFileFetch:              if (GET_S) return(rtFileFetch(hc1)); break;
  case _rtFileOpen:               if (GET_S) return(rtFileOpen(hc1)); break;
  case _rtFontDef:                if (GET_S) return(rtFontDef(hc1)); break;
  case _rtFontDefEnd:             return(rtFontDefEnd());
  case _rtGetCanLink:             if (!GET_D || rtGetCanLink(ld1,&ld2)!=ERR_OK) break;
                                  sprintf(hc2,"%ld",ld2);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetCanLink",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetCfgIO:               if (!GET_D || rtGetCfgIO(ld1,&ld2)!=ERR_OK) break;
                                  sprintf(hc2,"Config IO%ld = %ld",ld1,ld2);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetCfgIO",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetCounter:             if (rtGetCounter(&ld1)!=ERR_OK) break;
                                  sprintf(hc2,"returned value: %ld",ld1);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetCounter",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetFieldSize:           if (rtGetFieldSize(&lf1)!=ERR_OK) break;
                                  sprintf(hc2,"%lf",lf1);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetFieldSize",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetFileIndex:           if (!GET_S) break;
                                  rtGetFileIndex(hc1,&ld1);
                                  sprintf(hc1,"returned value: %ld",ld1);
                                  MessageBox(GetActiveWindow(),hc1,"rtGetFileIndex",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetFlashMemorySizes:    if (rtGetFlashMemorySizes(&ld1,&ld2)!=ERR_OK) break;
                                  sprintf(hc2,"Total:%ld Allocated:%ld",ld1,ld2);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetFlashMemorySizes",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetID:                  if (rtGetID(hc2)!=ERR_OK) break;
                                  MessageBox(GetActiveWindow(),hc2,"rtGetID",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetIP:                  if (!GET_SS || rtGetIP(hc1,hc2)!=ERR_OK) break;
                                  sprintf(hc1,"reply : %s",hc2);
                                  MessageBox(GetActiveWindow(),hc1,"rtGetIP",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetLaserLink:           if (!GET_D || rtGetLaserLink(ld1,&ld2)!=ERR_OK) break;
                                  sprintf(hc2,"%ld",ld2);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetLaserLink",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetMaxSpeed:            if (rtGetMaxSpeed(&lf1)!=ERR_OK) break;
                                  sprintf(hc2,"%lf",lf1); 
                                  MessageBox(GetActiveWindow(),hc2,"rtGetMaxSpeed",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetScannerDelay:        if (rtGetScannerDelay(&ld1)!=ERR_OK) break;
                                  sprintf(hc2,"%ld",ld1);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetScannerDelay",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtGetSetpointFilter:      if (rtGetSetpointFilter(&ld1)!=ERR_OK) break;
                                  sprintf(hc2,"%ld",ld1);
                                  MessageBox(GetActiveWindow(),hc2,"rtGetSetpointFilter",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtIfIO:                   if (GET_DD) return(rtIfIO(ld1,ld2)); break;
  case _rtIncrementCounter:       return(rtIncrementCounter());
  case _rtIndexFetch:             if (GET_D) return(rtIndexFetch(ld1)); break;
  case _rtJumpTo:                 if (GET_FF) return(rtJumpTo(lf1,lf2)); break;
  case _rtJumpTo3D:               if (GET_FFF) return(rtJumpTo3D(lf1,lf2,lf3)); break;
  case _rtLineTo:                 if (GET_FF) return(rtLineTo(lf1,lf2)); break;
  case _rtLineTo3D:               if (GET_FFF) return(rtLineTo3D(lf1,lf2,lf3)); break;
  case _rtListClose:              return(rtListClose());
  case _rtListOpen:               if (GET_D) return(rtListOpen(ld1)); break;
  case _rtLoadCalibrationFile:    if (GET_S) return(rtLoadCalibrationFile(hc1)); break;
  case _rtLoadCalibrationFileZ:   if (GET_S) return(rtLoadCalibrationFileZ(hc1)); break;
  case _rtMoveTo:                 if (GET_FF) return(rtMoveTo(lf1,lf2)); break;
  case _rtMoveTo3D:               if (GET_FFF) return(rtMoveTo3D(lf1,lf2,lf3)); break;
  case _rtOpenCanLink:            if (GET_D) return(rtOpenCanLink(ld1)); break;
  case _rtPrint:                  if (GET_S) return(rtPrint(hc1)); break;
  case _rtPulse:                  if (GET_FF) return(rtPulse(lf1,lf2)); break;
  case _rtPulse3D:                if (GET_FFF) return(rtPulse3D(lf1,lf2,lf3)); break;
  case _rtResetCalibration:       return(rtResetCalibration());
  case _rtResetCalibrationZ:      return(rtResetCalibrationZ());
  case _rtResetCounter:           return(rtResetCounter());
  case _rtResetEventCounter:      return(rtResetEventCounter());
  case _rtResetResolver:          if (GET_D) return(rtResetResolver(ld1)); break;
  case _rtScanCanLink:            if (GET_DDDD) return(rtScanCanLink(ld1,ld2,ld3,ld4)); break;
  case _rtSendUartLink:           if (GET_S) return(rtSendUartLink(hc1)); break;
  case _rtSetAnalog:              if (GET_DD) return(rtSetAnalog(ld1,ld2)); break;
  case _rtSetCanLink:             if (GET_DDDS) return(rtSetCanLink(ld1,ld2,ld3,hc2)); break;
  case _rtSetCfgIO:               if (GET_DD) return(rtSetCfgIO(ld1,ld2)); break;
  case _rtSetCounter:             if (GET_D) return(rtSetCounter(ld1)); break;
  case _rtSetFieldSize:           if (GET_F) return(rtSetFieldSize(lf1)); break;
  case _rtSetFont:                if (GET_S) return(rtSetFont(hc1)); break;
  case _rtSetHover:               if (GET_D) return(rtSetHover(ld1)); break;
  case _rtSetImageMatrix:         if (GET_FFFF) return(rtSetImageMatrix(lf1,lf2,lf3,lf4)); break;
  case _rtSetImageMatrix3D:       if (GET_FFFFF) return(rtSetImageMatrix3D(lf1,lf2,lf3,lf4,lf5,lf6)); break;
  case _rtSetImageOffsRelXY:      if (GET_FF) return(rtSetImageOffsRelXY(lf1,lf2)); break;
  case _rtSetImageOffsXY:         if (GET_FF) return(rtSetImageOffsXY(lf1,lf2)); break;
  case _rtSetImageOffsZ:          if (GET_F) return(rtSetImageOffsZ(lf1)); break;
  case _rtSetImageRotation:       if (GET_F) return(rtSetImageRotation(lf1)); break;
  case _rtSetIO:                  if (GET_DD) return(rtSetIO(ld1,ld2)); break;
  case _rtSetJumpSpeed:           if (GET_F) return(rtSetJumpSpeed(lf1)); break;
  case _rtSetLaser:               if (GET_D) return(rtSetLaser(ld1!=0)); break;
  case _rtSetLaserTimes:          if (GET_DD) return(rtSetLaserTimes(ld1,ld2)); break;
  case _rtSetLaserFirstPulse:     if (GET_F) return(rtSetLaserFirstPulse(lf1)); break;
  case _rtSetLaserLink:           if (GET_DD) return(rtSetLaserLink(ld1,ld2)); break;
  case _rtSetLead:                if (GET_D) return(rtSetLead(ld1)); break;
  case _rtSetLoop:                if (GET_D) return(rtSetLoop(ld1)); break;
  case _rtSetMatrix:              if (GET_FFFF) return(rtSetMatrix(lf1,lf2,lf3,lf4)); break;
  case _rtSetMaxSpeed:            if (GET_F) return(rtSetMaxSpeed(lf1)); break;
  case _rtSetMinGatePeriod:       if (GET_D) return(rtSetMinGatePeriod(ld1)); break;
  case _rtSetOffsIndex:           if (GET_D) return(rtSetOffsIndex(ld1)); break;
  case _rtSetOffsXY:              if (GET_FF) return(rtSetOffsXY(lf1,lf2)); break;
  case _rtSetOffsZ:               if (GET_F) return(rtSetOffsZ(lf1)); break;
  case _rtSetOscillator:          if (GET_DFF) return(rtSetOscillator(ld1,lf1,lf2)); break;
  case _rtSetOTF:                 if (GET_DD) return(rtSetOTF(ld1,ld2!=0)); break;
  case _rtSetPulseBulge:          if (GET_F) return(rtSetPulseBulge(lf1)); break;
  case _rtSetResolver:            if (GET_DFF) return(rtSetResolver(ld1,lf1,lf2)); break;
  case _rtSetResolverPosition:    if (GET_DF) return(rtSetResolverPosition(ld1,lf1)); break;
  case _rtSetResolverRange:       if (GET_DF) return(rtSetResolverRange(ld1,lf1)); break;
  case _rtSetResolverTrigger:     if (GET_DFD) return(rtSetResolverTrigger(ld1,lf1,ld2)); break;
  case _rtSetRotation:            if (GET_F) return(rtSetRotation(lf1)); break;
  case _rtSetSpeed:               if (GET_F) return(rtSetSpeed(lf1)); break;
  case _rtSetTable:               if (GET_DF) return(rtSetTable(ld1,lf1)); break;
  case _rtSetTableDelay:          if (GET_DD) return(rtSetTableDelay(ld1,ld2)); break;
  case _rtSetTableOffsXY:         if (GET_FF) return(rtSetTableOffsXY(lf1,lf2)); break;
  case _rtSetTableSnap:           if (GET_F) return(rtSetTableSnap(lf1)); break;
  case _rtSetTableSnapSize:       if (GET_DF) return(rtSetTableSnapSize(ld1,lf1)); break;
  case _rtSetTableStepSize:       if (GET_DF) return(rtSetTableStepSize(ld1,lf1)); break;
  case _rtSetTableWhileIO:        if (GET_DD) return rtSetTableWhileIO(ld1,ld2); break;
  case _rtSetTarget:              if (GET_D) return(rtSetTarget(ld1)); break;
  case _rtSetVarBlock:            if (GET_DD) return(rtSetVarBlock(ld1,char(ld2))); break;
  case _rtSetWhileIO:             if (GET_DD) return rtSetWhileIO(ld1,ld2); break;
  case _rtSetWobble:              if (GET_FD) return(rtSetWobble(lf1,ld1)); break;
  case _rtSetWobbleEx:            if (GET_DFDDFDD) return(rtSetWobbleEx(ld1,lf1,ld2,ld3,lf2,ld4,ld5)); break;
  case _rtSleep:                  if (GET_D) return(rtSleep(ld1)); break;
  case _rtSuspend:                return(rtSuspend());
  case _rtSynchronise:            return(rtSynchronise());
  case _rtSystemCheck:            Err=rtSystemCheck(&ld1,&ld2,&ld3);
                                  sprintf(hc2,"reply : %ld,%ld,%ld",ld1,ld2,ld3);
                                  MessageBox(GetActiveWindow(),hc2,"rtSystemCheck",MB_APPLMODAL | MB_OK);
                                  return Err;
  case _rtSystemSelectFile:       if (GET_D) return(rtSystemSelectFile(ld1)); break;
  case _rtSystemSetResolverSpeed: if (GET_DF) return(rtSystemSetResolverSpeed(ld1,lf1)); break;
  case _rtSystemUartOpen:         if (GET_DCD) return(rtSystemUartOpen(ld1,hc,char(ld2))); break;
  case _rtSystemUartWrite:        if (!GET_DS) break;
                                  for (i=0; size_t(i)<strlen(hc1); i++)
                                  {
                                    if (hc1[i]=='\\' && hc1[i+1]=='r') for (hc1[i]=13, j=i+1; size_t(j)<strlen(hc1); j++) hc1[j]=hc1[j+1];
                                    if (hc1[i]=='\\' && hc1[i+1]=='n') for (hc1[i]=10, j=i+1; size_t(j)<strlen(hc1); j++) hc1[j]=hc1[j+1];
                                  }
                                  return(rtSystemUartWrite(ld1,hc1));
  case _rtSystemUDPsend:          if (GET_SDS) return(rtSystemUDPsend(hc1,short(ld1),hc2)); break;
  case _rtTableArcTo:             if (GET_FFF) return(rtTableArcTo(lf1,lf2,lf3)); break;
  case _rtTableJog:               if (GET_DFD) return(rtTableJog(ld1,lf1,ld2)); break;
  case _rtTableJumpTo:            if (GET_FF) return(rtTableJumpTo(lf1,lf2)); break;
  case _rtTableJumpTo3D:          if (GET_FFFFF) return(rtTableJumpTo3D(lf1,lf2,lf3,lf4,lf5,lf6)); break;
  case _rtTableLineTo:            if (GET_FF) return(rtTableLineTo(lf1,lf2)); break;
  case _rtTableMove:              if (GET_DF) return(rtTableMove(ld1,lf1)); break;
  case _rtTableMoveTo:            if (GET_FF) return(rtTableMoveTo(lf1,lf2)); break;
  case _rtUartRead:               if (!GET_D || rtUartRead(&ld1,hc1)!=ERR_OK) break;
                                  hc1[ld1]=0; sprintf(hc2,"reply : %s",hc1);
                                  MessageBox(GetActiveWindow(),hc2,"rtSystemUartRead",MB_APPLMODAL | MB_OK);
                                  return ERR_OK;
  case _rtUDPsend:                if (GET_SDS) return(rtUDPsend(hc1,short(ld1),hc2)); break;
  case _rtVarBlockFetch:          if (GET_DDS) return(rtVarBlockFetch(ld1,ld2,hc1)); break;
  case _rtWaitCanLink:            if (GET_DDD) return(rtWaitCanLink(ld1,ld2,ld3)); break;
  case _rtWaitEventCounter:       if (GET_D) return(rtWaitEventCounter(ld1)); break;
  case _rtWaitIdle:               return(rtWaitIdle());
  case _rtWaitIO:                 if (GET_DD) return(rtWaitIO(ld1,ld2)); break;
  case _rtWaitPosition:           if (GET_F) return(rtWaitPosition(lf1)); break;
  case _rtWaitResolver:           if (GET_DFD) return(rtWaitResolver(ld1,lf1,ld2)); break;
  case _rtWaitStall:              return(rtWaitStall());
  case _rtWhileIO:                if (GET_DD) return(rtWhileIO(ld1,ld2)); break;
  }
#endif
  return ERR_DATA;
}

/*** not supported functions, kept for compatibility *******************************/

RHOTHOR int32_t CALLCONV bcSamplePoint(double X, double Y, int32_t Row, int32_t Col, double Sweep, double* OffsetX, double* OffsetY) { return ERR_IMPLEMENTATION; }
RHOTHOR int32_t CALLCONV bcSelectDevice(const char* CommPort) { return ERR_IMPLEMENTATION; }
RHOTHOR int32_t CALLCONV rtAddCalibrationDataZ(const char* FileName) { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtLoadCalibration() { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtLoadCalibrationFileZ(const char* FileName) { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtResetCalibrationZ() { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtResetEventCounter() { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtSendUartLink(const char* Data) { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtSetHover(int32_t Time)  { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtSetLead(int32_t Time) { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtSetMaxSpeed(double Speed) { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtSetTableOffsXY(double X, double Y)  { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtSetTableSnap(double Distance)  { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtStoreCalibration() { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtStoreCalibrationFileZ(const char* FileName) { return ERR_OK; }
RHOTHOR int32_t CALLCONV rtWaitEventCounter(int32_t Count) { return ERR_OK; }

