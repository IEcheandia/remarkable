/**
 *  @copyright:		Precitec Vision GmbH & Co. KG
 *  @author			OS / GUR
 *  @date			01/2015
 *  @file
 *  @brief			Performs a circle fit on a contour array, calculates a mean circle, and gives out
 * 					the center point and radius of the found circle
 */

//  Input:   Array with 'left' or 'right' contour points relative in x
//           Number of slices the image is divided into
//           Image to have the width/height from grey ROI x/y positions
//           Parameter MinRadius:     Min. size in mm for the found radius to be a (correct) circle
//           Parameter MaxRadius:     Max. size in mm for the found radius to be a (correct) circle
//           Parameter IgnoreTop:     Amount of slices to ignore (in percent of ROI height), from top of the given ROI (= in y direction)
//           Parameter IgnoreBottom:  Amount of slices to ignore (in percent of ROI height), from bottom of the given ROI (= in y direction)
//
//  Output:  ValueX/RankX, ValueY/RankY of center point
//           ValueR/RankR of radius
//           Flag whether circle was found or not

// local includes
#include "circleFitContour.h"

#include "image/image.h"				///< BImage
#include "overlay/overlayPrimitive.h"	///< overlay, Color
#include "module/moduleLogger.h"
#include "util/calibDataSingleton.h"
#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string CircleFitContour::m_oFilterName 			( std::string("CircleFitContour") );
const std::string CircleFitContour::m_oPipeOutNameX			( std::string("CenterX") );
const std::string CircleFitContour::m_oPipeOutNameY			( std::string("CenterY") );
const std::string CircleFitContour::m_oPipeOutNameR			( std::string("Radius") );
const std::string CircleFitContour::m_oPipeOutNameIsCircle	( std::string("IsCircle") );


/**
 * CircleFitContour - Constructor
 */
CircleFitContour::CircleFitContour() : TransformFilter( CircleFitContour::m_oFilterName, Poco::UUID{"A4CD307A-740D-4EE7-8C3E-2CF823A181F7"} ),
	m_pPipeInContourPointList	(nullptr),
	m_pPipeInNSlices			(nullptr),
	m_pPipeInImgSize			(nullptr),
	m_oPipeOutCenterPointX		(this, CircleFitContour::m_oPipeOutNameX),
	m_oPipeOutCenterPointY		(this, CircleFitContour::m_oPipeOutNameY),
	m_oPipeOutRadius			(this, CircleFitContour::m_oPipeOutNameR),
	m_oPipeOutIsCircle			(this, CircleFitContour::m_oPipeOutNameIsCircle),
	m_oMinRadiusMM 		(      5.0),
	m_oMinRadiusPix 	(   5000  ),
	m_oMaxRadiusMM 		(   1000.0),
	m_oMaxRadiusPix 	(1000000  ),
	m_oIgnoreTop		(      0  ),
	m_oIgnoreBottom		(      0  ),
	m_oNSlices			(     10  )
{
	parameters_.add("IgnoreTop",			Parameter::TYPE_int,	static_cast <int>		(m_oIgnoreTop));
	parameters_.add("IgnoreBottom",			Parameter::TYPE_int,	static_cast <int>		(m_oIgnoreBottom));
	parameters_.add("MinRadius",			Parameter::TYPE_double,	static_cast <double>	(m_oMinRadiusMM));
	parameters_.add("MaxRadius",			Parameter::TYPE_double,	static_cast <double>	(m_oMaxRadiusMM));

	m_resultX = m_resultY = m_resultR = 0;
	m_isValid = true;

    setInPipeConnectors({{Poco::UUID("A94B8A75-E8AB-48A3-A59A-F95556D5AE8F"), m_pPipeInContourPointList, "ContourPoints", 1, "ContourPoints"},
    {Poco::UUID("5D7C7BDE-AA83-45DA-8924-9572C1EE77BF"), m_pPipeInNSlices, "NSlices", 1, "NSlices"},
    {Poco::UUID("BD2E7F10-3503-42D2-AF99-919D220BDCF3"), m_pPipeInImgSize, "image_size", 1, "image_size"}});
    setOutPipeConnectors({{Poco::UUID("7EF1A4FE-5D91-4C27-BB6B-58CEBC08097B"), &m_oPipeOutCenterPointX, "CenterX", 0, ""},
    {Poco::UUID("29740421-9F8F-48CC-BD7D-23F7C7ADF63F"), &m_oPipeOutCenterPointY, "CenterY", 0, ""},
    {Poco::UUID("81F64209-83F4-420A-8918-8C1673A97DB5"), &m_oPipeOutRadius, "Radius", 0, ""},
    {Poco::UUID("75DDAF8D-4E9D-49B2-811C-1BC249766870"), &m_oPipeOutIsCircle, "IsCircle", 0, ""}});
    setVariantID(Poco::UUID("2912784B-9F34-429A-98D6-F78A7BA0F811"));
}


