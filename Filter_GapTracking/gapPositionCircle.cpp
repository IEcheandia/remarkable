/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief
*/

#include "gapPositionCircle.h"

#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "util/calibDataSingleton.h" // for tcp

#include "math/3D/projectiveMathStructures.h"

#include <fliplib/TypeToDataTypeImpl.h>

#ifndef PI
#define PI 3.141592653589793
#endif

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string GapPositionCircle::m_oFilterName = std::string("GapPositionCircle");
const std::string GapPositionCircle::m_oPipeXName = std::string("pos_x");
const std::string GapPositionCircle::m_oPipeYName = std::string("pos_y");


GapPositionCircle::GapPositionCircle() :
	TransformFilter(GapPositionCircle::m_oFilterName, Poco::UUID{"cd2c846f-064c-47b6-a723-2327cd55fd95"}),
	m_pPipeInImage(nullptr),
	m_pPipeInCenterRadiusX(nullptr),
	m_pPipeInCenterRadiusY (nullptr),
	m_pPipeInRadius (nullptr),
	m_oPipeOutCoordX (this, GapPositionCircle::m_oPipeXName),
	m_oPipeOutCoordY (this, GapPositionCircle::m_oPipeYName),
	m_oSearchDirection(true)
{
	parameters_.add("SearchDirection", Parameter::TYPE_int, static_cast<bool>(m_oSearchDirection));

    setInPipeConnectors({{Poco::UUID("c4e10ca9-f3ee-48bd-a9d1-629cf6649ecd"), m_pPipeInImage, "roi", 1, "roi"},
    {Poco::UUID("2831d1a4-b19e-4122-b64c-5df87606d414"), m_pPipeInCenterRadiusX, "Center_x", 1, "center_x"},
    {Poco::UUID("ec4ee0de-0703-401b-8098-0d09a32247b5"), m_pPipeInCenterRadiusY, "Center_y", 1, "center_y"},
    {Poco::UUID("c93aa817-3fc8-4da1-b231-a76e83d3b385"), m_pPipeInRadius, "Radius", 1, "radius"}});

    setOutPipeConnectors({{Poco::UUID("b83ceda3-9953-4e18-8e32-4714b2b35989"), &m_oPipeOutCoordX, "pos_x", 0, ""},
    {Poco::UUID("9acaaab3-ac73-40af-86ad-57f80a76aae9"), &m_oPipeOutCoordY, "pos_y", 0, ""}});
    setVariantID(Poco::UUID("462B6295-43A5-4D66-A2D5-065DDEB963DB"));
} // TCPDistance()



void GapPositionCircle::setParameter() {
	TransformFilter::setParameter();

	m_oSearchDirection = parameters_.getParameter("SearchDirection");



} // setParameter



void GapPositionCircle::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oXOut) ||  inputIsInvalid(m_oYOut) || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo			&rTrafo			( *m_oSpTrafo );
	OverlayCanvas		&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer		&rLayerLine		( rCanvas.getLayerLine());
	OverlayLayer		&rLayerContour  ( rCanvas.getLayerContour());
	OverlayLayer		&rLayerPosition ( rCanvas.getLayerPosition());

	const Point			oPositionOut(roundToT<int>(m_oPositionOut.x), roundToT<int>(m_oPositionOut.y));
	rLayerPosition.add(new OverlayCross(rTrafo(oPositionOut), Color::Green())); // draw cross at in-position

	if(m_oVerbosity < eMedium){
		return;
	} // if

	const Point			oTcpPosition	( roundToT<int>( m_oTcpPositionSensorCoordinates.x - m_oHwRoi.x), roundToT<int>( m_oTcpPositionSensorCoordinates.y - m_oHwRoi.y) );
	rLayerLine.add(new OverlayLine(rTrafo(oPositionOut), oTcpPosition, Color::Green())); // draw line from in-position to tcp-position

	if (m_oVerbosity >= eHigh)
	{
		double angleRad;
		const interface::ImageFrame & rImage(m_pPipeInImage->read(m_oCounter));
		double step = rImage.data().size().height * 0.35 / m_oRadiusIn;

		for (double angle = 0.0; angle < 360.0; angle += step)
		{
			angleRad = angle * PI / 180.0;
			int x = (int)(m_oCenterRadiusIn.x + m_oRadiusIn * cos(angleRad));
			int y = (int)(m_oCenterRadiusIn.y + m_oRadiusIn * sin(angleRad));
			const Point		oStartPoint(x, y);
			rLayerContour.add(new  OverlayPoint(rTrafo(oStartPoint), Color::Green()));
		}
	} // if

	if ( m_oVerbosity == eMax )
	{
		OverlayLayer	&rLayerText( rCanvas.getLayerText() );
		std::ostringstream	oMsg;
		oMsg << "Dist P-TCP: " << m_oXOut.getData()[0] << " " << m_oYOut.getData()[0];
		int offset = 5;
		int h = 20;
		int w = 400;
		rLayerText.add( new OverlayText( oMsg.str(), Font( 14 ),
			rTrafo(Rect(int(m_oPositionOut.x) + offset, int(m_oPositionOut.y) + offset, w, h)), Color::Green()));

	}



} // paint



