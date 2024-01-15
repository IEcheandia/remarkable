/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2017
 * 	@brief		This filter will only send a valid result out, if a second pipe delivers a value over a certain threshold. Otherwise the result will be declared invalid and then send out - which will cause the sum-errors to ignore it.
 */

// WM includes
#include "conditionalResult.h"
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <image/image.h>
#include <module/moduleLogger.h>
#include <geo/range.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter {


const std::string ConditionalResult::m_oFilterName 		( "ConditionalResult" );
const std::string ConditionalResult::m_oPipeResultName 	( "Result" );				///< Pipe: Result out-pipe.
const std::string ConditionalResult::m_oParamThreshold	( "Threshold" );			///< Parameter: Threshold below which a pixel is not part of the signal.
const std::string ConditionalResult::m_oParamResultType	( "ResultType" );			///< Parameter: User-defined nio type.
const std::string ConditionalResult::m_oParamNioType 	( "NioType" );				///< Parameter: User-defined nio type.



ConditionalResult::ConditionalResult() :
	ResultFilter			( ConditionalResult::m_oFilterName, Poco::UUID{"f13b1d50-9a7d-48f3-8866-1a5007a47bf8"} ),
	m_pPipeInDouble			( nullptr ),
	m_pPipeInValid			( nullptr ),
	m_oPipeResult			( this, ConditionalResult::m_oPipeResultName ),
	m_oThreshold			( 20 ),
	m_oUserResultType		( Value ),
	m_oUserNioType			( ValueOutOfLimits )
{
	parameters_.add( m_oParamThreshold,		fliplib::Parameter::TYPE_double,	m_oThreshold );
	parameters_.add( m_oParamResultType, 	fliplib::Parameter::TYPE_int,		static_cast<int>( m_oUserResultType ) );
	parameters_.add( m_oParamNioType, 		fliplib::Parameter::TYPE_int,		static_cast<int>( m_oUserNioType ) );

    setInPipeConnectors({{Poco::UUID("e2e514e8-c528-4707-bb7c-1b45ca7e8b1e"), m_pPipeInDouble, "Data", 1, "data"},
    {Poco::UUID("6226c696-19f4-4410-94d3-8a192c1de5e8"), m_pPipeInValid, "Valid", 1, "valid"}});
    setVariantID(Poco::UUID("c533649b-5f59-4177-ab78-da1925f89579"));
} // CTor.



ConditionalResult::~ConditionalResult()
{
} // DTor.



void ConditionalResult::setParameter()
{
	ResultFilter::setParameter();

	m_oThreshold 		=             parameters_.getParameter( ConditionalResult::m_oParamThreshold ).convert<int>();
	m_oUserResultType 	= ResultType( parameters_.getParameter( ConditionalResult::m_oParamResultType ).convert<int>() );
	m_oUserNioType 		= ResultType( parameters_.getParameter( ConditionalResult::m_oParamNioType ).convert<int>() );

} // setParameter.



bool ConditionalResult::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "data" )
	{
		m_pPipeInDouble = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	}
	else
	{
		m_pPipeInValid = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void ConditionalResult::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArgs )
{
	poco_assert_dbg( m_pPipeInDouble != nullptr );
	poco_assert_dbg( m_pPipeInValid  != nullptr );

	const GeoDoublearray&		rGeoDoubleIn		=	m_pPipeInDouble->read(m_oCounter);
	const ResultType			oResType			=	rGeoDoubleIn.analysisResult();
	const GeoDoublearray&		rGeoValidIn			=	m_pPipeInValid->read(m_oCounter);


	const geo2d::Range1d		oMinMaxRange( -100000.0, 100000.0 );

	// set validity of the result based on the input of the second pipe of the filter ...
	bool oValid = std::get<eData>(rGeoValidIn.ref()[0]) > m_oThreshold;

	if (oResType != interface::AnalysisOK)
	{
		const GeoDoublearray	oGeoValueOut	=	GeoDoublearray		{	rGeoDoubleIn.context(),
																			rGeoDoubleIn.ref(),
																			oResType,
																			NotPresent }; // bad rank
		ResultDoubleArray		oResultNIO		=	ResultDoubleArray	{	id(),
																			m_oUserResultType,
																			oResType,
																			rGeoDoubleIn.context(),
																			oGeoValueOut,
																			oMinMaxRange,
																			true }; // set nio
        oResultNIO.setValid( oValid );
																			
        preSignalAction();
		m_oPipeResult.signal(oResultNIO);

		return; // RETURN
	}

	const auto	oGeoValueOut			=	GeoDoublearray		{	rGeoDoubleIn.context(),
																	rGeoDoubleIn.ref(),
																	oResType,
																	rGeoDoubleIn.rank()	};

	auto		oResultDoubleOut		=	ResultDoubleArray	{	id(),
																	m_oUserResultType,
																	m_oUserNioType,
																	rGeoDoubleIn.context(),
																	oGeoValueOut,
																	oMinMaxRange,
																	false }; // set io

	oResultDoubleOut.setValid( oValid );

	// signal result
	preSignalAction();
	m_oPipeResult.signal( oResultDoubleOut );

} // proceed


} // namespace filter
} // namespace precitec
