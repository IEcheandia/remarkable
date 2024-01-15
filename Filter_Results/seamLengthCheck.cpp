/**
 * 	@file
 * 	@copyright	Precitec GmbH & Co. KG
 * 	@author		AL
 * 	@date		2015
 * 	@brief		Filter that checks the length of the seam an throws an NIO if length was less given parameter.
 *
 */


// WM includes
#include "seamLengthCheck.h"
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <image/image.h>
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter {


const std::string SeamLengthCheck::m_oFilterName 		( "SeamLengthCheck" );
const std::string SeamLengthCheck::m_oPipeResultName 	( "LenthNIO" );			///< Pipe: Result out-pipe.
const std::string SeamLengthCheck::m_oParamMinLength		( "MinimumLength" );///< Parameter: Minimum required length of seam
const std::string SeamLengthCheck::m_oParamNioType 	( "NioType" );			///< Parameter: User-defined nio type.


SeamLengthCheck::SeamLengthCheck() :
	ResultFilter			( SeamLengthCheck::m_oFilterName, Poco::UUID{"B2F4D29E-EDE3-4C6F-85A2-779001946FDD"} ),
	m_pPipeInData			( nullptr ),
	m_oPipeResult			( this, SeamLengthCheck::m_oPipeResultName ),
	m_oMinimumSeamLength	( 0 ),
	m_oLatestPosition		( 0.0 ),
	m_oUserNioType			( ValueOutOfLimits )
{
	parameters_.add( m_oParamMinLength,		fliplib::Parameter::TYPE_double,	m_oMinimumSeamLength );
	parameters_.add( m_oParamNioType, 		fliplib::Parameter::TYPE_int,	static_cast<int>( m_oUserNioType ) );

    setInPipeConnectors({{Poco::UUID("F9C9FC7B-B428-4D6C-8B40-B6647BBDF30C"), m_pPipeInData, "SeamLengthCheckDataIn", 0, "data"}});
    setVariantID(Poco::UUID("09A98934-90E0-4D84-BB38-CCAF4305F88B"));
} // CTor.



SeamLengthCheck::~SeamLengthCheck()
{

} // DTor.



void SeamLengthCheck::setParameter()
{
	ResultFilter::setParameter();

	m_oMinimumSeamLength = parameters_.getParameter( SeamLengthCheck::m_oParamMinLength ).convert<double>();
	m_oUserNioType 		= ResultType( parameters_.getParameter( SeamLengthCheck::m_oParamNioType ).convert<int>() );
} // setParameter.



/*virtual*/ void SeamLengthCheck::arm (const fliplib::ArmStateBase& state)
{
	if ( state.getStateID() == eSeamStart )
	{
		m_oLatestPosition = 0;
	}

	if ( state.getStateID() == eSeamEnd && m_oLatestPosition > 0 ) // skip if seam end armed but proceed was not called before
	{

		if(m_oVerbosity >= eHigh)
		{
			wmLog( eDebug, "SeamLengthCheck - arm state: eSeamEnd\n" );

		} // if

		if ( m_oLatestPosition < m_oMinimumSeamLength )
		{
			if(m_oVerbosity >= eHigh)
			{
				wmLog( eDebug, "SeamLengthCheck - seam to short, sending NIO result...\n" );

			} // if


			// send result
            // preSignalAction() only necessary within proceed / proceedGroup
			m_oPipeResult.signal( m_oOutValue );
		}
	} // if

} // arm



bool SeamLengthCheck::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SeamLengthCheck::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEventArgs )
{
	poco_assert_dbg( m_pPipeInData != nullptr );

	const GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(m_oCounter);
	m_oOutValue = ResultDoubleArray( id(), GapWidth, m_oUserNioType, rGeoDoubleArrayIn.context(), rGeoDoubleArrayIn, TRange<double>( 0, 10000 ), true );

	m_oLatestPosition = rGeoDoubleArrayIn.context().position() / 1000.0;
	if(m_oVerbosity >= eHigh)
	{
		wmLog( eDebug, "SeamLengthCheck - actual position: %f\n", m_oLatestPosition);

	} //

	preSignalAction(); // proceed only, arm signal() not measured
} // proceed


} // namespace filter
} // namespace precitec
