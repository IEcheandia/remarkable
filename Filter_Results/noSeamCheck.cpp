/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		SB
 * 	@date		2017
 * 	@brief		A filter that checks if the seam was welded long enough. The actual detection of the seam has to be provided by other filter, this filter here only sums up the
 * 				length via the images in which the seam was detected and compares that with the length of the seam as it was specified in the product tree.
 */


// WM includes
#include "noSeamCheck.h"
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <image/image.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter {


const std::string NoSeamCheck::m_oFilterName 		( "NoSeamCheck" );		///< Name
const std::string NoSeamCheck::m_oPipeResultName 	( "LengthNIO" );		///< Pipe: Result out-pipe.
const std::string NoSeamCheck::m_oParamTolerance	( "Tolerance" );		///< Parameter: Tolerance in percent.
const std::string NoSeamCheck::m_oParamInverse		( "Inverse" );			///< Parameter: Inverse check. One can also use the filter to test if an error is present for a certain length of the seam.
const std::string NoSeamCheck::m_oParamResultType	( "ResultType" );		///< Parameter: User-defined result type.
const std::string NoSeamCheck::m_oParamNioType 		( "NioType" );			///< Parameter: User-defined nio type.


NoSeamCheck::NoSeamCheck() :
	ResultFilter			( NoSeamCheck::m_oFilterName, Poco::UUID{"4d8d16f9-cde0-4780-9a58-f2e1342f1932"} ),
	m_pPipeInData			( nullptr ),
	m_pPipeInSpeed			( nullptr ),
	m_oPipeResult			( this, NoSeamCheck::m_oPipeResultName ),
	m_oTriggerFreq			( 1.0 ),
	m_oTolerance			( 0.0 ),
	m_oInverse				( false ),
	m_oCurrLength			( 0.0 ),
	m_oSeamLength			( 0.0 ),
	m_oUserResultType		( GapWidth ),
	m_oUserNioType			( ValueOutOfLimits )
{
	parameters_.add( m_oParamTolerance,		fliplib::Parameter::TYPE_double,	m_oTolerance );
	parameters_.add( m_oParamInverse,	    fliplib::Parameter::TYPE_bool,		m_oInverse );
	parameters_.add( m_oParamResultType,	fliplib::Parameter::TYPE_int,		static_cast<int>( m_oUserResultType ) );
	parameters_.add( m_oParamNioType, 		fliplib::Parameter::TYPE_int,		static_cast<int>( m_oUserNioType ) );

    setInPipeConnectors({{Poco::UUID("561685d0-95d9-4c97-b58e-789b40fcf630"), m_pPipeInData, "Data", 1, "data"},
    {Poco::UUID("e659a010-8aa9-4ee7-9287-4326db29ad27"), m_pPipeInSpeed, "Speed", 1, "speed"}});
    setVariantID(Poco::UUID("f9f3821c-cdc8-4b56-8e2f-084ebbf1d2a7"));
} // CTor.



NoSeamCheck::~NoSeamCheck()
{

} // DTor.



void NoSeamCheck::setParameter()
{
	ResultFilter::setParameter();

	m_oTolerance 		= ( 100.0 - parameters_.getParameter( NoSeamCheck::m_oParamTolerance ).convert<double>() ) * 0.01;
	m_oInverse 			= parameters_.getParameter( NoSeamCheck::m_oParamInverse );
	m_oUserResultType 	= ResultType( parameters_.getParameter( NoSeamCheck::m_oParamResultType ).convert<int>() );
	m_oUserNioType 		= ResultType( parameters_.getParameter( NoSeamCheck::m_oParamNioType ).convert<int>() );

} // setParameter.



/*virtual*/ void NoSeamCheck::arm (const fliplib::ArmStateBase& state)
{
	if ( state.getStateID() == eSeamStart )
	{
		// reset the currently accumulated seam length
		m_oCurrLength = 0.0;
		// get product information
		const analyzer::ProductData* pProductData = externalData<analyzer::ProductData>();
		// get seam length in mm
		m_oSeamLength = (pProductData->m_oTriggerDelta * pProductData->m_oNumTrigger) / 1000.0;
		// get the trigger frequency
		if ( pProductData->m_oNumTrigger > 0 )
		{
			m_oTriggerFreq = ( (double)(pProductData->m_oTriggerDelta) / (double)(pProductData->m_oInspectionVelocity) );

		} // if
		else
		{
			m_oTriggerFreq = 0.0;

		} // else

		wmLog( eDebug, "NoSeamCheck - TriggerDelta: %i um Velocity: %i um/ms\n", pProductData->m_oTriggerDelta, pProductData->m_oInspectionVelocity );
		wmLog( eDebug, "NoSeamCheck - SeamLength: %f mm TriggerFreq: %f ms \n", m_oSeamLength, m_oTriggerFreq * 1000.0 );
	}

	if ( state.getStateID() == eSeamEnd && m_oCurrLength > 0 ) // skip if seam end armed but proceed was not called before
	{
		if ( (!m_oInverse && (m_oCurrLength < m_oSeamLength * m_oTolerance )) || (m_oInverse && (m_oCurrLength > m_oSeamLength * m_oTolerance )) )
		{
			if(m_oVerbosity >= eHigh)
			{
				if ( !m_oInverse ) { wmLog( eDebug, "NoSeamCheck - seam too short, sending NIO result...\n" ); }
				if (  m_oInverse ) { wmLog( eDebug, "NoSeamCheck - faulty part of seam too long, sending NIO result...\n" ); }
				wmLog( eDebug, "NoSeamCheck - welded length, config length, tolerance: %f : %f : %f\n", m_oCurrLength, m_oSeamLength, m_oTolerance*m_oSeamLength );

			} // if

			// send result
			m_oOutValue.setNio( true );
			m_oPipeResult.signal( m_oOutValue );

		} // if
	} // if

} // arm



bool NoSeamCheck::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "data")
	{
		m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);

	} // if
	else if (p_rPipe.tag() == "speed")
	{
		m_pPipeInSpeed = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);

	} // else

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void NoSeamCheck::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArgs )
{
	poco_assert_dbg( m_pPipeInData  != nullptr );
	poco_assert_dbg( m_pPipeInSpeed != nullptr );

	// get the current robot speed ...
	double oSpeed = 67.0; // mm/s
	const GeoDoublearray &rGeoDoubleArraySpeedIn = m_pPipeInSpeed->read( m_oCounter );
	if ( rGeoDoubleArraySpeedIn.ref().size() >= 1 )
	{
		oSpeed = std::get<eData>(rGeoDoubleArraySpeedIn.ref()[0]);
	}

	// now get the information, if the seam is there or not ...
	const GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read( m_oCounter );
	const auto oSizeOfArray = rGeoDoubleArrayIn.ref().size();
	if ( oSizeOfArray >= 1 )
	{
		const auto oValue = std::get<eData>(rGeoDoubleArrayIn.ref()[0]);
		if ( oValue > 0 )
		{
			m_oCurrLength += m_oTriggerFreq * oSpeed;

		} // if

		if(m_oVerbosity >= eHigh)
		{
			wmLog( eDebug, "NoSeamCheck - welded length, config length, tolerance: %f : %f : %f\n", m_oCurrLength, m_oSeamLength, m_oTolerance*m_oSeamLength );

		} // if

	} // if

	// prepare result structures
	const auto				oResultValue		= Doublearray{1, m_oCurrLength, eRankMax};
	const GeoDoublearray	oGeoValueOut		( rGeoDoubleArrayIn.context(), oResultValue, rGeoDoubleArrayIn.analysisResult(), 1.0 );
	ResultDoubleArray 		oResultDoubleOut	( id(), m_oUserResultType, m_oUserNioType, rGeoDoubleArrayIn.context(), oGeoValueOut, TRange<double>( 0, 100000 ) );
	// remember last result for arm function
	m_oOutValue = oResultDoubleOut;
	// signal result out
    preSignalAction();
	m_oPipeResult.signal( oResultDoubleOut );

} // proceed


} // namespace filter
} // namespace precitec
