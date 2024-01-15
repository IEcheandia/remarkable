/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

#define USB3GSIZE 128

struct cmd3G
{
  uint16_t prm_X;
  uint16_t prm_Y;
  int8_t prm_Xh;
  int8_t prm_Yh;
  uint8_t opcode;
  uint8_t target;
};

struct TgtStatus {
  uint16_t inputs;
  uint16_t outputs;
  uint8_t me;
  uint8_t ReturnValue;
  int16_t h_counter;
  uint8_t c_Analog[4];
  int32_t d_OtfX;
  int32_t d_OtfY;
  int32_t d_TableX;
  int32_t d_TableY;
  int32_t d_TableZ;
  int32_t d_DeflX;
  int32_t d_DeflY;
  int32_t d_DeflZ;
};

class C3G
{
public:
  struct cmd3G cmd;

  operator char*()  { return (char*)&cmd; }
  operator cmd3G*()  { return &cmd; }

  C3G(uint8_t opc, uint8_t tgt)
  {
    cmd.prm_X=0;
    cmd.prm_Y=0;
    cmd.prm_Xh=0;
    cmd.prm_Yh=0;
    cmd.opcode=opc;
    cmd.target=tgt;
  }

  C3G(uint16_t X, uint8_t opc, uint8_t tgt)
  {
    cmd.prm_X=X;
    cmd.prm_Y=0;
    cmd.prm_Xh=0;
    cmd.prm_Yh=0;
    cmd.opcode=opc;
    cmd.target=tgt;
  }

