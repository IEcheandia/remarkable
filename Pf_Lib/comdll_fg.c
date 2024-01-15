/*
 * com_cl.c
 *
 * COMDLL interface layer for CLSER frame grabbers
 * 
 * $Id: comdll_fg.c,v 1.3 2008-03-06 12:13:07 hofmann Exp $
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <unistd.h>
#include "comdll_fg.h"
#include "clser.h"

//extern void DsyInit();

DWORD clSerialRead_FG(HANDLE h, BYTE *buff, DWORD *dwLength, DWORD time_max)
{
	return clSerialRead((void*)h, (char*)buff, dwLength, time_max);
}

DWORD clSerialWrite_FG(HANDLE h, BYTE *buff, DWORD *dwLength, DWORD time_max)
{
	return clSerialWrite((void *)h, (char*)buff, dwLength, time_max);
}

HANDLE clSerialInit_FG(int serialIndex, HANDLE *h)
{
	//for Leutron frame grabber
	//DsyInit();

	return clSerialInit(serialIndex, (void **)h);
}

int clSerialClose_FG(HANDLE h)
{
	clSerialClose((void*)h);
	return 1;
}
