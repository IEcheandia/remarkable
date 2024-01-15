/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2015
 * 	@brief 		This filter detects a step in the laser-line.
 */

// clib includes
#define _USE_MATH_DEFINES
#include <cmath>
// framework includes
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include <module/moduleLogger.h>
#include "util/calibDataSingleton.h"
#include <fliplib/TypeToDataTypeImpl.h>

// local includes
#include "lineStep.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineStep::m_oFilterName 	= std::string("LineStep");
const std::string LineStep::PIPENAME_OUT1	= std::string("PositionX");
const std::string LineStep::PIPENAME_OUT2	= std::string("PositionY");


LineStep::LineStep() :
	TransformFilter( LineStep::m_oFilterName, Poco::UUID{"21D7C995-05D4-4187-BCC0-7B8BEAA7F9AF"} ),
	m_pPipeInLaserLine( nullptr ),
	m_pPipeInLaserGradient( nullptr ),
	m_oPipePositionXOut	( this, LineStep::PIPENAME_OUT1 ),
	m_oPipePositionYOut	( this, LineStep::PIPENAME_OUT2 ),
	m_oMinLengthLeft( 2.0 ),
	m_oMinLengthRight( 2.0 ),
	m_oMinHeight( 1.0 ),
	m_oMaxHeight( 30.0 ),
	m_oMaxDistance( 20 ),
	m_oGradientThreshold( 1. ),
	m_oMaxAngle( 17.0 ),
	m_oSelectedLeft(-1),
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine)
{
	// Set default values of the parameters of the filter
	parameters_.add("MinLengthLeft",	Parameter::TYPE_double, m_oMinLengthLeft);
	parameters_.add("MinLengthRight",	Parameter::TYPE_double, m_oMinLengthRight);
	parameters_.add("MinHeight",		Parameter::TYPE_double, m_oMinHeight);
	parameters_.add("MaxHeight",		Parameter::TYPE_double, m_oMaxHeight);
	parameters_.add("MaxDistance",		Parameter::TYPE_uint, 	m_oMaxDistance);
	parameters_.add("GradientThreshold",Parameter::TYPE_double, m_oGradientThreshold);
	parameters_.add("MaxAngle",			Parameter::TYPE_double, m_oMaxAngle);
	parameters_.add("TypeOfLaserLine", Parameter::TYPE_int, int(m_oTypeOfLaserLine));

    setInPipeConnectors({{Poco::UUID("1BF39DBD-08D8-45FB-B5E0-767C5C662E48"), m_pPipeInLaserLine, "LaserLine", 1, "LaserLine"},
    {Poco::UUID("50CA78DF-F29B-4E03-9869-934777270E26"), m_pPipeInLaserGradient, "Gradient", 1, "Gradient"}});
    setOutPipeConnectors({{Poco::UUID("7A3CD94A-1CA7-4A9D-A4FE-9AEB64DF4922"), &m_oPipePositionXOut, PIPENAME_OUT1, 0, ""},
    {Poco::UUID("0CBD4DE0-6DA5-4CF9-818B-1F765B28EB5F"), &m_oPipePositionYOut, PIPENAME_OUT2, 0, ""}});
    setVariantID(Poco::UUID("C897DE44-6B44-456D-80B1-B4E99371C354"));
} // CTor.



LineStep::~LineStep()
{
} // DTor.



void LineStep::setParameter()
{
	TransformFilter::setParameter();

	m_oMinLengthLeft		= parameters_.getParameter("MinLengthLeft").convert<double>();
	m_oMinLengthRight		= parameters_.getParameter("MinLengthRight").convert<double>();
	m_oMinHeight			= parameters_.getParameter("MinHeight").convert<double>();
	m_oMaxHeight			= parameters_.getParameter("MaxHeight").convert<double>();
	m_oMaxDistance			= parameters_.getParameter("MaxDistance").convert<double>();
	m_oGradientThreshold	= parameters_.getParameter("GradientThreshold").convert<double>();
	m_oMaxAngle				= parameters_.getParameter("MaxAngle").convert<double>();
	m_oTypeOfLaserLine = castToLaserLine(parameters_.getParameter("TypeOfLaserLine").convert<int>());

} // setParameter