bool GapPositionCircle::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "roi"){
		m_pPipeInImage = dynamic_cast<fliplib::SynchronePipe < interface::ImageFrame > *>(&p_rPipe);
	} // if
	else if (p_rPipe.tag() == "center_x") {
		m_pPipeInCenterRadiusX = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if (p_rPipe.tag() == "center_y") {
		m_pPipeInCenterRadiusY = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if (p_rPipe.tag() == "radius") {
		m_pPipeInRadius = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void GapPositionCircle::proceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rEvent)
{
	poco_assert_dbg(m_pPipeInImage != nullptr);
	poco_check_ptr(m_pPipeInCenterRadiusX != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInCenterRadiusY != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInRadius != nullptr); // to be asserted by graph editor

	const GeoDoublearray&		rGeoCenterRadiusXIn	( m_pPipeInCenterRadiusX->read(m_oCounter));
	const GeoDoublearray&		rGeoCenterRadiusYIn	( m_pPipeInCenterRadiusY->read(m_oCounter));
	const GeoDoublearray&		rGeoRadiusIn		( m_pPipeInRadius->read(m_oCounter));
	const Doublearray&			rRadiusXIn			( rGeoCenterRadiusXIn.ref() );
	const Doublearray&			rRadiusYIn			( rGeoCenterRadiusYIn.ref());
	const Doublearray&			rRadiusRIn			( rGeoRadiusIn.ref());
	const ImageContext&			rContextX			( rGeoCenterRadiusXIn.context());
	const ImageContext&			rContextY			( rGeoCenterRadiusYIn.context());

	m_oHwRoi.x	= rContextX.HW_ROI_x0;
	m_oHwRoi.y	= rContextX.HW_ROI_y0;

	//get a copy of the calibration data parameters
    // we want to use ytcp , so let's set the laser to 1 //FIXME
	m_oTcpPositionSensorCoordinates = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0).getTCPCoordinate(m_oCounter % g_oNbPar, LaserLine::FrontLaserLine);
	// get context
	m_oSpTrafo = rGeoCenterRadiusXIn.context().trafo();

	// get Roi width
	const interface::ImageFrame & rImage(m_pPipeInImage->read(m_oCounter));
	int oRoiWidth = rImage.data().size().width;
	//int oRoiWidth = rGeoCenterRadiusXIn.context().trafo()->;

	// calc TCP position in Roi coord
	const Point			oTcpPosition(roundToT<int>( m_oTcpPositionSensorCoordinates.x - m_oHwRoi.x - m_oSpTrafo->dx()), roundToT<int>( m_oTcpPositionSensorCoordinates.y - m_oHwRoi.y - m_oSpTrafo->dy()));

	if (rRadiusXIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rRadiusXIn.size());
	}
	if (rRadiusYIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rRadiusYIn.size());
	}
	if (rRadiusRIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u R values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rRadiusRIn.size());
	}
	if (rGeoCenterRadiusXIn.context() != rGeoCenterRadiusYIn.context()) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoCenterRadiusXIn.context() << "', '" << rGeoCenterRadiusYIn.context() << "'\n";
		wmLog(eWarning, oMsg.str());
	}

	if (inputIsInvalid(rGeoCenterRadiusXIn) || inputIsInvalid(rGeoCenterRadiusYIn) || inputIsInvalid(rGeoRadiusIn)) {
		if(m_oVerbosity >= eMedium) {
			wmLog(eInfo, "Filter '%s': Input radius invalid.\n", m_oFilterName.c_str());
		} // if

		m_oXOut.assign(1, 0, eRankMin);
		m_oYOut.assign(1, 0, eRankMin);

		const GeoDoublearray		oGeoOutX(rContextX, m_oXOut, rGeoCenterRadiusXIn.analysisResult(), NotPresent); // bad geo rank
		const GeoDoublearray		oGeoOutY(rContextY, m_oYOut, rGeoCenterRadiusYIn.analysisResult(), NotPresent); // bad geo rank

		preSignalAction();
		m_oPipeOutCoordX.signal(oGeoOutX);
		m_oPipeOutCoordY.signal(oGeoOutY);

		return; // return
	} // if

	m_oCenterRadiusIn.x = rRadiusXIn.getData().front();
	m_oCenterRadiusIn.y = rRadiusYIn.getData().front();
	m_oRadiusIn = rRadiusRIn.getData().front();

	m_oPositionOut.y = oTcpPosition.y;
	m_oPositionOut.x = calcIntersectionCircleTCPx(m_oCenterRadiusIn, m_oRadiusIn, oTcpPosition.y, m_oSearchDirection, oRoiWidth);

	if (m_oPositionOut.x < 0) {
		if (m_oVerbosity >= eMedium) {
			wmLog(eInfo, "Filter '%s': no position found.\n", m_oFilterName.c_str());
		} // if

		m_oXOut.assign(1, 0, eRankMin);
		m_oYOut.assign(1, 0, eRankMin);

		const GeoDoublearray		oGeoOutX(rContextX, m_oXOut, rGeoCenterRadiusXIn.analysisResult(), NotPresent); // bad geo rank
		const GeoDoublearray		oGeoOutY(rContextY, m_oYOut, rGeoCenterRadiusYIn.analysisResult(), NotPresent); // bad geo rank

		preSignalAction();
		m_oPipeOutCoordX.signal(oGeoOutX);
		m_oPipeOutCoordY.signal(oGeoOutY);

		return; // return
	} // if

	if(m_oVerbosity >= eMedium){
		wmLog(eDebug, "Hardware ROI: (%f, %f).\n", m_oHwRoi.x, m_oHwRoi.y);
		wmLog(eDebug, "Input: Center Radius in ROI coordinates: (%f, %f).\n", m_oCenterRadiusIn.x, m_oCenterRadiusIn.y);
		wmLog(eDebug, "Input: Radius in pixel: (%f).\n", m_oRadiusIn);
		wmLog(eDebug, "Input: TCP in ROI coordinates: (%d, %d).\n", oTcpPosition.x, oTcpPosition.y);
		wmLog(eDebug, "Output: Position in ROI coordinates: (%f, %f).\n", m_oPositionOut.x, m_oPositionOut.y);
	} // if

	m_oXOut.assign(1, m_oPositionOut.x, eRankMax);
	m_oYOut.assign(1, m_oPositionOut.y, eRankMax);

	const GeoDoublearray	oGeoOutX(rContextX, m_oXOut, rGeoCenterRadiusXIn.analysisResult(), rGeoCenterRadiusXIn.rank());
	const GeoDoublearray	oGeoOutY(rContextY, m_oYOut, rGeoCenterRadiusYIn.analysisResult(), rGeoCenterRadiusYIn.rank());

	preSignalAction();
	m_oPipeOutCoordX.signal(oGeoOutX);
	m_oPipeOutCoordY.signal(oGeoOutY);

} // proceedGroup

