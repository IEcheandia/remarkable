/**
 * @defgroup Filter_GapTracking Filter_GapTracking
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BasePipe.h"
#include "fliplib/BaseFilterInterface.h"

#include "gapPositionGeometry.h"
#include "gapPositionProjection.h"
#include "gapPositionBreak.h"
#include "GapPosRefine1.h"
#include "Mismatch1.h"
#include "displacement1.h"
#include "TCPDistance.h"
#include "RegelPosition.h"
#include "HeightDifference.h"
#include "step.h"
#include "stepXT.h"
#include "ControlPositionInvertible.h"
#include "gapPositionCircle.h"

#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GapPositionGeometry)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GapPositionProjection)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GapPositionBreak)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GapPositionRefine)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Mismatch)
	FLIPLIB_EXPORT_CLASS(precitec::filter::displacement)
	FLIPLIB_EXPORT_CLASS(precitec::filter::TCPDistance)
	FLIPLIB_EXPORT_CLASS(precitec::filter::RegelPosition)
	FLIPLIB_EXPORT_CLASS(precitec::filter::HeightDifference)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Step)
	FLIPLIB_EXPORT_CLASS(precitec::filter::StepXT)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ControlPositionInvertible)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GapPositionCircle)
	FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.GapTracking component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.GapTracking component uninitializing.\n" );
#endif
}
