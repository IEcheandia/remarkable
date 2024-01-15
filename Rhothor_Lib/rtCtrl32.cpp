/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

//#include "stdafx.h"
#if NEWSON_LEGACY
#include "..\Sw01 [dll]\All.h"
#else
#include "All.h"
#endif

#define BIOSFE              0x0b020500
#define BIOSTGT             0x0b020800

#define SYSSTALL            uint8_t(0x40)
#define IOABORT             uint8_t(0x20)
#define TGTALL              uint8_t(0xFF)

#define CMD3G_NOP           uint8_t(0x00)
#define CMD3G_SETIO         uint8_t(0x02)
#define CMD3G_SETSPEED      uint8_t(0x03)
#define CMD3G_JUMPTO        uint8_t(0x04)
#define CMD3G_MOVETO        uint8_t(0x05)
#define CMD3G_PULSTO        uint8_t(0x06)
#define CMD3G_LINETO        uint8_t(0x07)
#define CMD3G_PARAMS        uint8_t(0x08)
#define CMD3G_ARCMOVE       uint8_t(0x09)
#define CMD3G_ARCLINE       uint8_t(0x0a)
#define CMD3G_SPEED         uint8_t(0x0b)
#define CMD3G_JUMPSPEED     uint8_t(0x0c)
#define CMD3G_SETOSC        uint8_t(0x0d)
#define CMD3G_SETLOOP       uint8_t(0x0e)
#define CMD3G_DOLOOP        uint8_t(0x0f)
#define CMD3G_SLEEP         uint8_t(0x10)
#define CMD3G_SUSPEND       uint8_t(0x11)
#define CMD3G_SETDELAYS     uint8_t(0x12)
#define CMD3G_SETLIDLE      uint8_t(0x13)
#define CMD3G_SM_LASER      uint8_t(0x14)
#define CMD3G_EXCHLLINK     uint8_t(0x15)
#define CMD3G_TABLESET      uint8_t(0x16)
#define CMD3G_TABLESSIZE    uint8_t(0x17)
#define CMD3G_TABLEDELAY    uint8_t(0x18)
#define CMD3G_TABLE1D       uint8_t(0x19)
#define CMD3G_SETFDATA      uint8_t(0x1a)
#define CMD3G_TABLEMOVETO   uint8_t(0x1b)
#define CMD3G_SETIMGAIJ     uint8_t(0x1c)
#define CMD3G_SETIMGROT     uint8_t(0x1d)
#define CMD3G_SETIMGOFFS    uint8_t(0x1e)
#define CMD3G_SETIMGOFFSR   uint8_t(0x1f)
#define CMD3G_SETIMGOFFSZ   uint8_t(0x20)
#define CMD3G_SETFS         uint8_t(0x21)
#define CMD3G_SETFSZ        uint8_t(0x22)
#define CMD3G_SETSPFLTR     uint8_t(0x23)
#define CMD3G_BURST         uint8_t(0x24)
#define CMD3G_IDXFETCH      uint8_t(0x25)
#define CMD3G_LIST          uint8_t(0x26)
#define CMD3G_SETCNTR       uint8_t(0x27)
#define CMD3G_SETANA        uint8_t(0x28)
#define CMD3G_CFG_IO        uint8_t(0x29)
#define CMD3G_OTFSTEP       uint8_t(0x2a)
#define CMD3G_OTFRANGE      uint8_t(0x2b)
#define CMD3G_OTFSET        uint8_t(0x2c)
#define CMD3G_WAITIO        uint8_t(0x2d)
#define CMD3G_OTFWAIT       uint8_t(0x2e)
#define CMD3G_OTFENABLE     uint8_t(0x2f)
#define CMD3G_SETMINGATE    uint8_t(0x30)
#define CMD3G_SETBULGE      uint8_t(0x31)
#define CMD3G_SETDDELAY     uint8_t(0x32)
#define CMD3G_CIRCLE        uint8_t(0x33)
#define CMD3G_TABLEJOG      uint8_t(0x34)
#define CMD3G_TABLELINETO   uint8_t(0x35)
#define CMD3G_TABLEJUMPTO   uint8_t(0x36)
#define CMD3G_TABLEARCLINE  uint8_t(0x37)
#define CMD3G_BRANCH        uint8_t(0x38)
#define CMD3G_SETAIJ        uint8_t(0x39)
#define CMD3G_SETROT        uint8_t(0x3a)
#define CMD3G_SETOFFS       uint8_t(0x3b)
#define CMD3G_SETOFFSZ      uint8_t(0x3c)
#define CMD3G_SETWOBBLE     uint8_t(0x3d)
#define CMD3G_JUMPTO3D      uint8_t(0x3e)
#define CMD3G_MOVETO3D      uint8_t(0x3f)
#define CMD3G_PULSTO3D      uint8_t(0x40)
#define CMD3G_LINETO3D      uint8_t(0x41)
#define CMD3G_TABLEWHILEIO  uint8_t(0x42)
#define CMD3G_BSTR0         uint8_t(0x43)
#define CMD3G_BSTRN         uint8_t(0x44)
#define CMD3G_UDPSEND       uint8_t(0x45)
#define CMD3G_TABLESNAP     uint8_t(0x46)
#define CMD3G_TABLEJUMP3D   uint8_t(0x47)
#define CMD3G_WHILEIO       uint8_t(0x48)
#define CMD3G_SETLASERDELAY uint8_t(0x49)
#define CMD3G_CCODE_BEGIN   uint8_t(0x4a)
#define CMD3G_CCODE_END     uint8_t(0x4b)
#define CMD3G_CHAR          uint8_t(0x4c)
#define CMD3G_CHAR_END      uint8_t(0x4d)
#define CMD3G_FONTHDR       uint8_t(0x4e)
#define CMD3G_VAR_CALL      uint8_t(0x4f)
#define CMD3G_VAR_SET       uint8_t(0x50)
#define CMD3G_TESTIO        uint8_t(0x51)
#define CMD3G_CAN_INIT      uint8_t(0x52)
#define CMD3G_CAN_SEND      uint8_t(0x53)
#define CMD3G_IDXUPLOAD     uint8_t(0x54)
#define CMD3G_PRINT         uint8_t(0x55)
#define CMD3G_CAN_SCAN      uint8_t(0x56)
#define CMD3G_CAN_WAIT      uint8_t(0x57)
#define CMD3G_EOF           uint8_t(0xFF)

#define INTCMD              uint8_t(0x80)
#define INTSUSPEND          uint8_t(0x81)
#define INTRESUME           uint8_t(0x82)
#define INTABORT            uint8_t(0x83)
#define INTDEFLWR           uint8_t(0x84)
#define INTRUNMODE          uint8_t(0x85)
#define INTFLASHEP          uint8_t(0x87)
#define INTFLASHPP          uint8_t(0x88)
#define INTSETIO            uint8_t(0x89)
#define INTUARTOPEN         uint8_t(0x8a)
#define INTUARTWRITE        uint8_t(0x8b)
#define INTEXCHLLINK        uint8_t(0x8c)
#define INTTESTIO           uint8_t(0x8d)
#define INTWAITIDLE         uint8_t(0x8e)
#define INTMAPTGT           uint8_t(0x8f)
#define INTSCANTGT          uint8_t(0x90)
#define INTUDPSEND          uint8_t(0x91)
#define INTWAITIO           uint8_t(0x92)
#define INTCHSETPOINTS      uint8_t(0x93)
#define INTCHTESTPARAMS     uint8_t(0x94)
#define INTSELFILE          uint8_t(0x95)
#define INTWAITSTALL        uint8_t(0x96)
#define INTOTFSPEED         uint8_t(0x97)

#define INTREPLY            uint8_t(0x40)
#define INTSTATUS           uint8_t(0xc0)
#define INTTGTSTATUS        uint8_t(0xc1)
#define INTDEFLEXCH         uint8_t(0xc2)
#define INTFLASHRD          uint8_t(0xc4)
#define INTGTID             uint8_t(0xc5)
#define INTFLASHRDY         uint8_t(0xc6)
#define INTUARTREAD         uint8_t(0xc7)
#define INTSCANREAD         uint8_t(0xc8)
#define INTGETIP            uint8_t(0xc9)
#define INTADDCALDATA       uint8_t(0xca)
#define INTSTORECALFILE     uint8_t(0xcb)
#define INTLOADCALFILE      uint8_t(0xcc)
#define INTRESETCAL         uint8_t(0xcd)
#define INTFLASHPP2         uint8_t(0xce)
#define INTCHFLASHRD        uint8_t(0xcf)
#define INTCHFLASHPP        uint8_t(0xd0)
#define INTCHACTUALS        uint8_t(0xd1)
// #define INTLISTWRITE        uint8_t(0xd2)
// #define INTLISTSTART        uint8_t(0xd3)
#define INTCANREAD          uint8_t(0xd4)

#define SCODE1              0xa5d2
#define MODE_STOP           0x00
#define MODE_LIST           0x01
#define MODE_CH             0x02
#define MODE_CHTEST         0x03

#define FIFORANGE           0xF
#define UARTFIFOSIZE        0x200

#define SIZEOF_CALTBL       ((33*33*8+129)*2)

#define ADDR_TBD            0
#define T_BASE              0
#define T_IF                1
#define T_DO                2
#define T_WHILE             3
#define T_ELSEIF            4

#define RAWSECTOR           0x3456
#define CALSIZE             ((33*33*8+129)*2)
struct ProfileHdr
{
  int16_t ID;
  int16_t Sector;
  int32_t DataSize;
  int32_t CheckSum;
};

/************************************************************/
/*** not supported (yet)                                  ***/
/************************************************************/

int32_t CrtCtrl32::SetOffsIndex(int32_t Index) { return ERR_OK; }
int32_t CrtCtrl32::SetResolverTrigger(int32_t Nr, double Position, int32_t TriggerIO) { return ERR_OK; }
int32_t CrtCtrl32::WaitPosition(double Window) { return ERR_OK; }

/************************************************************/
/*** CFifo supporting class                               ***/
/************************************************************/

CFifo::CFifo()
{
  pNext=this;
  pPrev=this;
}
  
