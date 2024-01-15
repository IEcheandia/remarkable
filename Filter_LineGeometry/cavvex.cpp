/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		OS
 * 	@date		2015
 * 	@brief 		This filter calculates the values for convexity, concavity and height difference
 */

// Diese Header sind auch in hough.cpp verwendet worden. Sehr interessant:
// Diese Zeilen muessen hier als erstes kommen! Das liegt daran, dass <cmath> wohl bereits an anderer Stelle
// inkludiert wird, aber ohne _USE_MATH_DEFINES. Aber weil <cmath> natuerlich Inkludeguards hat,
// hat eine zweite Einbindung weiter unten keinen Effekt - also muss das hier als erstes erfolgen!
#define _USE_MATH_DEFINES						/// pi constant
#include <cmath>								/// trigonometry, pi constant

// framework includes
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <common/geoContext.h>
#include <filter/algoArray.h>
#include "module/moduleLogger.h"
// local includes
#include "cavvex.h"
#include "line2D.h"
#include "util/calibDataSingleton.h"
#include <filter/armStates.h>

#include "math/3D/projectiveMathStructures.h"

#include <fliplib/TypeToDataTypeImpl.h>

#define PIXEL_IN_X_FOR_2_MM 180

using namespace precitec::math;

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string Cavvex::m_oFilterName 			= std::string("Cavvex");
const std::string Cavvex::PIPENAME_CONVEX_OUT		= std::string("ConvexityOut");		///< Name Out-Pipe
const std::string Cavvex::PIPENAME_CONCAVE_OUT		= std::string("ConcavityOut");		///< Name Out-Pipe
const std::string Cavvex::PIPENAME_HEIGHTDIFF_OUT	= std::string("HeightDifferenceOut");		///< Name Out-Pipe

const std::string Cavvex::m_oParamWhichFitName = std::string("FitWhich");
const std::string Cavvex::m_oParamTypeOfLaserLine = std::string("TypeOfLaserLine"); ///< Parameter: Type of LaserLine (e.g. FrontLaserLine, BehindLaserLine)


Cavvex::Cavvex() :
	TransformFilter( Cavvex::m_oFilterName, Poco::UUID{"2964BA3D-4D30-4EB9-848C-56FF31228837"} ),
	m_pPipeInLaserLine( NULL ),
	m_pPipeInSeamLeft (NULL),          //Uncommented
	m_pPipeInSeamRight (NULL),         //Uncommented
	m_pPipeInAngle (NULL),             //Uncommented
	m_oConvexityOut( 1 ),
	m_oConcavityOut( 1 ),
	m_oHeightDiffOut( 1 ),
	m_oMode(0),
	m_oCalcType(eNormal),
	m_oLeftLineRoiStart (10),
	m_oLeftLineRoiEnd (20),
	m_oRightLineRoiStart (80),
	m_oRightLineRoiEnd (90),
	m_oInvertHeightDiff(false),
	m_oWhichFit(eBoth),
	m_oTypeOfLaserLine(LaserLine::BehindLaserLine)
{
	m_pPipeOutConvexity = new SynchronePipe< GeoDoublearray >( this, Cavvex::PIPENAME_CONVEX_OUT );
	m_pPipeOutConcavity = new SynchronePipe< GeoDoublearray >( this, Cavvex::PIPENAME_CONCAVE_OUT );
	m_pPipeOutHeightDifference = new SynchronePipe< GeoDoublearray >( this, Cavvex::PIPENAME_HEIGHTDIFF_OUT );

	parameters_.add("Mode",    Parameter::TYPE_int, m_oMode);
	parameters_.add("CalcType", Parameter::TYPE_int, static_cast<int>(m_oCalcType));
	parameters_.add("LeftLineROIStart",    Parameter::TYPE_int, m_oLeftLineRoiStart);
	parameters_.add("LeftLineROIEnd", Parameter::TYPE_int, m_oLeftLineRoiEnd);
	parameters_.add("RightLineROIStart",    Parameter::TYPE_int, m_oRightLineRoiStart);
	parameters_.add("RightLineROIEnd", Parameter::TYPE_int, m_oRightLineRoiEnd);

	parameters_.add(Cavvex::m_oParamWhichFitName, Parameter::TYPE_int, static_cast<int>(m_oWhichFit));
	parameters_.add("InvertHeightDiff", Parameter::TYPE_bool, m_oInvertHeightDiff);

	int oLaserLineTemp = static_cast<int>(m_oTypeOfLaserLine);
	parameters_.add(Cavvex::m_oParamTypeOfLaserLine, fliplib::Parameter::TYPE_int, oLaserLineTemp);  // Fuege den Parameter mit dem soeben initialisierten Wert hinzu.

    setInPipeConnectors({{Poco::UUID("04236BBE-E5EA-49D7-9088-3A4E0A0F8D5A"), m_pPipeInLaserLine, "LaserLine", 1, "LaserLine"},
    {Poco::UUID("86BD7A56-739F-4943-8643-2AFC4683EEA6"), m_pPipeInSeamLeft, "SeamLeft", 1, "SeamLeft"},
    {Poco::UUID("A3C99679-D8C0-488D-B724-371CC8DD6C9F"), m_pPipeInSeamRight, "SeamRight", 1, "SeamRight"},
    {Poco::UUID("2EE682D4-4455-4D73-B39D-386F365FEC3B"), m_pPipeInAngle, "Angle", 1, "Angle"}});   //three uncommented connectors added!
    setOutPipeConnectors({{Poco::UUID("34E33478-81ED-4C43-AAC7-CBB0F0F62025"), m_pPipeOutConvexity, PIPENAME_CONVEX_OUT, 0, ""},
    {Poco::UUID("8722D752-C2C0-4D90-B26F-06F8928AD7F8"), m_pPipeOutConcavity, PIPENAME_CONCAVE_OUT, 0, ""},
    {Poco::UUID("9976A6F6-1B29-4C6D-BA6F-8BAE8D6F176E"), m_pPipeOutHeightDifference, PIPENAME_HEIGHTDIFF_OUT, 0, ""}});
    setVariantID(Poco::UUID("41CEC7B2-062A-4B3A-8421-E1A9E4F0339B"));
}