/**
 * Destructor
 */
CircleFitContour::~CircleFitContour()
{
}


void CircleFitContour::setParameter()
{
	TransformFilter::setParameter();
	m_oMinRadiusMM  = parameters_.getParameter("MinRadius");
	m_oMaxRadiusMM  = parameters_.getParameter("MaxRadius");
	m_oIgnoreTop    = parameters_.getParameter("IgnoreTop");
	m_oIgnoreBottom = parameters_.getParameter("IgnoreBottom");
}


bool CircleFitContour::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "ContourPoints") {
		m_pPipeInContourPointList = dynamic_cast< fliplib::SynchronePipe< interface::GeoDoublearray > * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "NSlices") {
		m_pPipeInNSlices = dynamic_cast< fliplib::SynchronePipe< interface::GeoDoublearray > * >(&p_rPipe);
	}
	else if (p_rPipe.tag() == "image_size") {
		m_pPipeInImgSize = dynamic_cast< fliplib::SynchronePipe< interface::GeoDoublearray > * >(&p_rPipe);
	}

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


void CircleFitContour::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
{
	// Variables for output of the "mean" circle
	geo2d::Doublearray oOutCenterPointX;
	oOutCenterPointX.assign( 1 );
	geo2d::Doublearray oOutCenterPointY;
	oOutCenterPointY.assign( 1 );
	geo2d::Doublearray oOutRadius;
	oOutRadius.assign( 1 );
	geo2d::Doublearray oOutIsCircle;
	oOutIsCircle.assign( 1 );
	double             oRankOutX     = 0.0;
	double             oRankOutY     = 0.0;
	double             oRankOutR     = 0.0;
	double             oRankIsCircle = 1.0;

	// Variablen fuer die Y-Werte zu den Konturpunkten! Die Konturpunkte-Arrays haben nur noch die X-Koordinate der Uebergaenge!
	poco_assert_dbg( m_pPipeInNSlices != nullptr ); // to be asserted by graph editor
	const auto  &rGeoNSlicesIn = m_pPipeInNSlices->read(m_oCounter).ref().getData();
	poco_assert_dbg( !rGeoNSlicesIn.empty() ); // to be asserted by graph editor
	m_oNSlices = (int) (rGeoNSlicesIn[0]);

	poco_assert_dbg( m_pPipeInImgSize != nullptr ); // to be asserted by graph editor
	const auto  &rImageIn = m_pPipeInImgSize->read(m_oCounter);
	auto rContext = rImageIn.context();
	auto rAnalysisResult = rImageIn.analysisResult();

	const SmpTrafo rTrafo = rContext.trafo();
	const Point    rTrafoOffset = rContext.trafo()->apply( Point(0,0) );
	m_oRoiOffsetX = rTrafoOffset.x;
	m_oRoiOffsetY = rTrafoOffset.y;

	m_oRoiHeight = int(rImageIn.ref().getData()[1]);
	m_oRoiWidth  = int(rImageIn.ref().getData()[0]);

    auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
	// Calibration factor in X
	double oFactX = rCalib.factorHorizontal(10,
		rTrafo->dx() + rContext.HW_ROI_x0 + (m_oRoiWidth / 2),    //center (x) of current image in sensor coordinates
		rTrafo->dy() + rContext.HW_ROI_y0 + (m_oRoiHeight / 2));  //center (y) of current image in sensor coordinates

	m_oMinRadiusPix = (int) (m_oMinRadiusMM * oFactX);
	m_oMaxRadiusPix = (int) (m_oMaxRadiusMM * oFactX);

	const int sliceHeight = m_oRoiHeight / m_oNSlices;


	// Analyze contour
	// ---------------

	m_isValid = true;

	poco_assert_dbg( m_pPipeInContourPointList != nullptr ); // to be asserted by graph editor

	if (m_pPipeInContourPointList == nullptr)
		m_isValid = false;

	// Get input data
	interface::GeoDoublearray GeoDPointArrayIn = m_pPipeInContourPointList->read(m_oCounter);
	bool hasInput = GeoDPointArrayIn.ref().size() > 0;

	if (hasInput && m_isValid)
	{
		doCircleFitContour (GeoDPointArrayIn,
							&m_resultX, &m_resultY, &m_resultR,
							&m_isValid,
							sliceHeight,
							m_oIgnoreTop, m_oIgnoreBottom,
							m_oMinRadiusPix, m_oMaxRadiusPix);

		if ( m_isValid && (m_oVerbosity > eNone) )
		{
			paintCircleContour	(	GeoDPointArrayIn,
									&m_resultX,
									&m_resultY,
									&m_resultR
								);
		}
	}
	else
	{
		m_isValid = false;
		m_resultX = 0;
		m_resultY = 0;
		m_resultR = 0;
	}


	// Check plausibility of found circle
	// ----------------------------------

	if ( !m_isValid )
	{
		// Found circle is bad!
		// => mark X and Y as 0.0 and R as 1.0
		double	oValue = 0.0;
		int		oRank  = 0;
		oOutCenterPointX[0] = std::tie( oValue, oRank );
		oOutCenterPointY[0] = std::tie( oValue, oRank );
		oValue = 1.0;
		oOutRadius[0] = std::tie( oValue, oRank );
		oRankOutX = 0.0;
		oRankOutY = 0.0;
		oRankOutR = 0.0;
	}

	else
	{
		// Circle was correct found!
		// Centerpoint out as 'relative'
		double	oValue = (double) m_resultX - m_oRoiOffsetX;
		int		oRank  = 255;
		oOutCenterPointX[0] = std::tie( oValue, oRank );
		oValue = (double) m_resultY - m_oRoiOffsetY;
		oOutCenterPointY[0] = std::tie( oValue, oRank );
		oValue = (double) m_resultR;
		oOutRadius[0] = std::tie( oValue, oRank );
		oRankOutX = 1.0;
		oRankOutY = 1.0;
		oRankOutR = 1.0;

		if (m_oVerbosity > eHigh)
		{
			std::ostringstream	oMsg;

			oMsg	<< "CircleFitContour:  Slice 1"
					<< ":  X = "	<< GeoDPointArrayIn.ref().getData()[0]
					<< '\n';
			wmLog( eDebug, oMsg.str() );

			for (int iSlice = 1; iSlice < m_oNSlices; iSlice++)
			{
				oMsg	<< "                   Slice "	<< (iSlice + 1)
						<< ":  X = "	<< GeoDPointArrayIn.ref().getData()[iSlice]
						<< '\n';
				wmLog( eDebug, oMsg.str() );
			}

			oMsg	<< "CircleFitContour:  Circle  XC = "	<< m_resultX
					<< "  YC = "							<< m_resultY
					<< "  Radius = "						<< m_resultR
					<< "  Pixel"							<<  '\n';
			wmLog( eDebug, oMsg.str() );
		}
	}


	// Pipe out end result
	// -------------------

	const interface::GeoDoublearray oGeoDoubleOutX( rContext, oOutCenterPointX, rAnalysisResult, oRankOutX );
	const interface::GeoDoublearray oGeoDoubleOutY( rContext, oOutCenterPointY, rAnalysisResult, oRankOutY );
	const interface::GeoDoublearray oGeoDoubleOutR( rContext, oOutRadius,       rAnalysisResult, oRankOutR );

	if (oRankOutR == 0.0)
	{
		// Datenfehler oder keinen passenden Kreis gefunden!
		double	oValue = 0.0;
		int		oRank  = 255;
		oOutIsCircle[0] = std::tie( oValue, oRank );
	}
	else
	{
		// All OK, passenden Kreis gefunden!
		double	oValue = 1.0;
		int		oRank  = 255;
		oOutIsCircle[0] = std::tie( oValue, oRank );
	}

	const interface::GeoDoublearray oGeoDoubleOutIsCircle( rContext, oOutIsCircle, rAnalysisResult, oRankIsCircle );

	// send the data out ...
	preSignalAction();
 	m_oPipeOutCenterPointX.signal ( oGeoDoubleOutX );
 	m_oPipeOutCenterPointY.signal ( oGeoDoubleOutY );
 	m_oPipeOutRadius.signal       ( oGeoDoubleOutR );
 	m_oPipeOutIsCircle.signal     ( oGeoDoubleOutIsCircle );
}