bool CFifo::Items()
{ 
  return pNext!=this;
}
  
void CFifo::AppendItem(CFifo* p)
{
  p->pPrev=pPrev;
  pPrev=p;
  p->pPrev->pNext=p;
  p->pNext=this;
}
  
void CFifo::DeleteItem(CFifo* p)
{
  p->pPrev->pNext=p->pNext;
  p->pNext->pPrev=p->pPrev;
  delete p;
}

int32_t CFifo::CountItems()
{
  int32_t i=0;
  CFifo* p=pNext;
  while (p!=this) { p=p->pNext; i++; }
  return i;
}

/************************************************************/
/*** CStack supporting class                              ***/
/************************************************************/
  
CStack::CStack(int32_t Address, int32_t Type)
{
  pNext=this;
  pPrev=this;
  d_Address=Address;
  d_Type=Type;
}

bool CStack::Items()
{
  return pNext!=this;
}

void CStack::Push(CStack* p)
{
  p->pPrev=pPrev;
  pPrev=p;
  p->pPrev->pNext=p;
  p->pNext=this;
}
  
void CStack::Pop()
{
  CStack* p=pPrev;
  p->pPrev->pNext=p->pNext;
  p->pNext->pPrev=p->pPrev;
  delete p;
}

/************************************************************/
/*** non member functions                                 ***/
/************************************************************

char(*ReadID2(void* p))[64] 
{
  static char Serial2[64];
  int32_t i;

  cmd3G query={0,0,0,0,INTGTID,0};
  ((CHardwareUSB32*)p)->Exchange((char*)(&query),&Serial2[0],sizeof(cmd3G),64,0);
  for (i=0; i<64 && Serial2[i]!=0; i++);
  if (i==0 || i==64) strncpy(Serial2,"Empty ID",64);

  return &Serial2;
}
*/
void* PollProc32(void* lpData)
{
  CrtCtrl32* pCtrl=(CrtCtrl32*)(lpData);
  cmd3G query={0x00,sizeof(char),0x00,0x00,INTSTATUS,0x00};
  cmd3G querytgt={0x00,sizeof(TgtStatus),0x00,0x00,INTTGTSTATUS,0x00};

  char c;
  while (!pCtrl->b_Destroy)
  {
    mutex_lock(pCtrl->h_Mutex);
    if (pCtrl->b_Polling)
    {
      pCtrl->p_Hw->Exchange((char*)(&query),&pCtrl->m_ListStatus,sizeof(cmd3G),sizeof(char),0);
      for (c=pCtrl->m_ListStatus&FIFORANGE; c>0 && pCtrl->m_CmdList.Items(); c--)
      {
        pCtrl->p_Hw->Exchange((char*)(pCtrl->m_CmdList.pNext->cData),NULL,USB3GSIZE*sizeof(cmd3G),0,0);
        pCtrl->m_CmdList.DeleteItem(pCtrl->m_CmdList.pNext);
        if (pCtrl->m_ListStatus&SYSSTALL) pCtrl->m_ListStatus&=~SYSSTALL;
      }
      querytgt.target=0x01<<pCtrl->c_qryidx;
      pCtrl->p_Hw->Exchange((char*)(&querytgt),(char*)(&pCtrl->m_TgtStatus),sizeof(cmd3G),sizeof(TgtStatus),0);
      pCtrl->m_TgtStatus.me=pCtrl->c_qryidx;
    }
    mutex_unlock(pCtrl->h_Mutex);
    thread_sleep(50);
  }
  return 0;
}

/************************************************************/
/*** CrtCtrl32 general functions                          ***/
/************************************************************/

int32_t HexToBString(uint8_t* bstr, char* hex)
{
  uint16_t k;
  int32_t i,j;

  j=strlen(hex)/2;
  if (j>255) return ERR_DATA;
  for (i=0; i<j; i++) { sscanf(&hex[i*2],"%02hx",&k); bstr[i+1]=k&0xff; }
  bstr[0]=j&0xff;
  return ERR_OK; 
}

int32_t CrtCtrl32::CmdListAdd(cmd3G* p)
{
  if (p_File) // compile file (listmode 1,3,4)
  {
    if ((ld_Size+8)%SECTOR_SIZE32==8)
      p_File=(char*)(realloc(p_File,SECTOR_SIZE32*(1+(ld_Size+8)/SECTOR_SIZE32)));
    for (int32_t i=0;i<8;i++) p_File[ld_Size++]=((char*)(p))[i];
  }
  else // streaming (listmode 2)
  {
    if (p_Cmd==NULL) { p_Cmd=new(CFifo); ld_WrPtr=0; }
    p_Cmd->cData[ld_WrPtr++]=*p;
    if (ld_WrPtr==USB3GSIZE)
    {
      mutex_lock(h_Mutex);
      m_CmdList.AppendItem(p_Cmd);
      p_Cmd=NULL;
      mutex_unlock(h_Mutex);
    }
  }
  return ERR_OK;
}


int32_t CrtCtrl32::CmdListAddHex(char* hexsrc)
{
  int32_t i,imax;
  uint8_t d[256];
  
  struct cmd3G bstr={0,0,0,0,CMD3G_BSTR0,0};
  int32_t Err=HexToBString(d,hexsrc);
  if (Err==ERR_OK) for (i=0, imax=d[0]+1; i<imax; i+=6)
  {
    memcpy(&bstr,&d[i],std::min(6,imax-i));
    CmdListAdd(&bstr);
    bstr.opcode=CMD3G_BSTRN;
  }
  return Err; 
}


int32_t CrtCtrl32::CmdListAddString(char* str, int32_t tgt)
{
  int32_t i,imax;
  uint8_t d[256];
  
  d[0]=std::min((int)strlen(str),254);
  struct cmd3G bstr={0,0,0,0,CMD3G_BSTR0,(uint8_t)tgt};
  memcpy(&d[1],str,d[0]);
  for (i=0, imax=d[0]+1; i<imax; i+=6)
  {
    memcpy(&bstr,&d[i],std::min(6,imax-i));
    CmdListAdd(&bstr);
    bstr.opcode=CMD3G_BSTRN;
  }
  return ERR_OK; 
}


int32_t CrtCtrl32::CmdInt(cmd3G* p)
{
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)p,NULL,sizeof(cmd3G),0,0);
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::Connect(const char* IP, bool StartPollerThread)
{
  if (h_Poller)
  {
    mutex_lock(h_Mutex);
    SuspendPolling();
  }
  else // first time initialise
  {
    mutex_create(h_Mutex);
    mutex_lock(h_Mutex);
    thread_init(h_Poller, PollProc32, this);
    b_Polling=false;
  }

  char Serial2[64];
  cmd3G query={0,0,0,0,INTGTID,0};
  p_Hw->SetAddress(IP);
  if (p_Hw->ConnectDSP()==ERR_OK && 
      p_Hw->Exchange((char*)(&query),&Serial2[0],sizeof(cmd3G),64,1)==ERR_OK)
  {
    CmdInt(C3G(uint16_t(250),INTSELFILE,0)); // prevent IO start
    CmdInt(C3G(INTABORT,TGTALL)); // abort hardware
    c_tgtmsk=TGTALL;
    c_tgtidx=0;
    c_qryidx=0;
    ReadFlash((char*)&m_Params);
    ResetStatus();
    LoadDirectory();
    ResumePolling();
    mutex_unlock(h_Mutex);
    return ERR_OK;
  }
  else
  {
    mutex_unlock(h_Mutex);
    return ERR_HARDWARE;
  }
}

CrtCtrl32::CrtCtrl32()
{
  ld_ListMode=0;
  p_Hw=NULL;
  h_Poller=0;
  b_Polling=false;
  b_Destroy=false;
  p_Cmd=NULL;
  c_tgtmsk=TGTALL;
  c_tgtidx=0;
  c_qryidx=0;
  c_Channel=0;
  mutex_init(h_Mutex);
}

CrtCtrl32::~CrtCtrl32()
{
  Disconnect();
  b_Destroy=true;
  if (h_Poller) thread_join(h_Poller);
}

int32_t CrtCtrl32::Disconnect()
{
  if (h_Poller)
  {
    if (ld_ListMode) Abort(); // abort when not idle 
    mutex_lock(h_Mutex);
    SuspendPolling(); // stop poller thread
    mutex_unlock(h_Mutex);
  }
  DELETE_(p_Hw); // remove hardware
  return ERR_OK;
}

bool CrtCtrl32::FlowCommands()
{ 
  return ld_ListMode==1 ||
         ld_ListMode==3 ||
         ld_ListMode==4 ||
         ld_ListMode==5 ||
         ld_ListMode==FILE_LOAD;
}

int32_t CrtCtrl32::GetSerial(int32_t* Serial)
{
  *Serial=m_Params.d_SerialNr;
  return ERR_OK;
}

int32_t CrtCtrl32::GetTarget(int32_t* Mask)
{
  *Mask=c_tgtmsk;
  return ERR_OK;
}

int32_t CrtCtrl32::GetQueryTarget(int32_t* Index)
{
  *Index=c_qryidx+1;
  return ERR_OK;
}

int32_t CrtCtrl32::MapTarget(uint16_t ID, char Tgt)
{
  return(CmdInt(C3G(ID,SCODE1,int8_t(0x01<<Tgt),0,INTMAPTGT,TGTALL)));
}

void CrtCtrl32::ResumePolling()
{
  mutex_lock(h_Mutex);
  b_Polling=true;
  mutex_unlock(h_Mutex);
}
int32_t CrtCtrl32::ScanTargets(char* ID)
{
  struct cmd3G cmd={0,SCODE1,0,0,INTSCANTGT,TGTALL};
  struct cmd3G cmdread={0,16,0,0,INTSCANREAD,0};
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)&cmd,NULL,sizeof(cmd3G),0,0);
  thread_sleep(1000); // scanning takes 327 millisec !
  p_Hw->Exchange((char*)&cmdread,(char*)ID,sizeof(cmd3G),16,0);
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::SystemSelectFile(int32_t Index)
{
  return(CmdInt(C3G(uint16_t(Index&0xffff),INTSELFILE,0)));
}

