/*!
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Simon Hilsenbeck (HS)
*  @date			2011
*  @file
*  @brief			Fliplib filter 'SelectSeamPos' in component 'Filter_SeamSearch'. Checks seam positions and selects final seam position.
*/


#include "selectSeamPos.h"

#include "system/types.h"							///< typedefs
#include "common/defines.h"							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "filter/armStates.h"						///< arm states for filter arm
#include "filter/algoArray.h"						///< algorithmic interface for class TArray

#include "overlay/overlayPrimitive.h"				///< paint overlay

#include "seamSearch.h"								///< rank calculation
#include <fliplib/TypeToDataTypeImpl.h>

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {

const std::string SelectSeamPos::m_oFilterName 		= std::string("SelectSeamPos");
const std::string SelectSeamPos::m_oPipeOutName1	= std::string("SeamPosLeft");
const std::string SelectSeamPos::m_oPipeOutName2	= std::string("SeamPosRight");



SelectSeamPos::SelectSeamPos() :
TransformFilter				( SelectSeamPos::m_oFilterName, Poco::UUID{"AF5DF9B4-3240-43b0-899F-3DE5CBB7D639"} ),
	m_pPipeInContourL		(nullptr),
	m_pPipeInContourR		(nullptr),
	m_oPipeOutSeamPosL		( this, m_oPipeOutName1 ),
	m_oPipeOutSeamPosR		( this, m_oPipeOutName2 ),
	m_oSeamPosL				( 1 ), // one out point
	m_oSeamPosR				( 1 ) // one out point
{
    setInPipeConnectors({{Poco::UUID("ED7F8E1C-DB09-4085-BDA0-D4F8BA3A0666"), m_pPipeInContourL, "ContourLeft", 1, "contour_left"},
    {Poco::UUID("EAA130AD-2812-40c6-BCF3-A466A23EC502"), m_pPipeInContourR, "ContourRight", 1, "contour_right"}});
    setOutPipeConnectors({{Poco::UUID("BBE35203-E475-48f2-8F6E-1A7639D9E7CD"), &m_oPipeOutSeamPosL, "SeamPosLeft", 0, ""},
    {Poco::UUID("86DABF82-3940-4511-B482-F97D1B767367"), &m_oPipeOutSeamPosR, "SeamPosRight", 0, ""}});
    setVariantID(Poco::UUID("5285C947-7332-4c39-AC05-6EF6FE414C7D"));
} // SelectSeamPos



void SelectSeamPos::setParameter() {
	TransformFilter::setParameter();
} // setParameter



/*virtual*/ void
SelectSeamPos::arm (const fliplib::ArmStateBase& state) {
	//std::cout << "\nFilter '" << m_oFilterName << "' armed at armstate " << state.getStateID() << std::endl;
	//if (state.getStateID() == filter::eSeamStart) {
	//}
} // arm



/*virtual*/
void SelectSeamPos::paint() {
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	auto oSpTrafo = m_pPipeInContourL->read(m_oCounter).context().trafo();
	const Trafo&				rTrafo					( *oSpTrafo );
	OverlayCanvas&				rOverlayCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer&				rLayerPosition			( rOverlayCanvas.getLayerPosition());
	OverlayLayer&				rLayerText				( rOverlayCanvas.getLayerText());

	rLayerText.add( new OverlayText("Final left contour point",		Font(16), Rect(10, 90, 200, 20), Color::Yellow() ) );
	rLayerText.add( new OverlayText("Final right contour point",	Font(16), Rect(10, 110, 200, 20), Color::Magenta() ) );

	const auto	oItEndSeamPosL		( m_oSeamPosL.getData().cend() );
	auto		oItSeamPosL			( m_oSeamPosL.getData().cbegin() );
	auto		oItSeamPosR			( m_oSeamPosR.getData().cbegin() );
	auto		oItSeamPosLRank		( m_oSeamPosL.getRank().cbegin() );
	auto		oItSeamPosRRank		( m_oSeamPosR.getRank().cbegin() );
	int			oY					( 400 ); // img height not known

	poco_assert_dbg(m_oSeamPosL.size() == m_oSeamPosR.size());
	poco_assert_dbg(m_oSeamPosL.size() != 0);

	const Point		oPositionL		( int(*oItSeamPosL), oY );
	const Point		oPositionR		( int(*oItSeamPosR), oY );
	const Color		oColorL			( Color::Yellow() );
	const Color		oColorR			( Color::Magenta() );

	rLayerPosition.add( new  OverlayCross(rTrafo(oPositionL), oColorL ) );
	rLayerPosition.add( new  OverlayCross(rTrafo(oPositionR), oColorR ) );

	if (m_oVerbosity >= eMedium) {
		std::ostringstream oRankLabel; oRankLabel << *oItSeamPosLRank;
		rLayerText.add(new OverlayText(oRankLabel.str(), image::Font(), rTrafo(Rect(oPositionL.x, oPositionL.y, 30, 20)), oColorL));
		oRankLabel.str(""); oRankLabel << *oItSeamPosLRank;
		rLayerText.add(new OverlayText(oRankLabel.str(), image::Font(), rTrafo(Rect(oPositionR.x, oPositionR.y, 30, 20)), oColorR));
	} // if
} // paint



bool SelectSeamPos::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "contour_left")
		m_pPipeInContourL  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	else if (p_rPipe.tag() == "contour_right")
		m_pPipeInContourR  = dynamic_cast< scalar_pipe_t* >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void SelectSeamPos::proceedGroup(const void* sender, PipeGroupEventArgs& e) {

	poco_assert_dbg(m_pPipeInContourL != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInContourR != nullptr); // to be asserted by graph editor


	// get data from frame

	const GeoDoublearray&	rContourLIn				= m_pPipeInContourL->read(m_oCounter);
	const GeoDoublearray&	rContourRIn				= m_pPipeInContourR->read(m_oCounter);

	// input validity check

	if ( inputIsInvalid( rContourLIn) || inputIsInvalid(rContourRIn ) || rContourLIn.ref().size() != rContourRIn.ref().size() ) {
		const GeoDoublearray oGeoSeamPosLeftOut	(rContourLIn.context(), m_oSeamPosL, rContourLIn.analysisResult(), 0.0); // bad rank
		const GeoDoublearray oGeoSeamPosRightOut	(rContourRIn.context(), m_oSeamPosR, rContourRIn.analysisResult(), 0.0); // bad rank
		preSignalAction();
		m_oPipeOutSeamPosL.signal( oGeoSeamPosLeftOut );
		m_oPipeOutSeamPosR.signal( oGeoSeamPosRightOut );

		return; // RETURN
	}

	reinitialize(); // (re)initialization of output structure

	calcSelectSeamPos( rContourLIn.ref(), rContourRIn.ref(), m_oSeamPosL, m_oSeamPosR); // signal processing

	//std::cout << "Pos (" << m_oSeamPosL.first << ", " << m_oSeamPosR.first << ") Rank (" << m_oSeamPosL.second << ", " << m_oSeamPosR.second << ")" <<  std::endl;

	const double oNewRankL	= ( rContourLIn.rank()	+ intToDoubleRank(m_oSeamPosL.getRank().front()) ) / 2.; // rank from value
	const double oNewRankR	= ( rContourRIn.rank()	+ intToDoubleRank(m_oSeamPosR.getRank().front()) ) / 2.; // rank from value
	const GeoDoublearray oGeoSeamPosLOut	(rContourLIn.context(), m_oSeamPosL, rContourLIn.analysisResult(), oNewRankL);
	const GeoDoublearray oGeoSeamPosROut	(rContourRIn.context(), m_oSeamPosR, rContourRIn.analysisResult(), oNewRankR);
	preSignalAction();
	m_oPipeOutSeamPosL.signal( oGeoSeamPosLOut );
 	m_oPipeOutSeamPosR.signal( oGeoSeamPosROut );

} // proceed



void SelectSeamPos::reinitialize() {
	m_oSeamPosL.reinitialize();
	m_oSeamPosR.reinitialize();
} // reinitialize



// actual signal processing
/*static*/
void SelectSeamPos::calcSelectSeamPos(
	const Doublearray					&p_rContourLIn,
	const Doublearray					&p_rContourRIn,
	Doublearray							&p_oSeamPosL,
	Doublearray							&p_oSeamPosR
	){
		p_oSeamPosL[0] = calcMedian1d(p_rContourLIn);
		p_oSeamPosR[0] = calcMedian1d(p_rContourRIn);
} // calcSelectSeamPos



	} // namespace filter
} // namespace precitec
