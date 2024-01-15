/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		SB
 * 	@date		2013
 * 	@brief		Filter that checks if image contains a signal and sends an NIO if not.
 */

// WM includes
#include "blackImageCheck.h"
#include <fliplib/PipeEventArgs.h>
#include <fliplib/Parameter.h>
#include <image/image.h>
#include <module/moduleLogger.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
using namespace interface;
using namespace image;
using namespace geo2d;
namespace filter {


const std::string BlackImageCheck::m_oFilterName 		( "BlackImageCheck" );
const std::string BlackImageCheck::m_oPipeResultName 	( "ImageIO" );			///< Pipe: Result out-pipe.
const std::string BlackImageCheck::m_oParamThreshold	( "Threshold" );			///< Parameter: Threshold below which a pixel is not part of the signal.
const std::string BlackImageCheck::m_oParamResultType	( "ResultType" );			///< Parameter: User-defined nio type.
const std::string BlackImageCheck::m_oParamNioType 		( "NioType" );			///< Parameter: User-defined nio type.


BlackImageCheck::BlackImageCheck() :
	ResultFilter			( BlackImageCheck::m_oFilterName, Poco::UUID{"C996BD2C-EA89-462D-A335-95BFE8298285"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_oPipeResult			( this, BlackImageCheck::m_oPipeResultName ),
	m_oThreshold			( 20 ),
	m_oUserResultType		( Value ),
	m_oUserNioType			( ValueOutOfLimits )
{
	parameters_.add( m_oParamThreshold,		fliplib::Parameter::TYPE_int,	m_oThreshold );
	parameters_.add( m_oParamResultType, 	fliplib::Parameter::TYPE_int,	static_cast<int>( m_oUserResultType ) );
	parameters_.add( m_oParamNioType, 		fliplib::Parameter::TYPE_int,	static_cast<int>( m_oUserNioType ) );

    setInPipeConnectors({{Poco::UUID("E858CE21-5580-4BAF-BCBC-8738152BB15C"), m_pPipeInImageFrame, "BlackImageCheckImageIn", 0, ""}});
    setVariantID(Poco::UUID("D65500D0-3FAB-4913-990E-DEAFACA1F300"));
} // CTor.



BlackImageCheck::~BlackImageCheck()
{

} // DTor.



void BlackImageCheck::setParameter()
{
	ResultFilter::setParameter();

	m_oThreshold 		=             parameters_.getParameter( BlackImageCheck::m_oParamThreshold ).convert<int>();
	m_oUserResultType 	= ResultType( parameters_.getParameter( BlackImageCheck::m_oParamResultType ).convert<int>() );
	m_oUserNioType 		= ResultType( parameters_.getParameter( BlackImageCheck::m_oParamNioType ).convert<int>() );
} // setParameter.



bool BlackImageCheck::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame = dynamic_cast< fliplib::SynchronePipe < ImageFrame > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void BlackImageCheck::proceed( const void* p_pSender, fliplib::PipeEventArgs& p_rEventArgs )
{
	poco_assert_dbg( m_pPipeInImageFrame != nullptr );

	// Read frame
	const ImageFrame&	rFrame		( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage&		rImage		( rFrame.data() );
	const int			oImgWidth	( rImage.width() );

	// Loop through frame and count occupied pixels
	unsigned int oCounter = 0;
	unsigned long int oSum = 0;
	for ( int y = 0; y < rImage.size().height; ++y )
	{
		const byte* pLine = rImage[y];

		for ( int x = 0; x < oImgWidth; ++x )
		{
			if ( pLine[x] > m_oThreshold )
				++oCounter;

			oSum += pLine[x];

		} // for
	} // for

	// signal result
	double oAvg = 0.0;
	if ( rImage.size().height * oImgWidth != 0 )
		oAvg = (double)(oSum) / ( rImage.size().height * oImgWidth );

	const auto				oResultValue		= Doublearray{1, oAvg, eRankMax};
	const GeoDoublearray	oGeoValueOut		( rFrame.context(), oResultValue, rFrame.analysisResult(), 1.0 );
	ResultDoubleArray 		oResultDoubleOut	( id(), m_oUserResultType, m_oUserNioType, rFrame.context(), oGeoValueOut, TRange<double>( 0, 10000 ) );
	if ( oCounter <= 30 )
	{
		oResultDoubleOut.setNio(true);
	}
    preSignalAction();
	m_oPipeResult.signal( oResultDoubleOut );
} // proceed


} // namespace filter
} // namespace precitec
