/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Stefan Birmanns (SB)
 * 	@date		2015
 * 	@brief 		This filter detects a step in the laser-line.
 */

// framework includes
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include <module/moduleLogger.h>
#include <math/calibration3DCoords.h>
#include "util/calibDataSingleton.h"

#include <fliplib/TypeToDataTypeImpl.h>

// clib
#define _USE_MATH_DEFINES
#include <math.h>
// local includes
#include "step.h"

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string Step::m_oFilterName 	= std::string("Step");
const std::string Step::PIPENAME_OUT1	= std::string("PositionX");
const std::string Step::PIPENAME_OUT2	= std::string("PositionY");


Step::Step() :
	TransformFilter( Step::m_oFilterName, Poco::UUID{"8689A79F-2DC4-4E17-9376-852A30588E4F"} ),
	m_pPipeInLaserLine( nullptr ),
	m_oPipePositionXOut	( this, Step::PIPENAME_OUT1 ),
	m_oPipePositionYOut	( this, Step::PIPENAME_OUT2 ),
	m_oLeft( true ),
	m_oHeight( 1.5 ),
	m_oMaxHeight( 5.0 ),
	m_oLengthLeft( 20.0 ),
	m_oLengthRight( 20.0 ),
	m_oAngle( 10.0 ),
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine)
{
	// Set default values of the parameters of the filter
	parameters_.add("LengthLeft",	Parameter::TYPE_double, m_oLengthLeft);
	parameters_.add("LengthRight",	Parameter::TYPE_double, m_oLengthRight);
	parameters_.add("MinHeight",		Parameter::TYPE_double, m_oMinHeight);
	parameters_.add("MaxHeight",		Parameter::TYPE_double, m_oMaxHeight);
	parameters_.add("Height",			Parameter::TYPE_double,	m_oHeight);
	parameters_.add("Angle",			Parameter::TYPE_double,	m_oAngle);
	parameters_.add("Left",				Parameter::TYPE_bool, 	m_oLeft);
	parameters_.add("TypeOfLaserLine", Parameter::TYPE_int, int(m_oTypeOfLaserLine));
	// todo: add direction parameter, to adjust search direction

    setInPipeConnectors({{Poco::UUID("18DD4900-8D98-4232-A539-1E70B8BB49D7"), m_pPipeInLaserLine, "LaserLine", 1, "LaserLine"}});
    setOutPipeConnectors({{Poco::UUID("4BF0E708-6CC8-402A-98EE-6EF8746352E7"), &m_oPipePositionXOut, "PositionX", 0, ""},
    {Poco::UUID("DD2304D8-A747-47F8-803E-E18573CAEAAB"), &m_oPipePositionYOut, "PositionY" , 0, ""}});
    setVariantID(Poco::UUID("1AD6B0AE-60BC-47CB-BDC7-B0D4232472C3"));
} // CTor.



Step::~Step()
{
} // DTor.



void Step::setParameter()
{
	TransformFilter::setParameter();

	// retrieve parameters
	m_oLeft					= parameters_.getParameter("Left").convert<bool>();
	m_oLengthLeft			= parameters_.getParameter("LengthLeft").convert<double>();
	m_oLengthRight			= parameters_.getParameter("LengthRight").convert<double>();
	m_oMinHeight			= parameters_.getParameter("MinHeight").convert<double>();
	m_oMaxHeight			= parameters_.getParameter("MaxHeight").convert<double>();
	m_oHeight				= parameters_.getParameter("Height").convert<double>();
	double oAngleDegrees	= parameters_.getParameter("Angle").convert<double>();
	m_oAngle 				= M_PI / 180.0 * oAngleDegrees;
	m_oTypeOfLaserLine = castToLaserLine(parameters_.getParameter("TypeOfLaserLine").convert<int>());

	// height
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
	auto oFact = rCalib.factorVertical(100, 512, 512, m_oTypeOfLaserLine);
	auto oFeatureHeight 	= (unsigned int)(m_oHeight * oFact);

	// generate template
	m_oFeature.clear();
	auto oTanAlpha 			= (double)(std::tan(m_oAngle));
	m_oFeatureCenter 		= oFeatureHeight * std::fabs( std::sin( m_oAngle ) ) * 2.0 + m_oLengthLeft;
	m_oFeatureLeft			= m_oLengthLeft - 1;
	m_oFeatureRight			= m_oFeatureCenter + 1;

	// left portion
	for (int oIndex = 0; oIndex < m_oLengthLeft; ++oIndex) {
		if ( m_oLeft ) {
			m_oFeature.push_back( ( oIndex * oTanAlpha ) - (oFeatureHeight * 0.5) );
		} else {
			m_oFeature.push_back( ( oIndex * oTanAlpha ) + (oFeatureHeight * 0.5) );
		} // if
	} // for
	// middle portion
	for (unsigned int oIndex = m_oLengthLeft; oIndex < m_oFeatureCenter; ++oIndex) {
		m_oFeature.push_back( 0.0 );
	} // for
	// right portion
	for (int oIndex = m_oFeatureCenter; oIndex < m_oFeatureCenter + m_oLengthRight; ++oIndex) {
		if ( m_oLeft ) {
			m_oFeature.push_back( ( oIndex * oTanAlpha ) + (oFeatureHeight * 0.5));
		} else {
			m_oFeature.push_back( ( oIndex * oTanAlpha ) - (oFeatureHeight * 0.5));
		} // if
	} // for

	// compute center correctly
	m_oFeatureCenter 		= (m_oFeatureCenter - m_oLengthLeft) * 0.5 + m_oLengthLeft;

} // setParameter



