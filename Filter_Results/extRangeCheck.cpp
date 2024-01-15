/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		Sk
 * 	@date		2015
 * 	@brief		Result filter for double values.  Performs a range check of the value given from external.
 */


#include "extRangeCheck.h"

#include "fliplib/Parameter.h"
#include "event/results.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"

#include <algorithm>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {

namespace filter {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

using namespace precitec::interface;
using namespace precitec::geo2d;

// filter name and out pipes' names
const std::string ExtRangeCheck::m_oFilterName			= std::string("ExtRangeCheck");
const std::string ExtRangeCheck::m_oPipeNameResult		= std::string("DoubleOK");
const std::string ExtRangeCheck::m_oResultType			= std::string("ResultType");
const std::string ExtRangeCheck::m_oNioType			= std::string("NioType");

// standard ctor
ExtRangeCheck::ExtRangeCheck() : ResultFilter(ExtRangeCheck::m_oFilterName, Poco::UUID{"38EB9B4F-2DA9-40FA-BF87-C67E707F15C1"}),
		m_pPipeInDouble		( nullptr),                                         // IN Pipe
		m_pPipeInDoubleMax	( nullptr),                                         // IN Pipe Max
		m_pPipeInDoubleMin	( nullptr),                                         // IN Pipe Min
		m_oPipeResultDouble	( this, ExtRangeCheck::m_oPipeNameResult ),			// OUT Pipe
		m_oMinMaxRange		( -1000000.0, 1000000.0 ),
		m_UserResultType	( Value ),
		m_UserNioType		( ValueOutOfLimits )
{
	parameters_.add(m_oResultType,	Parameter::TYPE_int,	static_cast<int>(m_UserResultType));
	parameters_.add(m_oNioType,		Parameter::TYPE_int,	static_cast<int>(m_UserNioType));

    setInPipeConnectors({{Poco::UUID("BB068F1D-9A13-44D2-BC5A-0C852B86F1DF"), m_pPipeInDouble, "extRangeCheckValIn", 1, "data_value"},
    {Poco::UUID("F25AE0B3-2B1A-4B52-A439-34C5AF05844A"), m_pPipeInDoubleMax, "extRangeCheckMaxIn", 1, "data_max"},
    {Poco::UUID("9E79DF28-896C-4A9C-B1E4-846314C1FEBE"), m_pPipeInDoubleMin, "extRangeCheckMinIn", 1, "data_min"}});
    setVariantID(Poco::UUID("774090F3-98A3-4768-8F5C-DEB331DF419E"));
} //ctor



void ExtRangeCheck::setParameter()
{
	ResultFilter::setParameter();
	m_UserResultType			= static_cast<ResultType>(parameters_.getParameter(ExtRangeCheck::m_oResultType).convert<int>());
	m_UserNioType				= static_cast<ResultType>(parameters_.getParameter(ExtRangeCheck::m_oNioType).convert<int>());
} // setParameter



bool ExtRangeCheck::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {

	if ( p_rPipe.tag() == "data_value" )
		m_pPipeInDouble  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
	if ( p_rPipe.tag() == "data_min" )
		m_pPipeInDoubleMin  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
	if ( p_rPipe.tag() == "data_max" )
		m_pPipeInDoubleMax  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void ExtRangeCheck::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
	poco_assert_dbg(m_pPipeInDouble != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleMax != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleMin != nullptr); // to be asserted by graph editor

	const GeoDoublearray&		rGeoArrayIn			=	m_pPipeInDouble->read(m_oCounter);
	const GeoDoublearray&		rGeoArrayInMax		=	m_pPipeInDoubleMax->read(m_oCounter);
	const GeoDoublearray&		rGeoArrayInMin		=	m_pPipeInDoubleMin->read(m_oCounter);

	const std::vector<double>&	rValIn				=	rGeoArrayIn.ref().getData();
	const ResultType			oResType			=	rGeoArrayIn.analysisResult();

	unsigned int sizeOfArrayMin = rGeoArrayInMin.ref().size();
	unsigned int sizeOfArrayMax = rGeoArrayInMax.ref().size();

	if (sizeOfArrayMin > 0)
	{
		m_oMinMaxRange.start()		= std::get<eData>( rGeoArrayInMin.ref()[0] );
	}

	if (sizeOfArrayMax > 0)
	{
		m_oMinMaxRange.end()		= std::get<eData>( rGeoArrayInMax.ref()[0] );
	}


	if (oResType != interface::AnalysisOK) {
			const GeoDoublearray	oGeoValueOut	=	GeoDoublearray	{	rGeoArrayIn.context(),
																			rGeoArrayIn.ref(),
																			oResType,
																			NotPresent }; // bad rank
		ResultDoubleArray		oResultNIO		=	ResultDoubleArray	{	id(),
																			m_UserResultType,
																			oResType,
																			rGeoArrayIn.context(),
																			oGeoValueOut,
																			m_oMinMaxRange,
																			true }; // set nio
        preSignalAction();
		m_oPipeResultDouble.signal(oResultNIO); // invoke linked filter(s)

		return; // RETURN
	}

	const auto	oIsinRange				=	[this](double p_oValue) { return m_oMinMaxRange.contains(p_oValue); };
	const auto	oAllValuesInRange		=	std::all_of(std::begin(rValIn), std::end(rValIn), oIsinRange);
	const auto	oIsNio					=	oAllValuesInRange == false;

	const auto	oGeoValueOut			=	GeoDoublearray		{	rGeoArrayIn.context(),
																	rGeoArrayIn.ref(),
																	oResType,
																	rGeoArrayIn.rank()	};
	auto		oResultDoubleOut		=	ResultDoubleArray	{	id(),
																	m_UserResultType,
																	m_UserNioType,
																	rGeoArrayIn.context(),
																	oGeoValueOut,
																	m_oMinMaxRange,
																	oIsNio };

	// signal result
	preSignalAction(); // result handler timing included
	m_oPipeResultDouble.signal( oResultDoubleOut );
} // proceed


} // namespace filter
} // namespace precitec
