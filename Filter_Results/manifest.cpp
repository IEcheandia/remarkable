/**
 * @defgroup Filter_Results Filter_Results
 */

#include <iostream>
#include "fliplib/Fliplib.h"
#include "fliplib/BaseFilterInterface.h"

#include "rangeCheck.h"
#include "blackImageCheck.h"
#include "deviationCheck.h"
#include "poreClassifier.h"
#include "seamEndResult.h"
#include "resultQualas.h"
#include "seamLengthCheck.h"
#include "pureResult.h"
#include "extRangeCheck.h"
#include "lineProfileResult.h"
#include "conditionalResult.h"
#include "noSeamCheck.h"
#include "seamWeldingResult.h"
#include "spotWeldingResult.h"

#include "module/moduleLogger.h"


using fliplib::BaseFilterInterface;

FLIPLIB_GET_VERSION

FLIPLIB_BEGIN_MANIFEST(fliplib::BaseFilterInterface)
	FLIPLIB_EXPORT_CLASS(precitec::filter::RangeCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::BlackImageCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::DeviationCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::PoreClassifier)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamEndResult)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ResultQualas)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamLengthCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::PureResult)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ExtRangeCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::LineProfileResult)
	FLIPLIB_EXPORT_CLASS(precitec::filter::ConditionalResult)
	FLIPLIB_EXPORT_CLASS(precitec::filter::NoSeamCheck)
	FLIPLIB_EXPORT_CLASS(precitec::filter::SeamWeldingResult)
    FLIPLIB_EXPORT_CLASS(precitec::filter::SpotWeldingResult)
FLIPLIB_END_MANIFEST

//no namespace active!
extern "C" void pocoInitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.Results component initializing.\n" );
#endif
}

extern "C" void pocoUninitializeLibrary()
{
#if !defined(NDEBUG)
    precitec::wmLog( precitec::eDebug, "Filter.Results component uninitializing.\n" );
#endif
}