Cavvex::~Cavvex()
{
	delete m_pPipeOutConvexity;
	delete m_pPipeOutConcavity;
	delete m_pPipeOutHeightDifference;
} //

void Cavvex::setParameter()
{
	TransformFilter::setParameter();
	m_oMode = parameters_.getParameter("Mode").convert<int>();
	m_oCalcType = static_cast<CalcType>(parameters_.getParameter("CalcType").convert<int>());
	m_oLeftLineRoiStart    = parameters_.getParameter("LeftLineROIStart").convert<int>();
	m_oLeftLineRoiEnd = parameters_.getParameter("LeftLineROIEnd").convert<int>();
	m_oRightLineRoiStart    = parameters_.getParameter("RightLineROIStart").convert<int>();
	m_oRightLineRoiEnd = parameters_.getParameter("RightLineROIEnd").convert<int>();

	int oTempFit = parameters_.getParameter(Cavvex::m_oParamWhichFitName).convert<int>();
	m_oWhichFit = static_cast<eWhichFit>(oTempFit);

	int oTempLine = parameters_.getParameter(Cavvex::m_oParamTypeOfLaserLine).convert<int>();
	m_oTypeOfLaserLine = static_cast<LaserLine>(oTempLine);

	m_oInvertHeightDiff = static_cast<bool>(parameters_.getParameter("InvertHeightDiff").convert<bool>());

} // setParameter

bool Cavvex::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	if ( p_rPipe.tag() == "SeamLeft" )
		m_pPipeInSeamLeft = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "SeamRight" )
		m_pPipeInSeamRight = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "Angle" )
		m_pPipeInAngle = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void Cavvex::paint()
{
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull())
	{
		return;
	} // if

	try
	{
		int yval;

		const Trafo		&rTrafo(*m_oSpTrafo);
		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

		bool oPaintLeftInAnyCase = m_oWhichFit == eBoth || m_oWhichFit == eLeft;
		int oIntervalLength = 8;
		if (oPaintLeftInAnyCase)
		{
			rLayerContour.add<OverlayLine>(rTrafo(Point( 0, (int)(0.5+m_paintYInterceptLeft))), rTrafo(Point(m_paintNumber, (int)(0.5+ m_paintNumber* m_paintSlopeLeft+ m_paintYInterceptLeft))), Color::Red());
		}
		else
		{
			for (int i = 0; i < m_paintNumber; ++i)
			{ // Gerade links zeichnen
				int oInterval = i / oIntervalLength;     // wir interessieren uns nur fuer den ganzzahligen Anteil
				bool oIntervalOdd = oInterval % 2 == 1;

				if (oIntervalOdd)
				{
					yval = (int)(0.5 + i * m_paintSlopeLeft + m_paintYInterceptLeft);
					rLayerContour.add<OverlayPoint>(rTrafo(Point(i, yval)), Color::Red());
				}
			} // for
		}

		// Startpunkt linke Gerade
		yval = (int)(0.5 + m_paintStartXLeft * m_paintSlopeLeft + m_paintYInterceptLeft);
		rLayerContour.add<OverlayCross>(rTrafo(Point(m_paintStartXLeft, yval)), Color::Green());

		// Endpunkt linke Gerade
		yval = (int)(0.5 + m_paintEndXLeft * m_paintSlopeLeft + m_paintYInterceptLeft);
		//rLayerContour.add(new OverlayCross(rTrafo(Point(m_paintEndXLeft, yval)), Color::Green()));

		// Naht links - das Kreuz soll in jedem Fall ausgegeben werden, weil wir schliesslich das Y-Datum aus der Laserlinie in jedem Fall beziehen.
		int yvalSeamLeft = (int)(0.5 + m_firstSeamLeft * m_paintSlopeLeft + m_paintYInterceptLeft);

        rLayerContour.add<OverlayLine> (rTrafo(Point(m_firstSeamLeft - 1, yvalSeamLeft - 1)), rTrafo(Point(m_firstSeamLeft - 5, yvalSeamLeft - 5)), Color::Red());
        rLayerContour.add<OverlayLine> (rTrafo(Point(m_firstSeamLeft - 1, yvalSeamLeft + 1)), rTrafo(Point(m_firstSeamLeft - 5, yvalSeamLeft + 5)), Color::Red());



		bool oPaintRightInAnyCase = m_oWhichFit == eBoth || m_oWhichFit == eRight;
		if (oPaintRightInAnyCase)
		{
			rLayerContour.add<OverlayLine>(rTrafo(Point( 0, (int)(0.5+m_paintYInterceptRight))), rTrafo(Point(m_paintNumber, (int)(0.5+ m_paintNumber* m_paintSlopeRight+ m_paintYInterceptRight))), Color::Red());
		}
		else
		{
			for (int i = 0; i < m_paintNumber; ++i)
			{ // Gerade rechts zeichnen
				int oInterval = i / oIntervalLength;     // wir interessieren uns nur fuer den ganzzahligen Anteil
				bool oIntervalOdd = oInterval % 2 == 1;

				if (oIntervalOdd)
				{
					yval = (int)(0.5 + i * m_paintSlopeRight + m_paintYInterceptRight);
					rLayerContour.add<OverlayPoint>(rTrafo(Point(i, yval)), Color::Red());
				}
			} // for
		}

		// Startpunkt rechte Gerade
		yval = (int)(0.5 + m_paintStartXRight * m_paintSlopeRight + m_paintYInterceptRight);
		//rLayerContour.add(new OverlayCross(rTrafo(Point(m_paintStartXRight, yval)), Color::Green()));

		// Endpunkt rechte Gerade
		yval = (int)(0.5 + m_paintEndXRight * m_paintSlopeRight + m_paintYInterceptRight);
		rLayerContour.add<OverlayCross>(rTrafo(Point(m_paintEndXRight, yval)), Color::Green());

		// Naht rechts
		int yvalSeamRight = (int)(0.5 + m_firstSeamRight * m_paintSlopeRight + m_paintYInterceptRight);


        rLayerContour.add<OverlayLine> (rTrafo(Point(m_firstSeamRight + 1, yvalSeamRight - 1)), rTrafo(Point(m_firstSeamRight + 5, yvalSeamRight - 5)), Color::Red());
        rLayerContour.add<OverlayLine> (rTrafo(Point(m_firstSeamRight + 1, yvalSeamRight + 1)), rTrafo(Point(m_firstSeamRight + 5, yvalSeamRight + 5)), Color::Red());


		// Verbindungslinie ideal Naht
		//rLayerContour.add(new OverlayLine(rTrafo(Point(m_firstSeamLeft, yvalSeamLeft)), rTrafo(Point(m_firstSeamRight, yvalSeamRight)), Color::Orange()));

		if (m_paintHeightDiffX1 * m_paintHeightDiffY1 * m_paintHeightDiffX2 * m_paintHeightDiffY2 != 0)
		{  // Linie fuer gemessene Hoehendiff einzeichnen
			if (m_paintHeightDiffY1 > m_paintHeightDiffY2)
			{
				int temp = m_paintHeightDiffY1;
				m_paintHeightDiffY1 = m_paintHeightDiffY2;
				m_paintHeightDiffY2 = temp;

				temp = m_paintHeightDiffX1;
				m_paintHeightDiffX1 = m_paintHeightDiffX2;
				m_paintHeightDiffX2 = temp;
			}

			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintHeightDiffX1, m_paintHeightDiffY1)), rTrafo(Point(m_paintHeightDiffX2, m_paintHeightDiffY2)), Color::Blue());

			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintHeightDiffX1 - 5, m_paintHeightDiffY1 - 5)), rTrafo(Point(m_paintHeightDiffX1, m_paintHeightDiffY1)), Color::Blue());
			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintHeightDiffX1 + 5, m_paintHeightDiffY1 - 5)), rTrafo(Point(m_paintHeightDiffX1, m_paintHeightDiffY1)), Color::Blue());

			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintHeightDiffX2 + 5, m_paintHeightDiffY2 + 5)), rTrafo(Point(m_paintHeightDiffX2, m_paintHeightDiffY2)), Color::Blue());
			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintHeightDiffX2 - 5, m_paintHeightDiffY2 + 5)), rTrafo(Point(m_paintHeightDiffX2, m_paintHeightDiffY2)), Color::Blue());
		}

		Color col1 = Color::Green(), col2 = Color::Red();
		if (m_oCalcType != CalcType::eNormal)
		{
			col1 = Color::Red();
			col2 = Color::Green();
		}

		if (m_hasConvex)
		{
			rLayerContour.add<OverlayCross>(rTrafo(Point(m_paintVexX, m_paintVexY)), col1); // Punkt mit groesster Konvexitaet
			rLayerContour.add<OverlayCross>(rTrafo(Point(m_paintVexLineX, m_paintVexLineY)), col1); // Punkt mit groesster Konvexitaet => auf Linie
			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintVexX, m_paintVexY)), rTrafo(Point(m_paintVexLineX, m_paintVexLineY)), col1); // Abstandslinie Konvexitaet
		}

		if (m_hasConcave)
		{
			rLayerContour.add<OverlayCross>(rTrafo(Point(m_paintCavX, m_paintCavY)), col2); // Punkt mit groesster Konkavitaet
			rLayerContour.add<OverlayCross>(rTrafo(Point(m_paintCavLineX, m_paintCavLineY)), col2); // Punkt mit groesster Konkavitaet => auf Linie
			rLayerContour.add<OverlayLine>(rTrafo(Point(m_paintCavX, m_paintCavY)), rTrafo(Point(m_paintCavLineX, m_paintCavLineY)), col2); // Abstandslinie Konkavitaet
		}
	}
	catch(...)
	{
		return;
	}

} // paint

