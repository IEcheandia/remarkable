// COMDLL.cpp : Defines the entry point for the DLL application.
//
// $Id: comdll_fg.h,v 1.2 2008-03-06 07:24:33 hofmann Exp $
//

#ifndef COMDLL_FG_H
#define COMDLL_FG_H

#if defined(__linux__) && defined(__x86_64)
	#define HANDLE intptr_t
#else
	#define HANDLE int
#endif
#define DWORD unsigned long
#define BYTE unsigned char


DWORD clSerialRead_FG(HANDLE h, BYTE *buff, DWORD *dwLength, DWORD time_max);
DWORD clSerialWrite_FG(HANDLE h, BYTE *buff, DWORD *dwLength, DWORD time_max);
HANDLE clSerialInit_FG(int serialIndex, HANDLE *h);
int clSerialClose_FG(HANDLE h);

#endif // COMDLL_FG_H
