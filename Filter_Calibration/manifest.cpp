/**
 * @defgroup Filter_Calibration Filter_Calibration
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BasePipe.h"
#include "fliplib/BaseFilterInterface.h"

#include "module/moduleLogger.h"

#include "detectCalibrationLayers.h"
#include "selectLayerRoi.h"
#include "calibrationResult.h"
#include "testCalibrationGrid.h"
#include "scanmasterFocusPoint.h"

using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
  FLIPLIB_EXPORT_CLASS(precitec::filter::DetectCalibrationLayers)
  FLIPLIB_EXPORT_CLASS(precitec::filter::SelectLayerRoi)
  FLIPLIB_EXPORT_CLASS(precitec::filter::CalibrationResult)
  FLIPLIB_EXPORT_CLASS(precitec::filter::TestCalibrationGrid)
  FLIPLIB_EXPORT_CLASS(precitec::filter::ScanmasterFocusPoint)

FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.Calibration component initializing.\n" );
#endif
}


extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.Calibration component uninitializing.\n" );
#endif
}
