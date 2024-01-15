
/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @brief			constants and defines
 */


#ifndef DEFINES_H_20111031
#define DEFINES_H_20111031

// poco includes
#include "Poco/Bugcheck.h"				///< Definition of the Bugcheck class and the self-testing macros.
// project includes
#include "InterfacesManifest.h"
#undef interface

const					std::size_t	g_oNbParMax	=	4;              // max number of threads available for pipelined inspection. should be equal to Poco::Environment::processorCount(), which is not known at compile time. Currently 4 cores on RTOS
extern INTERFACES_API 	std::size_t	g_oNbPar;                       // number of threads used for pipelined inspection. assertion g_oNbPar <= g_oNbParMax in analyzer
extern INTERFACES_API   bool        g_oDisableHWResults;            // disable the generation of hw results
extern INTERFACES_API   int         g_oLineLaser1DefaultIntensity;  // default intensity of the 1st line generator (typically only used for the calibration)
extern INTERFACES_API   int         g_oLineLaser2DefaultIntensity;  // default intensity of the 2nd line generator (typically only used for the calibration)
extern INTERFACES_API   int         g_oFieldLight1DefaultIntensity; // default intensity of the 3rd line generator/ 1st field light (typically only used for the calibration)
extern INTERFACES_API  bool g_oDebugTimings;        //displays additional log messages relative to processing time
extern INTERFACES_API  int g_oDebugInspectManagerTime_us;
extern INTERFACES_API  bool g_oUseScanmasterPosition;

namespace precitec {

// see dataAcquire.h ?
enum CameraType {
	CamGreyImage	= 1,
	CamLaserLine	= 2
}; // CameraType

enum { MAXIMAGE = 100 };												///< max nb of images that are loaded by grabber if there is no camera.

static const unsigned int MAX_CAMERA_WIDTH = 1600;
static const unsigned int MAX_CAMERA_HEIGHT = 1100;

namespace interface {

// localization keys for units

extern INTERFACES_API const std::string	g_oLangKeyUnitNone;				///< loclaization key for a unit
extern INTERFACES_API const std::string	g_oLangKeyUnitPixels;			///< loclaization key for a unit
extern INTERFACES_API const std::string	g_oLangKeyUnitDeltaI;			///< loclaization key for a unit
extern INTERFACES_API const std::string	g_oLangKeyUnitUm;				///< loclaization key for a unit
extern INTERFACES_API const std::string	g_oLangKeyUnitMm;				///< loclaization key for a unit
extern INTERFACES_API const std::string	g_oLangKeyUnitSqUm;				///< loclaization key for a unit
extern INTERFACES_API const std::string	g_oLangKeyUnitSqMm;				///< loclaization key for a unit


#ifdef NDEBUG
	extern INTERFACES_API const bool isDebug;						///< DEPRECATED - if this is a debug build
#else // NDEBUG
	extern INTERFACES_API const bool isDebug;						///< DEPRECATED - if this is a debug build
#endif // NOT NDEBUG

} // namespace interface
} // namespace precitec



/// macro that wraps the gcc specific unused attribute. see http://gcc.gnu.org/onlinedocs/gcc/Variable-Attributes.html#Variable-Attributes

#if defined __QNX__ || defined __linux__
	#define UNUSED __attribute__((unused))  // suppress -Wunused
#else
	#define UNUSED
#endif

/**
 * @brief	poco assertion macro
 * @details	needs to be redefined here for qnx because the '_DEBUG' macro used by poco is visual studio specific.
 */
#if defined __QNX__ || defined __linux__

	#if !defined (NDEBUG)

		#undef poco_assert_dbg
		#define poco_assert_dbg(cond) \
			if (!(cond)) Poco::Bugcheck::assertion(#cond, __FILE__, __LINE__); else (void) 0

		#undef poco_stdout_dbg
		#	define poco_stdout_dbg(outstr) \
			std::cout << __FILE__ << '(' << std::dec << __LINE__ << "):" << outstr << std::endl;

	#endif // not defined (NDEBUG)

#endif // defined(__QNX__ || __linux__) and not defined (NDEBUG)

#endif // DEFINES_H_20111031
