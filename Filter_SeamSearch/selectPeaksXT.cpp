/*!
*  @copyright		Precitec Vision GmbH & Co. KG
*  @author			Claudius Batzlen (CB)
*  @date			2019
*  @file
*  @brief			Fliplib filter 'SelectPeaks' in component 'Filter_SeamSearch'. Calculates right and left seam position.
*/


#include "selectPeaksXT.h"

#include "seamSearch.h"								///< input check, rank calculation

#include "system/types.h"							///< typedefs
#include "common/defines.h"							///< debug assert integrity assumptions
#include "module/moduleLogger.h"

#include "overlay/overlayPrimitive.h"				///< paint overlay
#include "image/image.h"							///< BImage

#include "filter/algoArray.h"						///< Doublearray algo

#include <limits>									///< min int
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace image;
	using namespace interface;
	using namespace geo2d;
namespace filter {


const std::string SelectPeaksXT::m_oFilterName 	= std::string("SelectPeaksXT");
const std::string SelectPeaksXT::m_oPipeOutName1	= std::string("ContourLeft");
const std::string SelectPeaksXT::m_oPipeOutName2	= std::string("ContourRight");


SelectPeaksXT::SelectPeaksXT() :
TransformFilter					( m_oFilterName, Poco::UUID{"3DFE0D89-D80B-4DC9-B3E7-F15AB25C072D"} ),
	m_pPipeInGradientLeft		( nullptr ),
	m_pPipeInGradientRight		( nullptr ),
	m_pPipeInMaxFLenght			( nullptr ),
	m_pPipeInImgSize			( nullptr ),
	m_pPipeInDefaultSeamWidth   ( nullptr ),
	m_oPipeOutContourLeft		( this, m_oPipeOutName1 ),
	m_oPipeOutContourRight		( this, m_oPipeOutName2 ),
	m_oMaxFilterLenght			( 20 ),
	m_oDefaultSeamWidth			( 220 ),
	m_oThresholdLeft			( 4 ),
	m_oThresholdRight			( 4 )
{
	// Defaultwerte der Parameter setzen
	//parameters_.add("DefaultSeamWidth",		Parameter::TYPE_int, m_oDefaultSeamWidth);
	parameters_.add("ThresholdLeft",		Parameter::TYPE_int, m_oThresholdLeft);
	parameters_.add("ThresholdRight",		Parameter::TYPE_int, m_oThresholdRight);

    setInPipeConnectors({{Poco::UUID("08387983-28ED-4F0C-A499-051398141588"), m_pPipeInGradientLeft, "Line", 1, "gradient_left"},
    {Poco::UUID("7058DFD7-10C8-4D24-B8BF-F41B3AB00F2F"), m_pPipeInGradientRight, "Line", 1, "gradient_right"},
    {Poco::UUID("254C0EC7-23E5-4434-B28B-00C984BDC3C3"), m_pPipeInMaxFLenght, "MaxFilterLength", 1, "max_filter_length"},
    {Poco::UUID("E878B0BB-EBA6-48FD-BB53-CC9F06E21428"), m_pPipeInImgSize, "ImageSize", 1, "image_size"},
    {Poco::UUID("76C86B39-B123-45B8-8FBF-E7A818854486"), m_pPipeInDefaultSeamWidth, "DefaultSeamWidth", 1, "default_seamWidth"}});
    setOutPipeConnectors({{Poco::UUID("F4A2FA25-0598-4879-BAAC-321937CC9DD1"), &m_oPipeOutContourLeft, m_oPipeOutName1, 0, ""},
    {Poco::UUID("16BBB444-BC94-4B91-83FF-EC3AA8512B08"), &m_oPipeOutContourRight, m_oPipeOutName2, 0, ""}});
    setVariantID(Poco::UUID("5CC471B9-E247-4BB9-9BE2-AD73AC18DBB2"));
} // SelectPeaks



void SelectPeaksXT::setParameter() {
	TransformFilter::setParameter();
	//m_oDefaultSeamWidth		= parameters_.getParameter("DefaultSeamWidth").convert<int>();
	m_oThresholdLeft		= parameters_.getParameter("ThresholdLeft").convert<int>();
	m_oThresholdRight		= parameters_.getParameter("ThresholdRight").convert<int>();

	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	poco_assert_dbg(m_oDefaultSeamWidth > 0);
} // setParameter



void SelectPeaksXT::paint() {
	if(m_oVerbosity <= eNone || m_oContourLeftOut.size() == 0) {
		return;
	} // if

	// paint contour points

	const GeoVecDoublearray &rGradientLeftIn	= m_pPipeInGradientLeft->read(m_oCounter);
	poco_assert_dbg( ! rGradientLeftIn.ref().empty() );
	const int					oCrossRadius			( 4 );
	const unsigned int			oNProfiles				( rGradientLeftIn.ref().size() );
	const int					oDeltaY					( m_oImageSize.height / oNProfiles );
	int							oY						( oDeltaY / 2 );

    auto oSpTrafo = rGradientLeftIn.context().trafo();
    if (oSpTrafo.isNull())
    {
        return;
    }
	const Trafo&				rTrafo					( *oSpTrafo );
	OverlayCanvas&				rOverlayCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer&				rLayerPosition			( rOverlayCanvas.getLayerPosition());
	OverlayLayer&				rLayerText				( rOverlayCanvas.getLayerText());

	for (unsigned int sliceN = 0; sliceN < oNProfiles; ++sliceN) { // loop over N profiles
		const Point			oPositionLeft	( int(m_oContourLeftOut.getData()[sliceN]), oY );
		const Color			oColorL			( Color::Yellow() );
		const Color			oColorR			( Color::Magenta() );
		std::stringstream oTmpL; oTmpL << m_oContourLeftOut.getRank()[sliceN];
		rLayerPosition.add(new  OverlayCross(rTrafo(oPositionLeft), oCrossRadius, oColorL ));
 		rLayerText.add(new OverlayText(oTmpL.str(), image::Font(), rTrafo(Rect(oPositionLeft.x, oPositionLeft.y, 30, 16)), oColorL ));

		const Point oPositionRight( int(m_oContourRightOut.getData()[sliceN]), oY );
		std::stringstream oTmpR; oTmpR << m_oContourRightOut.getRank()[sliceN];
		rLayerPosition.add(new  OverlayCross( rTrafo(oPositionRight), oCrossRadius, oColorR ));
		rLayerText.add(new OverlayText( oTmpR.str(), image::Font(), rTrafo(Rect(oPositionRight.x, oPositionRight.y, 30, 16)), oColorR ));

		oY += oDeltaY;
	}
} // paint



bool SelectPeaksXT::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "gradient_left") {
		m_pPipeInGradientLeft  = dynamic_cast< line_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "gradient_right") {
		m_pPipeInGradientRight  = dynamic_cast< line_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "max_filter_length") {
		m_pPipeInMaxFLenght = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "image_size") {
		m_pPipeInImgSize		= dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "default_seamWidth") {
		m_pPipeInDefaultSeamWidth = dynamic_cast< scalar_pipe_t* >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void SelectPeaksXT::proceedGroup(const void* sender, PipeGroupEventArgs& e) {

	poco_assert_dbg(m_pPipeInGradientLeft != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInGradientRight != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInMaxFLenght != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImgSize != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDefaultSeamWidth != nullptr); // to be asserted by graph editor


	// get data from frame

	const GeoVecDoublearray rGradientLeftIn		= m_pPipeInGradientLeft->read(m_oCounter);
	const GeoVecDoublearray rGradientRightIn	= m_pPipeInGradientRight->read(m_oCounter);
	const GeoDoublearray	rGeoImgSizeIn		= m_pPipeInImgSize->read(m_oCounter);
	const auto&				rImgSizeIn			= rGeoImgSizeIn.ref().getData();

	// input validity check

	const bool oDefaultSeamWTooHigh	( [&, this]()->bool{
		if (static_cast<unsigned int>(m_oDefaultSeamWidth) >= rGradientLeftIn.ref().front().size()) {
			wmLog(eWarning, "Default seam width is too high.\n");
			return true;
		}
		return false; }() );

	if ( inputIsInvalid( rGradientLeftIn) || inputIsInvalid(rGradientRightIn ) || inputIsInvalid(rGeoImgSizeIn) || rGradientLeftIn.ref().size() != rGradientRightIn.ref().size() || oDefaultSeamWTooHigh ) {
		const GeoDoublearray oGeoSeamPosLeftOut		( rGradientLeftIn.context(), m_oContourLeftOut, rGradientLeftIn.analysisResult(), 0.0 ); // bad rank
		const GeoDoublearray oGeoSeamPosRightOut	( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), 0.0 ); // bad rank
		preSignalAction();
		m_oPipeOutContourLeft.	signal( oGeoSeamPosLeftOut );
		m_oPipeOutContourRight.	signal( oGeoSeamPosRightOut );

		return; // RETURN
	}

	m_oImageSize.width		= int( rImgSizeIn[0] );
	m_oImageSize.height		= int( rImgSizeIn[1] );
	m_oMaxFilterLenght		= int( m_pPipeInMaxFLenght->read(m_oCounter).ref().getData().front() );
	m_oDefaultSeamWidth		= int( m_pPipeInDefaultSeamWidth->read(m_oCounter).ref().getData().front());
	poco_assert_dbg( ! rGradientLeftIn.ref().empty() );

	reinitialize( rGradientLeftIn.ref(), rGradientRightIn.ref() ); // (re)initialization of output structure

	calcSelectPeaks(
		rGradientLeftIn.ref(),
		rGradientRightIn.ref(),
		m_oMaxFilterLenght,
		m_oDefaultSeamWidth,
		m_oThresholdLeft,
		m_oThresholdRight,
		m_oContourLeftOut,
		m_oContourRightOut
	); // signal processing

	enforceIntegrity (m_oContourLeftOut, m_oContourRightOut, m_oImageSize.width, m_oDefaultSeamWidth); // validate integrity

	const double oNewRankLeft	= (rGradientLeftIn.rank()	+ 1.0) / 2.; // full rank
	const double oNewRankRight	= (rGradientRightIn.rank()	+ 1.0) / 2.; // full rank
	const GeoDoublearray oGeoSeamPosLeftOut		( rGradientLeftIn.context(), m_oContourLeftOut, rGradientLeftIn.analysisResult(), oNewRankLeft );
	const GeoDoublearray oGeoSeamPosRightOut	( rGradientRightIn.context(), m_oContourRightOut, rGradientRightIn.analysisResult(), oNewRankRight );

	preSignalAction();
	m_oPipeOutContourLeft.	signal( oGeoSeamPosLeftOut );
	m_oPipeOutContourRight.	signal( oGeoSeamPosRightOut );

} // proceed



