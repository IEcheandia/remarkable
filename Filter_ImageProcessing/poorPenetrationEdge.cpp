/*!
 *  @copyright:	Precitec Vision GmbH & Co. KG
 *  @author			Claudius Batzlen (CB)
 *  @date			2017
 *  @file
 *  @brief			Fliplib filter 'poorPenetrationEdge' in component 'Filter_ImageProcessing'. Performs line detection.
 */

#define _USE_MATH_DEFINES						/// pi constant
#include "poorPenetrationEdge.h"
#include "lookup.h"								/// Provides lookup table for trigonometric sin and cos functions.

#include "system/platform.h"					/// global and platform specific defines
#include "system/tools.h"						/// poco bugcheck
#include "module/moduleLogger.h"
#include "filter/algoArray.h"					/// array algos
#include "overlay/overlayPrimitive.h"			/// overlay
#include "util/calibDataSingleton.h"
#include <fliplib/TypeToDataTypeImpl.h>

#include <cmath>								/// trigonometry, pi constant


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string PoorPenetrationEdge::m_oFilterName 		= std::string("PoorPenetrationEdge");
const std::string PoorPenetrationEdge::m_oPipeOutCandidateName = std::string("HoughPPCandidate");


const static std::size_t		LU_SIZE				= 360;					///< lookup table size. value experimentally found - tradeoff between high computation time and low quantization error
const static Lookup<LU_SIZE>	g_oLookup			= Lookup<LU_SIZE>();	///< lookup table for sin and cos
const static double				g_oMIN_ANGLE_RAD	= - M_PI;				///< define from cmath
const static double				g_oMAX_ANGLE_RAD	= + M_PI;				///< define from cmath


PoorPenetrationEdge::PoorPenetrationEdge() :
    TransformFilter             (PoorPenetrationEdge::m_oFilterName, Poco::UUID{"f1f22fe3-9497-4e0a-bacd-a8c9ad1e8480"}),
    m_pPipeInImageBinaryFrame   ( nullptr),
	m_pPipeInImageGreyFrame     ( nullptr),
	m_oPipeOutHoughPPCandidate  ( this, m_oPipeOutCandidateName),
	m_oMinAngle                 ( -90),
	m_oMaxAngle				    ( +90),
	m_oMinRadiusDistance	    ( 0.01 ),
	m_oSearchStart			    ( eLeft ),
	m_oMaxima0_PosX             (0),
	m_oMaxima0_PosY             (0),
	m_oMaxima1_PosX             (0),
	m_oMaxima1_PosY             (0)
{
	poco_assert_dbg(g_oMIN_ANGLE_RAD <= g_oMAX_ANGLE_RAD);

	// set parameter default values

	parameters_.add( "MinAngle",			Parameter::TYPE_int,	m_oMinAngle );
	parameters_.add( "MaxAngle",			Parameter::TYPE_int,	m_oMaxAngle );
	parameters_.add( "MinLineDistance", Parameter::TYPE_double, m_oMinRadiusDistance);
	parameters_.add( "SearchStart",			Parameter::TYPE_int, static_cast<int>(m_oSearchStart));

	m_oMaxima.reserve(50); // prevent allocations on 1st image

    setInPipeConnectors({{Poco::UUID("D80CAC6C-B6B1-4D25-A178-B02BFCECE77A"), m_pPipeInImageBinaryFrame, "Binary_Image", 1, "Binary_ImageIn"},
    {Poco::UUID("C92BA68D-FE83-4E4B-AE9D-C00FB18E0207"), m_pPipeInImageGreyFrame, "Grey_Image", 1, "Grey_ImageIn"}});
    setOutPipeConnectors({{Poco::UUID("60F5A19B-3A7B-46C1-AFD9-005CE2681B23"), &m_oPipeOutHoughPPCandidate, m_oPipeOutCandidateName, 0, ""}});
    setVariantID(Poco::UUID("9ad01e1e-ff0a-43f4-890d-2b1484920d86"));
} // Hough



/*virtual*/ void
PoorPenetrationEdge::setParameter()
{
	TransformFilter::setParameter();

	m_oMinAngle				= parameters_.getParameter("MinAngle").convert<int>();
	m_oMaxAngle				= parameters_.getParameter("MaxAngle").convert<int>();
	m_oMinRadiusDistance	= parameters_.getParameter("MinLineDistance");			// in mm
	m_oMinRadiusDistancePix = 0; //will be updated based on the current image
	m_oSearchStart = static_cast<HoughDirectionType>(parameters_.getParameter("SearchStart").convert<int>());
} // setParameter