int32_t CrtCtrl32::SetTarget(int32_t Mask)
{
  if (ld_ListMode==5) return ERR_OK; // target is ignored in listmode 5 !
  c_tgtmsk=Mask;
  for (c_tgtidx=0; c_tgtidx<8 && !(c_tgtmsk&(0x01<<c_tgtidx)); c_tgtidx++);
  c_tgtidx&=7;
  return ERR_OK;
}

int32_t CrtCtrl32::SetQueryTarget(int32_t Index)
{
  if (Index<1 || Index>8) return ERR_DATA;
  Index-=1;
  if (c_qryidx!=Index)
  {
    if (b_Polling)
    {
      for (bool done=false; !done;)
      {
        mutex_lock(h_Mutex);
        c_qryidx=Index;
        done=m_TgtStatus.me==c_qryidx;
        mutex_unlock(h_Mutex);
      }
    }
    else c_qryidx=Index;
  }
  return ERR_OK;
}

void CrtCtrl32::SuspendPolling()
{
  mutex_lock(h_Mutex);
  b_Polling=false;
  mutex_unlock(h_Mutex);
}

int32_t CrtCtrl32::SystemCheck(int32_t *Target, int32_t* Powered, int32_t* Bios)
{
  int32_t Err=ERR_OK;
  int32_t i,bios;

  mutex_lock(h_Mutex);
  p_Hw->Exchange(C3G(int32_t(SECTOR_INFO*SECTOR_SIZE32),uint16_t(4),INTFLASHRD,0),(char*)&bios,sizeof(cmd3G),4,0);
  if ((bios^BIOSFE)&0xFFFFFF00) Err=WARNING_BIOS;
  *Target=0;
  *Powered=0;
  *Bios=0;
  for (i=0; i<8; i++)
    if (m_Params.d_Mac[i])
  {
    p_Hw->Exchange(C3G(uint16_t(m_Params.d_Mac[i]),uint16_t(SCODE1),0x01<<i,0,INTMAPTGT,TGTALL),NULL,sizeof(cmd3G),0,0);
    p_Hw->Exchange(C3G(int32_t(SECTOR_INFO*SECTOR_SIZE32),uint16_t(4),INTFLASHRD,0x01<<i),(char*)&bios,sizeof(cmd3G),4,0);
    *Target|=(0x01<<i);
    if (bios) *Powered|=(0x01<<i);
    else if (Err==ERR_OK)
      Err=ERR_HARDWARE;
    if (((bios^BIOSTGT)&0xFFFFFF00)==0) *Bios|=(0x01<<i);
    else if (Err==ERR_OK)
      Err=WARNING_BIOS;
  }
  mutex_unlock(h_Mutex);
  return Err;
}

/************************************************************/
/*** CrtCtrl32 front end functions (0)                    ***/
/************************************************************/

int32_t CrtCtrl32::Branch(int32_t Target)
{
  return CmdListAdd(C3G(int32_t(Target),CMD3G_BRANCH,(ld_ListMode==5)?c_tgtmsk:0));
}

/*
int32_t CrtCtrl32::Call(int32_t Target)
{
  return CmdListAdd(C3G(int32_t(Target),CMD3G_CALL,0));
}

int32_t CrtCtrl32::CallEnd()
{
  return CmdListAdd(C3G(CMD3G_CALL_END,0));
}
*/

int32_t CrtCtrl32::CharDef(int32_t Ascii)
{
  if (!ld_FontBase) return ERR_DATA;
  CmdListAdd(C3G(CMD3G_CHAR_END,0)); // return of previous character (if any)
  *(cmd3G*)&p_File[ld_FontBase+Ascii*sizeof(cmd3G)]=C3G(int(ld_Size),CMD3G_BRANCH,0).cmd;
  return ERR_OK;
}

int32_t CrtCtrl32::DoLoop()
{
  if (m_Stack.pPrev->d_Type!=T_DO) return ERR_DATA;
  CmdListAdd(C3G(int(m_Stack.pPrev->d_Address),CMD3G_DOLOOP,(ld_ListMode==5)?c_tgtmsk:0));
  m_Stack.Pop();
  return ERR_OK;
}

int32_t CrtCtrl32::EraseFromFlash(char* Name)
{
  Abort(); // abort current process (avoid file access by running job)
  int32_t sector=FileSearch(Name);
  if (sector<0) return ERR_OK; // file not found
  Flash_WR((SECTOR_FILES+sector)*SECTOR_SIZE32,NULL,0); // erase sector containing fileheader
  FAT[sector].b_Valid=false;
  return ERR_OK;
}

void CrtCtrl32::FileAbort()
{
  FileName[0]=0;
  DELETE_(p_File);
  ld_Size=0;
}


int32_t CrtCtrl32::FileAppendCommand(cmd3G* p)
{
  if ((ld_Size+sizeof(cmd3G))%SECTOR_SIZE32<=sizeof(cmd3G))
    p_File=(char*)(realloc(p_File,SECTOR_SIZE32*(1+(ld_Size+sizeof(cmd3G))/SECTOR_SIZE32)));
  memcpy(&p_File[ld_Size],p,sizeof(cmd3G));
  ld_Size+=sizeof(cmd3G);
  return ERR_OK;
}

int32_t CrtCtrl32::FileClose(int32_t sector)
{
  Abort(); // must be called before to force systems idle state
  EraseFromFlash(FileName);
	FileAppendCommand(C3G(CMD3G_EOF,0)); // add end of file command
  if (strcmp(FileName,"bootstart")==0)
  {
    if (ld_Size>=0x40000) { FileAbort(); return ERR_FLASH; };
    sector=0; // reserved sector for bootstart
  }
  else if (sector==-1) sector=FindFirstFreeSector(ld_Size);  
	*(uint32_t*)&p_File[0]=0xFFFFFFFF; // compile fileheader
  *(uint32_t*)&p_File[4]=0x0000FFFF;
  *(int32_t*)(&p_File[8])=ld_Size;
  strcpy(&p_File[12],FileName);
  Flash_WR((SECTOR_FILES+sector)*SECTOR_SIZE32,p_File,ld_Size); // store file data
  strcpy(FAT[sector].Name,FileName);
  FAT[sector].ld_Size=ld_Size;
  FAT[sector].b_Valid=true;
  FileAbort();
  return ERR_OK;
}

int32_t CrtCtrl32::FileCloseHost()
{
  FILE* pF=fopen(FileName,"w+b");
  if (pF==NULL) return ERR_DATA;
  FileAppendCommand(C3G(CMD3G_EOF,0)); // add end of file command
  fwrite(&(p_File[256]),1,ld_Size-256,pF);
  fclose(pF);
	FileAbort();
	ld_ListMode=0;
  return ERR_OK;
}

int32_t CrtCtrl32::FileDownload(char* FileName, char* DestFile)
{
  Abort(); // abort current process (avoid file access by running job)
  int32_t Sector=FileSearch(FileName);
  if (Sector<0) return ERR_DATA; // file not found
  FILE* pF=fopen(DestFile,"w+b");
  if (pF==NULL) return ERR_DATA;
  char* data=(char*)malloc(FAT[Sector].ld_Size);
  Flash_RD((SECTOR_FILES+Sector)*SECTOR_SIZE32,data,FAT[Sector].ld_Size);
  fwrite(&data[256],1,FAT[Sector].ld_Size-256,pF);
  fclose(pF);
  return ERR_OK;
}

int32_t CrtCtrl32::FileFetch(char* Name)
{
  if (strlen(Name)==0) return ERR_DATA;
  return SectorFetch(FileSearch(Name));
}

int32_t CrtCtrl32::FileOpen(char* Name)
{
//  Abort();
  if (strlen(Name)>=256-12) return ERR_DATA; // filename too int32_t
  strcpy(FileName,Name);
  DELETE_(p_File);
  p_File=(char*)(malloc(SECTOR_SIZE32));
  ld_Size=256;
  ld_FontBase=0;
  ld_ListMode=FILE_LOAD;
  while (m_Stack.Items()) m_Stack.Pop();
  return ERR_OK;
}

int32_t CrtCtrl32::FileSearch(char* Name)
{
  for (int32_t i=0; i<FILES; i++)
    if (FAT[i].b_Valid && strcmp(FAT[i].Name,Name)==0) return i; // file found
  return -1;
}

int32_t CrtCtrl32::FileUpload(char* SrcFile, char* FileName, int32_t Sector)
{
  Abort(); // abort current process (avoid file access by running job)
  int32_t Err=FileOpen(FileName);
  if (Err==ERR_OK)
  {
    FILE* pF=fopen(SrcFile,"r+b");
    if (pF==NULL) return ERR_DATA;
	  ld_Size = 256+fread(&p_File[256],1,SECTOR_SIZE32-256,pF);
	  while (!feof(pF))
    {
      p_File=(char*)realloc(p_File,ld_Size+SECTOR_SIZE32);
	    ld_Size += fread(&p_File[ld_Size],1,SECTOR_SIZE32,pF);
    }
    fclose(pF);
	  Err=FileClose(Sector);
  }
  return Err;
}

int32_t CrtCtrl32::FindFirstFreeSector(int32_t Size)
{
  int32_t sector=1; // sector 0 reserved for bootstart	
  int32_t j,FreeSize;
  while (sector<FILES)
  {
    for (j=sector, FreeSize=0; 
         j<FILES && FreeSize<Size && !FAT[j].b_Valid;
         j++, FreeSize+=SECTOR_SIZE32);
    if (j<FILES && FreeSize>=Size) return sector;
    sector=j+(FAT[j].ld_Size+SECTOR_SIZE32-1)/SECTOR_SIZE32;
  }
  return -1;
}

void CrtCtrl32::Flash_RD(int32_t Address, char* c, int32_t len)
{
  mutex_lock(h_Mutex);
  for (int32_t i=std::min(len,512); i; i=std::min(len,512))
  {
    p_Hw->Exchange(C3G(Address,uint16_t(i),INTFLASHRD,0),c,sizeof(cmd3G),i,0);
    Address+=i;
    c+=i;
    len-=i;
  }
  mutex_unlock(h_Mutex);
}

