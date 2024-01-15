/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		JS
* 	@date		2013
* 	@brief
*/

#include "RegelPosition.h"

#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

// Diese Inhalte stehen so in der DB
const std::string RegelPosition::m_oFilterName 		= std::string( "RegelPosition" ); //DB Namen
const std::string RegelPosition::m_oPipeXOutName	= std::string( "X1koordinate" );
const std::string RegelPosition::m_oPipeYOutName	= std::string( "Y1koordinate" );


RegelPosition::RegelPosition() :
	TransformFilter		( RegelPosition::m_oFilterName, Poco::UUID{"C16B6F2D-8FCA-4BD7-9CA7-03B1081252FB"} ),
	m_pPipeInPosX1		( nullptr ),
	m_pPipeInPosY1		( nullptr ),
	m_pPipeInPosX2		( nullptr ),
	m_pPipeInPosY2		( nullptr ),
	m_oPipeOutCoordX	( this, m_oPipeXOutName ),
	m_oPipeOutCoordY	( this, m_oPipeYOutName ),
	m_oImgPosX1         (0),
	m_oImgPosX2         (0),
	m_oImgPosY1         (0),
	m_oImgPosY2         (0)

{
	// Defaultwerte der Parameter setzen
	parameters_.add("PositionsGewichung", Parameter::TYPE_Int32, m_oPositionsGewichtung);  // Diese Parameternamen stehen so in der DB

    setInPipeConnectors({{Poco::UUID("0E56D9AC-9FA1-43C4-94C6-DBB41D5FDF60"), m_pPipeInPosX1, "Position1X", 1, "position1_x"},
    {Poco::UUID("37F15BF6-B476-4B5B-8723-8F695B91365C"), m_pPipeInPosY1, "Position1Y", 1, "position1_y"},
    {Poco::UUID("F8D573B7-6625-4025-B2A6-2C838A9F3A6E"), m_pPipeInPosX2, "Position2X", 1, "position2_x"},
    {Poco::UUID("4F32B02B-90D4-4489-A9A5-A10E85FEB63E"), m_pPipeInPosY2, "Position2Y", 1, "position2_y"}});
    setOutPipeConnectors({{Poco::UUID("D4461715-1523-4AC2-88FD-70A8F6A6CD40"), &m_oPipeOutCoordX, "X1koordinate", 0, ""},
    {Poco::UUID("305BDFFA-69AC-4A60-B92C-78FEA422BB47"), &m_oPipeOutCoordY, "Y1koordinate", 0, ""}});
    setVariantID(Poco::UUID("135A8CAC-29FC-4B1E-9353-2BDC2150180D"));
} // RegelPosition()



void RegelPosition::setParameter() {
	TransformFilter::setParameter();

	// Diese Parameternamen stehen so in der DB
	m_oPositionsGewichtung = parameters_.getParameter("PositionsGewichung").convert<int>();

} // setParameter



void RegelPosition::paint() {
	if(m_oVerbosity < eLow || inputIsInvalid(m_oXOut) ||  inputIsInvalid(m_oYOut) || m_oSpTrafo1.isNull() || m_oSpTrafo2.isNull()){
		return;
	} // if

	OverlayCanvas		&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer		&rLayerPosition	( rCanvas.getLayerPosition());


	if(m_oVerbosity < eLow){
		return;
	} // if

	const Point			DrawPositionIn		( roundToT<int>(m_oSensorPos.x - m_oHwRoi.x), roundToT<int>(m_oSensorPos.y - m_oHwRoi.y ) );
	//std::cout<<"paint: "<<DrawPositionIn<<std::endl;
	rLayerPosition.add( new OverlayCross(DrawPositionIn, Color::Red()) ); // draw cross at in-position

} // paint