bool Step::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "LaserLine" ) {
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);
	} // if

	return BaseFilter::subscribe( p_rPipe, p_oGroup );

} // subscribe



void Step::paint()
{
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){ return; }

	const Trafo		&rTrafo		( *m_oSpTrafo );
	OverlayCanvas	&rCanvas	( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayer		( rCanvas.getLayerPosition() );

	if ( !inputIsInvalid(m_oOut) )
	{
		// paint output positions
		if( m_oVerbosity > eNone ) {
			for ( unsigned int oIndex = 0; oIndex < m_oOut.size(); ++oIndex ) {
				rLayer.add( new	OverlayCross(rTrafo(m_oOut.getData()[oIndex]), Color::Cyan()));
			} // for
		} // if
	} // if

	// render score and display height
	if( m_oVerbosity >= eMedium ) {

		for ( unsigned int oIndex = 0; oIndex < m_oScores.size(); ++oIndex ) {
			geo2d::Point oPoint( oIndex, roundToT<int>( m_oScores[oIndex] * m_oScale ) );
			rLayer.add( new OverlayPoint(rTrafo(oPoint), std::get<eStepRank>(m_oStep) > eRankMin ? Color::Yellow() : Color::Red()) );
		} // for

		char oBuffer[10];
		geo2d::Rect oRectHeight( Point( std::get<eStepX>(m_oStep)-10, std::get<eStepY>(m_oStep)+40 ), Point( std::get<eStepX>(m_oStep)+90, std::get<eStepY>(m_oStep)+20 ) );
		sprintf(&(oBuffer[0]), "%.2f", std::get<eStepHeight>(m_oStep));
		rLayer.add( new OverlayText(
				&(oBuffer[0]),
				Font( 8 ),
				rTrafo(oRectHeight),
				Color::Green() )
		);
	} // if

	// render feature and candidates
	if( m_oVerbosity == eMax ) {
		for ( unsigned int oIndex = 0; oIndex < m_oFeature.size(); ++oIndex ) {
			if ( m_oFeature[oIndex] != 0.0 ) {
				geo2d::Point oPoint( oIndex, roundToT<int>( 20 + m_oFeature[oIndex]) );
				rLayer.add( new OverlayPoint(rTrafo(oPoint), Color::Green()) );
			} // if
		} // for
		for ( unsigned int oIndex = 0; oIndex < m_oCandidates.size(); ++oIndex ) {
			geo2d::Point oPoint( std::get<eStepX>(m_oCandidates[oIndex]), std::get<eStepY>(m_oCandidates[oIndex]));
			rLayer.add( new	OverlayCross(rTrafo(oPoint), Color::Yellow()));
		} // for
	} // if

} // paint



