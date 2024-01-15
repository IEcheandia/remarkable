/**
 * @defgroup Fliplib Fliplib
 */

///////////////////////////////////////////////////////////
//  Soulib.h
//  Implementation of Soulib Header
//  Created on:      17-Okt-2007 17:45:04
//  Original author: Sevitec
///////////////////////////////////////////////////////////

#ifndef FLIPLIB_H_
#define FLIPLIB_H_

#include "Poco/Poco.h" // get system specific stuff
#include "Poco/ClassLibrary.h"

#if !defined(FLIPLIB_API)
	#if defined(_WIN32)
		#if defined(FLIPLIB_EXPORTS)
			#define FLIPLIB_API __declspec(dllexport)
		#else
			#define FLIPLIB_API __declspec(dllimport)
		#endif
	#endif
#endif

#if !defined(FLIPLIB_API)
	#define FLIPLIB_API
#endif


/// analoges win dll macro fuer filter libs
/// da filter nicht gleichzeitig importieren und exportieren koennen
#if !defined(FILTER_API)
	#if defined(_WIN32)
		#if defined(FILTER_EXPORTS)
			#define FILTER_API __declspec(dllexport)
		#else
			#define FILTER_API __declspec(dllimport)
		#endif
	#endif
#endif

#if !defined(FILTER_API)
	#define FILTER_API
#endif

#define FLIPLIB_BEGIN_MANIFEST 			POCO_BEGIN_MANIFEST
#define FLIPLIB_EXPORT_CLASS 			POCO_EXPORT_CLASS
#define FLIPLIB_END_MANIFEST 			POCO_END_MANIFEST
#define FLIPLIB_VERSION 2

#define FLIPLIB_DECLARE_EXCEPTION 		POCO_DECLARE_EXCEPTION
#define FLIPLIB_IMPLEMENT_EXCEPTION 		POCO_IMPLEMENT_EXCEPTION
#define	FLIPLIB_SUBCLASSING_EXCEPTION(API, CLS) FLIPLIB_DECLARE_EXCEPTION(API, CLS, Poco::CLS)

#define FLIPLIB_GET_VERSION extern "C" int version() { return FLIPLIB_VERSION; }

#endif /*FLIPLIB_H_*/