void CrtCtrl32::Flash_WR(int32_t Address, char* c, int32_t len)
{
  mutex_lock(h_Mutex);
  int32_t i,err;
  do
  {
    i=std::min(len,512);
    p_Hw->Exchange(C3G(Address,uint16_t(i),INTFLASHPP2,0),(char*)&err,sizeof(cmd3G),4,0);
    if (i)
    {
      p_Hw->Exchange(c,(char*)&err,i,4,0);
      Address+=i;
      c+=i;
      len-=i;
    }
  } while (len);
  mutex_unlock(h_Mutex);
}

int32_t CrtCtrl32::FontDef(char* Name)
{
  int32_t i;

// font name is ignored, currently only 1 font can be used !
  ld_FontBase=ld_Size+8;
  CmdListAdd(C3G(ld_FontBase,CMD3G_FONTHDR,0));
  for (i=0; i<256; i++) CmdListAdd(C3G(CMD3G_CHAR_END,0));
  return ERR_OK;
}

int32_t CrtCtrl32::FontDefEnd()
{
  if (!ld_FontBase) return ERR_DATA;
  CmdListAdd(C3G(CMD3G_CHAR_END,0)); // return of last character (if any)
  *(int*)(&p_File[ld_FontBase-sizeof(cmd3G)])=ld_Size; // complete font header
  return ERR_OK;
}

int32_t CrtCtrl32::FormatFlash()
{
  Abort(); // abort current process (avoid file access by running job)
  for (int32_t i=0;i<FILES;i++) if (FAT[i].b_Valid)
	{
    Flash_WR((SECTOR_FILES+i)*SECTOR_SIZE32,NULL,0);
    FAT[i].b_Valid=false;
	}
  return ERR_OK;
}

int32_t CrtCtrl32::GetCanLink(int32_t Address, int32_t* Value)
{
  struct cmd3G cmd={0,16,0,0,INTCANREAD,0};
  uint8_t reply[16];
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)&cmd,(char*)reply,sizeof(cmd3G),16,0); // command header
  mutex_unlock(h_Mutex);
  *Value=reply[Address&0x0F];
  return ERR_OK;
}

FileEntry* CrtCtrl32::GetFAT()
{
  return FAT;
} 

int32_t CrtCtrl32::GetFlashFirstFileEntry(char* Name, int32_t* Size)
{
  ld_FileEntry=0;
  return GetFlashNextFileEntry(Name,Size);
}

int32_t CrtCtrl32::GetFlashMemorySizes(int32_t* Total, int32_t* Allocated)
{
  int32_t i,filesectors;
  for (*Total=0,*Allocated=0,i=0; i<FILES;)
  {
  	if (FAT[i].b_Valid)
	  {
	    filesectors=(FAT[i].ld_Size+SECTOR_SIZE32-1)/SECTOR_SIZE32;
	    *Total+=filesectors;
	    *Allocated+=filesectors;
	    i+=filesectors;
	  }
	  else
	  {
      *Total+=1;
	    i+=1;
	  }
  }
  *Total=*Total*SECTOR_SIZE32;
  *Allocated=*Allocated*SECTOR_SIZE32;
  return ERR_OK;
}

int32_t CrtCtrl32::GetFlashNextFileEntry(char* Name, int32_t* Size)
{
  while (ld_FileEntry<FILES && !FAT[ld_FileEntry].b_Valid) ld_FileEntry++;
  if (ld_FileEntry<FILES)
  {
    strcpy(Name,FAT[ld_FileEntry].Name);
    *Size=FAT[ld_FileEntry].ld_Size;
  }
  else
  {
    Name[0]=0;
    *Size=0;
  }
  ld_FileEntry++;
  return ERR_OK;
}

char* CrtCtrl32::GetID()
{
  return m_Params.s_ID;
}

int32_t CrtCtrl32::GetIP(char* Mac, char* IP)
{
  if (strlen(Mac)<17) return ERR_DATA;
  char d[4];
  mutex_lock(h_Mutex);
  struct cmd3G cmd={0,0,0,0,INTGETIP,0};
  uint16_t c;
  for (int32_t i=0; i<6; i++, Mac+=3) 
  {
    sscanf(Mac,"%02hx",&c);
    ((uint8_t*)&cmd.prm_X)[i]=(uint8_t)c;
  }
  cmd.opcode=INTGETIP;
  p_Hw->Exchange((char*)&cmd,d,sizeof(cmd3G),4,0); // command header
  sprintf(IP,"%d,%d,%d,%d",d[0]&0xff,d[1]&0xff,d[2]&0xff,d[3]&0xff);
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

void* CrtCtrl32::GetParamset()
{
  return &m_Params;
}

int32_t CrtCtrl32::GetStatus(int32_t* pCounter)
{
  mutex_lock(h_Mutex);
  int32_t err=ERR_BUSY;
  if (m_ListStatus&IOABORT) err=ERR_INTERLOCK;
  else if (p_Cmd==NULL && m_CmdList.Items()==0 && m_ListStatus&SYSSTALL) err=ERR_OK;
  if (pCounter)
  {
    *pCounter=m_CmdList.CountItems()*USB3GSIZE*sizeof(cmd3G);
    *pCounter+=(err==ERR_OK)?0:15*USB3GSIZE*sizeof(cmd3G);
  }
  mutex_unlock(h_Mutex);
  return err;
}
int32_t CrtCtrl32::If_Else()
{
  Branch(ADDR_TBD);
  if (m_Stack.pPrev->d_Type!=T_IF && m_Stack.pPrev->d_Type!=T_ELSEIF) return ERR_DATA;
  *(int*)(&p_File[m_Stack.pPrev->d_Address])=ld_Size;
  m_Stack.pPrev->d_Address=ld_Size-sizeof(cmd3G);
  return ERR_OK;
}

int32_t CrtCtrl32::If_ElseIO(int32_t Value, int32_t Mask)
{
  Branch(ADDR_TBD);
  if (m_Stack.pPrev->d_Type!=T_IF && m_Stack.pPrev->d_Type!=T_ELSEIF) return ERR_DATA;
  *(int*)(&p_File[m_Stack.pPrev->d_Address])=ld_Size;
  m_Stack.pPrev->d_Address=ld_Size-sizeof(cmd3G);
  m_Stack.Push(new CStack(ld_Size+sizeof(cmd3G),T_ELSEIF));
  TestIO(Value, Mask, 1);
  Branch(ADDR_TBD);
  return ERR_OK;
}

int32_t CrtCtrl32::If_End()
{
  while (m_Stack.pPrev->d_Type==T_ELSEIF)
  {
    *(int*)(&p_File[m_Stack.pPrev->d_Address])=ld_Size;
    m_Stack.Pop();
  }
  if (m_Stack.pPrev->d_Type!=T_IF) return ERR_DATA;
  *(int*)(&p_File[m_Stack.pPrev->d_Address])=ld_Size;
  m_Stack.Pop();
  return ERR_OK;
}

int32_t CrtCtrl32::If_IO(int32_t Value, int32_t Mask)
{
  m_Stack.Push(new CStack(ld_Size+2*sizeof(cmd3G),T_IF));
  TestIO(Value, Mask, 1);
  Branch(ADDR_TBD);
  return ERR_OK; 
}

int32_t CrtCtrl32::ListClose()
{
  if (ld_ListMode==3) return FileClose();

  int32_t Err=ERR_OK;
  mutex_lock(h_Mutex);
  if (ld_ListMode==1 || ld_ListMode==4 || ld_ListMode==5)
  {
    if (ld_Size<0x20000)
    {
      int i=256/sizeof(cmd3G);
      int imax=ld_Size/sizeof(cmd3G);
      cmd3G* command=(cmd3G*)p_File;
      p_File=NULL;
      CmdListAdd(C3G(int(ld_Size-256),CMD3G_CCODE_BEGIN,(ld_ListMode==5)?c_tgtmsk:0));
      for (;i<imax;i++) CmdListAdd(&command[i]);
      CmdListAdd(C3G(CMD3G_CCODE_END,(ld_ListMode==5)?c_tgtmsk:0));
      p_File=(char*)command;
    }
    else Err=ERR_DATA;
    FileAbort();
  }
  struct cmd3G cmd={0,0,0,0,CMD3G_NOP,0};
  while (p_Cmd) CmdListAdd(&cmd);
  ld_ListMode=0;
  mutex_unlock(h_Mutex);
  return Err;
}

int32_t CrtCtrl32::ListOpen(int32_t Mode)
{
  char s[12]="";
  switch (Mode)
  {
  case 1: FileOpen(s); break;
  case 2: break;
  case 3: strcpy(s,"bootstart"); FileOpen(s); break;
  case 4: FileOpen(s); break;
  case 5: FileOpen(s); break;
  default: return ERR_DATA;
  }
  ld_ListMode=Mode;
  return ERR_OK;
}

int32_t CrtCtrl32::LoadDirectory()
{
  char data[256];
  for (int32_t i=0; i<FILES; i++)
  {
    Flash_RD((SECTOR_FILES+i)*SECTOR_SIZE32,data,256);
    FAT[i].b_Valid= *(uint32_t*)&data[0]==0xFFFFFFFF && *(uint32_t*)&data[4]==0x0000FFFF;
	  if (FAT[i].b_Valid)
	  {
	    FAT[i].ld_Size=*(int32_t*)(&data[8]);
	    strncpy(FAT[i].Name,&data[12],256-12);
	  }
  }
  return ERR_OK;
}

int32_t CrtCtrl32::LoadProfile(const char* FileName, int32_t Include)
{
  int32_t i;
  int16_t idx;
  void* data;
  ProfileHdr Hdr;
  FILE* pF;
  int32_t Config = Include&0x01;
  int32_t TgtMap = Include&0x02;
  int32_t Calibration = Include&0x04;
  int32_t Files = Include&0x08;

  pF=fopen(FileName,"r+b");
  if (pF==NULL) return ERR_DATA;
  for (idx=1; fread(&Hdr,1,sizeof(ProfileHdr),pF); idx++)
  {
    if (Hdr.ID!=RAWSECTOR) { fclose(pF); free(data); return ERR_DATA; }
    data=malloc(Hdr.DataSize);
    fread(data,1,Hdr.DataSize,pF);
    for (i=0; i<Hdr.DataSize; i++) Hdr.CheckSum-=((char*)data)[i];
    if (Hdr.CheckSum) { fclose(pF); free(data); return ERR_DATA; }
    if (Hdr.Sector == SECTOR_SETTINGS) // load configuration
    {
      if (TgtMap) // overwrite target mapping
        for (i=0; i<8; i++) m_Params.d_Mac[i]=((ParamSet32*)data)->d_Mac[i];
      else // preserve target mapping
        for (i=0; i<8; i++) ((ParamSet32*)data)->d_Mac[i]=m_Params.d_Mac[i];
      if (Config)
      { 
        ((ParamSet32*)data)->d_HwID=m_Params.d_HwID; // preserve HwID
        ((ParamSet32*)data)->d_SerialNr=m_Params.d_SerialNr; // preserve serial nr.
        memcpy(&m_Params,data,std::min((int32_t)sizeof(ParamSet32),Hdr.DataSize));
      }
    }
    if ((Hdr.Sector == SECTOR_SETTINGS && (TgtMap|Config)) ||
        (Hdr.Sector == SECTOR_CALIBRATION && Calibration) ||
        (Hdr.Sector >= SECTOR_FILES && Files && (Hdr.DataSize || FAT[Hdr.Sector-SECTOR_FILES].b_Valid)))
      Flash_WR(Hdr.Sector*SECTOR_SIZE32,(char*)data,Hdr.DataSize);
    free(data);
  }
  fclose(pF);
  return ERR_OK;
}

int32_t CrtCtrl32::OpenCanLink(int32_t Baudrate)
{
  char s[16];
  CmdListAdd(C3G(int32_t(Baudrate),CMD3G_CAN_INIT,0));
  strcpy(s,"8200"); CmdListAddHex(s);
  CmdListAdd(C3G(uint16_t(0),uint16_t(0),uint16_t(0),CMD3G_CAN_SEND,0));
  CmdListAdd(C3G(int32_t(1000000),CMD3G_SLEEP,0));
  strcpy(s,"0100"); CmdListAddHex(s);
  CmdListAdd(C3G(uint16_t(0),uint16_t(0),uint16_t(0),CMD3G_CAN_SEND,0));
  return ERR_OK;
}

int32_t CrtCtrl32::ReadFlash(char* pPa)
{
  Flash_RD(SECTOR_SETTINGS*SECTOR_SIZE32, (char*)pPa,sizeof(ParamSet32));
  if (((ParamSet32*)pPa)->d_HwID!=10)
  {
    ZeroMemory(pPa,sizeof(ParamSet32));
    ((ParamSet32*)pPa)->d_VersionID=CU_SWID;
    ((ParamSet32*)pPa)->d_HwID=10;
    for (int32_t i=0; i<8; i++)
    {
      ((ParamSet32*)pPa)->lf_FieldSize[i]=100.;
      ((ParamSet32*)pPa)->hd_DragError[i]=80;
      ((ParamSet32*)pPa)->lf_FieldSizeZ[i]=100.;
    }
  }
  return ERR_OK;
}

int32_t CrtCtrl32::ReadFlashBootScript(char* c, int32_t size)
{
  Flash_RD(SECTOR_BOOT*SECTOR_SIZE32+(SECTOR_SIZE32>>1), c, size);
  int32_t l;
  for (l=0; l<(size-1) && c[l]!=0x00 && uint8_t(c[l])!=0xff; l++);
  c[l]=0x00;
  return ERR_OK;
}

void CrtCtrl32::ResetStatus()
{
  FileAbort();
  ld_ListMode=0;
}

int32_t CrtCtrl32::ScanCanLink(int32_t Address, int32_t Node, int32_t Index, int32_t SubIndex)
{
  CmdListAdd(C3G(uint16_t(Node),uint16_t(Index),int8_t(SubIndex),int8_t(Address),CMD3G_CAN_SCAN,0));
  return ERR_OK;
}

int32_t CrtCtrl32::SectorFetch(int32_t Sector)
{
	if (Sector<0 || Sector >= FILES) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(ld_Size+8),uint16_t(SECTOR_FILES+Sector),CMD3G_IDXFETCH,0));
}

