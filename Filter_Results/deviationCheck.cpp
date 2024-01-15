/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		AB, HS
 * 	@date		2011
 * 	@brief		Result filter for double values. Performs a range check of the value.
 */


#include "deviationCheck.h"

#include "fliplib/Parameter.h"
#include "event/results.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "system/typeTraits.h"
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
const std::string DeviationCheck::m_oFilterName     = std::string("DeviationCheck");
const std::string DeviationCheck::m_oPipeNameResult = std::string("DeviationOK");
const std::string DeviationCheck::m_oResultType     = std::string("ResultType");
const std::string DeviationCheck::m_oNioType        = std::string("NioType");

// standard ctor
DeviationCheck::DeviationCheck() : ResultFilter(DeviationCheck::m_oFilterName, Poco::UUID{"4E8FCE3C-4495-4FD0-AF2A-53E4AB0155DA"}),
		m_pPipeInDouble		(nullptr), // IN Pipe
		m_oPipeResultDouble	( this, DeviationCheck::m_oPipeNameResult ), // OUT Pipe
		m_oReferenceValue ( 0.0 ), m_oPercent ( 5.0 ), m_oAbove(true), m_oBelow(true),
		m_oMinMaxRange ( 0.0, 0.0 ),
		m_UserResultType	( Value ), m_UserNioType		( ValueOutOfLimits )
{
	parameters_.add("RefVal", Parameter::TYPE_double,	0.0 );
	parameters_.add("Percent", Parameter::TYPE_double,	0.0 );
	parameters_.add("Above", Parameter::TYPE_bool, true);
	parameters_.add("Below", Parameter::TYPE_bool, true);
	parameters_.add(m_oResultType,	Parameter::TYPE_int,	static_cast<int>(m_UserResultType));
	parameters_.add(m_oNioType,		Parameter::TYPE_int,	static_cast<int>(m_UserNioType));

    setInPipeConnectors({{Poco::UUID("73A3B9D1-6284-44E2-8DF0-EF4E920DC2D1"), m_pPipeInDouble, "deviationCheckValIn", 0, ""}});
    setVariantID(Poco::UUID("6BE32039-ECFF-44C2-8212-428274246385"));
} //ctor

DeviationCheck::~DeviationCheck() {}

void DeviationCheck::setParameter()
{
	ResultFilter::setParameter();
	m_oReferenceValue = static_cast<double>(parameters_.getParameter("RefVal").convert<double>());
	m_oPercent = static_cast<double>(parameters_.getParameter("Percent").convert<double>()); // [0.0, 100.0]
	m_oAbove = static_cast<bool>(parameters_.getParameter("Above").convert<bool>());
	m_oBelow = static_cast<bool>(parameters_.getParameter("Below").convert<bool>());
	m_UserResultType = static_cast<ResultType>(parameters_.getParameter(DeviationCheck::m_oResultType).convert<int>());
	m_UserNioType = static_cast<ResultType>(parameters_.getParameter(DeviationCheck::m_oNioType).convert<int>());
} // setParameter

bool DeviationCheck::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInDouble  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray >* >(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void DeviationCheck::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pPipeInDouble != nullptr); // to be asserted by graph editor

	const GeoDoublearray&		rGeoArrayIn			=	m_pPipeInDouble->read(m_oCounter);
	const Doublearray&			rArrayIn			=	rGeoArrayIn.ref();
	const auto&					rValIn				=	rArrayIn.getData();
	const ResultType			oResType			=	rGeoArrayIn.analysisResult();
	if ( oResType != interface::AnalysisOK )
	{
		const GeoDoublearray	oGeoValueOut	=	GeoDoublearray		{	rGeoArrayIn.context(),
																			rArrayIn,
																			oResType,
																			NotPresent }; // bad rank
		ResultDoubleArray	oResultNIO			=	ResultDoubleArray	{	id(),
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

	ResultType oRes = rGeoArrayIn.analysisResult();
	if (oRes != interface::AnalysisOK)
	{
		const GeoDoublearray oGeoValueOut(
			rGeoArrayIn.context(),
			rArrayIn, oRes,
			NotPresent ); // bad rank
		ResultDoubleArray oResultBadRank(
			id(),
			m_UserResultType,
			oRes,
			rGeoArrayIn.context(),
			oGeoValueOut,
			m_oMinMaxRange,
			true); // set nio

        preSignalAction();
		m_oPipeResultDouble.signal( oResultBadRank ); // invoke linked filter(s)

		return; // RETURN
	}

	const double oDev = m_oReferenceValue*m_oPercent/100.0;
	m_oMinMaxRange.start()	= std::numeric_limits<double>::lowest();
	m_oMinMaxRange.end()	= std::numeric_limits<double>::max();
	if (m_oBelow)
	{
		m_oMinMaxRange.start() = m_oReferenceValue - oDev;
	}
	m_oMinMaxRange.end() = m_oReferenceValue;
	if (m_oAbove)
	{
		m_oMinMaxRange.end() = m_oReferenceValue + oDev;
	}

	const auto	oIsinRange				=	[this](double p_oValue) { return m_oMinMaxRange.contains(p_oValue); };
	const auto	oAllValuesInRange		=	std::all_of(std::begin(rValIn), std::end(rValIn), oIsinRange);
	const auto	oIsNio					=	oAllValuesInRange == false;

	const GeoDoublearray oGeoValueOut( rGeoArrayIn.context(), rArrayIn, rGeoArrayIn.analysisResult(), rGeoArrayIn.rank() );
	ResultDoubleArray oResultDoubleOut( id(), m_UserResultType, m_UserNioType, rGeoArrayIn.context(), oGeoValueOut, m_oMinMaxRange, oIsNio );

	preSignalAction(); m_oPipeResultDouble.signal( oResultDoubleOut );
} // proceed


} // namespace filter
} // namespace precitec
