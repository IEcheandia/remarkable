
/*!
 *  @n\b Project:	WM
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			Simon Hilsenbeck (HS)
 *  @date			2011
 *  @file
 *  @brief			Fliplib filter 'SimpleTracking' in component 'Filter_SeamSearch'. Simple tracking based on extremum search and simple or gradient threshold.
 */

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

const std::string SimpleTracking::m_oFilterName 	= std::string("SimpleTracking");
const std::string SimpleTracking::m_oPipeOutName1	= std::string("ContourLeft");
const std::string SimpleTracking::m_oPipeOutName2	= std::string("ContourRight");


SimpleTracking::SimpleTracking() :
	TransformFilter			( SimpleTracking::m_oFilterName, Poco::UUID{"EC9D0E6C-DC1E-4558-AF28-E8010B786D57"} ),
	m_pPipeInProfile		( nullptr ),
	m_pPipeInPosition		( nullptr ),
	m_pPipeInImgSize		( nullptr ),
	m_oPipeOutPosL			( this, m_oPipeOutName1 ),
	m_oPipeOutPosR			( this, m_oPipeOutName2 ),
	m_oSeamPosL				( 1 ), // always 1 line
	m_oSeamPosR				( 1 ), // always 1 line
	m_oComparisonType		( eLess ), // track dark gap
	m_oThresholdL			( 128 ),
	m_oThresholdR			( 128 )
{
	// Set default values of the parameters of the filter
	parameters_.add( "ComparisonType",  Parameter::TYPE_int, static_cast<int>(m_oComparisonType) );
	parameters_.add( "ThresholdLeft",  Parameter::TYPE_int, m_oThresholdL );
	parameters_.add( "ThresholdRight",  Parameter::TYPE_int, m_oThresholdR );

    setInPipeConnectors({{Poco::UUID("C6F09FF7-2240-4645-BD6D-357A429B3F6E"), m_pPipeInProfile, "Line", 1, "line"},
    {Poco::UUID("833BCF6A-A08E-430e-B448-AFB533169455"), m_pPipeInPosition, "Position", 1, "position"},
    {Poco::UUID("8A53435E-22AA-4180-BE1F-EE4F19573385"), m_pPipeInImgSize, "ImageSize", 1, "image_size"}});
    setOutPipeConnectors({{Poco::UUID("58EC6B60-D800-4479-A65A-33BE31371026"), &m_oPipeOutPosL, "ContourLeft", 0, ""},
    {Poco::UUID("27370BBB-2D36-4e8e-9FD7-DBC695A37255"), &m_oPipeOutPosR, "ContourRight", 0, ""}});
    setVariantID(Poco::UUID("BFF359EA-C4E1-4e39-9AD3-A4DED098B823"));
} // SimpleTracking



/*virtual*/
void SimpleTracking::setParameter() {
	TransformFilter::setParameter();
	m_oComparisonType	= static_cast<ComparisonType>( parameters_.getParameter("ComparisonType").convert<int>() );
	m_oThresholdL	= parameters_.getParameter("ThresholdLeft").convert<int>();
	m_oThresholdR	= parameters_.getParameter("ThresholdRight").convert<int>();

} // setParameter



