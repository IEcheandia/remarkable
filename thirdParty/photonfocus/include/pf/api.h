////////////////////////////////////////////////////////////////////////////
//
//  API DEFINITION FILE
//
//  Here the behaviour of the exported API is specified.
//
//
////////////////////////////////////////////////////////////////////////////

#ifndef API_H_INCLUDED
#define API_H_INCLUDED

#include "platform.h"

// DEBUGGING
#ifdef SM2_DSP
	#ifdef DEBUG2
		#include <stdio.h>
		#define DEB(x) x
	#else
		#define DEB(x) 
	#endif
#else
	#ifdef DEBUG
		#include <stdio.h>
		#define DEB(x) x
	#else
		#define DEB(x) 
	#endif
#endif
// Some people might want to use the __stdcall calling convention.
// They will have to define USE_STDCALL.
#if defined(__linux__ ) || !defined(USE_STDCALL)
	#define APIDECL
#else
	#define APIDECL WINAPI
#endif


#ifndef EXTERN_C
	#ifdef __cplusplus
		#define EXTERN_C extern "C"
	#else
		#define EXTERN_C 
	#endif
#endif

#ifdef __cplusplus
	#define PF_INLINE inline
#else
	#ifdef __GNUC__
		#define PF_INLINE static __inline__
	#else
		#define PF_INLINE
	#endif
#endif

#ifndef SM2_DSP
	#if defined (WIN32)
		#if defined (CAMDLL_EXPORTS) || defined (CAMWRAPPER_EXPORTS)
			#define CAMDLL_API EXTERN_C __declspec(dllexport)
		#else
			#define CAMDLL_API EXTERN_C __declspec(dllimport)
		#endif

		#ifdef COMDLL_EXPORTS
			#define COMDLL_API EXTERN_C __declspec(dllexport)
		#else
			#define COMDLL_API EXTERN_C __declspec(dllimport)
		#endif
	#else
		#define CAMDLL_API
		#define COMDLL_API
	#endif
#else
	#define CAMDLL_API
	#define COMDLL_API EXTERN_C

#endif


#endif // API_H_INCLUDED
