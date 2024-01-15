/**
 * @defgroup Filter_ImageProcessing Filter_ImageProcessing
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "binarize.h"
#include "binarizeDynamic.h"
#include "binarizeDynamicOnOff.h"
#include "roiSelector.h"
#include "histogram.h"
#include "startEndRoiSelector.h"
#include "median.h"
#include "convolution3X3.h"
#include "hough.h"
#include "tileFeature.h"
#include "dynamicRoi.h"
#include "verticalShading.h"
#include "morphology.h"
#include "edgeDetection.h"
#include "circleFit.h"
#include "circleFitContour.h"
#include "circleHough.h"
#include "imageToPointList.h"
#include "lineEstimator.h"
#include "dynamicRoiSimple.h"
#include "startEndDynamicRoiSimple.h"
#include "selectImage.h"
#include "timedSimpleDynamicRoi.h"
#include "meltResult.h"
#include "meanBrightness.h"
#include "poorPenetration.h"
#include "edgeSkew.h"
#include "linearLut.h"
#include "lineShifter.h"
#include "surfaceCalculator.h"
#include "surfaceCalculator2Rows.h"
#include "surfaceClassifier.h"
#include "surfaceClassifier2Rows.h"
#include "templateMatching.h"
#include "shapeMatching.h"
#include "imageArithmetic.h"
#include "crossCorrelation.h"
#include "crossCorrelationDynamicTemplate.h"
#include "poorPenetrationEdge.h"
#include "poorPenetrationMulti.h"
#include "imageExtremum.h"
#include "surfaceCalculatorAdaptInput.h"
#include "startEndMisalignmentDetection.h"
#include "notchSize.h"
#include "circleFitXT.h"
#include "adjustContrast.h"
#include "metrology.h"
#include "contourProfile.h"
#include "parallelLocalExtremum.h"
#include "lineToContour.h"
#include "lineSmoothFFT.h"
#include "startEndROIChecker.h"
#include "removeBackground.h"
#include "stencil.h"
#include "caliper.h"
#include "module/moduleLogger.h"

FLIPLIB_GET_VERSION

using fliplib::BaseFilterInterface;

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Binarize)
    FLIPLIB_EXPORT_CLASS(precitec::filter::BinarizeDynamic)
    FLIPLIB_EXPORT_CLASS(precitec::filter::BinarizeDynamicOnOff)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ROISelector)
    FLIPLIB_EXPORT_CLASS(precitec::filter::StartEndROISelector)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Histogram)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ImageToPointList)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Median)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Convolution3X3)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Hough)
    FLIPLIB_EXPORT_CLASS(precitec::filter::TileFeature)
    FLIPLIB_EXPORT_CLASS(precitec::filter::DynamicRoi)
    FLIPLIB_EXPORT_CLASS(precitec::filter::VerticalShading)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Morphology)
    FLIPLIB_EXPORT_CLASS(precitec::filter::EdgeDetection)
    FLIPLIB_EXPORT_CLASS(precitec::filter::CircleFit)
    FLIPLIB_EXPORT_CLASS(precitec::filter::CircleFitContour)
    FLIPLIB_EXPORT_CLASS(precitec::filter::CircleHough)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineEstimator)
    FLIPLIB_EXPORT_CLASS(precitec::filter::DynamicRoiSimple)
    FLIPLIB_EXPORT_CLASS(precitec::filter::StartEndDynamicRoiSimple)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SelectImage)
    FLIPLIB_EXPORT_CLASS(precitec::filter::TimedSimpleDynamicRoi)
    FLIPLIB_EXPORT_CLASS(precitec::filter::MeltResult)
    FLIPLIB_EXPORT_CLASS(precitec::filter::EdgeSkew)
    FLIPLIB_EXPORT_CLASS(precitec::filter::PoorPenetration)
    FLIPLIB_EXPORT_CLASS(precitec::filter::MeanBrightness)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LinearLut)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineShifter)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SurfaceCalculator)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SurfaceCalculator2Rows)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SurfaceClassifier)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SurfaceClassifier2Rows)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ImageArithmetic)
    FLIPLIB_EXPORT_CLASS(precitec::filter::CrossCorrelation)
    FLIPLIB_EXPORT_CLASS(precitec::filter::CrossCorrelationDynamicTemplate)
    FLIPLIB_EXPORT_CLASS(precitec::filter::PoorPenetrationEdge)
    FLIPLIB_EXPORT_CLASS(precitec::filter::PoorPenetrationMulti)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ImageExtremum)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SurfaceCalculatorAdaptInput)
    FLIPLIB_EXPORT_CLASS(precitec::filter::StartEndMisalignmentDetection)
    FLIPLIB_EXPORT_CLASS(precitec::filter::NotchSize)
    FLIPLIB_EXPORT_CLASS(precitec::filter::CircleFitXT)
    FLIPLIB_EXPORT_CLASS(precitec::filter::AdjustContrast)
    FLIPLIB_EXPORT_CLASS(precitec::filter::TemplateMatching)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ShapeMatching)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Metrology)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ContourProfile)
    FLIPLIB_EXPORT_CLASS(precitec::filter::ParallelLocalExtremum)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineToContour)
    FLIPLIB_EXPORT_CLASS(precitec::filter::LineSmoothFFT)
    FLIPLIB_EXPORT_CLASS(precitec::filter::StartEndROIChecker)
    FLIPLIB_EXPORT_CLASS(precitec::filter::RemoveBackground)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Stencil)
    FLIPLIB_EXPORT_CLASS(precitec::filter::Caliper)
    FLIPLIB_END_MANIFEST


//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.ImageProcessing component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.ImageProcessing component uninitializing.\n" );
#endif
}
