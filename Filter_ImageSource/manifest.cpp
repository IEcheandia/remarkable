/**
 * @defgroup Filter_ImageSource Filter_ImageSource
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "imageSource.h"
#include "sampleToImageSource.h"

#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ImageSource)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SampleToImageSource)
FLIPLIB_END_MANIFEST


//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
	precitec::wmLog( precitec::eDebug, "Filter.ImageSource component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.ImageSource component uninitializing.\n" );
#endif
}