bool RegelPosition::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "position1_x") {	// Diese Tagnamen muessen so in der DB eingetragen werden
		m_pPipeInPosX1  = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // if
	else if (p_rPipe.tag() == "position1_y") {	// Diese Tagnamen stehen so in der DB
		m_pPipeInPosY1  = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if (p_rPipe.tag() == "position2_x") {	// Diese Tagnamen stehen so in der DB
		m_pPipeInPosX2  = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if
	else if (p_rPipe.tag() == "position2_y") {	// Diese Tagnamen stehen so in der DB
		m_pPipeInPosY2  = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if



	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void RegelPosition::proceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rEvent) {
	poco_check_ptr(m_pPipeInPosX1 != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInPosY1 != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInPosX2 != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInPosY2 != nullptr); // to be asserted by graph editor


	// In einen GeoDoubleArray lesen
	const GeoDoublearray&		rGeoPosX1In			( m_pPipeInPosX1->read(m_oCounter) );
	const GeoDoublearray&		rGeoPosY1In			( m_pPipeInPosY1->read(m_oCounter) );
	const GeoDoublearray&		rGeoPosX2In			( m_pPipeInPosX2->read(m_oCounter) );
	const GeoDoublearray&		rGeoPosY2In			( m_pPipeInPosY2->read(m_oCounter) );

	// Aus dem GeoDouble in einen Double Array lesen
	const Doublearray&			rPosX1In			( rGeoPosX1In.ref() );
	const Doublearray&			rPosY1In			( rGeoPosY1In.ref() );
	const Doublearray&			rPosX2In			( rGeoPosX2In.ref() );
	const Doublearray&			rPosY2In			( rGeoPosY2In.ref() );

    // Kontext auslesen
	const ImageContext&			rContextX			( rGeoPosX1In.context() );
	const ImageContext&			rContextY			( rGeoPosY1In.context() );

	// HW ROI Werte holen
	m_oHwRoi.x	= rContextX.HW_ROI_x0;
	m_oHwRoi.y	= rContextX.HW_ROI_y0;



	// Sw ROI bzw. Translation rausholen
	m_oSpTrafo1	= rGeoPosX1In.context().trafo();	// speichere trafo (SW-ROI) um spaeter korrekt zeichnen zu koennen
	m_oSpTrafo2	= rGeoPosX2In.context().trafo();	// speichere trafo (SW-ROI) um spaeter korrekt zeichnen zu koennen

	//Eingangswerte pruefen
	if (rPosX1In.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosX1In.size());
	}
	if (rPosY1In.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosY1In.size());
	}

	if (rPosX2In.size() != 1) { // result is always one point
			wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosX2In.size());
	}
	if (rPosY2In.size() != 1) { // result is always one point
			wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosY2In.size());
	}

	// Haben die Werte einen unterschiedlichen Kontext ...
	if (rGeoPosX1In.context() != rGeoPosY1In.context()) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosX1In.context() << "', '" << rGeoPosY1In.context() << "'\n";
		wmLog(eWarning, oMsg.str());
	}

	// Haben die Werte einen unterschiedlichen Kontext ...
	if (rGeoPosX2In.context() != rGeoPosY2In.context()) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosX2In.context() << "', '" << rGeoPosY2In.context() << "'\n";
		wmLog(eWarning, oMsg.str());
	}




	//Eingangswerte aus den Arrays holen
		m_oPositionIn1.x	= rPosX1In.getData().front();
		m_oPositionIn1.y	= rPosY1In.getData().front();
		m_oPositionIn2.x	= rPosX2In.getData().front();
		m_oPositionIn2.y	= rPosY2In.getData().front();

	/*if(inputIsInvalid(rGeoPosX1In) ||  inputIsInvalid(rGeoPosY1In)) {
		if(m_oVerbosity >= eMedium) {
			wmLog(eInfo, "RegelPosition: Input position invalid.\n");
		} // if*/

	if( ( m_oPositionIn1.x < 0.0)  || (m_oPositionIn2.x < 0.0) || (m_oPositionIn1.y <0.0)  ||(m_oPositionIn2.y <0.0 )  ) {
			if(m_oVerbosity >= eMedium) {
				wmLog(eInfo, "RegelPosition: Input position kleiner 0 \n");
			} //

		// Ausgangswerte auf Null setzen und schlechten rank mitgeben
		m_oXOut.assign(1, 0, eRankMin);
		m_oYOut.assign(1, 0, eRankMin);

		// schlechte Ausgabewerte bauen
		const GeoDoublearray		oGeoOutX		( rContextX, m_oXOut, rGeoPosX1In.analysisResult(), NotPresent ); // bad geo rank
		const GeoDoublearray		oGeoOutY		( rContextY, m_oYOut, rGeoPosY1In.analysisResult(), NotPresent ); // bad geo rank

		// senden
		preSignalAction();
		m_oPipeOutCoordX.signal(oGeoOutX);
		m_oPipeOutCoordY.signal(oGeoOutY);

		return; // return
	} // if



	// Die Werte koennen aus verschiedenen ROIs kommen
	// also auf Positionen bzgl. des HW ROis mal umrechnen
	// Absoluter Wert auf dem Sensor in Pixel
	// oGlobalPos Koordinate bzgl. HW Roi
	const TPoint<double>	oGlobalPos1			( m_oSpTrafo1->dx(), m_oSpTrafo1->dy() );			//	Offset ROI Koordinaten -> Bildkoordinaten
	const TPoint<double>	oGlobalPos2			( m_oSpTrafo2->dx(), m_oSpTrafo2->dy() );

	// SW Roi beruecksichtigen
	double m_oImgPosX1 = m_oPositionIn1.x + m_oSpTrafo1->dx();
	double m_oImgPosX2 = m_oPositionIn2.x + m_oSpTrafo2->dx();

	double m_oImgPosY1 = m_oPositionIn1.y + m_oSpTrafo1->dy();
	double m_oImgPosY2 = m_oPositionIn2.y + m_oSpTrafo2->dy();


 	double diffX =	m_oImgPosX2 - m_oImgPosX1;
	double diffY =	m_oImgPosY2 - m_oImgPosY1;

	// Koordinaten auf dem Sensor:
	//Ergebnis Position, Hw Roi beruecksichtigen -
	m_oSensorPos.x = m_oImgPosX1 + ( diffX/100.0 * (double)m_oPositionsGewichtung ) +  m_oHwRoi.x;
    m_oSensorPos.y = m_oImgPosY1 + ( diffY/100.0 * (double)m_oPositionsGewichtung ) +  m_oHwRoi.y;

    //Koordinaten auf dem HW Roi, nachfolgenden Filter verwenden diese Position ohne HW Roi
    m_oHWRoiPos.x = m_oImgPosX1 + ( diffX/100.0 * (double)m_oPositionsGewichtung ); // +  m_oHwRoi.x;
    m_oHWRoiPos.y = m_oImgPosY1 + ( diffY/100.0 * (double)m_oPositionsGewichtung ); // +  m_oHwRoi.y;



	if(m_oVerbosity >= eMedium){
		wmLog(eDebug, "Hardware ROI: (%f, %f).\n", m_oHwRoi.x, m_oHwRoi.y);
		wmLog(eDebug, "Input: Input coordinates in HW ROI: (%f, %f, %f, %f).\n", m_oImgPosX1, m_oImgPosY1,m_oImgPosX2, m_oImgPosY2);
		wmLog(eDebug, "Output: weighted difference in HW ROI: (%f, %f).\n", m_oHWRoiPos.x,m_oHWRoiPos.y);
	} // if


	// Ergebnis auf HW Roi
	m_oXOut.assign(1, m_oHWRoiPos.x, eRankMax);
	m_oYOut.assign(1, m_oHWRoiPos.y, eRankMax);



	// Trafo von globalImage zu subImage
	const SmpTrafo oSpZeroTrafo	= new LinearTrafo(0,0);

	/*
	ImageContext(ImageContext const& rhs, SmpTrafo trafo)
			: measureTaskPos_(rhs.measureTaskPos_), trafo_(trafo->clone()), imageNumber_(rhs.imageNumber()), position_(rhs.position_),
				relTime_(rhs.relativeTime()), id_(rhs.id_),  HW_ROI_x0(rhs.HW_ROI_x0), HW_ROI_y0(rhs.HW_ROI_y0),
				taskContext_(rhs.taskContext_) {}
	*/

	ImageContext oNewContext(rContextX, oSpZeroTrafo );
	//std::cout<<oNewContext<<std::endl;

	// HW ROI testen
	//int dummyROIX	= oNewContext.HW_ROI_x0;
	//int dummyROIY	= oNewContext.HW_ROI_y0;

	// SW ROI testen
	//int dummySWX = oNewContext.trafo()->dx();


//	const GeoDoublearray	oGeoOutX	( rContextX, m_oXOut, rGeoPosX1In.analysisResult(), rGeoPosX1In.rank() );
//	const GeoDoublearray	oGeoOutY	( rContextY, m_oYOut, rGeoPosY1In.analysisResult(), rGeoPosY1In.rank() );

	const GeoDoublearray	oGeoOutX	( oNewContext, m_oXOut, rGeoPosX1In.analysisResult(), rGeoPosX1In.rank() );
	const GeoDoublearray	oGeoOutY	( oNewContext, m_oYOut, rGeoPosY1In.analysisResult(), rGeoPosY1In.rank() );

	preSignalAction();
	m_oPipeOutCoordX.signal(oGeoOutX);
	m_oPipeOutCoordY.signal(oGeoOutY);

} // proceedGroup


} // namespace filter
} // namespace precitec



