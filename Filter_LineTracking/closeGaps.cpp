/*!
 *  @copyright		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 */


#include "closeGaps.h"

#include <algorithm>					///< find
#include <iterator>						///< distance
#include <functional>					///< bind2nd, not_equal_to
#include <cmath>						///< abs

#include <system/platform.h>			///< global and platform specific defines
#include <system/tools.h>				///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"	///< paint
#include "geo/rect.h"
#include "filter/algoArray.h"			///< Doublearray algo
#include "filter/parameterEnums.h"		///< rank

#include <fliplib/TypeToDataTypeImpl.h>


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string CloseGaps::m_oFilterName 		= std::string("CloseGaps");
const std::string CloseGaps::m_oPipeOutName1	= std::string("Line");



CloseGaps::CloseGaps() :
	TransformFilter( CloseGaps::m_oFilterName, Poco::UUID{"6A0225E0-23B5-4ee0-AD56-3425193D9057"} ),
	m_pPipeInLaserline		(NULL),
	m_oApPipeOutLaserline	( new SynchronePipe< GeoVecDoublearray >( this, m_oPipeOutName1 ) ),
	m_oLaserlineOut			(1),	// usually one out line
	m_oMaxGapX				(10),
	m_oMaxGapY				(10),
	m_oMaxClosureLength		(10),
	m_oSnippets				(30),	// pre allocate - usually we obtain 3-20 snippets
	m_oBigSnippets			(10)	// pre allocate - usually we obtain 3-6 big snippets
{
	// Defaultwerte der Parameter setzen
	parameters_.add("CloseGapX",		Parameter::TYPE_int, m_oMaxGapX);
	parameters_.add("MaxGapY",			Parameter::TYPE_int, m_oMaxGapY);
	parameters_.add("MaxClosureLength",	Parameter::TYPE_int, m_oMaxClosureLength);

    //Überprüfen!
    setInPipeConnectors({{Poco::UUID("2A827D05-0FBE-4e8d-BCC4-469931416A53"), m_pPipeInLaserline, "Line", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("ABEB6BF4-0E4B-4cd6-AA7E-B46FC1006DA8"), m_oApPipeOutLaserline.get(), "Line", 0, ""}});            //Unsicher!
    setVariantID(Poco::UUID("124DE07A-8DD3-4f13-8AA9-BAFE585AC68A"));
} // CloseGaps



void CloseGaps::setParameter() {
	TransformFilter::setParameter();
	m_oMaxGapX			= parameters_.getParameter("CloseGapX");
	m_oMaxGapY			= parameters_.getParameter("MaxGapY");
	m_oMaxClosureLength	= parameters_.getParameter("MaxClosureLength");

	poco_assert_dbg(m_oMaxGapX			>= 0);		// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oMaxGapY			>= 0);		// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(m_oMaxClosureLength >= 0);		// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
} // setParameter


void CloseGaps::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()) {
		return;
	} // if

	const Trafo		&rTrafo				( *m_oSpTrafo );
	OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
	OverlayLayer	&rLayerLine			( rCanvas.getLayerLine());

	const auto	oLaserLineY	( m_oLaserlineOut.front().getData() );
	for (unsigned int i = 0; i != oLaserLineY.size(); ++i) {
		rLayerContour.add( new OverlayPoint(rTrafo(Point(i, int( oLaserLineY[i] ))), Color::Cyan()) );
	} // for

	if(m_oVerbosity < eMedium) {
		return;
	} // if

	std::for_each (std::begin(m_oSnippets), std::end(m_oSnippets), [&](const Range &p_rSnippet) { // loop over all snippets
		rLayerLine.add( new OverlayRectangle( rTrafo(Rect( p_rSnippet.start(), 0, p_rSnippet.length(), 50 )), Color::Blue() ) ); // paint snippet
	}); // lamda

	std::for_each (std::begin(m_oBigSnippets), std::end(m_oBigSnippets), [&](const Range &p_rBigSnippet) { // loop over all big snippets
		rLayerLine.add( new OverlayRectangle( rTrafo(Rect( p_rBigSnippet.start(), 0, p_rBigSnippet.length(), 50 )), Color::Yellow() ) ); // paint big snippet
	}); // lamda

} // paint


