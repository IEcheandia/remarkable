/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AB, HS
 * 	@date		2011
 * 	@brief		Result filter for double values. Performs a range check of the value.
 */


#include "rangeCheck.h"

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
const std::string RangeCheck::m_oFilterName			= std::string("RangeCheck");
const std::string RangeCheck::m_oPipeNameResult		= std::string("DoubleOK");
const std::string RangeCheck::m_oResultType			= std::string("ResultType");
const std::string RangeCheck::m_oNioType			= std::string("NioType");

// standard ctor
RangeCheck::RangeCheck() : ResultFilter(RangeCheck::m_oFilterName, Poco::UUID{"8ED6CC67-B247-4FAF-B2CA-CABC19AF8C5C"}),
		m_pPipeInDouble		( nullptr),                                         // IN Pipe
		m_oPipeResultDouble	( this, RangeCheck::m_oPipeNameResult ),			// OUT Pipee
		m_oMinMaxRange		( -1000.0, 1000.0 ),
		m_UserResultType	( Value ),
		m_UserNioType		( ValueOutOfLimits )
{
	parameters_.add("Min",			Parameter::TYPE_double,	0.0 );
	parameters_.add("Max",			Parameter::TYPE_double,	0.0 );
	parameters_.add(m_oResultType,	Parameter::TYPE_int,	static_cast<int>(m_UserResultType));
	parameters_.add(m_oNioType,		Parameter::TYPE_int,	static_cast<int>(m_UserNioType));

    setInPipeConnectors({{Poco::UUID("4E43F647-2EE4-4F49-AA35-A43825DFA3DC"), m_pPipeInDouble, "rangeCheckValIn", 0, ""}});
    setVariantID(Poco::UUID("86BB9A88-1F9E-4EBB-8AC2-4705C912DACE"));
} //ctor



void RangeCheck::setParameter()
{
	ResultFilter::setParameter();
	m_oMinMaxRange.start()		= parameters_.getParameter("Min").convert<double>();
	m_oMinMaxRange.end()		= parameters_.getParameter("Max").convert<double>();
	m_UserResultType			= static_cast<ResultType>(parameters_.getParameter(RangeCheck::m_oResultType).convert<int>());
	m_UserNioType				= static_cast<ResultType>(parameters_.getParameter(RangeCheck::m_oNioType).convert<int>());
} // setParameter



bool RangeCheck::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInDouble  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void RangeCheck::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pPipeInDouble != nullptr); // to be asserted by graph editor

	const GeoDoublearray&		rGeoArrayIn			=	m_pPipeInDouble->read(m_oCounter);
	const std::vector<double>&	rValIn				=	rGeoArrayIn.ref().getData();
	const ResultType			oResType			=	rGeoArrayIn.analysisResult();

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
	preSignalAction();
	m_oPipeResultDouble.signal( oResultDoubleOut );
} // proceed


} // namespace filter
} // namespace precitec
