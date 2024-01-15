
/*!
 *  @n\b Project:	WM
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			Claudius Batzlen (CB)
 *  @date			2019
 *  @file
 *  @brief			Fliplib filter 'SimpleTracking' in component 'Filter_SeamSearch'. Simple tracking based on extremum search and simple or gradient threshold.
 */

#include "simpleTrackingXT.h"
#include "simpleTracking.h"
#include "seamSearch.h"								///< rank calculation

#include "system/types.h"							///< typedefs
#include "common/defines.h"							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"				///< paint overlay

#include "filter/algoArray.h"						///< algorithmic interface for class TArray
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

using interface::GeoVecDoublearray;
using interface::GeoDoublearray;
using geo2d::VecDoublearray;

const std::string SimpleTrackingXT::m_oFilterName 	= std::string("SimpleTrackingXT");
const std::string SimpleTrackingXT::m_oPipeOutName1	= std::string("ContourLeft");
const std::string SimpleTrackingXT::m_oPipeOutName2	= std::string("ContourRight");


SimpleTrackingXT::SimpleTrackingXT() :
	TransformFilter			( SimpleTrackingXT::m_oFilterName, Poco::UUID{"F6913304-FE4A-4826-B08D-3761724526D6"} ),
	m_pPipeInProfile		( nullptr ),
	m_pPipeInPosition		( nullptr ),
	m_pPipeInImgSize		( nullptr ),
	m_pPipeInThresholdL     ( nullptr ),
	m_pPipeInThresholdR     ( nullptr ),
	m_oPipeOutPosL           (this, m_oPipeOutName1 ),
	m_oPipeOutPosR			( this, m_oPipeOutName2 ),
	m_oSeamPosL				( 1 ), // always 1 line
	m_oSeamPosR				( 1 ), // always 1 line
	m_oComparisonType		( eLess ), // track dark gap
	m_oThresholdL			( 128 ),
	m_oThresholdR			( 128 )
{
	// Set default values of the parameters of the filter
	parameters_.add( "ComparisonType",  Parameter::TYPE_int, static_cast<int>(m_oComparisonType) );

    setInPipeConnectors({{Poco::UUID("1D5640E3-F90D-4497-9D96-F7683C072AC7"), m_pPipeInProfile, "Line", 1, "line"},
    {Poco::UUID("30CC9C98-9024-463C-9F3D-6999E54B75AA"), m_pPipeInPosition, "Position", 1, "position"},
    {Poco::UUID("D735BF85-2E52-4AAF-ACE2-546B3FEF411F"), m_pPipeInImgSize, "ImageSize", 1, "image_size"},
    {Poco::UUID("1CABF5D8-E301-4616-90F7-DFEFC4161464"), m_pPipeInThresholdL, "ThresholdLeft", 1, "threshold_left"},
    {Poco::UUID("F6E74DDE-2167-4611-ACB2-96ED4A2723E3"), m_pPipeInThresholdR, "ThresholdRight", 1, "threshold_right"}});
    setOutPipeConnectors({{Poco::UUID("30870305-0BF0-48DD-A4B8-365FD136FCB1"), &m_oPipeOutPosL, "ContourLeft", 0, ""},
    {Poco::UUID("3779DDF0-7BC3-4A57-9760-8AE42A7FBCED"), &m_oPipeOutPosR, "ContourRight", 0, ""}});
    setVariantID(Poco::UUID("C90CC357-4A68-4799-AA42-F56C135A6B07"));
} // SimpleTracking



/*virtual*/
void SimpleTrackingXT::setParameter() {
	TransformFilter::setParameter();
	m_oComparisonType	= static_cast<ComparisonType>( parameters_.getParameter("ComparisonType").convert<int>() );

} // setParameter



/*virtual*/
void SimpleTrackingXT::paint() {
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	auto oSpTrafo = ( m_pPipeInProfile->read(m_oCounter).context().trafo() );
    if (oSpTrafo.isNull())
    {
        return;
    }
	const Trafo&				rTrafo					( *oSpTrafo);
	OverlayCanvas&				rOverlayCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer&				rLayerPosition			( rOverlayCanvas.getLayerPosition());
	OverlayLayer&				rLayerText				( rOverlayCanvas.getLayerText());

	poco_assert_dbg(m_oSeamPosL.size() == m_oSeamPosR.size());
	poco_assert_dbg(m_oSeamPosL.size() != 0);

	const auto		oNbSlices = m_oSeamPosL.size();
	const auto		oSliceHeight = m_oImageSize.height / oNbSlices; // see assertion above
	const Color		oColorL(Color::Yellow());
	const Color		oColorR(Color::Magenta());

	rLayerText.add(new OverlayText("Left contour point(s)", Font(16), rTrafo(Rect(10, m_oImageSize.height - 40, 200, 20)), oColorL));
	rLayerText.add(new OverlayText("Right contour point(s)", Font(16), rTrafo(Rect(10, m_oImageSize.height - 20, 200, 20)), oColorR));

	for (std::size_t i = 0; i < oNbSlices; ++i) {
		const auto		oY			=	int( (i + 0.5) * oSliceHeight );
		const Point		oPositionL(roundToT<int>(m_oSeamPosL.getData()[i]), oY);
		const Point		oPositionR(roundToT<int>(m_oSeamPosR.getData()[i]), oY);

		rLayerPosition.add(new  OverlayCross(rTrafo(oPositionL), oColorL));
		rLayerPosition.add(new  OverlayCross(rTrafo(oPositionR), oColorR));
	} // for
} // paint



