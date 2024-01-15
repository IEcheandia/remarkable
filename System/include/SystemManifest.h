#ifndef SYSTEM_MANIFEST_H_
#define SYSTEM_MANIFEST_H_
#pragma once

/**
 * System::SystemManifest.h
 *
 *  Created on: 31.05.2010
 *      Author: Wolfgang Reichl
 *   Copyright: Precitec Vision KG
 */
#include "Poco/Poco.h" // get system specific stuff
#include "Poco/ClassLibrary.h"


namespace precitec
{
namespace system
{

#if !defined(SYSTEM_API)
	#if defined(_WIN32)
		#if defined(SYSTEM_EXPORTS)
			#define SYSTEM_API __declspec(dllexport)
		#else
			#define SYSTEM_API __declspec(dllimport)
		#endif
	#endif
#endif // !defined(SYSTEM_API)

#if !defined(SYSTEM_API)
	#define SYSTEM_API
#endif

#define SYSTEMLIB_BEGIN_MANIFEST 		POCO_BEGIN_MANIFEST
#define SYSTEMLIB_EXPORT_CLASS 			POCO_EXPORT_CLASS
#define SYSTEMLIB_END_MANIFEST 			POCO_END_MANIFEST

#define SYSTEMLIB_DECLARE_EXCEPTION 			POCO_DECLARE_EXCEPTION
#define SYSTEMLIB_IMPLEMENT_EXCEPTION 		POCO_IMPLEMENT_EXCEPTION
#define	SYSTEMLIB_SUBCLASSING_EXCEPTION(API, CLS) SYSTEMLIB_DECLARE_EXCEPTION(API, CLS, Poco::CLS)

} // namespace system
} //namespace precitec


#endif // SYSTEMLIB_MANIFEST_H_
