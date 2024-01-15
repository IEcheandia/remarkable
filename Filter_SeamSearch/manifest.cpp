/**
 *  @defgroup		Filter_SeamSearch Filter_SeamSearch
 *
 *  @file
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @brief			Fliplib manifest. Declares filter classes available in a component.
 */

#include <iostream>

#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "contourPointPairsCheck.h"
#include "intensityProfile.h"
#include "intensityProfileXT.h"
#include "gradient.h"
#include "maximum.h"
#include "selectPeaks.h"
#include "selectPeaksXT.h"
#include "selectFourPeaks.h"
#include "eliminateOutliers.h"
#include "correctWidth.h"
#include "selectSeamPos.h"
#include "simpleTracking.h"
#include "simpleTrackingXT.h"
#include "lineWidthMinimum.h"
#include "twoLinesWidthMinimum.h"
#include "lapJoint.h"
#include "lapJointXT.h"
#include "seamFindingDummy.h"
#include "seamFindingAdapter.h"
#include "seamFindingCollector.h"
#include "selectPeaksCont.h"
#include "selectPeaksLRCont.h"
#include "textureBased.h"
#include "intensityProfileLine.h"

#include "module/moduleLogger.h"

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ContourPointPairsCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::IntensityProfile)
	FLIPLIB_EXPORT_CLASS(precitec::filter::IntensityProfileXT)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Gradient)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Maximum)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SelectPeaks)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SelectPeaksXT)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SelectFourPeaks)
	FLIPLIB_EXPORT_CLASS(precitec::filter::EliminateOutliers)
	FLIPLIB_EXPORT_CLASS(precitec::filter::CorrectWidth)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SelectSeamPos)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SimpleTracking)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SimpleTrackingXT)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineWidthMinimum)
	FLIPLIB_EXPORT_CLASS(precitec::filter::TwoLinesWidthMinimum)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LapJoint)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LapJointXT)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SeamFindingDummy)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamFindingAdapter)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamFindingCollector)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SelectPeaksCont)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SelectPeaksLRCont)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamFindTextureBased)
	FLIPLIB_EXPORT_CLASS(precitec::filter::IntensityProfileLine)
	FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.SeamSearch component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.SeamSearch component uninitializing.\n" );
#endif
}
