/*!
 *  @copyright	Precitec Vision GmbH & Co. KG

 *  @author			Simon Hilsenbeck (HS), Stefan Birmanns (SB)
 *  @date			2011, 2016
 *  @file
 *  @brief			Fliplib filter 'IntensityProfileLine' in component 'Filter_SeamSearch'. Calculates a grey level profile around a laser line.
 */

#include "intensityProfileLine.h"

#include <system/platform.h>					///< global and platform specific defines
#include <system/tools.h>						///< poco bugcheck
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"
#include <fliplib/TypeToDataTypeImpl.h>


namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {
	using fliplib::SynchronePipe;
	using fliplib::PipeEventArgs;
	using fliplib::PipeGroupEventArgs;
	using fliplib::Parameter;

const std::string IntensityProfileLine::m_oFilterName 	= std::string("IntensityProfileLine");
const std::string IntensityProfileLine::m_oPipeOut1Name	= std::string("DataOut");


IntensityProfileLine::IntensityProfileLine() : TransformFilter( IntensityProfileLine::m_oFilterName, Poco::UUID{"EC4DA03A-045C-461B-B993-4D7844908B00"} ),
	m_pPipeInImageFrame		( nullptr ),
	m_pPipeInLaserLine		( nullptr ),
	m_oPipeOutProfile		( this, m_oPipeOut1Name ),
	m_pFrameIn				( nullptr ),
	m_oDistance				( 10 ),
	m_oThickness			( 20 ),
	m_oResX					( 1 ),
	m_oResY					( 1 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("Distance", 	Parameter::TYPE_int, m_oDistance);
	parameters_.add("Thickness", 	Parameter::TYPE_int, m_oThickness);
	parameters_.add("ResolutionX", 	Parameter::TYPE_int, m_oResX);
	parameters_.add("ResolutionY",	Parameter::TYPE_int, m_oResY);

    setInPipeConnectors({{Poco::UUID("113DC2B0-1569-4ECE-B5C1-C71BEC89DF19"), m_pPipeInImageFrame, "ImageIn", 1, "image"},
    {Poco::UUID("64C96032-B862-4AA7-8A3B-4FCE8FA466B8"), m_pPipeInLaserLine, "LineIn", 1, "line"}});
    setOutPipeConnectors({{Poco::UUID("AFD78975-6F82-47D3-945D-6A0DD01A06D0"), &m_oPipeOutProfile, m_oPipeOut1Name, 0, "profile"}});
    setVariantID(Poco::UUID("4570DD89-F0A0-409A-926F-E0274894168B"));
} // IntensityProfileLine



void IntensityProfileLine::setParameter()
{
	TransformFilter::setParameter();
	m_oDistance		= parameters_.getParameter("Distance").convert<int>();
	m_oThickness	= parameters_.getParameter("Thickness").convert<int>();
	m_oResX			= parameters_.getParameter("ResolutionX").convert<int>();
	m_oResY			= parameters_.getParameter("ResolutionY").convert<int>();

	poco_assert_dbg(m_oThickness >= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oResX		 >= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oResY		 >= 1);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

} // setParameter


void IntensityProfileLine::paint()
{
	if(m_oVerbosity <= eNone || m_oSpTrafo.isNull()) {
		return;
	} // if

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
	OverlayLayer	&rLayerLine			( rCanvas.getLayerLine());

	// draw the areas of the grayscale image, where the profiles are being calculated

	const auto	& rLaserLineY	( m_oLineIn.front().getData() );
    static const auto oColor = Color(0x1E90FF); // dodgerblue
    rLayerContour.add<OverlayPointList>( geo2d::Point{rTrafo.dx(), rTrafo.dy() - m_oDistance }, rLaserLineY, oColor) ;
    rLayerContour.add<OverlayPointList>( geo2d::Point{rTrafo.dx(), rTrafo.dy() + m_oDistance }, rLaserLineY, oColor) ;
    rLayerContour.add<OverlayPointList>( geo2d::Point{rTrafo.dx(), rTrafo.dy() - m_oDistance - m_oThickness}, rLaserLineY, oColor) ;
    rLayerContour.add<OverlayPointList>( geo2d::Point{rTrafo.dx(), rTrafo.dy() - m_oDistance + m_oThickness}, rLaserLineY, oColor) ;


	// draw the resulting profile

	const auto	& rProfile	( m_oProfileOut.front().getData() );
	for (unsigned int i = 1; i < rProfile.size(); ++i) {
		rLayerLine.add<OverlayLine>( rTrafo(Point(i-1, 255 - int( rProfile[i-1] ))), rTrafo(Point(i, 255 - int( rProfile[i] ))), Color::Green() ) ;
	} // for


} // paint


bool IntensityProfileLine::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "image" )
	{
		m_pPipeInImageFrame	= dynamic_cast<image_pipe_t*>(&p_rPipe);
	}

	if ( p_rPipe.tag() == "line" )
	{
		m_pPipeInLaserLine  = dynamic_cast<line_pipe_t*>(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void IntensityProfileLine::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
{
	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInLaserLine 	!= nullptr); // to be asserted by graph editor

	// get data from frame
	m_pFrameIn							= &( m_pPipeInImageFrame->read(m_oCounter) );
	const BImage &rImageIn				= m_pFrameIn->data();
	// get data from laser line
	const GeoVecDoublearray& rLineIn 	= m_pPipeInLaserLine->read(m_oCounter);
	const VecDoublearray& rLineArray 	= rLineIn.ref();
	m_oLineIn							= rLineIn.ref();
	m_oSpTrafo							= m_pFrameIn->context().trafo();

	// (re)initialization of output structure
	reinitialize(rImageIn);

	// input validity check
	const bool oResolutionTooHigh	( [&, this]()->bool{
		if (m_oResX >= rImageIn.size().width / 10 || m_oResY >= rImageIn.size().height / 10) {
			wmLog(eDebug, "Resolution parameter is set too high.\n");
			return true;
		}
		return false; }() );

	if ( inputIsInvalid(rImageIn) || oResolutionTooHigh )
	{
		const GeoVecDoublearray	oGeoProfileOut( m_pFrameIn->context(), m_oProfileOut, m_pFrameIn->analysisResult(), 0.0 ); // bad rank
		preSignalAction();
		m_oPipeOutProfile.signal( oGeoProfileOut );			// invoke linked filter(s)
		return; // RETURN
	}

	// ok, input is valid, now lets calculate the profile and signal it out ...
	calcIntensityProfile( rImageIn, rLineArray, m_oDistance, m_oThickness, m_oResX, m_oResY, m_oProfileOut ); // image processing

	const GeoVecDoublearray		oGeoProfileOut		( m_pFrameIn->context(), m_oProfileOut, m_pFrameIn->analysisResult(),  1.0 ); // full rank, detailed rank in Profile
	preSignalAction();
	m_oPipeOutProfile.signal( oGeoProfileOut );			// invoke linked filter(s)

} // proceed



bool IntensityProfileLine::inputIsInvalid(const BImage &p_rImageIn)
{
	const bool imgIsInvalid = ! p_rImageIn.isValid();
	if (imgIsInvalid) {
		wmLog(eDebug, "Input image invalid.\n");
	}

	return imgIsInvalid;
} // inputIsInvalid



void IntensityProfileLine::reinitialize(const BImage &p_rImageIn)
{
	const int oProfileOutWidth = p_rImageIn.size().width / m_oResX;
	m_oProfileOut.assign( 1, Doublearray( oProfileOutWidth ) );

} // reinitialize



/*static*/ void IntensityProfileLine::calcIntensityProfile(
	const BImage 			&p_rImageIn,
	const VecDoublearray 	&p_rLaserLineIn,
	unsigned int			p_oDistance,
	unsigned int			p_oThickness,
	unsigned int 			p_oResX,
	unsigned int 			p_oResY,
	VecDoublearray 			&p_rProfileOut
)
{
	poco_assert_dbg(p_oThickness > 0);	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	auto &rProfileVectorOut 	= p_rProfileOut[0].getData();
	auto &rRankVectorOut 		= p_rProfileOut[0].getRank();

	const auto& rLaserLineIn_Data = p_rLaserLineIn[0].getData();
	const auto& rLaserLineIn_Rank = p_rLaserLineIn[0].getRank();

	const unsigned int oImgHeight 	= p_rImageIn.size().height;

	// todo: check if oImgWidth and rLaserLineIn_Data.size() are equal. Somebody might have connected the filter to a different ROI as the line-tracker ...

	// loop over laser line
	int	oXOut = 0;
	int oSum = 0;
	for (unsigned int oX = 0; oX < rLaserLineIn_Data.size(); oX += p_oResX)
	{
		oSum = 0;

		// skip column if laser line is not valid at this point
		if ( rLaserLineIn_Rank[oX] < eRankMax )
		{
			rProfileVectorOut[oXOut] = 0;
			rRankVectorOut[oXOut] = eRankMin;
		}
		else
		{
			// loop over some lines, depending on thickness
			for (unsigned int oY = (unsigned int)(rLaserLineIn_Data[oX]) + p_oDistance; oY < (unsigned int)(rLaserLineIn_Data[oX]) + p_oDistance + p_oThickness; oY += p_oResY)
			{
				if ( oY < oImgHeight ) { oSum += p_rImageIn[oY][oX]; }
				//std::cout << "p_rImageIn" << '(' << y << ',' << x << "): " << (int)p_rImageIn[y][x] << std::endl;
			} // for

			for (unsigned int oY = (unsigned int)(rLaserLineIn_Data[oX]) - p_oDistance - p_oThickness; oY < (unsigned int)(rLaserLineIn_Data[oX]) - p_oDistance; oY += p_oResY)
			{
				if ( oY < oImgHeight ) { oSum += p_rImageIn[oY][oX]; }
				//std::cout << "p_rImageIn" << '(' << y << ',' << x << "): " << (int)p_rImageIn[y][x] << std::endl;
			} // for

			// rProfileVectorOut ist Array mit den Spaltenmittelwerten
			rProfileVectorOut[oXOut] = oSum / (2*p_oThickness);
			//std::cout << "\nrProfileVectorOut[x] " << rProfileVectorOut[x] << std::endl;
			rRankVectorOut[oXOut] = eRankMax;

		} // if rank

		++oXOut;
	} // for
	//std::cout << "\nrProfileVectorOut " << p_rProfileOut[sliceN] << std::endl;

} // calcIntensityProfile



} // namespace filter
} // namespace precitec
