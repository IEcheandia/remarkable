/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		Sk
 * 	@date		2015
 * 	@brief		Result filter for double values. No rangeCheck
 */


#include "pureResult.h"

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
const std::string PureResult::m_oFilterName			= std::string("PureResult");
const std::string PureResult::m_oPipeNameResult		= std::string("DoubleOK");
const std::string PureResult::m_oResultType			= std::string("ResultType");

// standard ctor
PureResult::PureResult() : ResultFilter(PureResult::m_oFilterName, Poco::UUID{"E40693D3-9E7F-4BA3-B3E1-A12D722EFB41"}),
		m_pPipeInDouble		( nullptr),                                         // IN Pipe
		m_oPipeResultDouble	( this, PureResult::m_oPipeNameResult ),			// OUT Pipe
		m_oMinMaxRange		( -100000.0, 100000.0 ),
		m_UserResultType	( Value ),
		m_UserNioType		( ValueOutOfLimits )
{
	parameters_.add(m_oResultType,	Parameter::TYPE_int,	static_cast<int>(m_UserResultType));

    setInPipeConnectors({{Poco::UUID("33E1C20B-1F4D-46B5-BF11-188C5D6AADB6"), m_pPipeInDouble, "pureResultValIn", 0, ""}});
    setVariantID(Poco::UUID("1D788750-1BA4-457E-B1AA-759B473A236C"));
} //ctor



void PureResult::setParameter()
{
	ResultFilter::setParameter();
	m_UserResultType			= static_cast<ResultType>(parameters_.getParameter(PureResult::m_oResultType).convert<int>());
} // setParameter



bool PureResult::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInDouble  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void PureResult::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArgs)
{
	poco_assert_dbg(m_pPipeInDouble != nullptr); // to be asserted by graph editor

	const GeoDoublearray&		rGeoArrayIn			=	m_pPipeInDouble->read(m_oCounter);
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

    auto oRankOut = rGeoArrayIn.rank();
    if (oRankOut != 0 && rGeoArrayIn.context().m_transposed)
    {
        wmLog(eWarning, "Result %d contains data from a transposed image, setting rank to 0 \n", oResType);
        oRankOut = 0.0;
    }

	const auto	oGeoValueOut			=	GeoDoublearray		{	rGeoArrayIn.context(),
																	rGeoArrayIn.ref(),
																	oResType,
																	oRankOut};
	auto		oResultDoubleOut		=	ResultDoubleArray	{	id(),
																	m_UserResultType,
																	m_UserNioType,
																	rGeoArrayIn.context(),
																	oGeoValueOut,
																	m_oMinMaxRange,
																	false };

	// signal result
    preSignalAction();
	m_oPipeResultDouble.signal( oResultDoubleOut );
} // proceed


} // namespace filter
} // namespace precitec