void Step::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); 	// to be asserted by graph editor

	// Read-out laserline
	const GeoVecDoublearray& rLaserLine	= m_pPipeInLaserLine->read(m_oCounter);
	m_oSpTrafo	= rLaserLine.context().trafo();
	m_pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, rLaserLine.context(), m_oTypeOfLaserLine);

	// Extract byte-array
	const VecDoublearray& rLaserLineArray = rLaserLine.ref();

	// Input validity check
	if ( inputIsInvalid(rLaserLine) )
	{
        // Reset output based on input size
        m_oOut.assign(rLaserLineArray.size(), Point(0, 0), eRankMin);
		const GeoDoublearray oGeoDoublearrayXOut(rLaserLine.context(), getCoordinate(m_oOut, eX), rLaserLine.analysisResult(), interface::NotPresent); // bad rank
		const GeoDoublearray oGeoDoublearrayYOut(rLaserLine.context(), getCoordinate(m_oOut, eY), rLaserLine.analysisResult(), interface::NotPresent); // bad rank
		preSignalAction();
		m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
		m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

		return; // RETURN
	} // if

	// Reset output based on input size
	m_oOut.resize(rLaserLineArray.size());
	// Now do the actual image processing
	findSteps( rLaserLineArray, m_oOut );

	// Create a new point, put the global context into the resulting profile and copy the rank over
	auto oAnalysisResult = rLaserLine.analysisResult();
	const GeoDoublearray oGeoDoublearrayXOut(rLaserLine.context(), getCoordinate(m_oOut, eX), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	const GeoDoublearray oGeoDoublearrayYOut(rLaserLine.context(), getCoordinate(m_oOut, eY), oAnalysisResult, interface::Limit); // full rank here, detailed rank in array
	preSignalAction();
	m_oPipePositionXOut.signal( oGeoDoublearrayXOut ); // invoke linked filter(s)
	m_oPipePositionYOut.signal( oGeoDoublearrayYOut ); // invoke linked filter(s)

} // proceedGroup