  C3G(int32_t X, uint8_t opc, uint8_t tgt)
  {
    *(int32_t*)(&cmd.prm_X)=X;
    cmd.prm_Xh=0;
    cmd.prm_Yh=0;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(float X, uint8_t opc, uint8_t tgt)
  {
    *(float*)(&cmd.prm_X)=X;
    cmd.prm_Xh=0;
    cmd.prm_Yh=0;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(uint16_t X, uint16_t Y, uint8_t opc,uint8_t tgt)
  {
    cmd.prm_X=X;
    cmd.prm_Y=Y;
    cmd.prm_Xh=0;
    cmd.prm_Yh=0;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(int32_t p1, uint16_t p2, uint8_t opc, uint8_t tgt)
  {
    *((int32_t*)&cmd.prm_X)=p1;
    *((uint16_t*)&cmd.prm_Xh)=p2;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(float p1, int16_t p2, uint8_t opc, uint8_t tgt)
  {
    *((float*)&cmd.prm_X)=p1;
    *((int16_t*)&cmd.prm_Xh)=p2;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(int32_t X, int32_t Y, uint8_t opc, uint8_t tgt)
  {
    cmd.prm_X=X&0xffff;
    cmd.prm_Y=Y&0xffff;
    cmd.prm_Xh=int8_t(X>>16);
    cmd.prm_Yh=int8_t(Y>>16);
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(int32_t X, int8_t p1, int8_t p2, uint8_t opc, uint8_t tgt)
  {
    *(int32_t*)(&cmd.prm_X)=X;
    cmd.prm_Xh=p1;
    cmd.prm_Yh=p2;
    cmd.opcode=opc;
    cmd.target=tgt;
  }

  C3G(float X, int8_t p1, int8_t p2, uint8_t opc, uint8_t tgt)
  {
    *(float*)(&cmd.prm_X)=X;
    cmd.prm_Xh=p1;
    cmd.prm_Yh=p2;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(int16_t X, int16_t Y, int8_t Xh, int8_t Yh, uint8_t opc, uint8_t tgt)
  {
    cmd.prm_X=X;
    cmd.prm_Y=Y;
    cmd.prm_Xh=Xh;
    cmd.prm_Yh=Yh;
    cmd.opcode=opc;
    cmd.target=tgt;
  }

  C3G(int16_t X, int16_t Y, int16_t Z, uint8_t opc, uint8_t tgt)
  {
    cmd.prm_X=X;
    cmd.prm_Y=Y;
    *(uint16_t*)(&cmd.prm_Xh)=Z;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
  
  C3G(int8_t b0, int8_t b1, int8_t b2, int8_t b3, int8_t b4, int8_t b5, uint8_t opc, uint8_t tgt)
  {
    cmd.prm_X=(b1<<8)|(b0&0xff);
    cmd.prm_Y=(b3<<8)|(b2&0xff);;
    cmd.prm_Xh=b4;
    cmd.prm_Yh=b5;
    cmd.opcode=opc;
    cmd.target=tgt;
  }
};

class CFifo
{
public:
  CFifo* pNext;
  CFifo* pPrev;
  cmd3G cData[USB3GSIZE];

  CFifo();
  bool Items();
  void DeleteItem(CFifo* p);
  void AppendItem(CFifo* p);
  int32_t CountItems();
};

class CStack
{
public: 
  CStack* pNext;
  CStack* pPrev;
  int32_t d_Address;
  int32_t d_Type;
  
  CStack(int32_t Address=0, int32_t Type=0);
  bool Items();
  void Push(CStack* p);
  void Pop();
};



#if NEWSON_LEGACY
class CrtCtrl32 : public CrtCtrlXX
{
#else
class CrtCtrl32
{
public:
#ifdef _WIN32
  HINSTANCE h_Dll;
#endif
  int32_t ld_ListMode;
#endif

public:
  bool b_Polling;
  thread_t h_Poller;
  bool b_Destroy;
  mutex_t h_Mutex;


public:
  CHardware32* p_Hw;
  ParamSet32 m_Params;
  uint16_t hd_Cal[33*33*8+129];
  int c_qryidx;
  int c_tgtmsk;
  int c_tgtidx;
  char c_Channel;

//' Construction
public:
  CrtCtrl32();
  virtual ~CrtCtrl32();

public:
  char m_ListStatus;
  struct TgtStatus m_TgtStatus;
  CFifo m_CmdList;
  CFifo* p_Cmd;
  int32_t ld_WrPtr;

  int32_t CmdListAdd(cmd3G* p);
  int32_t CmdListAddHex(char* hexsrc);
  int32_t CmdListAddString(char* str, int32_t tgt);
  int32_t CmdInt(cmd3G* p);

//' Private
  void ResetStatus();
  void SuspendPolling();
  void ResumePolling();

//' Public List Operations
public:
  int32_t SetTarget(int32_t Mask);
  int32_t GetTarget(int32_t* Mask);
  int32_t SetQueryTarget(int32_t Index);
  int32_t GetQueryTarget(int32_t* Index);
  int32_t Abort(); 
  int32_t ArcXY(ClfPoint pt_Co, double BF);
  int32_t ArcMoveXY(ClfPoint pt_Co, double BF);
  int32_t Burst(int Time);
  int32_t BurstEx(int32_t Time1, int32_t Time2, double PWM2);
  int32_t CharDef(int32_t Ascii);
  int32_t Circle(ClfPoint ptCo, double Angle, bool Laser);
  int32_t DoLoop();
  int32_t If_Else();
  int32_t If_ElseIO(int32_t Value, int32_t Mask);
  int32_t FileFetch(char* Name);
  int32_t FontDef(char* Name);
  int32_t FontDefEnd();
  int32_t If_End();
  int32_t If_IO(int32_t Value, int32_t Mask);
  int32_t IncrementCounter();
  int32_t JumpXY(ClfPoint pt_Co);
  int32_t JumpXYZ(ClfPoint pt_Co, double Z);
  int32_t LineXY(ClfPoint pt_Co);
  int32_t LineXYZ(ClfPoint pt_Co, double Z);
  int32_t LaserLink(int32_t Command, int32_t Value);
  int32_t ListClose();
  int32_t ListOpen(int32_t Mode);
  int32_t MoveXY(ClfPoint pt_Co);
  int32_t MoveXYZ(ClfPoint pt_Co, double Z);
  int32_t OpenCanLink(int32_t Baudrate);
  int32_t Print(char* data);
  int32_t PulseXY(ClfPoint pt_Co);
  int32_t PulseXYZ(ClfPoint pt_Co, double Z);
  int32_t SetFont(char* Name);
  int32_t ResetCounter();
  int32_t ResetResolver(int Nr);
  int32_t SectorFetch(int32_t Sector);
  int32_t SetAij(double A11, double A12, double A21, double A22);
  int32_t SetCfgIO(int32_t Nr, int32_t Value);
  int32_t SetCanLink(int32_t Node, int32_t Index, int32_t SubIndex, char* Data);
  int32_t SetCounter(int32_t Value);
  int32_t SetFieldSize(double Size);
  int32_t SetImageAij(double A11, double A12, double A21, double A22, double A31, double A32);
  int32_t SetImageOffsXY(ClfPoint pt_Co);
  int32_t SetImageOffsRelXY(ClfPoint pt_Co);
  int32_t SetImageOffsZ(double Z);
  int32_t SetImageRotation(double Angle);
  int32_t SetIO(int32_t value, int32_t mask);
  int32_t SetJumpSpeed(double Speed);
  int32_t SetLaser(bool OnOff);
  int32_t SetLaserFirstPulse(double Time);
  int32_t SetLaserTimes(int32_t GateOnDelay, int32_t GateOffDelay);
  int32_t SetLoop(int32_t LoopCtr);
  int32_t SetModeLaser(int32_t Mode);
  int32_t SetOffsIndex(int32_t Index);
  int32_t SetOffsXY(ClfPoint pt_Co);
  int32_t SetOffsZ(double Z);
  int32_t SetOTF(int32_t Nr, bool On);
  int32_t SetPWM(int Nr, double Period, double Pulse);
  int32_t SetResolver(int Nr, double StepSize, double Range);
  int32_t SetResolverPosition(int32_t Nr, double Position);
  int32_t SetResolverRange(int Nr, double Range);
  int32_t SystemSetResolverSpeed(int Nr, double Speed);
  int32_t SetResolverTrigger(int32_t Nr, double Position, int32_t TriggerIO);
  int32_t SetRotation(double Angle);
  int32_t SetSpeed(double Speed);
  int32_t SetTable(int32_t Nr, double Position);
  int32_t SetTableDelay(int32_t Nr, int32_t Delay);
  int32_t SetTableStepSize(int32_t Nr, double StepSize);
  int32_t SetVarBlock(int32_t i, char data);
  int32_t SetWobble(double Diam, int32_t Freq);
  int32_t SetWobbleEx(int32_t nType, double nAmpl, int32_t nFreq, int32_t tType, double tAmpl, int32_t tHarm, int32_t tPhase);
  int32_t Sleep(int32_t Time);
  int32_t Suspend();
  int32_t TableArcXY(ClfPoint pt_Co, double BF);
  int32_t SetTableWhileIO(int32_t Value, int32_t Mask);
  int32_t TableJump3D(double X, double Y, double Z, double SpeedX, double SpeedY, double SpeedZ);
  int32_t TableJog(int32_t Nr, double Speed, int32_t WhileIO);
  int32_t TableJumpXY(ClfPoint pt_Co);
  int32_t TableLineXY(ClfPoint pt_Co);
  int32_t TableMove(int32_t Nr, double Target);
  int32_t TableMoveXY(ClfPoint pt_Co);
  int32_t SystemUDPsend(char* IP, int16_t port, char* data);
  int32_t UDPsend(char* IP, short port, char* data);
  int32_t VarBlockFetch(int32_t Start, int32_t Size, char* FontName);
  int32_t WaitCanLink(int32_t Address, int32_t Value, int32_t Mask);
  int32_t WaitIO(int32_t Value, int32_t Mask);
  int32_t WaitPosition(double Window);
  int32_t WaitResolver(int Nr, double TriggerPos, int32_t TriggerMode);
  int32_t While_IO(int32_t Value, int32_t Mask);
  int32_t While_End();
  int32_t SystemSelectFile(int32_t Index);
  int32_t ScanCanLink(int32_t Address, int32_t Node, int32_t Index, int32_t SubIndex);
  int32_t GetCanLink(int32_t Address, int32_t* Value);

//' Public non list Functions 
public: 
  int32_t SystemReset();
  int32_t SystemSetIO(int32_t Value, int32_t Mask);
  int32_t SystemSuspend();
  int32_t SystemResume();
  int32_t UARTopen(int32_t baudrate, char parity, char stopbits);
  int32_t UARTwrite(int32_t bytes, char* data);
  int32_t UARTread(int32_t* bytes, char* data);
  int32_t GetCounter(int32_t* Value);
  double GetMaxSpeed();

  int32_t GetStatus(int32_t* pCounter);
  int32_t Connect(const char* IP, bool StartPollerThread);
  int32_t Disconnect();
  int32_t ReadFlash(char* pPa);
  int32_t ReadFlashBootScript(char* c, int32_t size);

//  int32_t SetpointFilter();
  int32_t Stop();
  int32_t StoreFlash();
  int32_t StoreFlashBootScript(char* c);
//' Align Functions
public:
  int32_t AddCalibrationData(const char* FileName, bool bReset);
  int32_t LoadCalibrationFile(const char* FileName);
  int32_t StoreCalibrationFile(const char* FileName);

// Flash functions
public:
  FileEntry FAT[FILES];
  int32_t ld_FileEntry;
  char FileName[256];
  char* p_File;
  int32_t ld_Size;
  CStack m_Stack;
  int32_t ld_FontBase;
  int32_t EraseFromFlash(char* Name);

  int32_t FormatFlash();
  int32_t FileUpload(char* SrcFile, char* FileName, int32_t Sector=-1);
  int32_t FileDownload(char* FileName, char* DestFile);
  int32_t GetFlashFirstFileEntry(char* Name, int32_t* Size);
  int32_t GetFlashMemorySizes(int32_t* Total, int32_t* Allocated);
  int32_t GetFlashNextFileEntry(char* Name, int32_t* Size);
  void Flash_WR(int32_t Address, char* c, int32_t len);
  void Flash_RD(int32_t Address, char* c, int32_t len);
  int32_t LoadDirectory();
  int32_t FileSearch(char* Name); 
  void FileAbort();/*
  int32_t FileCopy(char* SrcFile, char* DestFile);
  void FileLock(int32_t Address);*/
  int32_t FindFirstFreeSector(int32_t Size);
  int32_t FileClose(int32_t Sector=-1);
  int32_t FileCloseHost();
  int32_t FileOpen(char* Name);
  int32_t FileAppendCommand(cmd3G* p);/*
  int32_t FileSetCommand(int32_t CommandNr, cmd3G* p);*/
  int32_t LoadProfile(const char* FileName, int32_t Include);
  int32_t StoreProfile(const char* FileName, int32_t Include);

// low level access functions
//  void ListWrite(char* c, int32_t len);
  int32_t GetReturnValue();
  void* GetParamset();
  FileEntry* GetFAT();
  int32_t GetSetpointFilter(int32_t* TimeConst);
  void* GetCalibration();
  int32_t GetCfgIO(int32_t Nr, int32_t *Value);
  int32_t GetTablePositions(double* X, double* Y, double* Z);
  int32_t GetResolvers(double* X, double* Y);
  int32_t GetFieldSize(double* Size);
  int32_t GetFieldSizeZ(double* Size);
  char* GetID();
  int32_t GetLaserLink(int32_t Address, int32_t* Value);
  int32_t GetScannerDelay(int32_t* Delay);
  int32_t GetSerial(int32_t* Serial);
  int32_t GetIO();
  int32_t SetAnalog(int32_t Value, int32_t Mask);
  bool FlowCommands();
  uint32_t GetAnalog(int32_t Nr);
  int32_t SetMinGatePeriod(int32_t Time);
  int32_t SetBulge(double Factor);
  int32_t GetDeflReplies(int32_t* CH1, int32_t* CH2, int32_t* CH3);
  int32_t TestIO(int32_t Value, int32_t Mask, char Equal);
  int32_t WaitIO(int32_t Value, int32_t Mask, char Equal);
  int32_t Branch(int32_t Target);
  int32_t WaitIdle(int32_t tgt);
  int32_t WaitStall(int32_t tgt);
  int32_t ScanTargets(char* ID);
  int32_t MapTarget(uint16_t ID, char Tgt);
  int32_t GetIP(char* Mac, char* IP);
  int32_t SetTableSnapSize(int32_t Nr, double SnapSize);
  int32_t Synchronise();
  int32_t SetWhileIO(int32_t Value, int32_t Mask);
  int32_t SystemCheck(int32_t *Target, int32_t* Powered, int32_t* Bios);
//  int32_t Call(int32_t Target);
//  int32_t CallEnd();
  int32_t ChSelect(int32_t channel);
  int32_t ChTest();
  int32_t ChStoreFlash(char* data, int32_t size);
  int32_t ChReadFlash(char* data, int32_t size);
  int32_t ChSetpoints(char* data, int32_t size);
  int32_t ChActuals(char* data, int32_t size);
  int32_t ChParam(int32_t idx, int32_t value);
  int32_t ChRead(int32_t command, char* pData);
  int32_t ChWrite(int32_t command);
};
