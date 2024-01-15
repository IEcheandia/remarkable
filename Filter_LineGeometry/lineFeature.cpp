/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2014
 * 	@brief		Fliplib filter 'LineFeature' in component 'Filter_LineGeometry'.
 * 	@detail		Compares the shape of the input line with a user-defined line template.
 *				The form of the template consists of two connected straight line segments. Each segment is defined by a length and an angle.
 *				The result is the mean square error between the input line and the template at each point of the line.
 */

#define _USE_MATH_DEFINES
#include "lineFeature.h"

#include "module/moduleLogger.h"
#include "overlay/overlayPrimitive.h"	///< paint
#include "filter/algoArray.h"			///< Weighted mean

#include <cmath>
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace image;
namespace filter {

const std::string LineFeature::m_oFilterName 	= "LineFeature";
const std::string LineFeature::m_oPipeOutName	= "Line";


LineFeature::LineFeature() :
	TransformFilter		( LineFeature::m_oFilterName, Poco::UUID{"E5381979-1228-4237-8D25-C91C59E69FBC"} ),
	m_pPipeInLine		( nullptr ),
	m_oPipeOutLine		( this, m_oPipeOutName ),
	m_oLinesOut			( 1 ), // usually one out line
	m_oSegment1Length	( 10 ),
	m_oSegment2Length	( 10 ),
	m_oSegment1Angle	( -45 ),
	m_oSegment2Angle	( 45 )
{
	// Defaultwerte der Parameter setzen
	parameters_.add("Segment1Length",	Parameter::TYPE_UInt32,	m_oSegment1Length);
	parameters_.add("Segment2Length",	Parameter::TYPE_UInt32,	m_oSegment2Length);
	parameters_.add("Segment1Angle",	Parameter::TYPE_Int32,	m_oSegment1Angle);
	parameters_.add("Segment2Angle",	Parameter::TYPE_Int32,	m_oSegment2Angle);

    setInPipeConnectors({{Poco::UUID("2719901E-6C42-42cb-B4E8-7BEF69EF9ABD"), m_pPipeInLine, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("3A5D18A0-6E45-4d46-87F5-7CD7DA0F4CF3"), &m_oPipeOutLine, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("D9D596F7-5481-43a9-98DF-A17FE29BAD26"));
}; // LineFeature



void LineFeature::setParameter() {
	TransformFilter::setParameter();
	m_oSegment1Length	= parameters_.getParameter("Segment1Length");
	m_oSegment2Length	= parameters_.getParameter("Segment2Length");
	m_oSegment1Angle	= parameters_.getParameter("Segment1Angle");
	m_oSegment2Angle	= parameters_.getParameter("Segment2Angle");

	m_oSegment1Angle	*= -1; // so for the user 0 deg is at 3 o'clock and 90 ged is at s at 12 o'clock
	m_oSegment2Angle	*= -1; // so for the user 0 deg is at 3 o'clock and 90 ged is at s at 12 o'clock

	UNUSED const auto	oAllowedAngleRange	= Range(-89, 89);

	poco_assert_dbg(oAllowedAngleRange.contains(m_oSegment1Angle));	// should not throw if gui asserts the condition
	poco_assert_dbg(oAllowedAngleRange.contains(m_oSegment2Angle));	// should not throw if gui asserts the condition

	// with given paramters, calculate all y values of the template segments (feature). The connection between the segments is always zero and serves as reference.

	const auto	oSinAngle1		=	std::sin(m_oSegment1Angle * M_PI / 180.);		// degree to rad
	const auto	oSinAngle2		=	std::sin(m_oSegment2Angle * M_PI / 180.);		// degree to rad
	const auto	oCosAngle1		=	std::cos(m_oSegment1Angle * M_PI / 180.);		// degree to rad
	const auto	oCosAngle2		=	std::cos(m_oSegment2Angle * M_PI / 180.);		// degree to rad
	const auto	oRatio1			=	oSinAngle1 / oCosAngle1;
	const auto	oRatio2			=	oSinAngle2 / oCosAngle2;
	const auto	oFeatureLength	=	m_oSegment1Length + m_oSegment2Length + 1; // +1 for the connection which is zero

	m_oFeature.assign(oFeatureLength, 0);	// reset
	m_oFeature[m_oSegment1Length]	= 0;	// middle value is reference

	// calculate the y values to the left
	for (int oIndex = m_oSegment1Length - 1; oIndex >= 0; --oIndex) {
		const auto	oX	= m_oSegment1Length - oIndex; // in the middle x is 0. From the middle we go left, x increases.
		m_oFeature[oIndex]	= oX * oRatio1;
	} // for

	// the middle value remains zero

	// calculate the y values to the right
	for (auto oIndex = m_oSegment1Length + 1; oIndex < oFeatureLength; ++oIndex) {
		const auto	oX	= oIndex - m_oSegment1Length; // in the middle x is 0. From the middle we go right, x increases.
		m_oFeature[oIndex]	= oX * oRatio2;
	} // for
}; // setParameter



bool LineFeature::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInLine  = dynamic_cast<line_pipe_t*>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}; // subscribe



void LineFeature::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert_dbg(m_pPipeInLine != nullptr); // to be asserted by graph editor

	// get input data

	const GeoVecDoublearray &rGeoLinesIn = m_pPipeInLine->read(m_oCounter);
	m_oSpTrafo	= rGeoLinesIn.context().trafo();

	// (re)initialization of output structure
	resetFromInput<double>(rGeoLinesIn.ref(), m_oLinesOut);

	// input validity check

	if ( inputIsInvalid(rGeoLinesIn) ) {
		const GeoVecDoublearray geoIntarrayOut(rGeoLinesIn.context(), m_oLinesOut, rGeoLinesIn.analysisResult(), 0.0); // bad rank
		preSignalAction();  m_oPipeOutLine.signal( geoIntarrayOut ); // invoke linked filter(s)

		return; // RETURN
	} // if

	// process all lines in linevector

	auto oItLineOut	= m_oLinesOut.begin();
	for(auto& p_rLineIn : rGeoLinesIn.ref()) { // loop over all lines
		calcTemplateMatch(p_rLineIn, m_oFeature, m_oSegment1Length, *oItLineOut); // signal processing

		++oItLineOut;
	}; // for

	const auto	oNewRank			= (rGeoLinesIn.rank() + 1.0) / 2.;
	const auto	oAnalysisResult		= rGeoLinesIn.analysisResult() == AnalysisOK ? AnalysisOK : rGeoLinesIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const auto	oGeoVecOut			= GeoVecDoublearray( rGeoLinesIn.context(), m_oLinesOut, oAnalysisResult, oNewRank );

	preSignalAction();  m_oPipeOutLine.signal(oGeoVecOut); // invoke linked filter(s)
}; // proceed



/*static*/ void LineFeature::calcTemplateMatch(const Doublearray& p_rLineIn, const std::vector<double>& p_rFeature, std::size_t p_oSegment1Length, Doublearray& p_rLineOut)  {
	// reset out line
	p_rLineOut.assign(p_rLineIn.size(), 512, eRankMin); // big init value as output is an error function

	const auto&	rDataIn		= p_rLineIn.getData();
	const auto&	rRankIn		= p_rLineIn.getRank();
	auto&		rDataOut	= p_rLineOut.getData();
	auto&		rRankOut	= p_rLineOut.getRank();

	poco_assert_dbg(! rDataIn.empty());
	poco_assert_dbg(! rRankIn.empty());
	poco_assert_dbg(p_rLineIn.size() == p_rLineOut.size());

	const auto	oFeatureLength				= p_rFeature.size();

	if (rDataIn.size() <= oFeatureLength) {
		wmLog(eDebug, "In '%s': WARNING: Feature length (%i) to big\n", __FUNCTION__, oFeatureLength);
		return;
	} // if

	// move template over input line and calculate mean square error

	for (auto oLinePos = 0u; oLinePos < p_rLineIn.size() - oFeatureLength; ++oLinePos) { // loop over line
		const auto	oDataRef = rDataIn[oLinePos + p_oSegment1Length]; // middle value is refernce, diff is always zero at this pos
		auto		oErrorAccum	= 0.;	// reset error accumulator

		for (auto oFeatPos = 0u; oFeatPos < oFeatureLength; ++oFeatPos) { // loop over feature values
			const auto	oDiff = std::abs(oDataRef - rDataIn[oLinePos + oFeatPos] + p_rFeature[oFeatPos]); // take absolute difference
			//std::cout << "std::abs(" << oDataRef << " - " << rDataIn[oLinePos + oFeatPos] << " + " << m_oFeature[oFeatPos] << " = " << oDiff << "\n";
			oErrorAccum += oDiff * oDiff; // square the error
		} // for

		rDataOut[oLinePos + p_oSegment1Length] = oErrorAccum / oFeatureLength; // mean square error
		rRankOut[oLinePos + p_oSegment1Length] = rRankIn[oLinePos + p_oSegment1Length];
	} // for
} // calcTemplateMatch



void LineFeature::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const auto&		rTrafo			= *m_oSpTrafo;
	auto&			rCanvas			= canvas<OverlayCanvas>(m_oCounter);
	auto&			rLayerContour	= rCanvas.getLayerContour();
	auto&			rLayerLine		= rCanvas.getLayerLine();
	auto&			rLayerText		= rCanvas.getLayerText();

