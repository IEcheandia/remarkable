/**
 * @defgroup Filter_SampleSource Filter_SampleSource
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"
#include "sampleSource.h"
#include "laserPowerSource.h"
#include "encoderPositionSource.h"
#include "headMonitorSource.h"
#include "idmProfileSource.h"
#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SampleSource)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LaserPowerSource)
	FLIPLIB_EXPORT_CLASS(precitec::filter::EncoderPositionSource)
	FLIPLIB_EXPORT_CLASS(precitec::filter::HeadMonitorSource)
	FLIPLIB_EXPORT_CLASS(precitec::filter::IDMProfileSource)
FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.SampleSource component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.SampleSource component uninitializing.\n" );
#endif
}