int32_t CrtCtrl32::SetCanLink(int32_t Node, int32_t Index, int32_t SubIndex, char* Data)
{
  CmdListAddHex(Data);
  return CmdListAdd(C3G(uint16_t(Node),uint16_t(Index),int8_t(SubIndex),int8_t(0),CMD3G_CAN_SEND,0)); // command header
}

int32_t CrtCtrl32::SetLoop(int32_t LoopCtr)
{
  CmdListAdd(C3G(uint16_t(LoopCtr),CMD3G_SETLOOP,(ld_ListMode==5)?c_tgtmsk:0));
  m_Stack.Push(new CStack(ld_Size,T_DO));
  return ERR_OK;
}

int32_t CrtCtrl32::SetVarBlock(int32_t i, char data)
{
  return CmdListAdd(C3G(uint16_t(i),uint16_t(data),CMD3G_VAR_SET,0));
}

int32_t CrtCtrl32::StoreFlash()
{
  Flash_WR(SECTOR_SETTINGS*SECTOR_SIZE32,(char*)&m_Params,sizeof(ParamSet32));
  return ERR_OK;
}

int32_t CrtCtrl32::StoreFlashBootScript(char* c)
{
  int32_t l;
  for (l=0; l<65535 && c[l]!=0x00; l++);
  Flash_WR(SECTOR_BOOT*SECTOR_SIZE32+(SECTOR_SIZE32>>1), c, l+1);
  return ERR_OK;
}

int32_t CrtCtrl32::StoreProfile(const char* FileName, int32_t Include)
{
  int32_t i;
  int16_t idx;
  void* data;
  ProfileHdr Hdr;
  FILE* pF;
  int32_t Config = Include&0x01;
  int32_t TgtMap = Include&0x02;
  int32_t Calibration = Include&0x04;
  int32_t Files = Include&0x08;

  pF=fopen(FileName,"w+b");
  if (pF==NULL) return ERR_DATA;

  Hdr.ID=RAWSECTOR;
  if (Config || TgtMap)
  {
    Hdr.Sector=SECTOR_SETTINGS;
    Hdr.DataSize=sizeof(ParamSet32);
    for (Hdr.CheckSum=0, i=0; i<Hdr.DataSize; i++) Hdr.CheckSum+=((char*)&m_Params)[i];
    fwrite(&Hdr,1,sizeof(ProfileHdr),pF);
    fwrite(&m_Params,1,Hdr.DataSize,pF);
  }
  if (Calibration)
  {
    Hdr.Sector=SECTOR_CALIBRATION;
    Hdr.DataSize=CALSIZE*8;
    data=malloc(Hdr.DataSize);
    Flash_RD(SECTOR_CALIBRATION*SECTOR_SIZE32,(char*)data,Hdr.DataSize);
    for (Hdr.CheckSum=0, i=0; i<Hdr.DataSize; i++) Hdr.CheckSum+=((char*)data)[i];
    fwrite(&Hdr,1,sizeof(ProfileHdr),pF);
    fwrite(data,1,Hdr.DataSize,pF);
    free(data);
  }
  if (Files)
  {
    for (idx=0; idx<FILES; )
    {
      Hdr.Sector=SECTOR_FILES+idx;
      if (FAT[idx].ld_Size)
      {
        Hdr.DataSize=FAT[idx].ld_Size;
        data=malloc(Hdr.DataSize);
        Flash_RD((SECTOR_FILES+idx)*SECTOR_SIZE32,(char*)data,Hdr.DataSize);
        for (Hdr.CheckSum=0, i=0; i<Hdr.DataSize; i++) Hdr.CheckSum+=((char*)data)[i];
        fwrite(&Hdr,1,sizeof(ProfileHdr),pF);
        fwrite(data,1,Hdr.DataSize,pF);
        idx+=((SECTOR_SIZE32-1+FAT[idx].ld_Size)/SECTOR_SIZE32);
        free(data);
      }
      else
      {
        Hdr.DataSize=0;
        Hdr.CheckSum=0;
        fwrite(&Hdr,1,sizeof(ProfileHdr),pF);
        idx++;
      }
    }
  }
  fclose(pF);
  return ERR_OK;
}

int32_t CrtCtrl32::SystemUDPsend(char* IP, int16_t port, char* data)
{
  int32_t ip[4];
  uint8_t d[256]; 
  
  if (sscanf(IP,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3])!=4) return ERR_DATA;
  int32_t Err=HexToBString(d,data);
  if (Err==ERR_OK)
  {
    mutex_lock(h_Mutex);
    CmdInt(C3G(uint16_t(ip[0]|(ip[1]<<8)),uint16_t(ip[2]|(ip[3]<<8)),int8_t(port&0xff),int8_t(port>>8),INTUDPSEND,0));
    p_Hw->Exchange((char*)d,NULL,1+d[0],0,0);
    mutex_unlock(h_Mutex);
  }
  return Err;
}

int32_t CrtCtrl32::UARTopen(int32_t baudrate, char parity, char stopbits)
{
  return CmdInt(C3G(int32_t(baudrate),parity,stopbits,INTUARTOPEN,0));
}

int32_t CrtCtrl32::UARTread(int32_t* bytes, char* data)
{
  struct cmd3G cmd={(uint16_t)*bytes,0,0,0,INTUARTREAD,0};
  char reply[UARTFIFOSIZE+2];
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)&cmd,reply,sizeof(cmd3G),*bytes+2,0); // command header
  *bytes=*(uint16_t*)reply;
  memcpy(data,&reply[2],*bytes);
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::UARTwrite(int32_t bytes, char* data)
{
  struct cmd3G cmd={(uint16_t)bytes,0,0,0,INTUARTWRITE,0};
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)&cmd,NULL,sizeof(cmd3G),0,0); // command header
  p_Hw->Exchange(data,NULL,bytes,0,0);
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::UDPsend(char* IP, short port, char* data)
{
  int32_t ip[4];
  if (sscanf(IP,"%d.%d.%d.%d",&ip[0],&ip[1],&ip[2],&ip[3])!=4) return ERR_DATA;
  CmdListAddHex(data);
  CmdListAdd(C3G(ip[0]|(ip[1]<<8),ip[2]|(ip[3]<<8),port&0xff,port>>8,CMD3G_UDPSEND,0)); // command header
  return ERR_OK;
}

