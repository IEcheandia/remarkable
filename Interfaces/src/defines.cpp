
/**
 * 	@file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @brief			constants and defines
 */



#include "common/defines.h"
// poco includes

// project includes
#include "common/connectionConfiguration.h"

/*extern*/ 			std::size_t g_oNbPar                =   1;        // no pipelined inspection as default
/*extern*/ 			bool g_oDisableHWResults            =   false;    // inspection does not generate any hw related results
/*extern*/          int g_oLineLaser1DefaultIntensity   =   0;        // default intensity of the 1st line generator (typically only used for the calibration)
/*extern*/          int g_oLineLaser2DefaultIntensity   =   80;       // default intensity of the 2nd line generator (typically only used for the calibration) 
/*extern*/          int g_oFieldLight1DefaultIntensity  =   0;        // default intensity of the 3rd line generator/ 1st field light (typically only used for the calibration)
/*extern*/ 			bool g_oDebugTimings            =   false;   
/*extern*/ 			int  g_oDebugInspectManagerTime_us = 400;
/*extern*/ 			bool g_oUseScanmasterPosition =   false;   


namespace precitec {
namespace interface {

const std::string	g_oLangKeyUnitNone				= "Unit.None";		///< no unit, usually empty string
const std::string	g_oLangKeyUnitPixels			= "Unit.Pixels";	///< number of pixels, eg 512
const std::string	g_oLangKeyUnitDeltaI			= "Unit.DeltaI";	///< intensity delta, eg pixel gradient
const std::string	g_oLangKeyUnitUm				= "Unit.Um";		///< length, eg pore width
const std::string	g_oLangKeyUnitMm				= "Unit.Mm";		///< length, eg pore width
const std::string	g_oLangKeyUnitSqUm				= "Unit.SqUm";		///< length, eg pore size 
const std::string	g_oLangKeyUnitSqMm				= "Unit.SqMm";		///< length, eg pore size 


#ifdef NDEBUG
	const bool isDebug	= false;						///< if this is a debug build
#else // NDEBUG
	const bool isDebug	= true;							///< if this is a debug build
#endif // NOT NDEBUG

} // namespace interface
} // namespace precitec


