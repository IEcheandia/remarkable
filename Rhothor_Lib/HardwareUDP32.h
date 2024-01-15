/**************************************************************************************
 This software is developed by NEWSON NV
 Redistribution and use, with or without modification, are permitted
 
 THIS SOFTWARE IS PROVIDED BY NEWSON AS IS.
 ANY EXPRESS OR IMPLIED WARRANTIES ARE DISCLAIMED.
 IN NO EVENT SHALL NEWSON BE LIABLE FOR ANY DAMAGES ARISING
 OUT OF THE USE OF THIS SOFTWARE.
 **************************************************************************************/


class CHardwareUDP32:public CHardware32
{
public:
  public: ~CHardwareUDP32();
  public: CHardwareUDP32();

// low level communication
//////////////////////////
public:
  void SetAddress(const char* IP);
  int32_t ConnectDSP();
  int32_t Exchange(char* OutBytes, char* InBytes, int32_t BytesToWrite, int32_t BytesToRead, int32_t trials);
  int32_t CommType();

private:
	struct sockaddr_in m_sa;
	SOCKET h_Socket;

  void DisConnect();
};