void SelectPeaksXT::reinitialize(
		const VecDoublearray		&p_rGradientLeftIn,
		const VecDoublearray		&p_rGradientRightIn
	) {
	m_oContourLeftOut.assign( p_rGradientLeftIn.size(), 0, eRankMin ); // (re)initialize output based on input dimension
	m_oContourRightOut.assign( p_rGradientRightIn.size(), 0, eRankMin ); // (re)initialize output based on input dimension
} // reinitialize



// actual signal processing
void SelectPeaksXT::calcSelectPeaks(
	const VecDoublearray		&p_rGradientLeftIn,
	const VecDoublearray		&p_rGradientRightIn,
	int						p_oMaxFilterLenght,
	int						p_oDefaultSeamWidth,
	int						p_oThresholdLeft,
	int						p_oThresholdRight,
	Doublearray				&p_rContourLeftOut,
	Doublearray				&p_rContourRightOut
	)
{

	// Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	poco_assert_dbg( p_rGradientLeftIn.size()	==  p_rGradientRightIn.size());
	poco_assert_dbg( ! p_rGradientLeftIn.empty() );
	poco_assert_dbg( p_oMaxFilterLenght			>   0);

	const unsigned int oNProfiles				= p_rGradientLeftIn.size();
	const unsigned int oProfileSize				= p_rGradientLeftIn.front().getData().size();
	const unsigned int oMaxFilterLengthMinusOne	= p_oMaxFilterLenght - 1;

	poco_assert_dbg( static_cast<unsigned int>(p_oMaxFilterLenght) <= oProfileSize ); // Parameter assertion. Should be pre-checked by UI / MMI / GUI.

	// get references to data

	auto&	rContourLeftOutData		= p_rContourLeftOut.getData();
	auto&	rContourRightOutData	= p_rContourRightOut.getData();

	for (unsigned int profileN = 0; profileN < oNProfiles; ++profileN) { // loop over N profiles

		// range without boundary values
		auto		oItGradL		= p_rGradientLeftIn[profileN].getData().begin()		+ oMaxFilterLengthMinusOne;
		auto		oItGradR		= p_rGradientRightIn[profileN].getData().rbegin()	+ oMaxFilterLengthMinusOne;

		const auto	oItStartGradL	= p_rGradientLeftIn[profileN].getData().begin()		+ oMaxFilterLengthMinusOne;
		const auto	oItStartGradR	= p_rGradientRightIn[profileN].getData().begin()	+ oMaxFilterLengthMinusOne;
		const auto	oItEndGradL		= p_rGradientLeftIn[profileN].getData().end()		- oMaxFilterLengthMinusOne;
		//const auto	oItEndGradR		= p_rGradientRightIn[profileN].getData().end()		- oMaxFilterLengthMinusOne;

		vec_double_cit_t	oItAboveTholdL;
		vec_double_crit_t	oItAboveTholdR;
		const int		oMaxInit	= 0; // (re)initialisation value (small: 0 or std::limits::numeric_min)
		double			oMaxL		= oMaxInit;
		double			oMaxR		= oMaxInit;
		bool			oIsNewMaxL	= false;
		bool			oIsNewMaxR	= false;

		m_oPeaksL.clear();
		m_oPeaksR.clear();

		// same lenght for left and right asserted, therefore only one loop
		for (; oItGradL != oItEndGradL; ++oItGradL, ++oItGradR) { // loop over columns wo boundaries

			// detect maxima

			// new maximum if curr value bigger than threshold and bigger than old max and bigger than prev value (rising curve)
			if ( *oItGradL >= p_oThresholdLeft && *oItGradL > oMaxL && *oItGradL > *(oItGradL - 1) ) {
				oItAboveTholdL	= oItGradL;
				oMaxL			= *oItGradL;
				oIsNewMaxL		= true;
			} // if
			// new maximum if curr value bigger than threshold and bigger than old max and bigger than prev value (rising curve)
			if ( *oItGradR >= p_oThresholdRight && *oItGradR > oMaxR && *oItGradR > *(oItGradR - 1)) {
				oItAboveTholdR	= oItGradR;
				oMaxR			= *oItGradR;
				oIsNewMaxR		= true;
			} // if

			// save maxima if next value smaller (falling curve)

			if (oIsNewMaxL && *oItGradL < oMaxL) { // new max found and current value smaller (falling curve)
				m_oPeaksL.push_back(oItGradL - 1); // save last max
				oMaxL		= oMaxInit;
				oIsNewMaxL	= false;
			} // if
			if (oIsNewMaxR && *oItGradR < oMaxR) { // new max found and current value smaller (falling curve)
				m_oPeaksR.push_back( oItGradR.base()/*- 1 is implicit*/ ); // get std iterator // save last max
				oMaxR		= oMaxInit;
				oIsNewMaxR	= false;
			} // if
		} // for

		auto		oItPeaksL		= m_oPeaksL.begin();
		auto		oItPeaksR		= m_oPeaksR.begin();

		//if (m_oVerbosity > eNone && &canvas<OverlayCanvas>(m_oCounter) != nullptr) {
		//	// debug peak lists

//	//	const GeoVecDoublearray rGradientLeftIn			( m_pPipeInGradientLeft->read(m_oCounter) );
//	//	const Trafo				&rTrafo					( *rGradientLeftIn.context().trafo() );
//	//	OverlayCanvas&				rOverlayCanvas		( canvas<OverlayCanvas>(m_oCounter) );
//	//	OverlayLayer&				rLayerPosition		( rOverlayCanvas.getLayerPosition());
//	//	OverlayLayer&				rLayerText			( rOverlayCanvas.getLayerText());
//	//	const int				oLinesPerSlice			( m_oImageSize.height / oNProfiles );
		//	OverlayLayer &rLayer = c.getLayer(OverlayCanvas::BASE_LAYER); // DEBUG
		//	for (; oItPeaksL != m_oPeaksL.end(); ++oItPeaksL) { // loop over left peaks
		//		const Point oPositionLeft( x, y );
		//		std::stringstream oTmpL; oTmpL << **oItPeaksL;
		//		rLayer.add( new  OverlayCross(rTrafo(oPositionLeft), oCrossRadius, Color::Orange() ) );
		//		rLayer.add( new OverlayText(oTmpL.str(), oFont, rTrafo(Rect(x, y, 30, 16)), Color::Orange() ) );
		//	}
		//		rLayerPosition.add( new  OverlayCross(rTrafo(oPositionLeft), oCrossRadius, Color::Orange() ) );
		//		rLayerText.add( new OverlayText(oTmpL.str(), oFont, rTrafo(Rect(x, y, 30, 16)), Color::Orange() ) );
		//		const int y = static_cast<int>((profileN+0.4)*oLinesPerSlice);
		//		const Point oPositionRight( x, y );
		//		std::stringstream oTmpR; oTmpR << **oItPeaksR;
		//		rLayer.add( new  OverlayCross(rTrafo(oPositionRight), oCrossRadius, Color::Cyan() ) );
		//		rLayer.add( new OverlayText(oTmpR.str(), image::Font(oFont), rTrafo(Rect(x, y, 30, 16)), Color::Cyan() ) );
		//	}
		//		rLayerPosition.add( new  OverlayCross(rTrafo(oPositionRight), oCrossRadius, Color::Cyan() ) );
		//		rLayerText.add( new OverlayText(oTmpR.str(), image::Font(oFont), rTrafo(Rect(x, y, 30, 16)), Color::Cyan() ) );


		vec_double_cit_t				oItPeaksBestL;
		vec_double_cit_t				oItPeaksBestR;

		const std::vector<vec_double_cit_t>::const_iterator	oItPeaksLEnd	= m_oPeaksL.end();
		const std::vector<vec_double_cit_t>::const_iterator	oItPeaksREnd	= m_oPeaksR.end();

		int oMinDiff = std::numeric_limits<int>::max();

		//std::cout << "m_oPeaksL.size() " << m_oPeaksL.size() << std::endl;
		//std::cout << "m_oPeaksR.size() " << m_oPeaksR.size() << std::endl;
		//std::cout << "profileN " << profileN << std::endl;
		for (oItPeaksL = m_oPeaksL.begin(); oItPeaksL != oItPeaksLEnd; ++oItPeaksL) { // loop over left peaks
			for (oItPeaksR = m_oPeaksR.begin(); oItPeaksR != oItPeaksREnd; ++oItPeaksR) { // loop over right peaks
				// distance between right and left index
				const int oPeakDistance = std::distance(oItStartGradR, *oItPeaksR) - std::distance(oItStartGradL, *oItPeaksL);
				const int oDiff			= std::abs(oPeakDistance - p_oDefaultSeamWidth);
				//std::cout << "Peak distance between " << std::distance(oItStartGradL, *oItPeaksL) + oMaxFilterLengthMinusOne << ", " << **oItPeaksL <<
				//	" and " << std::distance(oItStartGradR, *oItPeaksR) + oMaxFilterLengthMinusOne << ", " << **oItPeaksR << " is " << oPeakDistance << std::endl;
				if ( oDiff < oMinDiff ) {
					oMinDiff = oDiff;
					oItPeaksBestL = *oItPeaksL;
					oItPeaksBestR = *oItPeaksR;
				}
			} // for
		} // for

		// get indices

		unsigned int	oSeamPosLeft		= 0;
		unsigned int	oSeamPosRight		= 0;
		if (! m_oPeaksL.empty() && ! m_oPeaksR.empty()) {
			oSeamPosLeft		= std::distance(oItStartGradL, oItPeaksBestL) + oMaxFilterLengthMinusOne;
			oSeamPosRight		= std::distance(oItStartGradR, oItPeaksBestR) + oMaxFilterLengthMinusOne;
		}

		// set result

		rContourLeftOutData[profileN]	= oSeamPosLeft;
		rContourRightOutData[profileN]	= oSeamPosRight;

	} // for profileN

	// calculate rank
	calcRank(p_rContourLeftOut, p_rContourRightOut, oProfileSize, p_oMaxFilterLenght);

} // calcSelectPeaks




	} // namespace filter
} // namespace precitec