Step::StepCandidate Step::findStep( const geo2d::Doublearray &p_rLineIn )
{
	StepCandidate oStep = std::make_tuple( 0, 1, eRankMin, 1., 0., false );
	auto& rLineRank 	= p_rLineIn.getRank();
	auto& rLineData 	= p_rLineIn.getData();

	m_oScores.assign( p_rLineIn.size(), 1.0E20 );
	m_oAvg.assign( p_rLineIn.size(), 1.0E20 );

    auto oLineSum 		= (double)(0.0);
    auto oLineAvgCnt 	= (unsigned int)(0);
    bool firstIteration = true;


	// compute deviation of feature for each pixel on laser-line

	for( unsigned int oIndexLine = m_oFeatureCenter + 2; oIndexLine < rLineData.size() - ( m_oFeatureCenter + m_oLengthRight + 2); ++oIndexLine ) {
		// compute average
		auto oScore 		= (double)(0.0);
        unsigned int oIndexFeatureOnLineStart = oIndexLine + (0 - m_oFeatureCenter);
        unsigned int oIndexFeatureOnLineEnd = oIndexLine + (m_oFeature.size() - m_oFeatureCenter);

#ifndef NDEBUG
        double oSumLocal = 0; double oCntLocal = 0;
		for ( unsigned int oIndexFeat = 0; oIndexFeat < m_oFeature.size(); ++oIndexFeat ) {

			if ( rLineRank[oIndexLine + (oIndexFeat - m_oFeatureCenter)] > eRankMin ) {

				oSumLocal += rLineData[oIndexLine + (oIndexFeat - m_oFeatureCenter)];

				oCntLocal++;

			}

		} // for
#endif

        if (firstIteration)
        {
            assert(oLineSum == 0.0);
            assert(oLineAvgCnt == 0);
            for ( unsigned int oIndexFeatureOnLine = oIndexFeatureOnLineStart; oIndexFeatureOnLine < oIndexFeatureOnLineEnd; ++oIndexFeatureOnLine )
            {
                if ( rLineRank[oIndexFeatureOnLine] > eRankMin )
                {
                    oLineSum += rLineData[oIndexFeatureOnLine];
                    oLineAvgCnt++;
                }
            } // for
            firstIteration = false;
        }
        else
        {
            //just update the values computed in the previous iteration
            if (rLineRank[oIndexFeatureOnLineStart -1] > eRankMin)
            {
                oLineSum -= rLineData[oIndexFeatureOnLineStart-1];
                oLineAvgCnt --;
            }
            if ( rLineRank[oIndexFeatureOnLineEnd-1] > eRankMin )
            {
                oLineSum += rLineData[oIndexFeatureOnLineEnd-1];
                oLineAvgCnt++;
            }
        }

        assert(oLineSum == oSumLocal);
        assert(oLineAvgCnt == oCntLocal);

		double oLineAvg = oLineSum / oLineAvgCnt;

		auto oInvalidCnt 	= (unsigned int)(0);
		for ( unsigned int oIndexFeat = 0, oIndexFeatureOnLine = oIndexFeatureOnLineStart; oIndexFeat < m_oFeature.size(); ++oIndexFeat, ++oIndexFeatureOnLine )
        {
            assert(oIndexFeatureOnLine == oIndexLine + (oIndexFeat - m_oFeatureCenter));
			if ( rLineRank[oIndexFeatureOnLine] > eRankMin ) {
                double oDiff = std::abs( rLineData[oIndexFeatureOnLine] - ( m_oFeature[oIndexFeat] + oLineAvg ) );
				oScore  += ( oDiff * oDiff ) ;
			} else {
				oInvalidCnt++;
				oScore	+= 100*oInvalidCnt;
			} // if
		} // for
		m_oScores[oIndexLine] = oScore / m_oFeature.size();
		m_oAvg[oIndexLine] = oLineAvg;

	} // for

	// find all minima
	m_oCandidates.clear();
	auto oMinScore = (double)(m_oScores[0]);
	auto oMinIndex = (unsigned int)(0);
	for( unsigned int oIndex = m_oFeatureCenter + 2; oIndex < rLineData.size() - ( m_oFeatureCenter + m_oLengthRight + 2); ++oIndex ) {
		if ( m_oScores[oIndex] < oMinScore && m_oScores[oIndex] < m_oScores[oIndex-1]) {
			oMinScore 	= m_oScores[oIndex];
			oMinIndex 	= oIndex;
		} else if ( m_oScores[oIndex] > oMinScore ) {
			StepCandidate oStep;
			oMinIndex = (oMinIndex + oIndex) * 0.5;
			std::get<eStepX>(oStep) 	= 	oMinIndex;
			std::get<eStepY>(oStep) 	= 	m_oAvg[oIndex];
			std::get<eStepRank>(oStep) 	= 	eRankMax;
			std::get<eStepScore>(oStep)	=	oMinScore;
			std::get<eStepValid>(oStep) =	true;
			m_oCandidates.push_back(oStep);
			oMinScore 					= 	1.0E10;
		} // if
	} // for

	// thin candidates out
	if ( m_oCandidates.size() > 0 ) for ( unsigned int oIndex = 0; oIndex < m_oCandidates.size()-1; ++oIndex ) {
		if ( std::get<eStepX>(m_oCandidates[oIndex+1]) - std::get<eStepX>(m_oCandidates[oIndex]) < 20 ) {
			if ( std::get<eStepScore>(m_oCandidates[oIndex+1]) < std::get<eStepScore>(m_oCandidates[oIndex]) ) {
				std::get<eStepValid>(m_oCandidates[oIndex]) 	= false;
			} else {
				std::get<eStepValid>(m_oCandidates[oIndex+1]) 	= false;
			} // if
		} // if
	} // for

	// another scan to check for the correct height
	for ( unsigned int oIndex = 0; oIndex < m_oCandidates.size(); ++oIndex ) {
		if ( std::get<eStepValid>(m_oCandidates[oIndex]) == true ) {
			calcHeight( p_rLineIn, m_oCandidates[oIndex] );
			if ( std::get<eStepHeight>(m_oCandidates[oIndex]) < m_oMinHeight || std::get<eStepHeight>(m_oCandidates[oIndex]) > m_oMaxHeight ) {
				std::get<eStepValid>(m_oCandidates[oIndex]) = false;
			} // if
		} // if
	} // for

	// find global minimum
	oMinScore = 1.0E10;
	oMinIndex = m_oCandidates.size();
	for ( unsigned int oIndex = 0; oIndex < m_oCandidates.size(); ++oIndex ) {
		if ( std::get<eStepScore>(m_oCandidates[oIndex]) < oMinScore && std::get<eStepValid>(m_oCandidates[oIndex]) == true ){
			oMinScore = std::get<eStepScore>(m_oCandidates[oIndex]);
			oMinIndex = oIndex;
		} // if
	} // for
	if ( oMinIndex < m_oCandidates.size() ) { oStep = m_oCandidates[oMinIndex]; }

	// scaling for paint routine
	auto oMaxScore = (double)(-1.0E10);
	for ( unsigned int oIndex = 0; oIndex < m_oScores.size(); ++oIndex ) {
		if ( m_oScores[oIndex] > oMaxScore && m_oScores[oIndex] < 1.0E20 ) {
			oMaxScore = m_oScores[oIndex];
		} // if
	} // for
	m_oScale = 100.0 / oMaxScore;

	return	oStep;

} // findStep



void Step::findSteps( const geo2d::VecDoublearray &p_rLineIn, geo2d::Pointarray &p_rPositionOut )
{
	const unsigned int oNbLines	= p_rLineIn.size();

	// if the size of the output signal is not equal to the input line size, resize (assign is not needed since each element will be assigned later)
	p_rPositionOut.resize(oNbLines);
	// loop over N lines
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) {
		const Doublearray	&rLineIn			= p_rLineIn[lineN];
		Point				&rPositionOut		= p_rPositionOut.getData()[lineN];
		int					&rPositionOutRank	= p_rPositionOut.getRank()[lineN];

		m_oStep				= findStep( rLineIn );
		rPositionOut.x		= std::get<eStepX>( m_oStep );
		rPositionOut.y		= std::get<eStepY>( m_oStep );
		rPositionOutRank	= std::get<eStepRank>( m_oStep );
	} // for

} // findSteps



