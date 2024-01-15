/**
 * @defgroup Filter_Utility Filter_Utility
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "temporalLowPass.h"
#include "parameterFilter.h"
#include "ringBufferRecorder.h"
#include "bufferRecorder.h"
#include "bufferPlayer.h"
#include "delay.h"
#include "arithmetic.h"
#include "arithmeticConstant.h"
#include "delayVariable.h"
#include "functionGenerator.h"
#include "posDisplay.h"
#include "valueDisplay.h"
#include "valueDisplayOnOff.h"
#include "posDistance.h"
#include "refCurve.h"
#include "dataSubsampling.h"
#include "dataSubsampling2.h"
#include "unaryArithmetic.h"
#include "delayPosData.h"
#include "conditional.h"
#include "OneSamplePlayer.h"
#include "OneSampleRecorder.h"
#include "setPosition.h"
#include "setPositionSample.h"
#include "extractRank.h"
#include "maxJump.h"
#include "maxJump2.h"
#include "modelCurve.h"
#include "poorPenetrationChecker.h"
#include "poorPenetrationCheckerTriple.h"
#include "houghChecker.h"
#include "systemConstant.h"
#include "imageWrite.h"
#include "lineWrite.h"
#include "intersect2Lines.h"
#include "PosDisplayWithRank.h"
#include "PosDisplayWithRankOptDraw.h"
#include "parameterFilterDouble.h"
#include "contextNormalizeDouble.h"
#include "seamConstant.h"
#include "conditionalImage.h"
#include "conditionalLine.h"
#include "generateContour.h"
#include "ContourBufferPlayer.h"
#include "ContourBufferRecorder.h"
#include "generateArcContour.h"
#include "parameterFilterLine.h"
#include "MergeContours.h"
#include "contourFromFile.h"
#include "rotateContour.h"
#include "stretchContour.h"
#include "shiftContour.h"
#include "offsetContour.h"
#include "conditionalContour.h"
#include "idmZCorrection.h"
#include "contourCoordinateTransform.h"
#include "contourPathLocation.h"
#include "powerRamp.h"
#include "speedCompensation.h"
#include "powerFunction.h"
#include "valueAtIndex.h"
#include "valueFromCSVFile.h"
#include "hardwareParameter.h"
#include "temporalLowPass2.h"
#include "stitchContour.h"
#include "luaInterpreter.h"

#include "module/moduleLogger.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
FLIPLIB_EXPORT_CLASS(precitec::filter::TemporalLowPass)
FLIPLIB_EXPORT_CLASS(precitec::filter::ParameterFilter)
FLIPLIB_EXPORT_CLASS(precitec::filter::BufferRecorder)
FLIPLIB_EXPORT_CLASS(precitec::filter::RingBufferRecorder)
FLIPLIB_EXPORT_CLASS(precitec::filter::BufferPlayer)
FLIPLIB_EXPORT_CLASS(precitec::filter::Delay)
FLIPLIB_EXPORT_CLASS(precitec::filter::Arithmetic)
FLIPLIB_EXPORT_CLASS(precitec::filter::ArithmeticConstant)
FLIPLIB_EXPORT_CLASS(precitec::filter::DelayVariable)
FLIPLIB_EXPORT_CLASS(precitec::filter::FunctionGenerator)
FLIPLIB_EXPORT_CLASS(precitec::filter::PosDisplay)
FLIPLIB_EXPORT_CLASS(precitec::filter::PosDistance)
FLIPLIB_EXPORT_CLASS(precitec::filter::RefCurve)
FLIPLIB_EXPORT_CLASS(precitec::filter::DataSubsampling)
FLIPLIB_EXPORT_CLASS(precitec::filter::DataSubsampling2)
FLIPLIB_EXPORT_CLASS(precitec::filter::DelayPosData)
FLIPLIB_EXPORT_CLASS(precitec::filter::UnaryArithmetic)
FLIPLIB_EXPORT_CLASS(precitec::filter::Conditional)
FLIPLIB_EXPORT_CLASS(precitec::filter::OneSampleRecorder)
FLIPLIB_EXPORT_CLASS(precitec::filter::OneSamplePlayer)
FLIPLIB_EXPORT_CLASS(precitec::filter::SetPosition)
FLIPLIB_EXPORT_CLASS(precitec::filter::SetPositionSample)
FLIPLIB_EXPORT_CLASS(precitec::filter::ExtractRank)
FLIPLIB_EXPORT_CLASS(precitec::filter::MaxJump)
FLIPLIB_EXPORT_CLASS(precitec::filter::MaxJump2)
FLIPLIB_EXPORT_CLASS(precitec::filter::ModelCurve)
FLIPLIB_EXPORT_CLASS(precitec::filter::PoorPenetrationChecker)
FLIPLIB_EXPORT_CLASS(precitec::filter::PoorPenetrationCheckerTriple)
FLIPLIB_EXPORT_CLASS(precitec::filter::HoughChecker)
FLIPLIB_EXPORT_CLASS(precitec::filter::SystemConstant)
FLIPLIB_EXPORT_CLASS(precitec::filter::ImageWrite)
FLIPLIB_EXPORT_CLASS(precitec::filter::LineWrite)
FLIPLIB_EXPORT_CLASS(precitec::filter::Intersect2Lines)
FLIPLIB_EXPORT_CLASS(precitec::filter::PosDisplayWithRank)
FLIPLIB_EXPORT_CLASS(precitec::filter::PosDisplayWithRankOptDraw)
FLIPLIB_EXPORT_CLASS(precitec::filter::ParameterFilterDouble)
FLIPLIB_EXPORT_CLASS(precitec::filter::ContextNormalizeDouble)
FLIPLIB_EXPORT_CLASS(precitec::filter::SeamConstant)
FLIPLIB_EXPORT_CLASS(precitec::filter::ConditionalImage)
FLIPLIB_EXPORT_CLASS(precitec::filter::GenerateContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::ContourBufferRecorder)
FLIPLIB_EXPORT_CLASS(precitec::filter::ContourBufferPlayer)
FLIPLIB_EXPORT_CLASS(precitec::filter::GenerateArcContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::ParameterFilterLine)
FLIPLIB_EXPORT_CLASS(precitec::filter::MergeContours)
FLIPLIB_EXPORT_CLASS(precitec::filter::ContourFromFile)
FLIPLIB_EXPORT_CLASS(precitec::filter::RotateContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::StretchContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::ShiftContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::OffsetContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::ConditionalLine)
FLIPLIB_EXPORT_CLASS(precitec::filter::ValueDisplay)
FLIPLIB_EXPORT_CLASS(precitec::filter::ValueDisplayOnOff)
FLIPLIB_EXPORT_CLASS(precitec::filter::ConditionalContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::IdmZCorrection)
FLIPLIB_EXPORT_CLASS(precitec::filter::ContourCoordinateTransform)
FLIPLIB_EXPORT_CLASS(precitec::filter::ContourPathLocation)
FLIPLIB_EXPORT_CLASS(precitec::filter::PowerRamp)
FLIPLIB_EXPORT_CLASS(precitec::filter::SpeedCompensation)
FLIPLIB_EXPORT_CLASS(precitec::filter::PowerFunction)
FLIPLIB_EXPORT_CLASS(precitec::filter::ValueAtIndex)
FLIPLIB_EXPORT_CLASS(precitec::filter::ValueFromCSVFile)
FLIPLIB_EXPORT_CLASS(precitec::filter::HardwareParameter)
FLIPLIB_EXPORT_CLASS(precitec::filter::TemporalLowPass2)
FLIPLIB_EXPORT_CLASS(precitec::filter::StitchContour)
FLIPLIB_EXPORT_CLASS(precitec::filter::LuaInterpreter)

FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.Utility component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.Utility component uninitializing.\n" );
#endif
}