void CircleFitContour::doCircleFitContour(
			interface::GeoDoublearray	p_contourPointList,
			int							*circleX,
			int							*circleY,
			int							*circleR,
			bool						*valid,
			int							sliceHeight,
			int							p_oIgnoreTop,
			int							p_oIgnoreBottom,
			int							p_oMinRadius,
			int							p_oMaxRadius)
{
	*circleX = 0;
	*circleY = 0;
	*circleR = 0;
	*valid   = true;

	// Anzahl x-Punkte im Array
	int totalSize = (int) ( p_contourPointList.ref().size() );
	const Doublearray &dataD ( p_contourPointList.ref() );

	if (totalSize < 5)
	{
		*valid = false;
		return;
	}

	// Schauen, welche Streifen oben und unten weggelassen werden sollen
	if ( (p_oIgnoreTop < 0) || (p_oIgnoreTop >= 100) )
		p_oIgnoreTop = 0;
	if ( (p_oIgnoreBottom < 0) || (p_oIgnoreBottom >= 100) )
		p_oIgnoreBottom = 0;

	int iStartSlice = totalSize * p_oIgnoreTop / 100;
	int iEndSlice = totalSize * (100 - p_oIgnoreBottom) / 100;
	if ( (iEndSlice - iStartSlice) < 5 )
	{
		*valid = false;
		return;
	}

    double x_ = 0, y_ = 0; // Variablen zum Speichern der Mittelwerte in x und y
    int    count = 0;      // Zaehlt die Anzahl der in Frage kommenden Punkte

    // Zunaechst die Mittelwerte bestimmen
//	for (int i = 0; i < totalSize; i++)
	for (int i = iStartSlice; i < iEndSlice; i++)
	{
		// Ist x-Wert ueberhaupt OK? => pruefe Rank
		int r = dataD.getRank()[i];
		if (r < 255)
		{
			// Halt! Dieser Punkt ist ungueltig!
			continue;
		}
		// Mit Absolutwerten rechnen
		double x = dataD.getData()[i] + m_oRoiOffsetX;
		double y = (i + 0.5) * sliceHeight + m_oRoiOffsetY;

        count++;
        x_ += x;
        y_ += y;
	}

	if (count < 5)
	{
		// Zu wenig Punkte fuer den Kreisfit
		*valid   = false;
		return;
	}

    x_ = x_ / count;
    y_ = y_ / count;

    // jetzt die notwendigen Summen bilden. Siehe PDF dazu.
    double u, v;
    double Suu = 0.0, Suv = 0.0, Svv = 0.0, Suvv = 0.0, Svuu = 0.0, Suuu = 0.0, Svvv = 0.0;

//	for (int i = 0; i < totalSize; i++)
	for (int i = iStartSlice; i < iEndSlice; i++)
	{
		// Ist x-Wert ueberhaupt OK? => pruefe Rank
		int r = dataD.getRank()[i];
		if (r < 255)
		{
			// Halt! Dieser Punkt ist ungueltig!
			continue;
		}
		// Mit Absolutwerten rechnen
		double x = dataD.getData()[i] + m_oRoiOffsetX;
		double y = (i + 0.5) * sliceHeight + m_oRoiOffsetY;

        u = x - x_;
        v = y - y_;

        Suu  += u * u;
        Suv  += u * v;
        Svv  += v * v;
        Suvv += u * v * v;
        Svuu += v * u * u;
        Suuu += u * u * u;
        Svvv += v * v * v;
	}

	if (    ( fabs(Suuu) < 100.0 )
		 && ( fabs(Svvv) < 100.0 )
	   )
	{
		// Ist eine Gerade!
		*circleX = 0;
		*circleY = 0;
		*circleR = -2;   // Als "Markierung" fuer schraege Gerade
		*valid   = false;
		return;
	}

    // Matrix: erste Zeile a1 a2, zweite Zeile a3 a4
    double a1 = Suu;
    double a2 = Suv;
    double a3 = Suv;
    double a4 = Svv;

    double b1 = 0.5 * (Suuu + Suvv);
    double b2 = 0.5 * (Svvv + Svuu);

	if (    (a1 == 0)
		 || ((a2 * a3) == (a4 * a1))
	   )
	{
		*circleX = 0;
		*circleY = 0;
		*circleR = -5;   // Als "Markierung" fuer "Null"-Koeffizienten
		*valid   = false;
		return;
	}

	double vc = (b1 * a3 - b2 * a1) / (a2 * a3 - a4 * a1);
    double uc = (b1 - a2 * vc) / a1;

    double r = sqrt( uc * uc + vc * vc + (Suu + Svv) / count );

	if (r < p_oMinRadius)
	{
		// Ist ungefaehr eine Gerade!
		*circleX = 0;
		*circleY = 0;
		*circleR = -3;   // Als "Markierung" fuer Kreis ueber schraege Gerade
		*valid   = false;
		return;
	}
	else if (r > p_oMaxRadius)
	{
		// Ist ungefaehr eine Gerade!
		*circleX = 0;
		*circleY = 0;
		*circleR = -4;   // Als "Markierung" fuer Gerade
		*valid   = false;
		return;
	}

	*circleX = (int) (uc + x_ + 0.5);
	*circleY = (int) (vc + y_ + 0.5);
	*circleR = (int) (r + 0.5);

	return;
}


