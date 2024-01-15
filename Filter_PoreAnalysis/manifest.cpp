/**
 * @defgroup Filter_PoreAnalysis Filter_PoreAnalysis
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "blobDetection.h"
#include "boundingBox.h"
#include "principalComponents.h"
#include "contour.h"
#include "poreGradient.h"
#include "surface.h"
#include "blobAdapter.h"
#include "poreClassifierOutput.h"
#include "poreClassifierOutputTriple.h"
#include "poreClassifierTypes.h"
#include "poreDetection.h"


#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::BlobDetection)
	FLIPLIB_EXPORT_CLASS(precitec::filter::BoundingBox)
	FLIPLIB_EXPORT_CLASS(precitec::filter::PrincipalComponents)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Contour)
	FLIPLIB_EXPORT_CLASS(precitec::filter::PoreGradient)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Surface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::BlobAdapter)
	FLIPLIB_EXPORT_CLASS(precitec::filter::PoreClassifierOutput)
    FLIPLIB_EXPORT_CLASS(precitec::filter::PoreClassifierOutputTriple)
    FLIPLIB_EXPORT_CLASS(precitec::filter::PoreDetection)
	FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.PoreAnalysis component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.PoreAnalysis component uninitializing.\n" );
#endif
}
