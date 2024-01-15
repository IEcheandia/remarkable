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

CHardware32::~CHardware32(){}
CHardware32::CHardware32() {}
void CHardware32::SetAddress(const char* IP) {}
int32_t CHardware32::ConnectDSP() { return ERR_IMPLEMENTATION; }
int32_t CHardware32::Exchange(char* OutBytes, char* InBytes, int32_t BytesToWrite, int32_t BytesToRead, int32_t trials) { return ERR_IMPLEMENTATION; }
int32_t CHardware32::CommType() { return CUA32_NONE; }

