#ifndef PHOTON_FOCUS_H
#	define PHOTON_FOCUS_H

#	include <strings.h>

extern "C"
{
#include "pf/platform.h"
#include "pf/pftypes.h"
#include "pf/api.h"
#include "pf/pfcam.h"
//# include "comdll_fg.h"
//#	include "baseTypes.h"
	

	
	
	// hier folgen einige Nachdefinitionen, die
	// PF (noch) nicht in ihren eigenen Headern haben
namespace precitec
{
namespace ip
{
namespace pf
{
		/// Umgehug des Macros TOKEN wenn ok -> TOKEN undefinen
		typedef   TOKEN Handle;
		/// Umgehug des Macros MAX_CAMERAS wenn ok -> MAX_CAMERAS undefinen
		static const int LimCameras = MAX_CAMERAS;
		/// Umgehug des Macros INVALID_TOKEN wenn ok -> INVALID_TOKEN undefinen
		static const unsigned int NoHandle   = INVALID_TOKEN;
		/// Achtung!!! inoffizielle Info. Muss in irgendeinem PF-Header definiert werden
		const int MaxStringLen = 50;
} // pf
} // ip
} // precitec
} // extern "C"

#undef BYTE

#if defined (__linux__) || defined (__QNX__)
#	undef HANDLE
#	undef DWORD
#	undef Sleep
# undef BYTE
#endif



#if defined(WIN32) && !defined(__CYGWIN__)
#	undef strcasecmp
#	undef strncasecmp
#endif


#ifdef WIN32
#	undef DLLHANDLE
#	undef COMMHANDLE
#else 
#	undef DLLHANDLE
#	undef COMMHANDLE
#	undef INVALID_HANDLE_VALUE
#endif

#endif // PHOTON_FOCUS_H
