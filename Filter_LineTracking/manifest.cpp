/**
 *  @defgroup 		Filter_LineTracking Filter_LineTracking
 *
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2010-2011
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "lineTracking.h"
#include "lineTrackingXT.h"
#include "lineTrackingDist.h"
#include "parallelMaximum.h"
#include "parallelMaximumXT.h"
#include "closeGaps.h"
#include "boundaryTracking.h"
#include "improveLine.h"
#include "parallelMaximumOriented.h"
#include "findGap.h"

#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineTracking)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineTrackingXT)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineTrackingDist)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ParallelMaximum)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ParallelMaximumXT)
	FLIPLIB_EXPORT_CLASS(precitec::filter::CloseGaps)
	FLIPLIB_EXPORT_CLASS(precitec::filter::BoundaryTracking)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ImproveLine)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ParallelMaximumOriented)
    FLIPLIB_EXPORT_CLASS(precitec::filter::FindGap)
FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
	precitec::wmLog( precitec::eDebug, "Filter.ParallelTracking component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.ParallelTracking component uninitializing.\n" );
#endif
}