/*virtual*/
void SimpleTracking::paint() {
	if(m_oVerbosity <= eNone) {
		return;
	} // if

	auto oSpTrafo = ( m_pPipeInProfile->read(m_oCounter).context().trafo() );
    if (oSpTrafo.isNull())
    {
        return;
    }
    auto & rTrafo = *oSpTrafo;

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
bool SimpleTracking::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "line") {
		m_pPipeInProfile = dynamic_cast< line_pipe_t * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "position") {
		m_pPipeInPosition = dynamic_cast< scalar_pipe_t * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "image_size") {
		m_pPipeInImgSize = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/
void  SimpleTracking::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg) {

	poco_assert_dbg(m_pPipeInProfile != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInPosition != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor

	// get input data
	const auto&	rGeoProfileIn = m_pPipeInProfile->read(m_oCounter);
	const auto&	rGeoPositionIn = m_pPipeInPosition->read(m_oCounter);
	const auto&	rImgSizeIn = m_pPipeInImgSize->read(m_oCounter).ref().getData();
	poco_assert_dbg(!rGeoProfileIn.ref().empty());
	poco_assert_dbg(!rGeoPositionIn.ref().getData().empty());
	poco_assert_dbg(!rImgSizeIn.empty());

	// input validity check

	if ( inputIsInvalid(rGeoProfileIn) || inputIsInvalid(rGeoPositionIn) || rGeoProfileIn.ref().size() != rGeoPositionIn.ref().size() ) {
		const GeoDoublearray oGeoPosLOut(rGeoProfileIn.context(), m_oSeamPosL, rGeoProfileIn.analysisResult(), 0.0); // bad rank
		const GeoDoublearray oGeoPosROut(rGeoProfileIn.context(), m_oSeamPosR, rGeoProfileIn.analysisResult(), 0.0); // bad rank

		preSignalAction();
		m_oPipeOutPosL.signal	( oGeoPosLOut ); // invoke linked filter(s)
		m_oPipeOutPosR.signal	( oGeoPosROut ); // invoke linked filter(s)

		return; // RETURN
	}

	m_oImageSize.width = int(rImgSizeIn[0]);
	m_oImageSize.height = int(rImgSizeIn[1]);

	// (re)initialization of output structure
	reinitialize(rGeoProfileIn.ref());

	calcSimpleTracking(
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



void SimpleTracking::reinitialize(const geo2d::VecDoublearray	&p_rProfileIn) {
	// there is always 1 line from construction
	m_oSeamPosL.assign(p_rProfileIn.size(), 0, eRankMin);	// (re)initialize output based on input dimension
	m_oSeamPosR.assign(p_rProfileIn.size(), 0, eRankMin);	// (re)initialize output based on input dimension
} // reinitialize



// actual signal processing
/*static*/
void SimpleTracking::calcSimpleTracking(
		const geo2d::VecDoublearray	&p_rProfileIn,
		const geo2d::Doublearray		&p_rPositionIn,
		ComparisonType				p_oComparisonType,
		const std::pair<double, double>	p_oThresholds,
		Doublearray					&p_rPosLOut,
		Doublearray					&p_rPosROut
	)
{
	// validate preconditions

	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.
	poco_assert_dbg(! p_rProfileIn.empty()); // there must be at least one profile line
	poco_assert_dbg(! p_rPositionIn.getData().empty()); // there must be at least one profile line

	std::function<bool (double, double)>	oCompFtor		= std::greater<double>();	// comparison type functor object
	if (p_oComparisonType == filter::eLess) {
		oCompFtor	= std::less<double>();						// comparison type functor object
	}
	else if (p_oComparisonType == filter::eGreaterEqual) {
		oCompFtor	= std::greater<double>();					// comparison type functor object
	}
	else {
		oCompFtor	= std::less<double>();
		std::ostringstream	oMsg;
		oMsg << "WARNING: Invalid ComparisonType '" << p_oComparisonType << "' in filter '" << m_oFilterName << "'." << std::endl;
		wmLog(eWarning, "%s", oMsg.str().c_str());
	}
	poco_assert_dbg( oCompFtor != nullptr );

	auto&	rSeamPosL	= p_rPosLOut.getData();
	auto&	rSeamPosR	= p_rPosROut.getData();

	int oLineN = 0;
	for (auto oIt = p_rProfileIn.begin(); oIt != p_rProfileIn.end(); ++oIt) { // loop over N profiles

		const auto&	rInData		= oIt->getData();

		const int		oStart	= int(p_rPositionIn.getData()[oLineN]);
		int				oStepsL	= 0;
		int				oStepsR	= 0;
		// starting from extremum search to the left
		while (oCompFtor(rInData[oStart + oStepsL], p_oThresholds.first) &&  oStart + oStepsL > 0) {
			--oStepsL;
		}
		// starting from extremum search to the right
		while (oCompFtor(rInData[oStart + oStepsR], p_oThresholds.second) &&  oStart + oStepsR < int( rInData.size()-1 )) {
			++oStepsR;
		}

		const double	oSeamPosL	= oStart + oStepsL;
		const double	oSeamPosR	= oStart + oStepsR;
		//const double	oY			= (p_oImageHeight / p_rProfileIn.size()) * oLineN;
		rSeamPosL[oLineN]	= oSeamPosL;
		rSeamPosR[oLineN]	= oSeamPosR;

		++oLineN;
	}

	// rank calculation

	const int		oLineLength		= p_rProfileIn.front().getData().size();
	calcRank(p_rPosLOut, p_rPosROut, oLineLength, 10); // TODO make boundary length parameter

} // calcSimpleTracking


} // namespace filter
} // namespace precitec