int32_t CrtCtrl32::VarBlockFetch(int32_t Start, int32_t Size, char* FontName)
{
  int32_t i;
  for (i=0; i<Size; i++) CmdListAdd(C3G(uint16_t(Start+i),CMD3G_VAR_CALL,0));
  return ERR_OK;
}

int32_t CrtCtrl32::WaitCanLink(int32_t Address, int32_t Value, int32_t Mask)
{
  WaitIdle(c_tgtidx);
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),char(0),char(Address),CMD3G_CAN_WAIT,0));
}

int32_t CrtCtrl32::WaitIO(int32_t Value, int32_t Mask)
{
  WaitIO(Value,Mask,1);
  return ERR_OK;
}

int32_t CrtCtrl32::While_IO(int32_t Value, int32_t Mask)
{
  m_Stack.Push(new CStack(ld_Size,T_WHILE));
  TestIO(Value, Mask, 1);
  Branch(ADDR_TBD);
  return ERR_OK;
}

int32_t CrtCtrl32::While_End()
{
  if (m_Stack.pPrev->d_Type!=T_WHILE) return ERR_DATA;
  Branch(m_Stack.pPrev->d_Address);
  if (ld_ListMode==5) *(int32_t*)(&p_File[m_Stack.pPrev->d_Address+sizeof(cmd3G)])=ld_Size;
  else *(int32_t*)(&p_File[m_Stack.pPrev->d_Address+2*sizeof(cmd3G)])=ld_Size;
  m_Stack.Pop();
  return ERR_OK;
}

/************************************************************/
/*** CrtCtrl32 single target functions (c_tgtidx)         ***/
/************************************************************/

int32_t CrtCtrl32::AddCalibrationData(const char* FileName, bool bReset)
{
  char s[]="stdio";
  int32_t Err=ERR_OK;
  mutex_lock(h_Mutex);
  if (bReset) p_Hw->Exchange(C3G(uint16_t(c_tgtidx),INTRESETCAL,0),(char*)&Err,sizeof(cmd3G),4,0);
  if (FileName[0]!=0)
  {
    Err=FileUpload((char*)(FileName),s,SECTOR_STDIO-SECTOR_FILES);
    if (Err==ERR_OK) p_Hw->Exchange(C3G(uint16_t(c_tgtidx),INTADDCALDATA,0),(char*)&Err,sizeof(cmd3G),4,0);
  }
  if (Err==ERR_OK) Flash_RD(SECTOR_CALIBRATION*SECTOR_SIZE32+c_tgtidx*SIZEOF_CALTBL,(char*)(hd_Cal),SIZEOF_CALTBL);
  mutex_unlock(h_Mutex);
  return Err;
}

void* CrtCtrl32::GetCalibration()
{
  Flash_RD(SECTOR_CALIBRATION*SECTOR_SIZE32+c_tgtidx*SIZEOF_CALTBL,(char*)(hd_Cal),SIZEOF_CALTBL);
  return hd_Cal;
}

int32_t CrtCtrl32::LoadCalibrationFile(const char* FileName)
{
  char s[]="stdio";
  mutex_lock(h_Mutex);
  int32_t Err=FileUpload((char*)(FileName),s,SECTOR_STDIO-SECTOR_FILES); 
  if (Err==ERR_OK) p_Hw->Exchange(C3G(uint16_t(c_tgtidx),INTLOADCALFILE,0),(char*)&Err,sizeof(cmd3G),4,0);
  if (Err==ERR_OK) Flash_RD(SECTOR_CALIBRATION*SECTOR_SIZE32+c_tgtidx*SIZEOF_CALTBL,(char*)(hd_Cal),SIZEOF_CALTBL);
  mutex_unlock(h_Mutex);
  return Err;
}

int32_t CrtCtrl32::StoreCalibrationFile(const char* FileName)
{
  mutex_lock(h_Mutex);
  FILE* pF=fopen(FileName,"w+b");
  if (pF)
  {
    int32_t Size;
    p_Hw->Exchange(C3G(uint16_t(c_tgtidx),INTSTORECALFILE,0),(char*)&Size,sizeof(cmd3G),4,0);
    char* data=(char*)malloc(Size);
    Flash_RD(SECTOR_STDIO*SECTOR_SIZE32,data,Size);
    fwrite(&data[256],1,Size-256,pF);
    fclose(pF);
  }
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::TestIO(int32_t Value, int32_t Mask, char Equal)
{
  if (ld_ListMode==5) return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),Equal,0,CMD3G_TESTIO,c_tgtmsk));
  WaitIdle(c_tgtidx);
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),Equal,0,INTTESTIO,0x01<<c_tgtidx));
}

int32_t CrtCtrl32::WaitIO(int32_t Value, int32_t Mask, char Equal)
{
  if (ld_ListMode==5) return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),Equal,0,CMD3G_WAITIO,c_tgtmsk));
  WaitIdle(c_tgtidx);
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),Equal,0,INTWAITIO,0x01<<c_tgtidx));
}

int32_t CrtCtrl32::WaitIdle(int32_t tgt)
{
  return CmdListAdd(C3G(INTWAITIDLE,0x01<<tgt));
}

int32_t CrtCtrl32::WaitStall(int32_t tgt)
{
  return CmdListAdd(C3G(INTWAITSTALL,0x01<<tgt));
}

/************************************************************/
/*** CrtCtrl32 broadcast target functions  (c_tgtmsk)     ***/
/************************************************************/

int32_t CrtCtrl32::ArcMoveXY(ClfPoint pt_Co, double BF) 
{
  if (fabs(BF)<0.000001) return MoveXY(pt_Co);
  CmdListAdd(C3G(int(pt_Co.x*1000.),int(pt_Co.y*1000.),CMD3G_ARCMOVE,c_tgtmsk));
  CmdListAdd(C3G(float(BF),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::ArcXY(ClfPoint pt_Co, double BF) 
{
  if (fabs(BF)<0.000001) return LineXY(pt_Co);
  CmdListAdd(C3G(int(pt_Co.x*1000.),int(pt_Co.y*1000.),CMD3G_ARCLINE,c_tgtmsk));
  CmdListAdd(C3G(float(BF),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::Burst(int Time) 
{
  return CmdListAdd(C3G(int(Time),CMD3G_BURST,c_tgtmsk));
}

int32_t CrtCtrl32::BurstEx(int32_t Time, int32_t Time2, double PWM2) 
{
  if (PWM2) return CmdListAdd(C3G(uint16_t(Time),uint16_t(Time2),uint16_t(PWM2*80.),CMD3G_BURST,c_tgtmsk));
  else return CmdListAdd(C3G(int(Time),CMD3G_BURST,c_tgtmsk));
}

int32_t CrtCtrl32::Circle(ClfPoint pt_Co, double Angle, bool Laser) 
{
  CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_CIRCLE,c_tgtmsk));
  CmdListAdd(C3G(float(Angle),int16_t(Laser),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::IncrementCounter() 
{
  return CmdListAdd(C3G(uint16_t(0xffff),uint16_t(0x0001),CMD3G_SETCNTR,c_tgtmsk));
}

int32_t CrtCtrl32::JumpXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_JUMPTO,c_tgtmsk));
}

int32_t CrtCtrl32::JumpXYZ(ClfPoint pt_Co, double Z)
{
  CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_JUMPTO3D,c_tgtmsk));
  CmdListAdd(C3G(int32_t(Z*1000.),int32_t(0),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK; 
}

int32_t CrtCtrl32::LaserLink(int32_t Command, int32_t Value) 
{
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Command),CMD3G_EXCHLLINK,c_tgtmsk));
}

int32_t CrtCtrl32::LineXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_LINETO,c_tgtmsk));
}

int32_t CrtCtrl32::LineXYZ(ClfPoint pt_Co, double Z) 
{
  CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_LINETO3D,c_tgtmsk));
  CmdListAdd(C3G(int32_t(Z*1000.),int32_t(0),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::MoveXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_MOVETO,c_tgtmsk));
}

int32_t CrtCtrl32::MoveXYZ(ClfPoint pt_Co, double Z)
{
  CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_MOVETO3D,c_tgtmsk));
  CmdListAdd(C3G(int(Z*1000.),int(0),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::PulseXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_PULSTO,c_tgtmsk));
}

int32_t CrtCtrl32::PulseXYZ(ClfPoint pt_Co, double Z) 
{
  return PulseXY(pt_Co);
}

int32_t CrtCtrl32::Print(char* data)
{
  CmdListAddString(data,c_tgtmsk);
  CmdListAdd(C3G(ld_Size,CMD3G_PRINT,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::ResetCounter() 
{
  return CmdListAdd(C3G(CMD3G_SETCNTR,c_tgtmsk));
}

int32_t CrtCtrl32::ResetResolver(int32_t Nr) 
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  return CmdListAdd(C3G(0,0,int16_t(Nr),0,CMD3G_OTFSET,c_tgtmsk));
}

int32_t CrtCtrl32::SetAij(double A11, double A12, double A21, double A22) 
{
  CmdListAdd(C3G(float(A11),1,1,CMD3G_SETAIJ,c_tgtmsk));
  CmdListAdd(C3G(float(A12),1,2,CMD3G_SETAIJ,c_tgtmsk));
  CmdListAdd(C3G(float(A21),2,1,CMD3G_SETAIJ,c_tgtmsk));
  CmdListAdd(C3G(float(A22),2,2,CMD3G_SETAIJ,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::SetAnalog(int32_t Value, int32_t Mask) 
{
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),CMD3G_SETANA,c_tgtmsk));
}

int32_t CrtCtrl32::SetBulge(double Factor) 
{
  return CmdListAdd(C3G(float(Factor),CMD3G_SETBULGE,c_tgtmsk));
}

int32_t CrtCtrl32::SetCfgIO(int32_t Nr, int32_t Value) 
{
  if (Nr<1 || Nr>17) return ERR_DATA;
  m_Params.d_FctIO1[(Nr-1)*8+c_tgtidx]=uint16_t(Value);
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Nr),(int8_t)(Value>>16),0,CMD3G_CFG_IO,c_tgtmsk));
}

