////////////////////////////////////////////////////////////////////////////
//
// - Platform dependent settings plus configuration of exported
// symbols
// 
// (c) 2002, Photonfocus AG, CH-8853 Lachen
//
// $Id: platform.h,v 1.13 2008-07-18 13:20:27 hofmann Exp $
//
////////////////////////////////////////////////////////////////////////////

#ifndef PLATFORM_H_INCLUDED
#define PLATFORM_H_INCLUDED

#if defined (WIN32) || defined (__CYGWIN__)
	#define WIN32_LEAN_AND_MEAN		// Exclude crap
	#include <windows.h>
#endif

#define BYTE unsigned char

#ifndef NULL
	#define NULL 0
#endif

#if defined (__linux__) || defined (__QNX__)
	#ifndef HANDLE
		#define HANDLE int
	#endif
	#define DWORD unsigned long
	#include <unistd.h>
//	#define Sleep usleep -- Auskommentiert wg. Warnung JS 14.05.12
#endif

#if defined(SM2_DSP)
	#ifndef PFHANDLE
		#define PFHANDLE int
	#endif
	#define DWORD unsigned long
#endif

#if defined(__QNX__)
	#include <strings.h>
#endif

#if defined(WIN32) && !defined(__CYGWIN__)
	#define strcasecmp _stricmp
	#define strncasecmp _strnicmp
#else
	#include <string.h>
#endif

#ifdef WIN32
	#define DLLHANDLE  HINSTANCE
	#define COMMHANDLE HANDLE
#else 
	#define DLLHANDLE  void *
	#define COMMHANDLE int
	#if defined(SM2_DSP)
		#define INVALID_HANDLE_VALUE (PFHANDLE) -1
	#else // Auskommentiert wg. Warnung JS 14.05.12
//		#define INVALID_HANDLE_VALUE (HANDLE) -1
	#endif
#endif


#endif // PLATFORM_H_INCLUDED