bool LineStep::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "LaserLine" )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "Gradient" )
		m_pPipeInLaserGradient  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void LineStep::paint()
{
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo		( *m_oSpTrafo );
	OverlayCanvas	&rCanvas	( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayer		( rCanvas.getLayerPosition());

	if ( !inputIsInvalid(m_oOut) )
	{
		// paint output positions
		for ( unsigned int oIndex = 0; oIndex < m_oOut.size(); ++oIndex ) {
			rLayer.add( new	OverlayCross(rTrafo(m_oOut.getData()[oIndex]), Color::Cyan()));
		} // for
	} // if

	if( m_oVerbosity == eMax && m_oSelectedLeft != -1 )
	{
		char oBuffer[10];
		geo2d::Rect oRectHeight( Point( std::get<eStepX>(m_oStep)-10, std::get<eStepY>(m_oStep)+40 ), Point( std::get<eStepX>(m_oStep)+90, std::get<eStepY>(m_oStep)+20 ) );
		sprintf(&(oBuffer[0]), "%.2f", std::get<eStepHeight>(m_oStep));
		rLayer.add( new OverlayText(
				&(oBuffer[0]),
				Font( 8 ),
				rTrafo(oRectHeight),
				Color::Green() )
		);
	}

	if( m_oVerbosity >= eMedium )
	{
		// paint the segments
		for ( int oIndex=0; oIndex < int(m_oSegments.size()); ++oIndex )
		{
			bool oSelected = (m_oSelectedLeft != -1) &&		// was the step found at all?
							 (m_oSelectedLeft == oIndex ||  // is the current segment the left segment?
						      m_oSelectedLeft == oIndex-1); // is the current segment the right segment?

			// render the segments as lines
			Rect oStart( std::get<eStart>(m_oSegments[oIndex]).x - 2, std::get<eStart>(m_oSegments[oIndex]).y - 2, 5, 5 );
			rLayer.add( new OverlayRectangle(
					rTrafo(oStart),
					oSelected ? Color::Green() : Color::White() )
			);
			rLayer.add( new OverlayLine(
					rTrafo(std::get<eStart>(m_oSegments[oIndex])),
					rTrafo(std::get<eEnd>  (m_oSegments[oIndex])),
					oSelected ? Color::Green() : Color::White() )
			);
			Rect oEnd( std::get<eEnd>(m_oSegments[oIndex]).x - 2, std::get<eEnd>(m_oSegments[oIndex]).y - 2, 5, 5 );
			rLayer.add( new OverlayRectangle(
					rTrafo(oEnd),
					oSelected ? Color::Green() : Color::White() )
			);

			int oTextX = (std::get<eStart>(m_oSegments[oIndex]).x + std::get<eEnd>(m_oSegments[oIndex]).x) / 2;
			int oTextY = (std::get<eStart>(m_oSegments[oIndex]).y + std::get<eEnd>(m_oSegments[oIndex]).y) / 2;
			char oBuffer[10];

			// output text with the length of the segments
			if( m_oVerbosity >= eHigh )
			{
				geo2d::Rect oRectLength( Point( oTextX - 10, oTextY ), Point( oTextX + 90, oTextY - 20 ) );
				sprintf(&(oBuffer[0]), "%.2f", std::get<eLength>(m_oSegments[oIndex]));
				rLayer.add( new OverlayText(
						&(oBuffer[0]),
						Font(oSelected ? 12 : 8 ),
						rTrafo(oRectLength),
						oSelected ? Color::Green() : Color::White() )
				);
			} // if

			// output text with the angles of the segments
			if( m_oVerbosity == eMax )
			{
				geo2d::Rect oRectAngle( Point( oTextX - 10, oTextY+30 ), Point( oTextX + 90, oTextY+10 ) );
				sprintf(&(oBuffer[0]), "%.2f", std::get<eSlope>(m_oSegments[oIndex]));
				rLayer.add( new OverlayText(
						&(oBuffer[0]),
						Font(oSelected ? 12 : 8 ),
						rTrafo(oRectAngle),
						oSelected ? Color::Green() : Color::White() )
				);
			} // if
		} // for
	} // if

} // paint



void LineStep::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); 	// to be asserted by graph editor
	poco_assert_dbg(m_pPipeInLaserGradient != nullptr); // to be asserted by graph editor

	// Read-out laserline and gradient
	const GeoVecDoublearray& rLaserLine 	= m_pPipeInLaserLine->read(m_oCounter);
	const GeoVecDoublearray& rLaserGradient = m_pPipeInLaserGradient->read(m_oCounter);
	m_oSpTrafo	= rLaserLine.context().trafo();
	m_pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, rLaserLine.context(), m_oTypeOfLaserLine);

	// Extract byte-array
	const VecDoublearray& rLaserLineArray 		= rLaserLine.ref();
	const VecDoublearray& rLaserGradientArray 	= rLaserGradient.ref();
	// Reset output based on input size
	m_oOut.assign(rLaserLineArray.size(), Point(0, 0), eRankMin);
	// Input validity check
	if ( inputIsInvalid(rLaserLine) || inputIsInvalid(rLaserGradient) )
	{
		const GeoDoublearray oGeoDoublearrayXOut(rLaserLine.context(), getCoordinate(m_oOut, eX), rLaserLine.analysisResult(), interface::NotPresent); // bad rank
		const GeoDoublearray oGeoDoublearrayYOut(rLaserLine.context(), getCoordinate(m_oOut, eY), rLaserLine.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction();
		m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
		m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

		return; // RETURN
	} // if

	// Now do the actual image processing
	findSteps( rLaserLineArray, rLaserGradientArray, m_oOut );

	// Create a new point, put the global context into the resulting profile and copy the rank over
	auto oAnalysisResult = rLaserLine.analysisResult(); if ( rLaserGradient.analysisResult() != AnalysisOK ) { oAnalysisResult = rLaserGradient.analysisResult(); }
	const GeoDoublearray oGeoDoublearrayXOut(rLaserLine.context(), getCoordinate(m_oOut, eX), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	const GeoDoublearray oGeoDoublearrayYOut(rLaserLine.context(), getCoordinate(m_oOut, eY), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	preSignalAction();
	m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
	m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

} // proceedGroup



LineStep::Step LineStep::findStep( const geo2d::Doublearray &p_rLineIn, const geo2d::Doublearray &p_rGradientIn )
{

	Step oStep 			= std::make_tuple( 0, 1, eRankMin, 1. );
	Segment oSegment 	= std::make_tuple<Point,Point,int>(Point(0,0),Point(0,0),0.,0.);
	auto& rGradData 	= p_rGradientIn.getData();
	auto& rGradRank 	= p_rGradientIn.getRank();
	auto& rLineData 	= p_rLineIn.getData();
	bool oSegmentOpen 	= false;
	unsigned int oBadRankCount = 0;
	bool oFirst 		= true;
	int oLastValid 		= 0;
	m_oSelectedLeft		= -1;
	Point oPoint;
	m_oSegments.clear();

	// variables, could potentially be converted to adjustable parameters
	unsigned int oThreshBadRank 	= 5;
	int oThreshLength	= 10;
	double oThreshAngle = std::tan( (m_oMaxAngle * M_PI) / 180.0 );
	wmLog( eInfo, "ThreshAngle: %f\n", oThreshAngle );

	// get calibration data
	assert(m_pCoordTransformer && "m_pCoordTransformer called before it was set in ProceedGroup");

	// lets get all straight line segments of at least a minimal length...
	for( unsigned int oIndex = 0; oIndex < rGradData.size(); ++oIndex )
	{
		// bad rank counter
		if ( rGradRank[oIndex] > eRankMin ) { oBadRankCount = 0; } else { oBadRankCount++; }

		// start of segment?
		if ( (std::abs( rGradData[oIndex] ) < m_oGradientThreshold || oFirst ) && rGradRank[oIndex] > eRankMin && !oSegmentOpen )
		{
			std::get<eStart>(oSegment) = Point( oIndex , rLineData[oIndex]);
			oSegmentOpen = true;
			oFirst = false;
		}
		// end of segment?
		else if ( ( ( std::abs( rGradData[oIndex] ) > m_oGradientThreshold && rGradRank[oIndex] > eRankMin ) || oBadRankCount > oThreshBadRank) && oSegmentOpen )
		{
			std::get<eEnd>(oSegment) = Point( oIndex-oBadRankCount , rLineData[oIndex-oBadRankCount]);
			if ( std::get<eEnd>(oSegment).x - std::get<eStart>(oSegment).x > oThreshLength ) { m_oSegments.push_back( oSegment ); } // if segment is too small, do not add it to the final list
			oSegmentOpen = false;
		} // if

		if ( rGradRank[oIndex] > eRankMin ) { oLastValid = oIndex; }
	} // for

	// is the last segment still open?
	if ( oSegmentOpen )
	{
		std::get<eEnd>(oSegment) = Point( oLastValid , rLineData[oLastValid]);
		std::get<eLength>(oSegment) = oLastValid - std::get<eStart>(oSegment).x;
		if ( std::get<eLength>(oSegment) > oThreshLength ) { m_oSegments.push_back( oSegment ); } // if segment is too small, do not add it to the final list
	} // if

	// find segment, where length + length > min and end - start not too big ...
	if ( m_oSegments.size() > 0 ) for( unsigned int oIndex = 0; oIndex < m_oSegments.size()-1; ++oIndex )
	{
		// transform 2d image coords into 3d coords
		math::Vec3D oCoord3D_0 = m_pCoordTransformer->imageCoordTo3D( std::get<eStart>( m_oSegments[oIndex  ] ).x, std::get<eStart>( m_oSegments[oIndex  ] ).y);
		math::Vec3D oCoord3D_1 = m_pCoordTransformer->imageCoordTo3D( std::get<eEnd>  ( m_oSegments[oIndex  ] ).x, std::get<eEnd>  ( m_oSegments[oIndex  ] ).y);
		math::Vec3D oCoord3D_2 = m_pCoordTransformer->imageCoordTo3D( std::get<eStart>( m_oSegments[oIndex+1] ).x, std::get<eStart>( m_oSegments[oIndex+1] ).y);
		math::Vec3D oCoord3D_3 = m_pCoordTransformer->imageCoordTo3D( std::get<eEnd>  ( m_oSegments[oIndex+1] ).x, std::get<eEnd>  ( m_oSegments[oIndex+1] ).y);

		// compute step height
		double oStepHeight = std::abs( std::get<eStart>( m_oSegments[oIndex+1] ).y - std::get<eEnd>( m_oSegments[oIndex  ] ).y );
		// compute slope of left and right segment
		std::get<eSlope>(m_oSegments[oIndex  ]) = ( std::get<eStart>( m_oSegments[oIndex  ] ).y - std::get<eEnd>( m_oSegments[oIndex  ] ).y ) / double( std::get<eEnd>( m_oSegments[oIndex  ] ).x - std::get<eStart>( m_oSegments[oIndex  ] ).x );
		std::get<eSlope>(m_oSegments[oIndex+1]) = ( std::get<eStart>( m_oSegments[oIndex+1] ).y - std::get<eEnd>( m_oSegments[oIndex+1] ).y ) / double( std::get<eEnd>( m_oSegments[oIndex+1] ).x - std::get<eStart>( m_oSegments[oIndex+1] ).x );
		// compute real-world length of segments
		std::get<eLength>(m_oSegments[oIndex  ]) = std::sqrt( oCoord3D_1.dist2( oCoord3D_0 ) );
		std::get<eLength>(m_oSegments[oIndex+1]) = std::sqrt( oCoord3D_3.dist2( oCoord3D_2 ) );

		// ok, now lets find the actual step...
		if (
				// does the current and next segment meet the minimum length criterium?
				( std::get<eLength>( m_oSegments[oIndex] ) > m_oMinLengthLeft && std::get<eLength>( m_oSegments[oIndex+1] ) > m_oMinLengthRight ) &&
				// is the gap between the two segments within the tolerable limits?
				( std::get<eStart>( m_oSegments[oIndex+1] ).x - std::get<eEnd>( m_oSegments[oIndex  ] ).x < int(m_oMaxDistance) ) &&
				// is the height of the step within the limits?
				( oStepHeight < m_oMaxHeight && oStepHeight > m_oMinHeight ) &&
				// is the angle difference tolerable
				( std::abs( std::get<eSlope>(m_oSegments[oIndex]) - std::get<eSlope>(m_oSegments[oIndex+1]) ) < oThreshAngle )
			)
		{
			// ok, this seems to be the correct step position. Now compute the mid-point between the end of the first segment and the start of the next.
			oStep = std::make_tuple(
					( std::get<eEnd>( m_oSegments[oIndex] ).x + std::get<eStart>( m_oSegments[oIndex+1] ).x ) * 0.5,
					( std::get<eEnd>( m_oSegments[oIndex] ).y + std::get<eStart>( m_oSegments[oIndex+1] ).y ) * 0.5,
					eRankMax,
					oStepHeight
					);
			m_oSelectedLeft = oIndex;
		} // if
	} // if

	return	oStep;

} // findStep



void LineStep::findSteps( const geo2d::VecDoublearray &p_rLineIn, const geo2d::VecDoublearray &p_rGradientIn, geo2d::Pointarray &p_rPositionOut )
{
	if ( p_rLineIn.size() != p_rGradientIn.size() ) { return; } // todo: error handling, should have the same amount of laser lines and gradients
	const unsigned int oNbLines	= p_rLineIn.size();

	// if the size of the output signal is not equal to the input line size, resize
	p_rPositionOut.assign(oNbLines, Point(0, 0));
	// loop over N lines
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{
		const Doublearray	&rLineIn			= p_rLineIn[lineN];
		const Doublearray	&rGradientIn		= p_rGradientIn[lineN];
		Point				&rPositionOut		= p_rPositionOut.getData()[lineN];
		int					&rPositionOutRank	= p_rPositionOut.getRank()[lineN];

		m_oStep				= findStep( rLineIn, rGradientIn );
		rPositionOut.x		= std::get<eStepX>( m_oStep );
		rPositionOut.y		= std::get<eStepY>( m_oStep );
		rPositionOutRank	= std::get<eStepRank>( m_oStep );
	} // for

} // findSteps


} // namespace precitec
} // namespace filter