int32_t CrtCtrl32::SetCounter(int32_t Value) 
{
  return CmdListAdd(C3G(uint16_t(0x0000),uint16_t(Value),CMD3G_SETCNTR,c_tgtmsk));
}

int32_t CrtCtrl32::SetFieldSize(double Size) 
{
  return CmdListAdd(C3G(int32_t(Size*1000.),int8_t(m_Params.d_FctCH1[c_tgtidx]),int8_t(m_Params.d_FctCH2[c_tgtidx]),CMD3G_SETFS,c_tgtmsk));
}

int32_t CrtCtrl32::SetFont(char* Name)
{
  if (strlen(Name)==0) return ERR_DATA;
  int32_t Sector=FileSearch(Name);
	if (Sector<0 || Sector >= FILES) return ERR_DATA;
  CmdListAdd(C3G(int32_t(ld_Size+8),uint16_t(SECTOR_FILES+Sector),CMD3G_IDXUPLOAD,0));
  return ERR_OK;
}

int32_t CrtCtrl32::SetImageAij(double A11, double A12, double A21, double A22, double A31, double A32) 
{
  CmdListAdd(C3G(float(A11),1,1,CMD3G_SETIMGAIJ,c_tgtmsk));
  CmdListAdd(C3G(float(A12),1,2,CMD3G_SETIMGAIJ,c_tgtmsk));
  CmdListAdd(C3G(float(A21),2,1,CMD3G_SETIMGAIJ,c_tgtmsk));
  CmdListAdd(C3G(float(A22),2,2,CMD3G_SETIMGAIJ,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::SetImageOffsXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_SETIMGOFFS,c_tgtmsk));
}

int32_t CrtCtrl32::SetImageOffsRelXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_SETIMGOFFSR,c_tgtmsk));
}

int32_t CrtCtrl32::SetImageOffsZ(double Z) 
{
  return CmdListAdd(C3G(int32_t(Z*1000.),CMD3G_SETIMGOFFSZ,c_tgtmsk));
}

int32_t CrtCtrl32::SetImageRotation(double Angle) 
{
  return CmdListAdd(C3G(float(Angle),CMD3G_SETIMGROT,c_tgtmsk));
}

int32_t CrtCtrl32::SetIO(int32_t value, int32_t mask) 
{
  return CmdListAdd(C3G(uint16_t(value),uint16_t(mask),CMD3G_SETIO,c_tgtmsk));
}

int32_t CrtCtrl32::SetJumpSpeed(double Speed) 
{
  return CmdListAdd(C3G(float(Speed),CMD3G_JUMPSPEED,c_tgtmsk));
}

int32_t CrtCtrl32::SetLaser(bool OnOff) 
{
  return CmdListAdd(C3G(uint16_t(OnOff),CMD3G_SETLIDLE,c_tgtmsk));
}

int32_t CrtCtrl32::SetLaserFirstPulse(double Time)
{
  return CmdListAdd(C3G(int32_t(Time*80.),CMD3G_SETLASERDELAY,c_tgtmsk));
}

int32_t CrtCtrl32::SetLaserTimes(int32_t GateOnDelay, int32_t GateOffDelay) 
{
  return CmdListAdd(C3G(uint16_t(GateOnDelay),uint16_t(GateOffDelay),CMD3G_SETDELAYS,c_tgtmsk));
}

int32_t CrtCtrl32::SetMinGatePeriod(int32_t Period) 
{
  return CmdListAdd(C3G(uint16_t(Period),CMD3G_SETMINGATE,c_tgtmsk));
}

int32_t CrtCtrl32::SetModeLaser(int32_t Mode) 
{
  return CmdListAdd(C3G(uint16_t(Mode),CMD3G_SM_LASER,c_tgtmsk));
}

int32_t CrtCtrl32::SetOffsXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_SETOFFS,c_tgtmsk));
}

int32_t CrtCtrl32::SetOffsZ(double Z) 
{
  return CmdListAdd(C3G(int32_t(Z*1000.),CMD3G_SETOFFSZ,c_tgtmsk));
}

int32_t CrtCtrl32::SetOTF(int32_t Nr, bool On) 
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  return CmdListAdd(C3G(uint16_t(On),uint16_t(0),int8_t(Nr),0,CMD3G_OTFENABLE,c_tgtmsk));
}

int32_t CrtCtrl32::SetPWM(int Nr, double Period, double Pulse) 
{
  char mult;
  for (mult=0; Period>500 && mult<6; Period/=2., Pulse/=2., mult++);
//  for (mult=0; Period>400 && mult<6; Period/=8., Pulse/=8., mult+=3);
  return CmdListAdd(C3G(uint16_t(Period*80.),uint16_t(Pulse*80.),int8_t(Nr),mult,CMD3G_SETOSC,c_tgtmsk));
} 

int32_t CrtCtrl32::SetResolver(int Nr, double StepSize, double Range) 
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  CmdListAdd(C3G(float(StepSize*1000.),int8_t(Nr),0,CMD3G_OTFSTEP,c_tgtmsk));
  CmdListAdd(C3G(int32_t(Range*1000.),int8_t(Nr),0,CMD3G_OTFRANGE,c_tgtmsk));
  CmdListAdd(C3G(1,0,int8_t(Nr),0,CMD3G_OTFSET,c_tgtmsk));
  CmdListAdd(C3G(1,0,int8_t(Nr),0,CMD3G_OTFENABLE,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::SetResolverPosition(int32_t Nr, double Position) 
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(Position*1000.),int8_t(Nr),0,CMD3G_OTFSET,c_tgtmsk));
}

int32_t CrtCtrl32::SetResolverRange(int Nr, double Range) 
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(Range*1000.),int8_t(Nr),0,CMD3G_OTFRANGE,c_tgtmsk));
}

int32_t CrtCtrl32::SystemSetResolverSpeed(int Nr, double Speed)
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  return CmdInt(C3G(float(Speed),int8_t(Nr),0,INTOTFSPEED,0x01<<c_qryidx));
}

int32_t CrtCtrl32::SetRotation(double Angle) 
{
  return CmdListAdd(C3G(float(Angle),CMD3G_SETROT,c_tgtmsk));
}

int32_t CrtCtrl32::SetSpeed(double Speed) 
{
  return CmdListAdd(C3G(float(Speed),CMD3G_SPEED,c_tgtmsk));
}

int32_t CrtCtrl32::SetTable(int32_t Nr, double Position) 
{
  if (Nr<1 || Nr>3) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(Position*1000.),int8_t(Nr),0,CMD3G_TABLESET,c_tgtmsk));
}

int32_t CrtCtrl32::SetTableDelay(int32_t Nr, int32_t Delay) 
{
  if (Nr<1 || Nr>3) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(Delay),int8_t(Nr),0,CMD3G_TABLEDELAY,c_tgtmsk));
}

int32_t CrtCtrl32::SetTableSnapSize(int32_t Nr, double SnapSize) 
{
  if (Nr<1 || Nr>3) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(SnapSize*1000.),int8_t(Nr),0,CMD3G_TABLESNAP,c_tgtmsk));
}

int32_t CrtCtrl32::SetTableStepSize(int32_t Nr, double StepSize) 
{
  if (Nr<1 || Nr>3) return ERR_DATA;
  return CmdListAdd(C3G(float(StepSize*1000.),int8_t(Nr),0,CMD3G_TABLESSIZE,c_tgtmsk));
}

int32_t CrtCtrl32::SetTableWhileIO(int32_t Value, int32_t Mask) 
{
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),CMD3G_TABLEWHILEIO,c_tgtmsk));
}

int32_t CrtCtrl32::SetWhileIO(int32_t Value, int32_t Mask) 
{
  return CmdListAdd(C3G(uint16_t(Value),uint16_t(Mask),CMD3G_WHILEIO,c_tgtmsk));
}

int32_t CrtCtrl32::SetWobble(double Diam, int32_t Freq) 
{
  return SetWobbleEx(1,Diam/2.,Freq,1,Diam/2.,0,90);
}

int32_t CrtCtrl32::SetWobbleEx(int32_t nType, double nAmpl, int32_t nFreq, int32_t tType, double tAmpl, int32_t tHarm, int32_t tPhase) 
{
  CmdListAdd(C3G(float(nAmpl*1000.),int16_t(nFreq),CMD3G_SETWOBBLE,c_tgtmsk));
  CmdListAdd(C3G(float(tAmpl*1000.),int16_t(tPhase),CMD3G_PARAMS,c_tgtmsk));
  CmdListAdd(C3G(int32_t(tHarm),int8_t(nType),int8_t(tType),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::Sleep(int32_t Time) 
{
  return CmdListAdd(C3G(int32_t(Time),CMD3G_SLEEP,c_tgtmsk));
}

int32_t CrtCtrl32::Suspend() 
{
  return CmdListAdd(C3G(CMD3G_SUSPEND,c_tgtmsk));
}

int32_t CrtCtrl32::Synchronise() 
{
  int32_t i;
  for (i=0; i<8; i++) if (c_tgtmsk&(0x01<<i)) WaitIdle(i);
  return ERR_OK;
}

int32_t CrtCtrl32::TableArcXY(ClfPoint pt_Co, double BF) 
{
  if (fabs(BF)<0.000001) return TableLineXY(pt_Co);
  CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_TABLEARCLINE,c_tgtmsk));
  CmdListAdd(C3G(float(BF),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::TableJog(int32_t Nr, double Speed, int32_t WhileIO) 
{
  return CmdListAdd(C3G(float(Speed),int8_t(Nr),int8_t(WhileIO),CMD3G_TABLEJOG,c_tgtmsk));
}

int32_t CrtCtrl32::TableJump3D(double X, double Y, double Z, double SpeedX, double SpeedY, double SpeedZ)
{
  CmdListAdd(C3G(int32_t(X*1000.),int32_t(SpeedX*1000.),CMD3G_TABLEJUMP3D,c_tgtmsk));
  CmdListAdd(C3G(int32_t(Y*1000.),int32_t(SpeedY*1000.),CMD3G_PARAMS,c_tgtmsk));
  CmdListAdd(C3G(int32_t(Z*1000.),int32_t(SpeedZ*1000.),CMD3G_PARAMS,c_tgtmsk));
  return ERR_OK;
}

int32_t CrtCtrl32::TableJumpXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_TABLEJUMPTO,c_tgtmsk));
}

int32_t CrtCtrl32::TableLineXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_TABLELINETO,c_tgtmsk));
}