void Cavvex::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	m_badInput = false;


	m_useSEL100Calculation = (m_oMode == 10) || (m_oMode == 11);
	m_isCalculationLeftPossible = m_isCalculationRightPossible = true;
	m_leftCorrected = m_rightCorrected = false;

	int heightDiffSign = 1;

	bool usePixels = (m_oMode == 1) || (m_oMode == 11);

	m_overlayMin = 1000000;
	m_overlayMax = -1000000;
	m_hasConvex = false;
	m_hasConcave = false;
	m_paintHeightDiffX1 = m_paintHeightDiffY1 = m_paintHeightDiffX2 = m_paintHeightDiffY2 = 0;

	// Read-out laserline
	const GeoVecDoublearray& rLaserLineIn = m_pPipeInLaserLine->read(m_oCounter);
	m_oSpTrafo	= rLaserLineIn.context().trafo();
	// And extract byte-array
	const VecDoublearray& rLaserarray = rLaserLineIn.ref();
	// input validity check

	const ImageContext& imageContext = rLaserLineIn.context();
	const TPoint<double> hwRoi (imageContext.HW_ROI_x0, imageContext.HW_ROI_y0);

	auto &rCalib( system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0) );

	m_rGeoDoubleArraySeamLeft = m_pPipeInSeamLeft->read(m_oCounter);
	m_rGeoDoubleArraySeamRight = m_pPipeInSeamRight->read(m_oCounter);
	m_rGeoDoubleArrayAngle = m_pPipeInAngle->read(m_oCounter);

	geo2d::Doublearray oOutConcavity;
	geo2d::Doublearray oOutConvexity;
	geo2d::Doublearray oOutHightDiff;

	if (m_rGeoDoubleArraySeamLeft.rank()<0.001) m_badInput = true;
	if (m_rGeoDoubleArraySeamRight.rank()<0.001) m_badInput = true;

	if ((m_rGeoDoubleArraySeamLeft.ref().getData().size() > 0) && (m_rGeoDoubleArraySeamRight.ref().getData().size() > 0))
	{
		double seamLeft = m_rGeoDoubleArraySeamLeft.ref().getData()[0];
		double seamRight = m_rGeoDoubleArraySeamRight.ref().getData()[0];

		if (std::abs(seamRight - seamLeft) < 5) m_badInput = true;
	}

	if ( m_badInput || inputIsInvalid(rLaserLineIn) ||
		(m_rGeoDoubleArraySeamLeft.ref().getData().size() <= 0) ||
		(m_rGeoDoubleArraySeamRight.ref().getData().size() <= 0) ||
		(m_rGeoDoubleArrayAngle.ref().getData().size() <= 0)
		)
	{
		oOutConcavity.getData().push_back(0);
		oOutConcavity.getRank().push_back(0);
		oOutConvexity.getData().push_back(0);
		oOutConvexity.getRank().push_back(0);
		oOutHightDiff.getData().push_back(0);
		oOutHightDiff.getRank().push_back(0);

		const GeoDoublearray &rGeoConcave = GeoDoublearray( rLaserLineIn.context(), oOutConcavity, rLaserLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rGeoConvex = GeoDoublearray( rLaserLineIn.context(), oOutConvexity, rLaserLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rGeoHeightDiff = GeoDoublearray( rLaserLineIn.context(), oOutHightDiff, rLaserLineIn.analysisResult(), interface::NotPresent );

		m_oSpTrafo = nullptr;  // Trafo zu null setzen, so dass keine Ausgabe erfolgt. Wir fuehren hier also keine Rechnung durch, also wollen wir auch nichts ausgeben.

        preSignalAction();
        m_pPipeOutConcavity->signal( rGeoConcave );
        m_pPipeOutConvexity->signal( rGeoConvex );
        m_pPipeOutHeightDifference->signal( rGeoHeightDiff );

		return; // RETURN
	}

	const unsigned int	oNbLines	= rLaserarray.size();

	for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
	{ // loop over N lines
		double seamLeft, seamRight, oAngleInRad;

		SmpTrafo oSmpTrafoLeft = m_rGeoDoubleArraySeamLeft.context().trafo();
		SmpTrafo oSmpTrafoRight = m_rGeoDoubleArraySeamRight.context().trafo();
		double oDiffLeft = static_cast<double>(oSmpTrafoLeft->dx() - imageContext.trafo()->dx());  // imageContext ist der Context der Laserlinie
		double oDiffRight = static_cast<double>(oSmpTrafoRight->dx() - imageContext.trafo()->dx());

		if (m_rGeoDoubleArraySeamLeft.ref().getData().size() > lineN ||
			m_rGeoDoubleArraySeamRight.ref().getData().size() > lineN ||
			m_rGeoDoubleArrayAngle.ref().getData().size() > lineN
			)
		{
			seamLeft = m_rGeoDoubleArraySeamLeft.ref().getData()[lineN] + oDiffLeft;
			seamRight = m_rGeoDoubleArraySeamRight.ref().getData()[lineN] + oDiffRight;
			double oTemp = m_rGeoDoubleArrayAngle.ref().getData()[lineN];
			oAngleInRad = oTemp * M_PI / 180.0;
		}
		else
		{
			seamLeft = m_rGeoDoubleArraySeamLeft.ref().getData()[m_rGeoDoubleArraySeamLeft.ref().getData().size() - 1] + oDiffLeft;
			seamRight = m_rGeoDoubleArraySeamRight.ref().getData()[m_rGeoDoubleArraySeamRight.ref().getData().size() - 1] + oDiffRight;
			double oTemp = m_rGeoDoubleArrayAngle.ref().getData()[m_rGeoDoubleArrayAngle.ref().getData().size()-1];
			oAngleInRad = oTemp * M_PI / 180.0;
		}

		if (seamLeft > seamRight) // linker Rand sollte links liegen, rechter rechts
		{
			double oTemp = seamLeft;
			seamLeft = seamRight;
			seamRight = oTemp;
		}

		if (lineN == 0)
		{
			m_firstSeamLeft = (int)(seamLeft + 0.5);
			m_firstSeamRight = (int)(seamRight + 0.5);
		}

		const std::vector<double, std::allocator<double>> & rLaserLineIn_Data = rLaserarray[lineN].getData();
		const std::vector<int, std::allocator<int>> & rLaserLineIn_Rank = rLaserarray[lineN].getRank();

		if (seamLeft < 0 || seamRight >= rLaserLineIn_Data.size()) // stimmt was nicht
		{
			oOutConcavity.getData().push_back(0);
			oOutConcavity.getRank().push_back(0);
			oOutConvexity.getData().push_back(0);
			oOutConvexity.getRank().push_back(0);
			oOutHightDiff.getData().push_back(0);
			oOutHightDiff.getRank().push_back(0);

			const GeoDoublearray &rGeoConcave = GeoDoublearray(rLaserLineIn.context(), oOutConcavity, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoConvex = GeoDoublearray(rLaserLineIn.context(), oOutConvexity, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoHeightDiff = GeoDoublearray(rLaserLineIn.context(), oOutHightDiff, rLaserLineIn.analysisResult(), interface::NotPresent);

			m_oSpTrafo = nullptr;  // Trafo zu null setzen, so dass keine Ausgabe erfolgt.

			preSignalAction();
            m_pPipeOutConcavity->signal(rGeoConcave);
            m_pPipeOutConvexity->signal(rGeoConvex);
			m_pPipeOutHeightDifference->signal(rGeoHeightDiff);

			return; // RETURN
		}

		int startXLeft, endXLeft, startXRight, endXRight;

		// left side
		double slopeLeft, yInterceptionLeft;
		int slopeRankLeft, yInterceptionRankLeft;
		double slopeRight, yInterceptionRight;
		int slopeRankRight, yInterceptionRankRight;

		int middleSeam = (int)(0.5+(seamLeft+seamRight)/2);

		if (m_useSEL100Calculation)
		{
			startXLeft = middleSeam - PIXEL_IN_X_FOR_2_MM;
			endXLeft = (int)(0.5+seamLeft-5);
			startXRight = (int)(0.5+seamRight+5);
			endXRight = middleSeam + PIXEL_IN_X_FOR_2_MM;
		}
		else
		{
			startXLeft = (int)(0.5 + rLaserLineIn_Data.size() *  m_oLeftLineRoiStart / 100.0);
			endXLeft = (int)(0.5 + rLaserLineIn_Data.size() *  m_oLeftLineRoiEnd / 100.0 - 1);
			startXRight = (int)(0.5 + rLaserLineIn_Data.size() *  m_oRightLineRoiStart / 100.0);
			endXRight = (int)(0.5 + rLaserLineIn_Data.size() *  m_oRightLineRoiEnd / 100.0 -1);
		}

		// jetzt ueberpruefen
		if (startXLeft<0)
		{
			startXLeft = 0;
			m_leftCorrected = true;
		}

		if ((endXLeft - startXLeft) < 5) // zu klein fuer sinnvollen Geradenfit
		{
			m_isCalculationLeftPossible = false;
		}

		if (endXRight >= (int)rLaserLineIn_Data.size())
		{
			endXRight = rLaserLineIn_Data.size() - 1;
			m_rightCorrected = true;
		}

		if ((endXRight - startXRight) < 5) // zu klein fuer sinnvollen Geradenfit
		{
			m_isCalculationRightPossible = false;
		}

		// wenn ein Geradefit nicht moeglich ist, wird abgebrochen => bliebt zu klaeren, ob dann evtl. ein Analysefehler raus soll
		if (!m_isCalculationLeftPossible || !m_isCalculationRightPossible)
		{
			oOutConcavity.getData().push_back(0);
			oOutConcavity.getRank().push_back(0);
			oOutConvexity.getData().push_back(0);
			oOutConvexity.getRank().push_back(0);
			oOutHightDiff.getData().push_back(0);
			oOutHightDiff.getRank().push_back(0);

			const GeoDoublearray &rGeoConcave = GeoDoublearray(rLaserLineIn.context(), oOutConcavity, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoConvex = GeoDoublearray(rLaserLineIn.context(), oOutConvexity, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoHeightDiff = GeoDoublearray(rLaserLineIn.context(), oOutHightDiff, rLaserLineIn.analysisResult(), interface::NotPresent);

			m_oSpTrafo = nullptr;  // Trafo zu null setzen, so dass keine Ausgabe erfolgt.

			preSignalAction();

			m_pPipeOutConcavity->signal(rGeoConcave);
			m_pPipeOutConvexity->signal(rGeoConvex);
			m_pPipeOutHeightDifference->signal(rGeoHeightDiff);

			return; // RETURN
		}

		// Den Fit rechts und links fuehren wir zunaechst in jedem Fall aus, unabhaengig davon, ob der Benutzer das angewaehlt hat.
		// calc left side
		calcOneLine(rLaserLineIn_Data, rLaserLineIn_Rank, startXLeft, endXLeft, slopeLeft, slopeRankLeft, yInterceptionLeft, yInterceptionRankLeft, true, (int)(0.5+seamLeft), (int)(0.5+seamRight));

		// calc right side
		calcOneLine(rLaserLineIn_Data, rLaserLineIn_Rank, startXRight, endXRight, slopeRight, slopeRankRight, yInterceptionRight, yInterceptionRankRight, false, (int)(0.5 + seamLeft), (int)(0.5 + seamRight));

		// Jetzt muessen wir uns damit auseinandersetzen, welche dieser Fits relevant sind, und muessen die entsprechenden Variablen setzen:
		applyResultsFromOneSideToOther(rLaserLineIn_Data, rLaserLineIn_Rank, slopeLeft, slopeRankLeft, yInterceptionLeft, yInterceptionRankLeft, slopeRight, slopeRankRight, yInterceptionRight, yInterceptionRankRight);

		// Die Behandlung im Fall der ersten Laserlinie kann gemeinsam nach den beiden Aufrufen des Linienfits durchgefuehrt werden:
		if (lineN == 0)
		{
			m_paintNumber = rLaserLineIn_Data.size();
			m_paintSlopeLeft = slopeLeft;
			m_paintYInterceptLeft = yInterceptionLeft;
			m_paintStartXLeft = startXLeft;
			m_paintEndXLeft = endXLeft;

			m_paintSlopeRight = slopeRight;
			m_paintYInterceptRight = yInterceptionRight;
			m_paintStartXRight = startXRight;
			m_paintEndXRight = endXRight;
		}

		// Data from left side
		Line2D lineLeft(slopeLeft, yInterceptionLeft);
		double seamLeftY = lineLeft.getY(seamLeft);

		// Data from right side
		Line2D lineRight(slopeRight, yInterceptionRight);
		double seamRightY = lineRight.getY(seamRight);

		double heightDiff = 0;
		//Statistic1D stat4HeightDiff;

		bool leftIsLower = seamLeftY < seamRightY;
		double maxConcavity = 0;
		double maxConvexity = 0;

		m_paintCavX = m_paintCavY = m_paintVexX = m_paintVexY = 0;
		bool foundOne = false;
		bool foundMiddleSeam = false;

		const TPoint<double> oGlobalPos ( m_oSpTrafo->dx(), m_oSpTrafo->dy() );	//	Offset ROI Koordinaten -> Bildkoordinaten

		for (int xPos=(int)seamLeft+1; xPos<(int)seamRight; xPos++)
		{
			if (rLaserLineIn_Rank[xPos]<=0) continue;
			foundOne = true;

			double trackingY = rLaserLineIn_Data[xPos];

			double calcYPosLeft = lineLeft.getY(xPos);
			double calcYPosRight = lineRight.getY(xPos);

			// HeightDiff
			//if ((xPos == (int)seamLeft + 1) || (xPos == (int)seamRight - 1)) //erster und letzter Punkt reichen
			if (xPos == middleSeam) //jetzt in der Mitte berechnen
			{
				double yPosLineLeft = lineLeft.getY(xPos);
				double yPosLineRight = lineRight.getY(xPos);

				foundMiddleSeam = true;

				m_paintHeightDiffX1 = xPos;
				m_paintHeightDiffY1 = (int)(0.5+yPosLineLeft);
				m_paintHeightDiffX2 = xPos;
				m_paintHeightDiffY2 = (int)(0.5+yPosLineRight);

				//double xOnLineRight;
				heightDiff = yPosLineRight - yPosLineLeft;
				if (heightDiff < 0) heightDiffSign = -1;

				if (!usePixels)
				{
					const TPoint<double> pointLeft(xPos, yPosLineLeft);
					const TPoint<double> sensorPointLeft(pointLeft + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
					const TPoint<double> pointRight(xPos, yPosLineRight);
					const TPoint<double> sensorPointRight(pointRight + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten


					math::Vec3D sensorCoordLeft = rCalib.to3D(static_cast<int>(sensorPointLeft.x), static_cast<int>(sensorPointLeft.y), m_oTypeOfLaserLine);
					math::Vec3D sensorCoordRight = rCalib.to3D(static_cast<int>(sensorPointRight.x), static_cast<int>(sensorPointRight.y), m_oTypeOfLaserLine);
					math::Vec3D diffVector = sensorCoordRight - sensorCoordLeft;
					math::Vec3D diffVectorRotated = Calibration3DCoords::FromCalibratedToRotated(diffVector, oAngleInRad);
					heightDiff = heightDiffSign * std::abs(diffVectorRotated[2]);
				}

				if (m_oInvertHeightDiff) heightDiff *= -1;

				//double xOnLineLeft;
				//double diff2 = std::abs(lineLeft.calcDistance(xPos, calcYPosRight, xOnLineLeft));
				//double yOnLineLeft = lineLeft.getY(xOnLineLeft);

				//if (!usePixels)
				//{
				//	const TPoint<double> pointRight(xPos, calcYPosRight);
				//	const TPoint<double> sensorPointRight(pointRight + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				//	const TPoint<double> pointOnLineLeft(xOnLineLeft, yOnLineLeft);
				//	const TPoint<double> sensorPointOnLineLeft(pointOnLineLeft + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				//	math::Vec3D sensorCoordRight = rCalib.to3D(static_cast<int>(sensorPointRight.x), static_cast<int>(sensorPointRight.y), 0, m_oTypeOfLaserLine); // 0 is sensor id
				//	math::Vec3D sensorCoordOnLineLeft = rCalib.to3D(static_cast<int>(sensorPointOnLineLeft.x), static_cast<int>(sensorPointOnLineLeft.y), 0, m_oTypeOfLaserLine); // 0 is sensor id
				//	math::Vec3D diffVector = sensorCoordOnLineLeft - sensorCoordRight;
				//	math::Vec3D diffVectorRotated = Calibration3DCoords::FromCalibratedToRotated(diffVector, oAngleInRad);
				//	diff2 = std::abs(diffVectorRotated[2]);
				//}

				//stat4HeightDiff.addValues(diff1, diff2);
			} // if (erster oder letzter)

			double curConcavity = 0;
			double curConvexity = 0;

			double outX = -1.0;
			double outY = -1.0;  // initialisieren, Compiler meckert sonst im Momentix.

			if (leftIsLower)
			{
				if (calcYPosLeft > trackingY)
				{
					curConvexity = std::abs(lineLeft.calcDistance(xPos, trackingY, outX));
					outY = lineLeft.getY(outX);
				}

				if (trackingY > calcYPosRight)
				{
					curConcavity = std::abs(lineRight.calcDistance(xPos, trackingY, outX));
					outY = lineRight.getY(outX);
				}
			}
			else // right is lower
			{
				if (calcYPosRight > trackingY)
				{
					curConvexity = std::abs(lineRight.calcDistance(xPos, trackingY, outX));
					outY = lineRight.getY(outX);
				}

				if (trackingY > calcYPosLeft)
				{
					curConcavity = std::abs(lineLeft.calcDistance(xPos, trackingY, outX));
					outY = lineLeft.getY(outX);
				}
			}

			if (curConvexity > maxConvexity) // groessere Konvexitaet gefunden
			{
				m_hasConvex = true;
				maxConvexity = curConvexity;
				m_paintVexX = xPos;
				m_paintVexY = (int)(trackingY+0.5);
				m_paintVexLineX = (int)(outX + 0.5);
				m_paintVexLineY = (int)(outY + 0.5);
			}

			if (curConcavity > maxConcavity) // groessere Konkavitaet gefunden
			{
				m_hasConcave = true;
				maxConcavity = curConcavity;
				m_paintCavX = xPos;
				m_paintCavY = (int)(trackingY+0.5);
				m_paintCavLineX = (int)(outX + 0.5);
				m_paintCavLineY = (int)(outY + 0.5);
			}
		}

		if (!usePixels)
		{
			if (m_hasConcave)
			{
				// Diese Berechnung veraendert den Wert von maxConcavity. Das ist die Variable, die spaeter dann auch ausgegeben wird.
				// Das darf nur passieren, wenn wir auch tatsaechlich eine Konkavitaet haben - wenn das bei jeder Berechnung
				// veraendert wird, dann geben wir einen Wert ungleich null auch dann aus, wenn das nicht berechtigt ist.
				const TPoint<double> pointCav(m_paintCavX, m_paintCavY);
				const TPoint<double> sensorPosCav(pointCav + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				const TPoint<double> pointCavLine(m_paintCavLineX, m_paintCavLineY);
				const TPoint<double> sensorPosCavLine(pointCavLine + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				math::Vec3D sensorCoordCav = rCalib.to3D(static_cast<int>(sensorPosCav.x), static_cast<int>(sensorPosCav.y), m_oTypeOfLaserLine);
				math::Vec3D sensorCoordCavLine = rCalib.to3D(static_cast<int>(sensorPosCavLine.x), static_cast<int>(sensorPosCavLine.y), m_oTypeOfLaserLine);
				math::Vec3D diffVector = sensorCoordCavLine - sensorCoordCav;
				math::Vec3D diffVectorRotated = Calibration3DCoords::FromCalibratedToRotated(diffVector, oAngleInRad);
				maxConcavity = std::abs(diffVectorRotated[2]);
			}

			if (m_hasConvex)
			{
				// Der obige Kommentar gilt entsprechend.
				const TPoint<double> pointVex(m_paintVexX, m_paintVexY);
				const TPoint<double> sensorPosVex(pointVex + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				const TPoint<double> pointVexLine(m_paintVexLineX, m_paintVexLineY);
				const TPoint<double> sensorPosVexLine(pointVexLine + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				math::Vec3D sensorCoordVex = rCalib.to3D(static_cast<int>(sensorPosVex.x), static_cast<int>(sensorPosVex.y), m_oTypeOfLaserLine);
				math::Vec3D sensorCoordVexLine = rCalib.to3D(static_cast<int>(sensorPosVexLine.x), static_cast<int>(sensorPosVexLine.y), m_oTypeOfLaserLine);
				math::Vec3D diffVector = sensorCoordVexLine - sensorCoordVex;
				math::Vec3D diffVectorRotated = Calibration3DCoords::FromCalibratedToRotated(diffVector, oAngleInRad);
				maxConvexity = std::abs(diffVectorRotated[2]);
			}
		}

		//HeightDiff = foundOne? stat4HeightDiff.getCurrentMean() : 0;

		if (m_oCalcType != CalcType::eNormal)
		{
			double d = maxConcavity;
			maxConcavity = maxConvexity;
			maxConvexity = d;

			//bool b = m_hasConcave;
			//m_hasConcave = m_hasConvex;
			//m_hasConvex = b;


		}

		oOutConcavity.getData().push_back(maxConcavity);
		oOutConvexity.getData().push_back(maxConvexity);
		oOutHightDiff.getData().push_back(heightDiff);

		int rank = (foundOne) ? 255 : 0;
		oOutConcavity.getRank().push_back(rank);
		oOutConvexity.getRank().push_back(rank);
		rank = (foundMiddleSeam) ? 255 : 0;
		oOutHightDiff.getRank().push_back(rank);
	} // over lines

	interface::ResultType oGeoAnalysisResult = rLaserLineIn.analysisResult();

	const interface::GeoDoublearray oGeoDoubleOutConvexity( rLaserLineIn.context(), oOutConvexity, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutConcavity( rLaserLineIn.context(), oOutConcavity, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutHeightDiff( rLaserLineIn.context(), oOutHightDiff, oGeoAnalysisResult, 1.0 );
	// send the data out ...
	preSignalAction();
	m_pPipeOutConvexity->signal( oGeoDoubleOutConvexity );
	m_pPipeOutConcavity->signal( oGeoDoubleOutConcavity );
	m_pPipeOutHeightDifference->signal( oGeoDoubleOutHeightDiff );
} // proceedGroup

// Fast alle Argumente koennten je nach Situation sowohl Eingabe als auch Ausgabe sein:
void Cavvex::applyResultsFromOneSideToOther(const std::vector<double> &p_rLaserLineIn_Data, const std::vector<int> &p_rLaserLineIn_Rank, double& p_rSlopeLeft, int& p_rSlopeRankLeft, double& p_rYInterceptionLeft, int& p_rYInterceptionRankLeft, double& p_rSlopeRight, int& p_rSlopeRankRight, double& p_rYInterceptionRight, int& p_rYInterceptionRankRight)
{
	if (m_oWhichFit == eLeft)
	{
		// wenn nur der linke Fit relevant ist, muessen die Daten der rechten Seite auf andere Weise generiert werden:
		p_rSlopeRight = p_rSlopeLeft;  // die Steigung koennen wir uebernehmen unter der Annahme, dass die beiden Bleche dieselbe Orientierung haben.
		p_rSlopeRankRight = p_rSlopeRankLeft;

		// Zur Bestimmung des Y-Werts laufen wir durch einen ganz kurzen Bereich der Laserlinie.
		// Der Bereich ist nicht lang, weil das sonst das Ergebnis zu sehr verfaelscht.
		// Eine Schleife ueber einen ganz kurzen Bereich ist sinnvoll, weil wir uns die Show nicht von einem gelegentlichen ungueltigen Datum kaputt machen lassen wollen.
		int oStartTemp = m_firstSeamRight - 1;
		if (oStartTemp < 0)      // die rechte Nahtbegrenzung sollte so gross sein (ist schliesslich rechts und nicht links), dass wir ruhig 1 abziehen koennen sollten, aber wir machen diese Ueberpruefung trotzdem
		{
			oStartTemp = 0;
		}

		int oEndTemp = m_firstSeamRight + 1;
		if (oEndTemp > (int)p_rLaserLineIn_Data.size() - 1)  // Diese Ueberpruefung ist in seltenen Faellen relevant, und in diesen Faellen wird sie uns vor einem Absturz bewahren:
		{
			oEndTemp = p_rLaserLineIn_Data.size() - 1;
		}

		double oNumeratorY = 0.0;
		double oNumeratorX = 0.0;
		double oDenominator = 0.0;
		for (int oTempIndex = oStartTemp; oTempIndex <= oEndTemp; ++oTempIndex)  // Achtung, Schleife laeuft bis einschliesslich oEndTemp und das soll so sein.
		{
			if (p_rLaserLineIn_Rank[oTempIndex] > eRankMin)
			{
				oNumeratorY = oNumeratorY + p_rLaserLineIn_Data[oTempIndex];
				p_rYInterceptionRankRight = p_rLaserLineIn_Rank[oTempIndex];          // das setzen wir hier nur der Vollstaendigkeit wegen, tatsaechlich wird dieses Datum selber nicht verwendet. Auf das Mitteln des Ranks verzichten wir hier, wir akzeptieren einen der gueltigen Werte.
				oNumeratorX = oNumeratorX + oTempIndex;
				oDenominator++;
			}
		}
		oNumeratorY = oNumeratorY / oDenominator;   // Mittelung durchfuehren und die Variablen recyclen, um das Ergebnis drin zu speichern...
		oNumeratorX = oNumeratorX / oDenominator;
		p_rYInterceptionRight = oNumeratorY - p_rSlopeRight * oNumeratorX;
	}

	// hier auf keinen Fall ein "else", es gibt noch eine 3. Option, und die muss nicht behandelt werden!
	if (m_oWhichFit == eRight)
	{
		// wenn nur der rechte Fit relevant ist, muessen die Daten der linken Seite auf andere Weise generiert werden:
		p_rSlopeLeft = p_rSlopeRight;  // die Steigung koennen wir uebernehmen unter der Annahme, dass die beiden Bleche dieselbe Orientierung haben.
		p_rSlopeRankLeft = p_rSlopeRankRight;

		// Zur Bestimmung des Y-Werts laufen wir durch einen ganz kurzen Bereich der Laserlinie.
		// Der Bereich ist nicht lang, weil das sonst das Ergebnis zu sehr verfaelscht.
		// Eine Schleife ueber einen ganz kurzen Bereich ist sinnvoll, weil wir uns die Show nicht von einem gelegentlichen ungueltigen Datum kaputt machen lassen wollen.
		int oStartTemp = m_firstSeamLeft - 1;
		if (oStartTemp < 0)      // die linke Nahtbegrenzung koennte am linken Rand sein, also muss das hier ueberprueft werden
		{
			oStartTemp = 0;
		}

		int oEndTemp = m_firstSeamLeft + 1;
		if (oEndTemp > (int)p_rLaserLineIn_Data.size() - 1)  // Diese Ueberpruefung sollte nicht relevant sein (ist schliesslich die linke Begrenzung) aber wir machen sie trotzdem
		{
			oEndTemp = p_rLaserLineIn_Data.size() - 1;
		}

		double oNumeratorY = 0.0;
		double oNumeratorX = 0.0;
		double oDenominator = 0.0;
		for (int oTempIndex = oStartTemp; oTempIndex <= oEndTemp; ++oTempIndex)  // Achtung, Schleife laeuft bis einschliesslich oEndTemp und das soll so sein.
		{
			if (p_rLaserLineIn_Rank[oTempIndex] > eRankMin)
			{
				oNumeratorY = oNumeratorY + p_rLaserLineIn_Data[oTempIndex];
				p_rYInterceptionRankRight = p_rLaserLineIn_Rank[oTempIndex];          // das setzen wir hier nur der Vollstaendigkeit wegen, tatsaechlich wird dieses Datum selber nicht verwendet. Auf das Mitteln des Ranks verzichten wir hier, wir akzeptieren einen der gueltigen Werte.
				oNumeratorX = oNumeratorX + oTempIndex;
				oDenominator++;
			}
		}
		oNumeratorY = oNumeratorY / oDenominator;   // Mittelung durchfuehren und die Variablen recyclen, um das Ergebnis drin zu speichern...
		oNumeratorX = oNumeratorX / oDenominator;
		p_rYInterceptionLeft = oNumeratorY - p_rSlopeLeft * oNumeratorX;
	}
}

/*virtual*/ void
Cavvex::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

double Cavvex::calc2dDistance(double x1, double y1, double x2, double y2)
{
	double distance;
	distance  = (x1 - x2) * (x1 - x2);
	distance += (y1 - y2) * (y1 - y2);
	return sqrt(distance);
}

void Cavvex::calcOneLine(const std::vector<double> &rLaserLineIn_Data, const std::vector<int> &rLaserLineIn_Rank,
	int startX, int endX, double & slopeOut, int & slopeOutRank, double & yInterceptionOut, int & yInterceptionOutRank, bool isLeftSide, int seamLeft, int seamRight)
{
		LineFitter lineFitter;

		int lineY;

		for (int x=startX; x<endX; x++)
		{
			lineY = int( rLaserLineIn_Data[x] );
			int rank = int(rLaserLineIn_Rank[x]);
			if (lineY<=0) continue;
			if (rank>0)
			{
				lineY = int(rLaserLineIn_Data[x]);
				lineFitter.addPoint(x, lineY);
			}
		}

		double m, b;
		lineFitter.calcMB(m, b);
		//if (lineN == 0)
		//{
		//	m_paintNumber = rLaserLineIn_Data.size();
		//	m_paintSlope = m;
		//	m_paintYIntercept = b;
		//	m_paintStartX = startX;
		//	m_paintEndX = endX;
		//}
		slopeOut = m;
		slopeOutRank = 255;
		yInterceptionOut = b;
		yInterceptionOutRank = 255;
}



Statistic1D::Statistic1D()
{
	reset();
}

Statistic1D::~Statistic1D()
{
}

void Statistic1D::addValue(double value)
{
	_currentSum += value;
	_count++;
}

void Statistic1D::addValues(double value1, double value2)
{
	addValue(value1);
	addValue(value2);
}


void Statistic1D::delValue(double value)
{
	if (_count<=0) return;
	_currentSum = _currentSum - value;
	_count--;
}

void Statistic1D::reset()
{
	_count = 0;
	_currentSum = 0;
}

double Statistic1D::getCurrentMean()
{
	if (_count <= 0) return 0;
	return _currentSum / _count;
}


} // namespace precitec
} // namespace filter