void Step::calcHeight( const geo2d::Doublearray &p_rLineIn, StepCandidate& p_rStep )
{
	auto oLeftX0 	= 	(unsigned int)( std::get<eStepX>( p_rStep ) - m_oFeatureCenter );
	auto oLeftX1 	= 	(unsigned int)( std::get<eStepX>( p_rStep ) - m_oFeatureCenter + m_oFeatureLeft  - 2 );
	auto oRightX0 	= 	(unsigned int)( std::get<eStepX>( p_rStep ) - m_oFeatureCenter + m_oFeatureRight + 2 );
	auto oRightX1 	= 	(unsigned int)( std::get<eStepX>( p_rStep ) - m_oFeatureCenter + m_oFeature.size());
	auto oStepX		=	int(std::get<eStepX>(p_rStep));

	auto oM0 		= 	double(0.0);
	auto oM1 		= 	double(0.0);
	auto oT0 		= 	double(0.0);
	auto oT1 		= 	double(0.0);

	lineFit( p_rLineIn, oLeftX0,  oLeftX1,  oM0, oT0 );
	lineFit( p_rLineIn, oRightX0, oRightX1, oM1, oT1 );

	const TPoint<double> oPos1(oStepX, oStepX*oM0 + oT0);
	const TPoint<double> oPos2(oStepX, oStepX*oM1 + oT1);

	assert(m_pCoordTransformer && "m_pCoordTransformer called before it was set in ProceedGroup");
	const math::Vec3D oSensorCoord1 = m_pCoordTransformer->imageCoordTo3D(static_cast<int>(oPos1.x), static_cast<int>(oPos1.y));
	const math::Vec3D oSensorCoord2 = m_pCoordTransformer->imageCoordTo3D(static_cast<int>(oPos2.x), static_cast<int>(oPos2.y));

	std::get<eStepHeight>(p_rStep) = std::abs(oSensorCoord1[2] - oSensorCoord2[2]);

} // calcHeight



void Step::lineFit( const geo2d::Doublearray &p_rLineIn, unsigned int p_oMinIndex, unsigned int p_oMaxIndex, double& p_rMOut, double& p_rTOut )
{
	auto& rLineData 	= p_rLineIn.getData();
	auto& rLineRank = p_rLineIn.getRank();

	if ( p_oMinIndex > p_rLineIn.size() ) 	{ p_oMinIndex = p_rLineIn.size(); }
	if ( p_oMaxIndex > p_rLineIn.size() ) 	{ p_oMaxIndex = p_rLineIn.size(); }
	if ( p_oMinIndex > p_oMaxIndex )		{ auto oTmp = p_oMinIndex; p_oMinIndex = p_oMaxIndex; p_oMaxIndex = oTmp; }
	if ( p_oMinIndex == p_oMaxIndex ) 		{ return; }

	auto oS20 	= 	double(0.0);
	auto oS10	= 	double(0.0);
	auto oS11 	= 	double(0.0);
	auto oS01 	= 	double(0.0);
	auto oX 	= 	int(0);
	auto oY 	= 	int(0);
	auto oY0 	= 	int( rLineData[p_oMinIndex] );
	auto oY1 	= 	int(0);
	auto oN 	= 	int(0);

	for( auto oIndex = p_oMinIndex; oIndex <= p_oMaxIndex; ++oIndex ) {
		oX 		= 	oIndex - p_oMinIndex;
		oY 		= 	int( rLineData[oIndex] );
		if ( oY<0 ) continue;

		if (rLineRank[oIndex] == eRankMin) continue;

		oY1 	= 	oY - oY0;
		oS20 	+= 	oX*oX;
		oS10 	+= 	oX;
		oS11 	+= 	oX*oY1;
		oS01 	+= 	oY1;
		oN++;
	} // for

	auto oS00 	= 	double(oN);
	auto oNenn 	= 	-oS10*oS10+oS00*oS20;
	auto oT		=	double(0.0);

	if( std::fabs(oNenn) > 1.0e-7 ) {
		oT 		=  	 (-oS10*oS11+oS01*oS20) / oNenn;
		p_rMOut	= 	-( oS10*oS01-oS11*oS00) / oNenn;
		p_rTOut	= 	oT + oY0 - ( p_rMOut * p_oMinIndex );
	} // if

} // lineFit


} // namespace precitec
} // namespace filter