void CircleFitContour::paintCircleContour(
			const interface::GeoDoublearray	p_contourPointList,
			int								*circleX,
			int								*circleY,
			int								*circleR)
{
	// Anzeige der Punkte
	OverlayCanvas					&rCanvas( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer					&rLayerContour( rCanvas.getLayerContour() );
	OverlayLayer					&rLayerPosition( rCanvas.getLayerPosition() );

	int								totalSize = (int) ( p_contourPointList.ref().size() );
	const Doublearray				&dataD ( p_contourPointList.ref() );
	const int						sliceHeight = m_oRoiHeight / m_oNSlices;

	if (m_oVerbosity >= eLow)
	{
		if (m_oVerbosity >= eHigh)
		{
			std::cout	<< "CircleFitContour: circle xc "	<< *circleX
						<< " yc "		<< *circleY
						<< " Radius "	<< *circleR
						<< std::endl;
		}

		// Sind als Absolutwerte da
		const Point oCenterPoint (*circleX, *circleY);
		rLayerPosition.add( new OverlayCross( oCenterPoint, Color::Green() ) );

		// Anzeige der Konturpunkte
//		for (int i = 0; i < totalSize; i++)
		int iStartSlice = totalSize * m_oIgnoreTop / 100;
		int iEndSlice = totalSize * (100 - m_oIgnoreBottom) / 100;

		if ( (m_oVerbosity >= eMedium) && (iStartSlice > 0) )
		{
			// Nicht benutzte Kontur-Punkte  =>  rot markieren
			for (int i = 0; i < iStartSlice; i++)
			{
				double x = dataD.getData()[i] + m_oRoiOffsetX;
				double y = (i + 0.5) * sliceHeight + m_oRoiOffsetY;
				const Point	oContourPoint( (int) x, (int) y );
				rLayerPosition.add( new OverlayCross( oContourPoint, Color::Red() ) );
			}
		}

		for (int i = iStartSlice; i < iEndSlice; i++)
		{
			double x = dataD.getData()[i] + m_oRoiOffsetX;
			double y = (i + 0.5) * sliceHeight + m_oRoiOffsetY;
			const Point	oContourPoint( (int) x, (int) y );
			int r = dataD.getRank()[i];
			if (r < 255)
			{
				// Dieser Punkt ist ungueltig!
				if (m_oVerbosity >= eMedium)
					rLayerPosition.add( new OverlayCross( oContourPoint, Color::Red() ) );
			}
			else
			{
				// Dieser Punkt ist OK
				rLayerPosition.add( new OverlayCross( oContourPoint, Color::Green() ) );
			}
		}

		if ( (m_oVerbosity >= eMedium) && (iEndSlice < totalSize) )
		{
			// Nicht benutzte Kontur-Punkte  =>  rot markieren
			for (int i = iEndSlice; i < totalSize; i++)
			{
				double x = dataD.getData()[i] + m_oRoiOffsetX;
				double y = (i + 0.5) * sliceHeight + m_oRoiOffsetY;
				const Point	oContourPoint( (int) x, (int) y );
				rLayerPosition.add( new OverlayCross( oContourPoint, Color::Red() ) );
			}
		}
	}

	if (m_oVerbosity >= eMedium)
	{
		for (int yi = m_oRoiOffsetY; yi < (m_oRoiHeight + m_oRoiOffsetY); yi += 3)
		{
			int yTest = yi - *circleY;

			if ( abs(yTest) >= *circleR )
				continue;

			int xi = sqrt (*circleR * *circleR - yTest * yTest);
			int xTest;

			if (*circleX < 0)
			{
				// Kreisbogen rechts vom Kreis-Mittelpunkt
				xTest = *circleX + xi;
			}
			else
			{
				// Kreisbogen links vom Kreis-Mittelpunkt
				xTest = *circleX - xi;
			}

			if (    (xTest > m_oRoiOffsetX)
					&& ( xTest < (m_oRoiOffsetX + m_oRoiWidth) )
				)
			{
				// yi und xTest sind Absolut-Werte
				const Point	oCirclePoint(xTest, yi);
				rLayerContour.add( new OverlayPoint( oCirclePoint, Color::Blue() ) );
			}
		}
	}
}  // paintCircleContour


} // namespace filter
} // namespace precitec