/*virtual*/
bool SimpleTrackingXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "line") {
		m_pPipeInProfile = dynamic_cast< line_pipe_t * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "position") {
		m_pPipeInPosition = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "image_size") {
		m_pPipeInImgSize = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "threshold_left") {
		m_pPipeInThresholdL = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "threshold_right") {
		m_pPipeInThresholdR = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/
void  SimpleTrackingXT::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg) {

	poco_assert_dbg(m_pPipeInProfile != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInPosition != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInThresholdL != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInThresholdR != nullptr); // to be asserted by graph editor

	// get input data
	const auto&	rGeoProfileIn = m_pPipeInProfile->read(m_oCounter);
	const auto&	rGeoPositionIn = m_pPipeInPosition->read(m_oCounter);
	const auto&	rGeoImgSizeIn = m_pPipeInImgSize->read(m_oCounter).ref().getData();
	const auto& rGeoThresholdLeftIn = m_pPipeInThresholdL->read(m_oCounter).ref().getData();
	const auto& rGeoThresholdRightIn = m_pPipeInThresholdR->read(m_oCounter).ref().getData();
	poco_assert_dbg(!rGeoProfileIn.ref().empty());
	poco_assert_dbg(!rGeoPositionIn.ref().getData().empty());
	poco_assert_dbg(!rGeoImgSizeIn.empty());
	poco_assert_dbg(!rGeoThresholdLeftIn.empty()); // to be asserted by graph editor
	poco_assert_dbg(!rGeoThresholdRightIn.empty()); // to be asserted by graph editor

	// input validity check

	if ( inputIsInvalid(rGeoProfileIn) || inputIsInvalid(rGeoPositionIn) || rGeoProfileIn.ref().size() != rGeoPositionIn.ref().size() ) {
		const GeoDoublearray oGeoPosLOut(rGeoProfileIn.context(), m_oSeamPosL, rGeoProfileIn.analysisResult(), 0.0); // bad rank
		const GeoDoublearray oGeoPosROut(rGeoProfileIn.context(), m_oSeamPosR, rGeoProfileIn.analysisResult(), 0.0); // bad rank

		preSignalAction();
		m_oPipeOutPosL.signal	( oGeoPosLOut ); // invoke linked filter(s)
		m_oPipeOutPosR.signal	( oGeoPosROut ); // invoke linked filter(s)

		return; // RETURN
	}

	m_oImageSize.width = int(rGeoImgSizeIn[0]);
	m_oImageSize.height = int(rGeoImgSizeIn[1]);
	m_oThresholdL = int(rGeoThresholdLeftIn[0]);
	m_oThresholdR = int(rGeoThresholdRightIn[0]);

	// (re)initialization of output structure
	reinitialize(rGeoProfileIn.ref());

	SimpleTracking::calcSimpleTracking(
		rGeoProfileIn.ref(),
		rGeoPositionIn.ref(),
		m_oComparisonType,
		std::make_pair(m_oThresholdL, m_oThresholdR),
		m_oSeamPosL,
		m_oSeamPosR
	); // signal processing

	const int	oLineLength	= rGeoProfileIn.ref().front().getData().size(); // note non-empty assertion above
	enforceIntegrity (m_oSeamPosL, m_oSeamPosR, oLineLength, 50); // check integrity and fix // TODO parameter

	const double oNewGeoRankL	= ( rGeoProfileIn.rank()	+ 1 ) / 2.; // full rank, detailed rank in TArray
	const double oNewGeoRankR	= ( rGeoProfileIn.rank()	+ 1 ) / 2.; // full rank, detailed rank in TArray
	const GeoDoublearray oGeoSeamPosLOut	(rGeoProfileIn.context(), m_oSeamPosL, rGeoProfileIn.analysisResult(), oNewGeoRankL);
	const GeoDoublearray oGeoSeamPosROut	(rGeoProfileIn.context(), m_oSeamPosR, rGeoProfileIn.analysisResult(), oNewGeoRankR);
	preSignalAction();
	m_oPipeOutPosL.signal( oGeoSeamPosLOut ); // invoke linked filter(s)
	m_oPipeOutPosR.signal( oGeoSeamPosROut ); // invoke linked filter(s)

} // proceed



void SimpleTrackingXT::reinitialize(const geo2d::VecDoublearray	&p_rProfileIn) {
	// there is always 1 line from construction
	m_oSeamPosL.assign(p_rProfileIn.size(), 0, eRankMin);	// (re)initialize output based on input dimension
	m_oSeamPosR.assign(p_rProfileIn.size(), 0, eRankMin);	// (re)initialize output based on input dimension
} // reinitialize


} // namespace filter
} // namespace precitec
