/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		Filter that produces a result when the current seam ends. The result filter will only produce a single result, will not send a result with every frame.
 *  The filter listens on the input pipe and stores the value of a specific frame internally. When the seam ends, it will send out this value as a result.
 */

// WM includes
#include "seamEndResult.h"
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


const std::string SeamEndResult::m_oFilterName 		( "SeamEndResult" );
const std::string SeamEndResult::m_oPipeResultName 	( "ImageIO" );			///< Pipe: Result out-pipe.
const std::string SeamEndResult::m_oParamFrame		( "Frame" );			///< Parameter: Frame number of the data that will get send out as result (0 = first value, very high value = last).
const std::string SeamEndResult::m_oParamResultType	( "ResultType" );		///< Parameter: User-defined nio type.
const std::string SeamEndResult::m_oParamNioType 	( "NioType" );			///< Parameter: User-defined nio type.


SeamEndResult::SeamEndResult() :
	ResultFilter			( SeamEndResult::m_oFilterName, Poco::UUID{"77513630-D7C1-4369-8E33-C3D1BD6B85E9"} ),
	m_pPipeInData			( nullptr ),
	m_oPipeResult			( this, SeamEndResult::m_oPipeResultName ),
	m_oFrame				( 0 ),
	m_oCounter				( 0 ),
	m_oUserResultType		( Value ),
	m_oUserNioType			( ValueOutOfLimits )
{
	parameters_.add( m_oParamFrame,			fliplib::Parameter::TYPE_int,	m_oFrame );
	parameters_.add( m_oParamResultType, 	fliplib::Parameter::TYPE_int,	static_cast<int>( m_oUserResultType ) );
	parameters_.add( m_oParamNioType, 		fliplib::Parameter::TYPE_int,	static_cast<int>( m_oUserNioType ) );

    setInPipeConnectors({{Poco::UUID("0968DC9A-DA68-43C5-94CA-03DB4E2FEEB1"), m_pPipeInData, "SeamEndResultDataIn", 0, "data"}});
    setVariantID(Poco::UUID("14BBC4CC-4FE9-4960-B104-0EF5E547E9F3"));
} // CTor.



SeamEndResult::~SeamEndResult()
{

} // DTor.



void SeamEndResult::setParameter()
{
	ResultFilter::setParameter();

	m_oFrame 			=       	  parameters_.getParameter( SeamEndResult::m_oParamFrame ).convert<int>();
	m_oUserResultType 	= ResultType( parameters_.getParameter( SeamEndResult::m_oParamResultType ).convert<int>() );
	m_oUserNioType 		= ResultType( parameters_.getParameter( SeamEndResult::m_oParamNioType ).convert<int>() );
} // setParameter.



/*virtual*/ void SeamEndResult::arm (const fliplib::ArmStateBase& state)
{
	if ( state.getStateID() == eSeamStart )
	{
		m_oCounter = 0;
	}

	if ( state.getStateID() == eSeamEnd && m_oCounter != 0 ) // skip if seam end armed but proceed was not called before
	{
		wmLog( eDebug, "SeamEndResult - arm 1\n" );

		// send result
        // preSignalAction() only necessary within proceed / proceedGroup
		m_oPipeResult.writeToAllSlots( m_oOutValue );
		m_oPipeResult.signal( ResultFilter::m_oCounter );

		wmLog( eDebug, "SeamEndResult - arm 2\n" );

		if(m_oVerbosity >= eHigh)
		{
			wmLog( eDebug, "SeamEndResult - sending result now ...\n" );

		} // if
	} // if

} // arm



bool SeamEndResult::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void SeamEndResult::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEventArgs )
{
	poco_assert_dbg( m_pPipeInData != nullptr );

	// store data internally
	if ( m_oCounter <= m_oFrame )
	{
		const GeoDoublearray &rGeoDoubleArrayIn = m_pPipeInData->read(ResultFilter::m_oCounter);
		m_oOutValue = ResultDoubleArray( id(), m_oUserResultType, m_oUserNioType, rGeoDoubleArrayIn.context(), rGeoDoubleArrayIn, TRange<double>( 0, 10000 ) );
	}
	m_oCounter++;

	preSignalAction(); // proceed only, arm signal() not measured
} // proceed


} // namespace filter
} // namespace precitec