int GapPositionCircle::calcIntersectionCircleTCPx(TPoint<double> m_oCenterRadius, double m_oRadius, int oTcpPositionY, bool oFromRight, int oRoiWidth){

	int oXPos = -1;

	// calc root
	double oExponent1 = m_oRadius * m_oRadius;
	double oExponent2 = (oTcpPositionY - m_oCenterRadius.y) * (oTcpPositionY - m_oCenterRadius.y);
	double oRadiusRoot = sqrt(oExponent1 - oExponent2);

	if (oRadiusRoot)
	{
		int X1 = (int) ((-1 * oRadiusRoot) + m_oCenterRadius.x);		// left found Position
		int X2 = (int) (oRadiusRoot  + m_oCenterRadius.x);		// right found Position


		if (!oFromRight)
		{
			if ((X1 < 0) && (X2 > 0) && (X2 <= oRoiWidth))
			{
				oXPos = X2;
			}
			else if (X1 > 0 && (X1 <= oRoiWidth))
			{
				oXPos = X1;
			}
		}
		else
		{
			if ((X2 > oRoiWidth) && (X1 > 0) && (X1 <= oRoiWidth))
			{
				oXPos = X1;
			}
			else if (X2 > 0 && (X2 <= oRoiWidth))
			{
				oXPos = X2;
			}
		}
	}

	return oXPos;
}

} // namespace filter
} // namespace precitec