	const auto&	rFilteredLine	( m_oLinesOut.front().getData() );
	for (unsigned int i = 0; i != rFilteredLine.size(); ++i) {
		rLayerContour.add( new OverlayPoint(rTrafo(Point(i, roundToT<int>( rFilteredLine[i]))), Color::m_oScarletRed) );
	} // for

	if(m_oVerbosity < eMedium){
		return;
	} // if

	const auto	oY					= 100;
	const auto	oTextFont			= Font(16);
	const auto	oTextPosition		= rTrafo(Point(10, oY)); // paint info close to left roi border
	const auto	oTextSize			= Size(100, 20);
	const auto	oTextBox			= Rect(oTextPosition, oTextSize);
	const auto	oFeatureMiddle		= Point(oTextPosition.x + oTextSize.width + m_oSegment1Length, oTextPosition.y + oTextSize.height / 2);
	const auto	oFeatureLeftEnd		= oFeatureMiddle + Point(-1 * m_oSegment1Length, roundToT<int>(m_oFeature.front()));
	const auto	oFeatureRighttEnd	= oFeatureMiddle + Point( m_oSegment2Length, roundToT<int>(m_oFeature.back()));

	rLayerText.add(new OverlayText("Line feature:", oTextFont, oTextBox, Color::m_oScarletRed));
	rLayerLine.add(new OverlayLine(oFeatureLeftEnd, oFeatureMiddle, Color::m_oScarletRed));
	rLayerLine.add(new OverlayLine(oFeatureMiddle, oFeatureRighttEnd, Color::m_oScarletRed));
} // paint



} // namespace filter
} // namespace precitec
