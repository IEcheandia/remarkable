/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/

#define CUA32_NONE 1
#define CUA32_USB 2
#define CUA32_UDP 3
#define CUA32_TCP 4

class CHardware32
{
public:
  CHardware32();
  virtual ~CHardware32();

// low level communication
//////////////////////////
public:
  virtual void SetAddress(const char* IP);
  virtual int32_t ConnectDSP();
  virtual int32_t Exchange(char* OutBytes, char* InBytes, int32_t BytesToWrite, int32_t BytesToRead, int32_t trials);
  virtual int32_t CommType();
};