bool CloseGaps::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	m_pPipeInLaserline  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void CloseGaps::proceed(const void* sender, fliplib::PipeEventArgs& e) {
	poco_assert(m_pPipeInLaserline != nullptr); // prevented by graph editor

	// get input data

	const GeoVecDoublearray &rGeoIntarrayIn = m_pPipeInLaserline->read(m_oCounter);
	m_oSpTrafo	= rGeoIntarrayIn.context().trafo();

	// input validity check

	if ( inputIsInvalid( rGeoIntarrayIn ) ) {
		const GeoVecDoublearray geoLaserlineOut(rGeoIntarrayIn.context(), m_oLaserlineOut, rGeoIntarrayIn.analysisResult(), interface::NotPresent);
        m_oSpTrafo.reset(); //disable paint
        preSignalAction();
        m_oApPipeOutLaserline->signal( geoLaserlineOut );

		return; // RETURN
	}
	resetFromInput(rGeoIntarrayIn.ref(), m_oLaserlineOut);

	closeGaps( rGeoIntarrayIn.ref() );

	double oNewRank = (rGeoIntarrayIn.rank() + 1.0) / 2.;
	const auto oAnalysisResult	= rGeoIntarrayIn.analysisResult() == AnalysisOK ? AnalysisOK : rGeoIntarrayIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoVecDoublearray geoLaserlineOut(rGeoIntarrayIn.context(), m_oLaserlineOut, oAnalysisResult, oNewRank);
	preSignalAction(); m_oApPipeOutLaserline->signal( geoLaserlineOut );

} // proceed