/*virtual*/ bool
PoorPenetrationEdge::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{

	if (p_rPipe.tag() == "Binary_ImageIn")
	    m_pPipeInImageBinaryFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);
	if (p_rPipe.tag() == "Grey_ImageIn")
		m_pPipeInImageGreyFrame = dynamic_cast<image_pipe_t*>(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



/*virtual*/ void
PoorPenetrationEdge::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{

	poco_assert_dbg(m_pPipeInImageBinaryFrame != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInImageGreyFrame != nullptr); // to be asserted by graph editor

	// get data from frame
	const ImageFrame	&rFrameIn_Binary = m_pPipeInImageBinaryFrame->read(m_oCounter);
	const BImage		&rImageIn_Binary = rFrameIn_Binary.data();

	const ImageFrame	&rFrameIn_Grey = m_pPipeInImageGreyFrame->read(m_oCounter);
	const BImage		&rImageIn_Grey = rFrameIn_Grey.data();

	auto rContext = rFrameIn_Binary.context();
	m_oSpTrafo = rFrameIn_Binary.context().trafo();

	//update average radius distance in pixel (in the case of scheimpflug calibration, it's an approximation based on the current roi)
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

	// calibration factor in X
	double oFactX = rCalib.factorHorizontal(10,
		m_oSpTrafo->dx() + rContext.HW_ROI_x0 + (rFrameIn_Binary.data().width() / 2), //center (x) of current image in sensor coordinates
		m_oSpTrafo->dy() + rContext.HW_ROI_y0 + (rFrameIn_Binary.data().height() / 2)); //center (y) of current image in sensor coordinates

	m_oMinRadiusDistancePix = (unsigned int) (m_oMinRadiusDistance * oFactX);


	geo2d::HoughPPCandidatearray oOutCandidate;

	// input validity check

	HoughPPCandidate cand;

	if (!rImageIn_Binary.isValid() || m_oMinAngle >= m_oMaxAngle)
	{
		oOutCandidate.getData().push_back(cand);
		oOutCandidate.getRank().push_back(0);

		const GeoHoughPPCandidatearray oGeoOutHoughPPCandidate(rFrameIn_Binary.context(), oOutCandidate, rFrameIn_Binary.analysisResult(), interface::NotPresent);

		preSignalAction();

		m_oPipeOutHoughPPCandidate.signal(oGeoOutHoughPPCandidate);	// invoke linked filter(s)

		return; // RETURN
	}

	//Rank schon mal hochsetzen
//	m_oPosLeftOut.assign(1, 0, eRankMax);
//	m_oPosRightOut.assign(1, 0, eRankMax);

	calcHough(rImageIn_Binary, rImageIn_Grey, cand); // image processing

	oOutCandidate.getData().push_back(cand);
	oOutCandidate.getRank().push_back(255);

	const auto	oAnalysisResult = rFrameIn_Binary.analysisResult();
	const GeoHoughPPCandidatearray oGeoOutHoughPPCandidate(rFrameIn_Binary.context(), oOutCandidate, oAnalysisResult, Limit);
	preSignalAction();

	m_oPipeOutHoughPPCandidate.signal(oGeoOutHoughPPCandidate);	// invoke linked filter(s)
} // proceed


/*virtual*/ void
PoorPenetrationEdge::paint() {
	if (m_oVerbosity <= eNone || m_oMaxima.size() <= 1 || m_oSpTrafo.isNull())
	{
		return;
	} // if

	const ImageFrame&		rFrameIn(m_pPipeInImageBinaryFrame->read(m_oCounter));
	const BImage&			rImageIn(rFrameIn.data());
	const Size2d	        oSizeImgIn(rImageIn.size());
	const int				oHalfImgHeight(rImageIn.size().height / 2);

	const Point				oHoughOriginEnd(0, oHalfImgHeight); // at hough space origin, which lies at (0, half img height)
	const Point				oLineNormStart(oHoughOriginEnd);
	const Point				oLineNormEndMaxima0(m_oMaxima0_PosX, m_oMaxima0_PosY); // 1st hough line

	/**
	* @brief Given the normal vector of hough line, start and end point of hough line are calculated depending on length parameter.
	* @param p_oLineNormVecStartEnd	Start and end point of normal vector of hough line.
	* @param p_oLength					Length of line.
	* @return 							Start and end point of hough line.
	*/
	const auto oCalcHoughLine = [](std::pair<Point, Point> p_oLineNormVecStartEnd, int p_oLength)->std::pair<Point, Point>{
		const Point				oLineNormVec(p_oLineNormVecStartEnd.second - p_oLineNormVecStartEnd.first);
		const Point				oLineVecCross(oLineNormVec.y, -oLineNormVec.x); // line vector is cross product from line norm vector
		const auto				oLineVecCrossLength(std::sqrt(oLineVecCross.x * oLineVecCross.x + oLineVecCross.y * oLineVecCross.y));
		const DPoint			oLineVecCrossUnit(oLineVecCross.x / oLineVecCrossLength, oLineVecCross.y / oLineVecCrossLength);
		const Point				oLineVecHalf(roundToT<int>(oLineVecCrossUnit.x * p_oLength / 2), roundToT<int>(oLineVecCrossUnit.y * p_oLength / 2));
		const Point				oLineStart(p_oLineNormVecStartEnd.second + oLineVecHalf);	// length adjusted line vector added to line norm vector
		const Point				oLineEnd(p_oLineNormVecStartEnd.second - oLineVecHalf);	// length adjusted line vector substracted from line norm vector

		return std::make_pair(oLineStart, oLineEnd);
	};

	const auto				oHoughLineLeft = oCalcHoughLine(std::make_pair(oLineNormStart, oLineNormEndMaxima0), std::get<eMax>(m_oMaxima[0]));// sorted asc by rad -> lower rad (0) is left, higher (1) is right
	const Trafo&			rTrafo(*m_oSpTrafo);
	OverlayCanvas&			rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer&			rLayerLine(rCanvas.getLayerLine());
	OverlayLayer&			rLayerPosition(rCanvas.getLayerPosition());
	OverlayLayer&			rLayerText(rCanvas.getLayerText());
	const Color				oColorMaxima0(Color::Yellow()); // color analog to seam search positions
	const Color				oColorMaxima1(Color::Magenta()); // color analog to seam search positions

	if (m_oVerbosity >= eMedium) { // normal vector not that interesting
		rLayerLine.add(new  OverlayLine(rTrafo(oLineNormStart), rTrafo(oLineNormEndMaxima0), Color::Green()));
	} // if
	rLayerLine.add(new  OverlayLine(rTrafo(oHoughLineLeft.first), rTrafo(oHoughLineLeft.second), oColorMaxima0));
	rLayerPosition.add(new  OverlayCross(rTrafo(oLineNormEndMaxima0), oColorMaxima0));

	const Point				oLineNormEndRight(m_oMaxima1_PosX, m_oMaxima1_PosY); // 2nd hough line
	const auto				oHoughLineRight = oCalcHoughLine(std::make_pair(oLineNormStart, oLineNormEndRight), std::get<eMax>(m_oMaxima[1]));// sorted asc by rad -> lower rad (0) is left, higher (1) is right

	if (m_oVerbosity >= eMedium) { // normal vector not that interesting
		rLayerLine.add(new  OverlayLine(rTrafo(oLineNormStart), rTrafo(oLineNormEndRight), Color::Green()));
	} // if
	rLayerLine.add(new  OverlayLine(rTrafo(oHoughLineRight.first), rTrafo(oHoughLineRight.second), oColorMaxima1));
	rLayerPosition.add(new  OverlayCross(rTrafo(oLineNormEndRight), oColorMaxima1));

	// draw caption

	rLayerText.add(new OverlayText("1st Maxima", Font(16), rTrafo(Rect(10, m_oImageSize.height - 40, 200, 20)), oColorMaxima0));
	rLayerText.add(new OverlayText("2nd Maxima", Font(16), rTrafo(Rect(10, m_oImageSize.height - 20, 200, 20)), oColorMaxima1));


	if (m_oVerbosity < eMedium){
		return;
	} // if

	auto&		rLayerImage = rCanvas.getLayerImage();
	const auto	oTitle = OverlayText("Hough space", Font(), Rect(150, 18), Color::Black());

	rLayerImage.add(new OverlayImage(/*rTrafo*/(Point()), m_oHoughImage, oTitle));
} // paint


void
PoorPenetrationEdge::calcHough(const BImage& p_rImageIn_Binary, const BImage& p_rImageIn_Grey, HoughPPCandidate & houghCandidate)
{
	m_oImageSize = p_rImageIn_Binary.size();

	// Folgende Werte muessen beim houghCandidate gesetzt werden:
	// unsigned int m_oMeanBrightness; // mittlere Helligkeit zwischen den Linien
	// bool m_oLineIntersection; // gibt an, ob sich die beiden Linien im ROI schneiden

	// Geradenbezogen
	// unsigned int m_oPosition1; // Position im ROI
	// unsigned int m_oPosition2;

	// unsigned int m_oDiffToMiddle1; // DIfferenz zur Mitte
	// unsigned int m_oDiffToMiddle2;

	// double m_oNumberOfPixelOnLine1; // Anzahl Pixel auf der Linie
	// double m_oNumberOfPixelOnLine2;

	// unsigned int m_oBiggestInterruption1; //groesste zusammenhaengende Unterbrechung
	// unsigned int m_oBiggestInterruption2;

	// Danach m_oTwoLinesFound auf true setzen!

	const int		oImgWidth		= m_oImageSize.width;
	const int		oImgHeight		= m_oImageSize.height;
	const int		oHalfImgHeight	= roundToT<int>(oImgHeight / 2. );
	const int		oHalfImgWidth   = roundToT<int>(oImgWidth / 2. );
	const int		oDiagSize		= int( std::ceil( std::sqrt(oImgWidth * oImgWidth + oHalfImgHeight * oHalfImgHeight) ) ); // upper bound of max radius
	// see http://stackoverflow.com/questions/6884359/c-practical-computational-complexity-of-cmath-sqrt

	poco_assert_dbg( g_oLookup.size() != 0 );

	const double		oMinRad			= m_oMinAngle * M_PI / 180.;					// degree to rad
	const double		oMaxRad			= m_oMaxAngle * M_PI / 180.;					// degree to rad
	const int			oAngleMinInd	= angle2index<LU_SIZE>(oMinRad);				// map angle to lookup index
	const int			oAngleMaxInd	= angle2index<LU_SIZE>(oMaxRad);				// map angle to lookup index
	const int			oHistAngleSize	= oAngleMaxInd - oAngleMinInd + 1;
	const int			oHistRadiusSize	= oDiagSize;					// max radius is diagonale of half image

	poco_assert_dbg(g_oMIN_ANGLE_RAD	<= oMinRad && oMinRad <= g_oMAX_ANGLE_RAD);
	poco_assert_dbg(g_oMIN_ANGLE_RAD	<= oMaxRad && oMaxRad <= g_oMAX_ANGLE_RAD);
	poco_assert_dbg(oMinRad				<= oMaxRad);
	poco_assert_dbg(oAngleMinInd		>= 0);
	poco_assert_dbg(oAngleMaxInd		>= 0);
	poco_assert_dbg(oAngleMinInd		<= static_cast<int>( LU_SIZE ));
	poco_assert_dbg(oAngleMaxInd		<= static_cast<int>( LU_SIZE ));

	m_oHistogram.assign( oHistRadiusSize, std::vector<int>(oHistAngleSize, 0) ); // reset.

	houghCandidate.m_oTwoLinesFound = false;

	// fill hough histogram
	// in case of artefacts from the binarization at the border of the image
	// the binarization is calculated in 20x20 windows
	unsigned int startx = 20;
	unsigned int starty = 20;

	for (int oY = starty; oY < oImgHeight-20; ++oY)
	{
		const byte	*pLineInCur = p_rImageIn_Binary[oY]; // transform hough space y index
		for (int oX = startx; oX < oImgWidth-20; ++oX)
		{
			if (pLineInCur[oX] == 0) // accumulate over all non-zero positions
			{
				continue;
			} // if

			for (int oAngleInd = oAngleMinInd; oAngleInd < oAngleMaxInd; ++oAngleInd)  // all angles of our hough space
			{
				const int	oYOff = oY - oHalfImgHeight; // the origin of our hough space lies at (0, oHalfImgHeight)
				const int	oRadius = roundToT<int>( std::abs(
					oX		* g_oLookup.m_oCos[oAngleInd] +
					oYOff	* g_oLookup.m_oSin[oAngleInd] ) );
				const int	oAngleHist	= oAngleInd - oAngleMinInd; // map angle lookup index to histogram index
				poco_assert_dbg(oAngleHist >= 0);
				poco_assert_dbg(oAngleHist < oHistAngleSize);
				poco_assert_dbg(oRadius < oHistRadiusSize);
				++ m_oHistogram[oRadius][oAngleHist];
			} // for
		} // for
	} // for

	// find maximum in hough space

	m_oMaxima.clear();
	auto oHoughMax	= std::make_tuple(0, 0, 0);
	auto oPrevVal	= 0;
	auto oIsNewMax	= false;
	int oRadInd		= 0;

	if (m_oSearchStart == eRight)		// from right
	{
		for (oRadInd = oHistRadiusSize-1; oRadInd >= 0; --oRadInd)
		{
			for (int oPhiInd = 0; oPhiInd < oHistAngleSize; ++oPhiInd)
			{
				const auto oCurrVal = m_oHistogram[oRadInd][oPhiInd];

				// save maxima if is new maximum and next value smaller (falling curve)
				if (oIsNewMax && oCurrVal < oPrevVal)
				{
					m_oMaxima.push_back(oHoughMax);
					oIsNewMax = false;
					oHoughMax = std::make_tuple(0, 0, 0);
				} // if

				// new maximum if curr val bigger than old max and bigger than prev value (rising curve)

				if (oCurrVal > std::get<eMax>(oHoughMax) && oCurrVal > oPrevVal)
				{
					oHoughMax = std::make_tuple(oCurrVal, oRadInd, oPhiInd + oAngleMinInd); // add min angle offset to phi
					oIsNewMax = true;
				} // if

				oPrevVal = oCurrVal;
			} // for
		} // for
	}
	else // from left
	{
		for (oRadInd = 0; oRadInd < oHistRadiusSize; ++oRadInd)
		{
			for (int oPhiInd = 0; oPhiInd < oHistAngleSize; ++oPhiInd)
			{
				const auto oCurrVal = m_oHistogram[oRadInd][oPhiInd];

				// save maxima if is new maximum and next value smaller (falling curve)
				if (oIsNewMax && oCurrVal < oPrevVal)
				{
					m_oMaxima.push_back(oHoughMax);
					oIsNewMax = false;
					oHoughMax = std::make_tuple(0, 0, 0);
				} // if

				// new maximum if curr val bigger than old max and bigger than prev value (rising curve)

				if (oCurrVal > std::get<eMax>(oHoughMax) && oCurrVal > oPrevVal)
				{
					oHoughMax = std::make_tuple(oCurrVal, oRadInd, oPhiInd + oAngleMinInd); // add min angle offset to phi
					oIsNewMax = true;
				} // if

				oPrevVal = oCurrVal;
			} // for
		} // for
	}
	if (m_oMaxima.size() < 2) {
		wmLog(eDebug, "WARNING - only 1 hough line found.\n");

		return;
	} // if

	// sort all maxima desc by maximum value
	const auto oPredSortMaxDesc = [](const triple_int_t& p_rLhs, const triple_int_t& p_rRhs)->bool{
		return std::get<eMax>(p_rLhs) > std::get<eMax>(p_rRhs);};	// sorted desc by length resp. max

	std::sort(std::begin(m_oMaxima), std::end(m_oMaxima), oPredSortMaxDesc);

	// First search with Maxima 0
	// proceed from the 2nd entry, comparing it to previous entry until radius differnce is bigger than min radius distance
	// then take middle value, keep it, and delete all previous (similar in radius) values

	bool bDistFound = false;
	int iStartLoop = 1;

	for (auto oIndex = iStartLoop; oIndex < (int)m_oMaxima.size(); ++oIndex) { // start with 2nd entry
		const unsigned int	oRadiusDiff = std::abs(std::get<eRad>(m_oMaxima[oIndex]) - std::get<eRad>(m_oMaxima[0]));
		if ((oRadiusDiff > m_oMinRadiusDistancePix) && (oIndex != (int)m_oMaxima.size() )) { // bigger than min radius distance, but not if it's the whole vector
			// take the next value inside the range, remove all other entrys inbetween
			if (oIndex > iStartLoop)
				m_oMaxima.erase(std::begin(m_oMaxima)+1, std::begin(m_oMaxima) + oIndex ); // deletes including oIndex - 1

			bDistFound = true;

			break;
		} // if
	} // for

	//Second chance mit Maxima 1
	if (!bDistFound)
	{
		// find next Maxima to compare
		for (auto oIndex = 1; oIndex < (int)m_oMaxima.size()-1; ++oIndex)
		{
			auto oRadiusDist		= std::abs(std::get<eRad>(m_oMaxima[oIndex]) - std::get<eRad>(m_oMaxima[0]));
			auto oRadiusDistPrev	= std::abs(std::get<eRad>(m_oMaxima[oIndex+1]) - std::get<eRad>(m_oMaxima[0]));
			if (oRadiusDist >= (int)m_oMinRadiusDistancePix)
			{
				if ((oRadiusDistPrev >= (int)m_oMinRadiusDistancePix) && (oRadiusDistPrev < oRadiusDist))
				{
					std::swap(m_oMaxima[1], m_oMaxima[oIndex+1]);
					break;
				}
				else
				{
					if (oIndex != 1)
						std::swap(m_oMaxima[1], m_oMaxima[oIndex]);
					break;
				}
			}
		}
		// Range check
		iStartLoop = 2;
		for (auto oIndex = iStartLoop; oIndex < (int)m_oMaxima.size(); ++oIndex) { // start with 3nd entry
			const unsigned int	oRadiusDiff = std::abs(std::get<eRad>(m_oMaxima[oIndex]) - std::get<eRad>(m_oMaxima[1]));
			if ((oRadiusDiff > m_oMinRadiusDistancePix) && (oIndex != (int)m_oMaxima.size() )) { // bigger than min radius distance, but not if it's the whole vector
				// take the next value inside the range, remove all other entrys inbetween

				 m_oMaxima.erase(std::begin(m_oMaxima));				// delete m_oMaxima[0]
				 if (oIndex > iStartLoop)
					m_oMaxima.erase(std::begin(m_oMaxima) +1, std::begin(m_oMaxima) + oIndex-1); // deletes including oIndex - 1 because Maxima0 is deleted

				 bDistFound = true;
				 break;
			} // if
		} // for
	}
	const auto oMax	= std::get<eMax>(m_oMaxima.front());
	if (oMax == 0) {
		wmLog(eDebug, "WARNING - hough space max value is zero.\n");
		return;
	} // if

	if (!bDistFound)
	{
		// no Position in min/max angle Range (gap distance)
		// set rank 0
		wmLog(eDebug, "WARNING - no second hough line found -> Rank 0.\n");
	}

	const auto oRadMaxima0 = std::get<eRad>(m_oMaxima[0]);
	const auto oRadMaxima1 = std::get<eRad>(m_oMaxima[1]);
	const auto oPhiMaxima0 = std::get<ePhi>(m_oMaxima[0]);
	const auto oPhiMaxima1 = std::get<ePhi>(m_oMaxima[1]);

	// Calculate of line parameters (m and b, y=m*x+b) of the 2 found hough lines for intersection check
	if (oRadMaxima0 > 0 && oRadMaxima0 <= oImgWidth
		&& oPhiMaxima0 >= 0 && oPhiMaxima0 <= 360)
	{
		// LinePos and Line Angle in range
		m_oMaxima0_LineFkt_b = calcLineFktParameter_b(oRadMaxima0, oPhiMaxima0);
		m_oMaxima0_LineFkt_m = calcLineFktParameter_m(oPhiMaxima0);
	}
	else
	{
		// LinePos and Line Angle out of range
		m_oMaxima0_LineFkt_b = 1;
		m_oMaxima0_LineFkt_m = 1;
	}

	if (oRadMaxima1 > 0 && oRadMaxima1 <= oImgWidth
		&& oPhiMaxima1 >= 0 && oPhiMaxima1 <= 360)
	{
		// LinePos and Line Angle in range
		m_oMaxima1_LineFkt_b = calcLineFktParameter_b(oRadMaxima1, oPhiMaxima1);
		m_oMaxima1_LineFkt_m = calcLineFktParameter_m(oPhiMaxima1);
	}
	else
	{
		// LinePos and Line Angle out of range
		m_oMaxima1_LineFkt_b = 5;
		m_oMaxima1_LineFkt_m = 1;
	}

	// Set values for Candidates
	houghCandidate.m_oTwoLinesFound = true;
	houghCandidate.m_oNumberOfPixelOnLine1 = std::get<eMax>(m_oMaxima[0]) / (oImgHeight * 0.01);   // in % relativ to image height
	houghCandidate.m_oNumberOfPixelOnLine2 = std::get<eMax>(m_oMaxima[1]) / (oImgHeight * 0.01);   // in % relativ to image height
	houghCandidate.m_oPosition1 = oRadMaxima0;
	houghCandidate.m_oPosition2 = oRadMaxima1;
	houghCandidate.m_oDiffToMiddle1 = abs(oHalfImgWidth - oRadMaxima0);
	houghCandidate.m_oDiffToMiddle2 = abs(oHalfImgWidth - oRadMaxima1);
	// Calculate line Intersection, line Interruptions and GreyScale of gap
	calcRoiLinePos(m_oImageSize, m_oMaxima[0], m_oMaxima[1]);
	houghCandidate.m_oLineIntersection = calcLineIntersection(m_oImageSize, oRadMaxima0, oRadMaxima1);
	calcLineInterupttions(p_rImageIn_Binary, m_oMaxima[0], m_oMaxima[1], houghCandidate);
	houghCandidate.m_oMeanBrightness = calcGreyScaleOfGap(p_rImageIn_Grey);
	houghCandidate.m_oHeightRoi = oImgHeight;

	// Calc position X/Y of hough lines
	///////////////////////////////

	const	auto	oRadMax0 = std::get<eRad>(m_oMaxima[0]);	// sorted asc by rad -> lower rad (0) is left, higher (1) is right
	const	auto	oPhiMax0 = std::get<ePhi>(m_oMaxima[0]);	// sorted asc by rad -> lower rad (0) is left, higher (1) is right
	const	auto	oRadMax1 = std::get<eRad>(m_oMaxima[1]);	// sorted asc by rad -> lower rad (0) is left, higher (1) is right
	const	auto	oPhiMax1 = std::get<ePhi>(m_oMaxima[1]);	// sorted asc by rad -> lower rad (0) is left, higher (1) is right



	m_oMaxima0_PosX = roundToT<int>(oRadMax0 * g_oLookup.m_oCos[oPhiMax0]);	// angels [-PI; PI] - doublecheck with lookup generator
	m_oMaxima0_PosY = roundToT<int>(oRadMax0 * g_oLookup.m_oSin[oPhiMax0]);	// angels [-PI; PI] - doublecheck with lookup generator
	m_oMaxima0_PosY += oHalfImgHeight;                                      //  move origin from (0, halfimgsize) to (0, 0)

	m_oMaxima1_PosX = roundToT<int>(oRadMax1 * g_oLookup.m_oCos[oPhiMax1]);	// angels [-PI; PI] - doublecheck with lookup generator
	m_oMaxima1_PosY = roundToT<int>(oRadMax1 * g_oLookup.m_oSin[oPhiMax1]);	// angels [-PI; PI] - doublecheck with lookup generator
	m_oMaxima1_PosY += oHalfImgHeight;                                          //  move origin from (0, halfimgsize) to (0, 0)

	// if medium verbosity, normalize and paint hough space as image
	if (m_oVerbosity >= eMedium) {

		wmLog(eDebug, "PoorPenetrationEdge:proceed  -> Edge Position Maxima0/Maxima1: %i, %i\n", m_oMaxima0_PosX, m_oMaxima1_PosX);


		m_oHoughImage.resize({ oHistAngleSize, oHistRadiusSize });

		// map feature values to 8 bit

		for (int oRad = 0; oRad < oHistRadiusSize; ++oRad) {
			byte		*pLine		= m_oHoughImage[oRad];
			for (int oPhi = 0; oPhi < oHistAngleSize; ++oPhi) {
				// treat hough space as image. phi hist and img x sizes are equal
				pLine[oPhi] = static_cast<byte>( double(m_oHistogram[oRad][oPhi]) / oMax * 255 ); // map to 8 bit
			} // for
		} // for
	} // if
} // calcHough


/***********************************************************************
* Description:  Calculate of the line Paramters m and b                *
*                                                                      *
* Parameter:                                                           *
************************************************************************/

double PoorPenetrationEdge::calcLineFktParameter_m(int theta)
{
	if (theta == 180)
	{
		return 0;
	}
	return (g_oLookup.m_oCos[theta-90] / -g_oLookup.m_oSin[theta-90]);
}
double PoorPenetrationEdge::calcLineFktParameter_b(int rho, int theta)
{
	if (theta == 180)
	{
		return rho;
	}

	return (rho / g_oLookup.m_oSin[theta-90]);
}

int PoorPenetrationEdge::getHoughLinePosX(int y, int rho, int theta)
{
	// Berechnung x-Koordinate aus rho und theta
	//int x = (rho- y * cos(theta*pi/180))/sin(theta*pi/180);
	int x = (int)(((double)rho - (double)y * g_oLookup.m_oCos[theta-90])
		/ -(g_oLookup.m_oSin[theta-90])		);

	return x;

}

/***********************************************************************
* Description:  Calculation of Hough line Positions in ROI             *
*                                                                      *
* Parameter:                                                           *
************************************************************************/

void PoorPenetrationEdge::calcRoiLinePos(const Size2d	oSizeImgIn, triple_int_t ResLine1, triple_int_t ResLine2)
{

	int iLinePos1 = std::get<eRad>(ResLine1);
	int iLinePos2 = std::get<eRad>(ResLine2);
	int iLineAngle1 = std::get<ePhi>(ResLine1);
	int iLineAngle2 = std::get<ePhi>(ResLine2);

	// plausiblen rho und theta Linie 1
	if (iLinePos1 > 0 && iLinePos1 <= oSizeImgIn.width)
	{
		// X-Position Hough-Linie im Hough-ROI
		m_oXPosRoiStartLine1 = getHoughLinePosX(0, iLinePos1, iLineAngle1);
		m_oXPosRoiEndLine1 = getHoughLinePosX(oSizeImgIn.height, iLinePos1, iLineAngle1);
	}
	else if (iLinePos1 == 0)
	{
		m_oXPosRoiStartLine1 = 0;
		m_oXPosRoiEndLine1 = oSizeImgIn.height;
	}

	// plausiblen rho und theta Linie 2
	if (iLinePos2 > 0 && iLinePos2 <= oSizeImgIn.width)
	{
		// X-Position Hough-Linie im Hough-ROI
		m_oXPosRoiStartLine2 = getHoughLinePosX(0, iLinePos2, iLineAngle2);
		m_oXPosRoiEndLine2 = getHoughLinePosX(oSizeImgIn.height, iLinePos2, iLineAngle2);
	}
	else if (iLinePos2 == 0)
	{
		m_oXPosRoiStartLine2 = 0;
		m_oXPosRoiEndLine2 = oSizeImgIn.height;
	}
}

/***********************************************************************
* Description:  Intersection check of the 2 hough lines                *
*                                                                      *
* Parameter:                                                           *
************************************************************************/

bool PoorPenetrationEdge::calcLineIntersection(const Size2d	oSizeImgIn, const int iLinePos1, const int iLinePos2)
{
	double dSub_b, dSub_m;

	if (m_oMaxima0_LineFkt_m == 0 && m_oMaxima1_LineFkt_m != 0)
	{
		int y = (int)(m_oMaxima1_LineFkt_m * iLinePos1 + m_oMaxima1_LineFkt_b) * -1;
		// dazughoeriges y berechnen
		//printf("y %d\n",y);

		// ist y ausserhalb Hough-ROI
		if (y < 0 || y > oSizeImgIn.height)
			//if(y < 0 || y >  (int)BorgGreyImg_.roi().dy)
		{
			return false;
		}

		return true;
	}

	if (m_oMaxima1_LineFkt_m == 0 && m_oMaxima0_LineFkt_m != 0)
	{

		int y = (int)(m_oMaxima0_LineFkt_m * iLinePos2 + m_oMaxima0_LineFkt_b) * -1;
		// dazughoeriges y berechnen
		//printf("y %d\n",y);

		// ist y ausserhalb Hough-ROI
		if (y < 0 || y > oSizeImgIn.height)
			//if(y < 0 || y >  (int)BorgGreyImg_.roi().dy)
		{
			return false;
		}

		return true;
	}
	// Formel: x = (b2 - b1) / (m1 - m2)

	// Subtraktion der m-Faktoren
	dSub_m = m_oMaxima0_LineFkt_m - m_oMaxima1_LineFkt_m;

	if (dSub_m == 0)
	{
		// keine Schnittpunkt
		return false;
	}

	// Subtraktion der b-Faktoren
	dSub_b = m_oMaxima1_LineFkt_b - m_oMaxima0_LineFkt_b;
	// eigentliche Formel berechnen
	int x = (int)(dSub_b / dSub_m);
	//printf("x %d\n",x);
	// ist x ausserhalb Hough-ROI
	if (x < 0 || x >(int) oSizeImgIn.width)
	{
		return false;
	}

	// dazughoeriges y berechnen
	int y = (int)(m_oMaxima0_LineFkt_m * x + m_oMaxima0_LineFkt_b) * -1;
	//printf("y %d\n",y);
	// ist y ausserhalb Hough-ROI
	if (y < 0 || y > oSizeImgIn.height)
		//if(y < 0 || y >  (int)BorgGreyImg_.roi().dy)
	{
		return false;
	}

	return true;
}


/***********************************************************************
* Description:  Calculation of the max interruption of the hough lines *
*               and count of interruption                              *
* Parameter:                                                           *
************************************************************************/

void PoorPenetrationEdge::calcLineInterupttions(const BImage&	p_rImageIn, triple_int_t ResLine1, triple_int_t ResLine2, geo2d::HoughPPCandidate & houghCandiate)
{
	int xLine1, xLine2;
	int PixGW1, PixGW2;

	unsigned int iInteruptCount1 = 0;
	unsigned int iInteruptCount2 = 0;

	bool iInterupttion1 = false;
	bool iInterupttion2 = false;

	const byte *vSource;

	// ROI-Höhe und breite setzen
	int iDy = p_rImageIn.size().height;
	int iDx = p_rImageIn.size().width;

	houghCandiate.m_oBiggestInterruption1 = 0;
	houghCandiate.m_oBiggestInterruption2 = 0;
	houghCandiate.m_oCountInterruptions1 = 0;
	houghCandiate.m_oCountInterruptions2 = 0;

	// nur berechnen bei plausiblen rho und theta
	//if (rho = 0 && theta <= 360 && theta >= 0)
	//{
	// ganzer ROI in Y durchlaufen
	for (int iRow = 1; iRow < iDy - 1; iRow++)
	{
		// Zeile kopieren
		//vSource = BbinaryImg_[iRow];
		vSource = p_rImageIn[iRow];


		// X-Koordinaten der Linie berechnen
		xLine1 = getHoughLinePosX(iRow, std::get<eRad>(ResLine1), std::get<ePhi>(ResLine1));
		xLine2 = getHoughLinePosX(iRow, std::get<eRad>(ResLine2), std::get<ePhi>(ResLine2));

		if ((xLine1 < 1) || (xLine1 > iDx) || (xLine2 < 1) || (xLine2 > iDx))
		{
			// Koordinaten nicht im ROI
			//printf("Interupption STOP \n");
			return;
		}

		// Grauwert vom Pixel speichern
		//PixGW1 = vSource[xLine1];
		PixGW1 = vSource[xLine1] + vSource[xLine1 - 1] + vSource[xLine1 + 1];
		//PixGW2 = vSource[xLine2];
		PixGW2 = vSource[xLine2] + vSource[xLine2 - 1] + vSource[xLine2 + 1];

		// Linie 1
		// Heller Pixel ?
		if (PixGW1 > 200)
		{
			// Unterbruch zu Ende
			// Abfrage Max.Wert
			if (houghCandiate.m_oBiggestInterruption1 < iInteruptCount1)
			{
				houghCandiate.m_oBiggestInterruption1 = iInteruptCount1;	// bisher laengster Unterbruch
				iInteruptCount1 = 0;				// Zaehler zuruecksetzen
			}

			iInterupttion1 = false;
		}
		else
		{
			// Unterbruch
			iInteruptCount1++;
			if (!iInterupttion1)
			{
				houghCandiate.m_oCountInterruptions1++;
				iInterupttion1 = true;
			}
		}

		// Linie 2
		// Heller Pixel ?
		if (PixGW2 > 200)
		{
			// Unterbruch zu Ende
			// Abfrage Max.Wert
			if (houghCandiate.m_oBiggestInterruption2 < iInteruptCount2)
			{
				houghCandiate.m_oBiggestInterruption2 = iInteruptCount2;	// bisher laengster Unterbruch
				iInteruptCount2 = 0;				// Zaehler zuruecksetzen
			}

			iInterupttion2 = false;
		}
		else
		{
			// Unterbruch
			iInteruptCount2++;
			if (!iInterupttion2)
			{
				houghCandiate.m_oCountInterruptions2++;
				iInterupttion2 = true;
			}
		}
	}

	// letzte Abfrage Max.Wert
	if (houghCandiate.m_oBiggestInterruption1 < iInteruptCount1)
	{
		houghCandiate.m_oBiggestInterruption1 = iInteruptCount1;
	}
	if (houghCandiate.m_oBiggestInterruption2 < iInteruptCount2)
	{
		houghCandiate.m_oBiggestInterruption2 = iInteruptCount2;
	}

	// Sortieren des Ergebnisse
	if (houghCandiate.m_oBiggestInterruption1 > houghCandiate.m_oBiggestInterruption2)
	{
		int temp;

		temp = houghCandiate.m_oBiggestInterruption1;
		houghCandiate.m_oBiggestInterruption1 = houghCandiate.m_oBiggestInterruption2;
		houghCandiate.m_oBiggestInterruption2 = temp;
	}
	if (houghCandiate.m_oCountInterruptions1 > houghCandiate.m_oCountInterruptions2)
	{
		int temp;

		temp = houghCandiate.m_oCountInterruptions1;
		houghCandiate.m_oCountInterruptions1 = houghCandiate.m_oCountInterruptions2;
		houghCandiate.m_oCountInterruptions2 = temp;
	}

	//}
	//return iInteruptMax;
}


/***********************************************************************
* Description:  Calcutlation of mean grey level inside hough lines     *
*                                                                      *
* Parameter:                                                           *
************************************************************************/

int PoorPenetrationEdge::calcGreyScaleOfGap(const BImage&	p_rImageIn)
{
	int iXLeftStart, iXLeftEnd;
	int iXRightStart, iXRightEnd;

	int iGreySum = 0;
	int iGreyMean = 0;
	int count = 0;

	const byte *vSource;

	// ROI-Höhe und breite setzen
	int iDy = p_rImageIn.size().height;

	// Spaltbreite und Posistion berechnen

	// Vergleich Position am ROI-Start
	if (m_oXPosRoiStartLine1 <m_oXPosRoiStartLine2)
	{
		iXLeftStart = m_oXPosRoiStartLine1;
		iXRightStart = m_oXPosRoiStartLine2;
	}
	else
	{
		iXLeftStart = m_oXPosRoiStartLine2;
		iXRightStart = m_oXPosRoiStartLine1;
	}

	// Vergleich Position am ROI-Ende
	if (m_oXPosRoiEndLine1 < m_oXPosRoiEndLine2)
	{
		iXLeftEnd = m_oXPosRoiEndLine1;
		iXRightEnd = m_oXPosRoiEndLine2;
	}
	else
	{
		iXLeftEnd = m_oXPosRoiEndLine2;
		iXRightEnd = m_oXPosRoiEndLine1;
	}

	//Plaus-Check
	if (iXLeftStart <= 0)
		iXLeftStart = 1;

	if (iXRightEnd > iDy)
		iXRightEnd = iDy;

	// Spalt oben zu klein?
	if ((iXRightStart - iXLeftStart) <= 1)
		iXRightStart = iXLeftStart + 2;

	// Spalt unten zu klein?
	if ((iXRightEnd - iXLeftEnd) <= 1)
		iXRightEnd = iXLeftEnd + 2;


	//mittlerer Abstand der Geraden:
	int meanAbstand = (std::abs((iXLeftEnd - iXRightEnd)) + std::abs((iXLeftStart - iXRightStart)) ) / 2;

	// Berechnung der Steigung
	double dSteigungLeft = (double)(iXLeftEnd - iXLeftStart) / iDy;
	double dSteigungRight = (double)(iXRightEnd - iXRightStart) / iDy;


	// ganzer ROI in Y durchlaufen
	int minsum = 0;
	for (int iRow = 1; iRow <= iDy -1 ; iRow++)
	{
		//printf("Row %d\n", iRow);

		// Zeile kopieren
		vSource = p_rImageIn[iRow];

		// Spaltbreite in X durchlaufen
		// Raender weglassen anstelle +1/-1 besser +3/-3
		int colStart = iXLeftStart + int(iRow * dSteigungLeft) + 3;
		int colEnd = iXRightStart + int(iRow * dSteigungRight) - 3;

		int min = 1000;


		if (colEnd >= colStart)
		{

			for (int iCol = colStart; iCol <= colEnd; iCol++)
			{
				//printf("Row %d Col %d \n", iRow, iCol);
				if (vSource[iCol] < min)
				{
					min = vSource[iCol];
				}
				iGreySum += vSource[iCol];
				count++;
				//printf("Row %d Col %d GreySum %d count %d\n", iRow, iCol, iGreySum, count);
				//if (paraHough_.iDisplayOrgImg == 5 )
				//	putPixel (BorgGreyImg_.x().start + iCol, BorgGreyImg_.y().start + iRow-1, INT_DBG_COLOR_YELLOW);
			}
			minsum += min;
			//printf("Row %d ColLeft %d ColRight %d \n", iRow, (int)(iXLeftStart + (iRow * dSteigungLeft) + 1) , (int)(iXRightStart + (iRow * dSteigungRight)-1 ));
		}
		else
		{
			for (int iCol = colEnd; iCol <= colStart; iCol++)//change begin and end
			{
				if (vSource[iCol] < min)
				{
					min = vSource[iCol];
				}

				iGreySum += vSource[iCol];
				count++;

			}
			minsum += min;

		}
	}


	if (iDy>0)
	    minsum /= iDy;

	if (count > 0)
	{
		if (meanAbstand < 5) //Abstand der Geraden ist sehr klein, die Kantenwerte sollen nicht in die Grauwerte eingehen
		{
			iGreyMean = minsum;
		}
		else
		    iGreyMean = iGreySum / count;
	}
	else
		iGreyMean = 0; //Gap should be 0


	return iGreyMean;
}


  } // namespace filter
} // namespace precitec