int32_t CrtCtrl32::TableMove(int32_t Nr, double Target) 
{
  return CmdListAdd(C3G(int32_t(Target*1000.),int8_t(Nr),0,CMD3G_TABLE1D,c_tgtmsk));
}

int32_t CrtCtrl32::TableMoveXY(ClfPoint pt_Co) 
{
  return CmdListAdd(C3G(int32_t(pt_Co.x*1000.),int32_t(pt_Co.y*1000.),CMD3G_TABLEMOVETO,c_tgtmsk));
}

int32_t CrtCtrl32::WaitResolver(int Nr, double TriggerPos, int32_t TriggerMode) 
{
  if (Nr<1 || Nr>2) return ERR_DATA;
  return CmdListAdd(C3G(int32_t(TriggerPos*1000.),int8_t(Nr),int8_t(TriggerMode),CMD3G_OTFWAIT,c_tgtmsk));
}

/************************************************************/
/*** CrtCtrl32 all slaves target functions  (TGTALL)      ***/
/************************************************************/

int32_t CrtCtrl32::Abort() 
{
  mutex_lock(h_Mutex);
  DELETE_(p_Cmd);
  while (m_CmdList.Items()) m_CmdList.DeleteItem(m_CmdList.pNext);
  CmdInt(C3G(uint16_t(250),INTSELFILE,0)); // prevent IO start
  CmdInt(C3G(INTABORT,TGTALL));
  ld_ListMode=0;
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::Stop()
{
  Abort();
  struct cmd3G cmd={MODE_STOP,SCODE1,0,0,INTRUNMODE,TGTALL};
  char ackn;
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)&cmd,&ackn,sizeof(cmd3G),1,0);
  p_Hw->Exchange((char*)&cmd,&ackn,sizeof(cmd3G),1,0); // command has to be repeated when leaving list mode !
  SuspendPolling();
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::SystemReset() {
  Abort();
  struct cmd3G cmd={MODE_LIST,SCODE1,0,0,INTRUNMODE,TGTALL};
  char ackn;
  mutex_lock(h_Mutex);
  p_Hw->Exchange((char*)&cmd,&ackn,sizeof(cmd3G),1,0);
  ResumePolling();
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::SystemResume()
{
  return CmdInt(C3G(INTRESUME,TGTALL));
}

int32_t CrtCtrl32::SystemSuspend()
{
  return CmdInt(C3G(INTSUSPEND,TGTALL));
}

/************************************************************/
/*** CrtCtrl32 query target functions (c_qryidx)          ***/
/************************************************************/

int32_t CrtCtrl32::ChActuals(char* data, int32_t size)
{
  int32_t i,j;
  for (i=0; i<size; i+=j)
  {
    j=std::min(512,size-i);
    p_Hw->Exchange(C3G(uint16_t(i),uint16_t(j),INTCHACTUALS,0x01<<c_qryidx),&data[i],sizeof(cmd3G),j,0);
  }
  return ERR_OK;
}

int32_t CrtCtrl32::ChParam(int32_t idx, int32_t value)
{
  return p_Hw->Exchange(C3G(uint16_t(idx),uint16_t(value),INTCHTESTPARAMS,0x01<<c_qryidx),NULL,sizeof(cmd3G),0,0);
}

int32_t CrtCtrl32::ChRead(int32_t command, char* pData)
{
  return p_Hw->Exchange(C3G(uint16_t(command&0xff),uint16_t(1),INTDEFLEXCH,0x01<<c_qryidx),pData,sizeof(cmd3G),1,0);
}

int32_t CrtCtrl32::ChWrite(int32_t command)
{
  return p_Hw->Exchange(C3G(uint16_t(command&0xff),uint16_t(0),INTDEFLWR,0x01<<c_qryidx),NULL,sizeof(cmd3G),0,0);
}

int32_t CrtCtrl32::ChReadFlash(char* data, int32_t size)
{
  return p_Hw->Exchange(C3G(uint16_t(0),uint16_t(size),INTCHFLASHRD,0x01<<c_qryidx),data,sizeof(cmd3G),size,0);
}

int32_t CrtCtrl32::ChSelect(int32_t channel)
{
  Abort();
  if (channel<-1 || channel>2) return ERR_DATA;
  if (channel>=0) c_Channel=char(channel);
  char ackn;
  mutex_lock(h_Mutex);
  p_Hw->Exchange(C3G(MODE_CH,SCODE1,c_Channel,0,INTRUNMODE,0x01<<c_qryidx),&ackn,sizeof(cmd3G),1,0);
  p_Hw->Exchange(C3G(MODE_CH,SCODE1,c_Channel,0,INTRUNMODE,0x01<<c_qryidx),&ackn,sizeof(cmd3G),1,0);
  SuspendPolling();
  mutex_unlock(h_Mutex);
  return ERR_OK;
}

int32_t CrtCtrl32::ChSetpoints(char* data, int32_t size)
{
  int32_t i,j;
  p_Hw->Exchange(C3G(uint16_t(0),uint16_t(size),INTCHSETPOINTS,0x01<<c_qryidx),NULL,sizeof(cmd3G),0,0);
  for (i=0; i<size; i+=j)
  {
    j=std::min(512,size-i);
    p_Hw->Exchange(&data[i],NULL,j,0,0);
  }
  return ERR_OK;
}

int32_t CrtCtrl32::ChStoreFlash(char* data, int32_t size)
{
  char ackn;
  p_Hw->Exchange(C3G(uint16_t(0x1800),uint16_t(size),INTCHFLASHPP,0x01<<c_qryidx),&ackn,sizeof(cmd3G),1,0);
  p_Hw->Exchange(data,&ackn,size,1,0);
  return ERR_OK;
}

int32_t CrtCtrl32::ChTest()
{
  char ackn;
  return p_Hw->Exchange(C3G(MODE_CHTEST,SCODE1,c_Channel,0,INTRUNMODE,0x01<<c_qryidx),&ackn,sizeof(cmd3G),1,0);
}

uint32_t CrtCtrl32::GetAnalog(int32_t Nr)
{
  if (Nr<5 || Nr>8) return 0;
  return m_TgtStatus.c_Analog[Nr-5];
}

int32_t CrtCtrl32::GetCfgIO(int32_t Nr, int32_t *Value)
{
  if (Nr<1 || Nr>17) return ERR_DATA;
  *Value=m_Params.d_FctIO1[8*(Nr-1)+c_qryidx];
  return ERR_OK;
}

int32_t CrtCtrl32::GetCounter(int32_t* Value)
{
  *Value=m_TgtStatus.h_counter;
  return ERR_OK;
}

int32_t CrtCtrl32::GetDeflReplies(int32_t* CH1, int32_t* CH2, int32_t* CH3)
{
  *CH1=m_TgtStatus.d_DeflX;
  *CH2=m_TgtStatus.d_DeflY;
  *CH3=m_TgtStatus.d_DeflZ;
  return ERR_OK;
}

int32_t CrtCtrl32::GetFieldSize(double* Size)
{
  *Size=m_Params.lf_FieldSize[c_qryidx];
  return ERR_OK;
}

int32_t CrtCtrl32::GetFieldSizeZ(double* Size)
{
  *Size=m_Params.lf_FieldSizeZ[c_qryidx];
  return ERR_OK;
}

int32_t CrtCtrl32::GetLaserLink(int32_t Address, int32_t* Value)
{
  struct cmd3G querytgt={0x00,sizeof(TgtStatus),0x00,0x00,INTTGTSTATUS,uint8_t(0x01<<c_qryidx)};
  mutex_lock(h_Mutex);
  p_Hw->Exchange(C3G(0,uint16_t(0x80|Address),0,0,INTEXCHLLINK,0x01<<c_qryidx),NULL,sizeof(cmd3G),0,0);
  p_Hw->Exchange((char*)(&querytgt),(char*)(&m_TgtStatus),sizeof(cmd3G),sizeof(TgtStatus),0);
  p_Hw->Exchange((char*)(&querytgt),(char*)(&m_TgtStatus),sizeof(cmd3G),sizeof(TgtStatus),0);
  mutex_unlock(h_Mutex);
  *Value=m_TgtStatus.ReturnValue;
  return ERR_OK;
}

int32_t CrtCtrl32::GetIO()
{
  return m_TgtStatus.inputs|(m_TgtStatus.outputs<<16);
}

double CrtCtrl32::GetMaxSpeed()
{
  return m_Params.lf_FieldSize[c_qryidx]*100;
}

int32_t CrtCtrl32::GetResolvers(double* X, double* Y)
{
  *X=m_TgtStatus.d_OtfX/1000.;
  *Y=m_TgtStatus.d_OtfY/1000.;
  return ERR_OK;
};

int32_t CrtCtrl32::GetReturnValue()
{
  return m_TgtStatus.ReturnValue;
}

int32_t CrtCtrl32::GetScannerDelay(int32_t* Delay)
{
  *Delay=m_Params.hd_DragError[c_qryidx];
  return ERR_OK;
}

int32_t CrtCtrl32::GetSetpointFilter(int32_t* TimeConst)
{
  *TimeConst=m_Params.d_SetpointFIR[c_qryidx]*m_Params.hd_DragError[c_qryidx]/2;
  return ERR_OK;
}

int32_t CrtCtrl32::GetTablePositions(double* X, double* Y, double* Z)
{
  *X=m_TgtStatus.d_TableX/1000.;
  *Y=m_TgtStatus.d_TableY/1000.;
  *Z=m_TgtStatus.d_TableZ/1000.;
  return ERR_OK;
};

int32_t CrtCtrl32::SystemSetIO(int32_t Value, int32_t Mask)
{
  return CmdInt(C3G(uint16_t(Value),uint16_t(Mask),0,0,INTSETIO,0x01<<c_qryidx));
}