// actual signal processing
void CloseGaps::closeGaps(const VecDoublearray &p_rLaserlineIn)  {
	const unsigned int oNbLines	= p_rLaserlineIn.size();
	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN) { // loop over N lines
		const auto	&rLaserVectorIn			= p_rLaserlineIn[lineN].getData();
		const auto	&rRankVectorIn			= p_rLaserlineIn[lineN].getRank();
		auto		&rLaserVectorOut		= m_oLaserlineOut[lineN].getData();
		auto		&rRankVectorOut			= m_oLaserlineOut[lineN].getRank();

		poco_assert_dbg(rLaserVectorIn.size() >= rLaserVectorOut.size());
		// ok: asserted by initialization of p_rLaserlineOut

		// get position of first good value - usually the first value from filter ParallelMaximum should be valid
		const auto oItFirstGood = std::find_if(
			rRankVectorIn.begin(),
			rRankVectorIn.end(),
			std::bind(std::not_equal_to<int>(), std::placeholders::_1, eRankMin)
		);
		const int oFirstGoodOffset = std::distance(rRankVectorIn.begin(), oItFirstGood);
		if (oFirstGoodOffset == int(rLaserVectorIn.size()))
		{
			if(m_oVerbosity >= eMax) 
			{
				wmLog(eDebug, "CloseGaps: could not process line %d,  no valid points \n", lineN);
			} // if
			
			continue;
		}

		// get position of last good value
		const auto oItLasstGood = std::find_if(
			rRankVectorIn.rbegin(),
			rRankVectorIn.rend(),
			std::bind(std::not_equal_to<int>(), std::placeholders::_1, eRankMin)
		);
		const int oPastLastGood		= std::distance(oItLasstGood, rRankVectorIn.rend() );
		const int oLastGood			= oPastLastGood - 1;
		assert(oLastGood > oFirstGoodOffset);
        
		m_oSnippets.clear();
		m_oBigSnippets.clear();

		Range	oCurrent(oFirstGoodOffset, oFirstGoodOffset); // first snipet init
		bool	oSnipetOpen		= true;
		for(int	x = oFirstGoodOffset; x < oLastGood; ++x) { // not past last good due to x+1 calculations
			const bool oIsJumpY	( std::abs( rLaserVectorIn[x] - rLaserVectorIn[x+1] ) > m_oMaxGapY );
			// close current snipet

			if( oSnipetOpen == true						// there is an open snipet
				&& (oIsJumpY							// AND (new jump
				|| rRankVectorIn[x+1] == eRankMin) ){	// OR next value has bad rank)
				oCurrent.end() = x + 1;
				m_oSnippets.push_back(oCurrent);
				oSnipetOpen = false;
			} // if
			// open new snipet

			if( oSnipetOpen == false					// no open snipet
				&& oIsJumpY								// AND new jump
				&& rRankVectorIn[x + 1] != eRankMin) {	// AND next value is NOT bad rank
				oCurrent.start() = x + 1;
				oSnipetOpen = true;
			}
		} // for

		if( oSnipetOpen == true) {					// last value reached but smipet still open
			oCurrent.end() = oPastLastGood;			// last index is past last good (like an end iterator)
			m_oSnippets.push_back(oCurrent);
			oSnipetOpen = false;
		} // if

		// Alle snipets groesser als maxGapsize werden in bigSnipets gesichert
		// erstes und letztes koennen allerdings rausfliegen bei sehr grossem CloseGapX parameter - dann roi verbreitern

		auto oItDataIn	= std::begin(rLaserVectorIn);
		auto oItRankIn	= std::begin(rRankVectorIn);
		auto oItDataOut	= std::begin(rLaserVectorOut);
		auto oItRankOut	= std::begin(rRankVectorOut);
		for(unsigned int i = 0; i < m_oSnippets.size(); ++i)
		{
			oCurrent = m_oSnippets[i];
			if( oCurrent.length() > m_oMaxGapX ) { // snipet bleibt
				m_oBigSnippets.push_back( oCurrent );
				std::copy(oItDataIn + oCurrent.start(), oItDataIn + oCurrent.end(), oItDataOut + oCurrent.start());
				std::copy(oItRankIn + oCurrent.start(), oItRankIn + oCurrent.end(), oItRankOut + oCurrent.start());
			} // if
		} // for

		if(m_oVerbosity >= eMax) {
			std::ostringstream	oMsg;
			oMsg << "<Snippets= " << m_oSnippets << '\n';
			wmLog(eDebug, oMsg.str()); oMsg.str("");
			oMsg << "<Big snippets= " << m_oBigSnippets << '\n';
			wmLog(eDebug, oMsg.str());
		} // if

		// Wenn zwischen den grossen Schnippseln noch Luecken sind, werden diese
		// mit linearer Interpolation geschlossen.

		if (m_oBigSnippets.size() > 1) { // nur sinnvoll wenn mind 2 grosse snipets
			for(unsigned int i = 0; i < m_oBigSnippets.size() - 1; ++i) {	// -1 because we work with current and next snipet
				const int	oStart = m_oBigSnippets[i].end() - 1;			// anfang des 1. snipets ausgeschlossen.
				const int	oEnd   = m_oBigSnippets[i+1].start();			// ende  des letzten snipets ausgeschlossen.
				const int	oGapX = oEnd - oStart;							// start, end are valid indices around the area to be interpolatet

				const double	oYStart = rLaserVectorIn[oStart];
				const double	oYEnd   = rLaserVectorIn[oEnd];
				const double	oYJump  = oYEnd-oYStart;

				if(m_oVerbosity >= eMax) {
					std::ostringstream	oMsg;
					oMsg << "interpolate: "    << oStart  << ' ' << oYStart <<  "|" << oEnd << ' ' << oYEnd  << '\n';
					wmLog(eDebug, oMsg.str());
				} // if

				// Abstand der grossen snipets groesser 1: Laserlinie schliessen.
				if(oGapX > 1 && oGapX <= m_oMaxClosureLength) {
					const double	oM		= oYJump / oGapX;
					const int		oRankIpol	= std::min(rRankVectorIn[oStart], rRankVectorIn[oEnd ]) / 2; // interpolation gets maximal half max rank

					for(int x = 0; x <= oGapX; ++x) { // interpoliere ueber die luecke
						rLaserVectorOut[oStart + x]	= oYStart + static_cast<int>( x * oM );
						rRankVectorOut[oStart + x]	= oRankIpol;
					} // for
				} // if
			} // for
		} // if
	} // for

} // closeGaps



	} // namespace filter
} // namespace precitec
