/**
 * @defgroup Filter_LineGeometry Filter_LineGeometry
 */

// system includes
#include <iostream>
// fliplib includes
#include <fliplib/Fliplib.h>
#include <fliplib/BaseFilterInterface.h>
// local includes
#include "lineProfile.h"
#include "lineLowPassAndScale.h"
#include "lineForwardDiff.h"
#include "lineWeightedSum.h"
#include "lineNormalize.h"
#include "lineExtremum.h"
#include "lineExtremumNumber.h"
#include "lineExtract.h"
#include "lineExtractDynamic.h"
#include "linePos.h"
#include "lineDisplay.h"
#include "lineMovingAverage.h"
#include "kCurvation.h"
#include "getRunBaseline.h"
#include "dispersion.h"
#include "gradientTrend.h"
#include "discoverRuns.h"
#include "getRunData.h"
#include "getRunData3D.h"
#include "findBeadEnds.h"
#include "lineFeature.h"
#include "seamData3D.h"
#include "seamQuality.h"
#include "lineLength.h"
#include "lineWidth.h"
#include "lineBrightness.h"
#include "lineAdd.h"
#include "lineFit.h"
#include "lineFitPos.h"
#include "cavvex.h"
#include "cavvexSimple.h"
#include "lineStep.h"
#include "computeAngle.h"
#include "lineRandkerbe3Inputs.h"
#include "seamGeometryTwb.h"
#include "parabelFit.h"
#include "lineTrackingQuality.h"
#include "lineSelectLocalExtremum.h"
#include "lineModelToLaserline.h"
#include "lineTemporalFilter.h"
#include "lineArithmetic.h"

#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineProfile)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineLowPassAndScale)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineForwardDiff)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineWeightedSum)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineNormalize)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineExtremum)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineExtract)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineExtractDynamic)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LinePos)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineDisplay)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineMovingAverage)
	FLIPLIB_EXPORT_CLASS(precitec::filter::KCurvation)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GetRunBaseline)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Dispersion)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GradientTrend)
	FLIPLIB_EXPORT_CLASS(precitec::filter::DiscoverRuns)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GetRunData)
	FLIPLIB_EXPORT_CLASS(precitec::filter::GetRunData3D)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineExtremumNumber)
	FLIPLIB_EXPORT_CLASS(precitec::filter::FindBeadEnds)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineFeature)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamData3D)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamQuality)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineLength)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineWidth)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineBrightness)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineAdd)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineFit)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineFitPos)
	FLIPLIB_EXPORT_CLASS(precitec::filter::Cavvex)
	FLIPLIB_EXPORT_CLASS(precitec::filter::CavvexSimple)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineStep)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ComputeAngle)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineRandkerbe3Inputs)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamGeometryTwb)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ParabelFit)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineTrackingQuality)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineSelectLocalExtremum)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineModelToLaserline)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineTemporalFilter)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineArithmetic)
	FLIPLIB_END_MANIFEST

extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.LineGeometry component initializing.\n" );
#endif
}

extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.LineGeometry component uninitializing.\n" );
#endif
}
