/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		CB
 * 	@date		2020
 * 	@brief 		This filter calculates the values for convexity, concavity and height difference without lineFit
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
#include "cavvexSimple.h"
#include "line2D.h"
#include "util/calibDataSingleton.h"
#include <filter/armStates.h>

#include "fliplib/TypeToDataTypeImpl.h"

#include "math/3D/projectiveMathStructures.h"

#define PIXEL_IN_X_FOR_2_MM 180

using namespace precitec::math;

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string CavvexSimple::m_oFilterName 			= std::string("CavvexSimple");
const std::string CavvexSimple::PIPENAME_CONVEX_OUT		= std::string("ConvexityOut");		///< Name Out-Pipe
const std::string CavvexSimple::PIPENAME_CONCAVE_OUT		= std::string("ConcavityOut");		///< Name Out-Pipe
const std::string CavvexSimple::PIPENAME_CONVEXPOSX_OUT = std::string("ConvexityPosX_Out");		///< Name Out-Pipe
const std::string CavvexSimple::PIPENAME_CONCAVEPOSX_OUT = std::string("ConcavityPosX_Out");		///< Name Out-Pipe
const std::string CavvexSimple::PIPENAME_HEIGHTDIFF_OUT	= std::string("HeightDifferenceOut");		///< Name Out-Pipe

const std::string CavvexSimple::m_oParamTypeOfLaserLine = std::string("TypeOfLaserLine"); ///< Parameter: Type of LaserLine (e.g. FrontLaserLine, BehindLaserLine)


CavvexSimple::CavvexSimple() :
	TransformFilter( CavvexSimple::m_oFilterName, Poco::UUID("792D79B4-E8FD-4BAB-B7E7-B41FA122686E") ),
	m_pPipeInLaserLine( nullptr ),
	m_pPipeInSeamLeft(nullptr),
	m_pPipeInSeamRight(nullptr),
	m_pPipeInAngle(nullptr),
	m_pPipeInLineLeft(nullptr),
	m_pPipeInLineRight(nullptr),
	m_pPipeInLeftLineSlope( nullptr),
	m_pPipeInLeftLineYIntercept(nullptr),
	m_pPipeInRightLineSlope(nullptr),
	m_pPipeInRightLineYIntercept(nullptr),

	m_oConvexityOut(1),
	m_oConcavityOut( 1 ),
	m_oConvexityOutPosX(1),
	m_oConcavityOutPosX(1),
	m_oHeightDiffOut( 1 ),
	m_oMode(0),
	m_oCalcType(eNormal),
	m_oInvertHeightDiff(false),
	m_oTypeOfLaserLine(BehindLaserLine)    // im Zusammenhang des CavvexSimple macht es mehr Sinn, die Auswertung auf der nachlaufenden Linie durchzufuehren, also initialisiere zu diesem Wert
{
	m_pPipeOutConvexity = new SynchronePipe< GeoDoublearray >( this, CavvexSimple::PIPENAME_CONVEX_OUT );
	m_pPipeOutConvexityPosX = new SynchronePipe< GeoDoublearray >(this, CavvexSimple::PIPENAME_CONVEXPOSX_OUT);
	m_pPipeOutConcavity = new SynchronePipe< GeoDoublearray >( this, CavvexSimple::PIPENAME_CONCAVE_OUT );
	m_pPipeOutConcavityPosX = new SynchronePipe< GeoDoublearray >(this, CavvexSimple::PIPENAME_CONCAVEPOSX_OUT);
	m_pPipeOutHeightDifference = new SynchronePipe< GeoDoublearray >( this, CavvexSimple::PIPENAME_HEIGHTDIFF_OUT );

	parameters_.add("Mode",    Parameter::TYPE_int, m_oMode);
	parameters_.add("CalcType", Parameter::TYPE_int, static_cast<int>(m_oCalcType));
	parameters_.add("InvertHeightDiff", Parameter::TYPE_bool, m_oInvertHeightDiff);

	int oLaserLineTemp = static_cast<int>(m_oTypeOfLaserLine);
	parameters_.add(CavvexSimple::m_oParamTypeOfLaserLine, fliplib::Parameter::TYPE_int, oLaserLineTemp);  // Fuege den Parameter mit dem soeben initialisierten Wert hinzu.

    setInPipeConnectors({{Poco::UUID("80A4EB8D-F038-47BA-9352-E284EFC03D47"), m_pPipeInLaserLine, "LaserLine", 1, "LaserLine"},
    {Poco::UUID("21FBFED1-D848-4F90-A104-C85ED414EAC1"), m_pPipeInSeamLeft, "SeamLeft", 1, "SeamLeft"},
    {Poco::UUID("30538DAC-65BB-4F85-8CF5-C78262BBAE0F"), m_pPipeInSeamRight, "SeamRight", 1, "SeamRight"},
    {Poco::UUID("7B76D4CD-401D-44B9-9B10-73B11AB9FB3F"), m_pPipeInAngle, "Angle", 1, "Angle"},
    {Poco::UUID("97A730E2-22F6-4073-B0AB-90BE5336431E"), m_pPipeInLeftLineSlope, "LeftSlope", 1, "LeftSlope"},
    {Poco::UUID("5C196743-0ACA-433C-B89E-6B09E0ADB02D"), m_pPipeInLeftLineYIntercept, "LeftYIntercept", 1, "LeftYIntercept"},
    {Poco::UUID("082AB8C9-B3DE-432F-8290-CB247CFF7DA2"), m_pPipeInRightLineSlope, "RightSlope", 1, "RightSlope"},
    {Poco::UUID("8E433EE0-A849-4FF0-8405-AACD61F4E8E1"), m_pPipeInRightLineYIntercept, "RightYIntercept", 1, "RightYIntercept"}});
    setOutPipeConnectors({{Poco::UUID("C2EB57A6-6ED8-4157-8064-3C9D0ED13043"), m_pPipeOutConvexity, PIPENAME_CONVEX_OUT, 0, ""},
    {Poco::UUID("065A8757-3087-4353-BFC0-5C8628506AD0"), m_pPipeOutConcavity, PIPENAME_CONCAVE_OUT, 0, ""},
    {Poco::UUID("78235DDB-D7E2-4BE9-9EDF-AAF8E1CCFE9C"), m_pPipeOutHeightDifference, PIPENAME_HEIGHTDIFF_OUT, 0, ""},
    {Poco::UUID("AAEC944D-2CC7-4626-8F35-B806B3743FA2"), m_pPipeOutConvexity, PIPENAME_CONVEX_OUT, 0, ""},
    {Poco::UUID("DB6A659E-1B28-448D-B765-7D8FB9C9E972"), m_pPipeOutConcavity, PIPENAME_CONCAVE_OUT, 0, ""}});
    setVariantID(Poco::UUID("493A09FE-E81C-4759-967B-F78B98E127AA"));
}

CavvexSimple::~CavvexSimple()
{
	delete m_pPipeOutConvexity;
	delete m_pPipeOutConcavity;
	delete m_pPipeOutHeightDifference;
} //

void CavvexSimple::setParameter()
{
	TransformFilter::setParameter();
	m_oMode = parameters_.getParameter("Mode").convert<int>();
	m_oCalcType = static_cast<CalcType>(parameters_.getParameter("CalcType").convert<int>());

	int oTempLine = parameters_.getParameter(CavvexSimple::m_oParamTypeOfLaserLine).convert<int>();
	m_oTypeOfLaserLine = static_cast<LaserLine>(oTempLine);

	m_oInvertHeightDiff = static_cast<bool>(parameters_.getParameter("InvertHeightDiff").convert<bool>());

} // setParameter

bool CavvexSimple::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.type() == typeid(GeoVecDoublearray) )
		m_pPipeInLaserLine  = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray > * >(&p_rPipe);

	if ( p_rPipe.tag() == "SeamLeft" )
		m_pPipeInSeamLeft = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "SeamRight" )
		m_pPipeInSeamRight = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if ( p_rPipe.tag() == "Angle" )
		m_pPipeInAngle = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if (p_rPipe.tag() == "LeftSlope")
		m_pPipeInLeftLineSlope = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if (p_rPipe.tag() == "LeftYIntercept")
		m_pPipeInLeftLineYIntercept = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if (p_rPipe.tag() == "RightSlope")
		m_pPipeInRightLineSlope = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);
	if (p_rPipe.tag() == "RightYIntercept")
		m_pPipeInRightLineYIntercept = dynamic_cast< fliplib::SynchronePipe < interface::GeoDoublearray > * >(&p_rPipe);


	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void CavvexSimple::paint()
{
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull())
	{
		return;
	} // if

	try
	{
		//int yval;

		const Trafo		&rTrafo(*m_oSpTrafo);
		OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
		OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

		// Naht links - das Kreuz soll in jedem Fall ausgegeben werden, weil wir schliesslich das Y-Datum aus der Laserlinie in jedem Fall beziehen.
		int yvalSeamLeft = (int)(0.5 + m_firstSeamLeft * m_paintSlopeLeft + m_paintYInterceptLeft);

        rLayerContour.add<OverlayLine> (rTrafo(Point(m_firstSeamLeft - 1, yvalSeamLeft - 1)), rTrafo(Point(m_firstSeamLeft - 5, yvalSeamLeft - 5)), Color::Red());
        rLayerContour.add<OverlayLine> (rTrafo(Point(m_firstSeamLeft - 1, yvalSeamLeft + 1)), rTrafo(Point(m_firstSeamLeft - 5, yvalSeamLeft + 5)), Color::Red());

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

void CavvexSimple::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
{
	poco_assert_dbg(m_pPipeInLaserLine != nullptr); // to be asserted by graph editor
	m_badInput = false;

	//m_useSEL100Calculation = (m_oMode == 10) || (m_oMode == 11);
	//m_isCalculationLeftPossible = m_isCalculationRightPossible = true;
	//m_leftCorrected = m_rightCorrected = false;

	int heightDiffSign = 1;

	bool usePixels = (m_oMode == 1);

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

	const GeoDoublearray& leftLineSlopeIn = m_pPipeInLeftLineSlope->read(m_oCounter);
	const Doublearray leftLineSlope = leftLineSlopeIn.ref();
	m_leftLineSlope = leftLineSlope.getData()[0];

	const GeoDoublearray& leftLineYInterceptIn = m_pPipeInLeftLineYIntercept->read(m_oCounter);
	const Doublearray leftLineYIntercept = leftLineYInterceptIn.ref();
	m_leftLineYIntercept = leftLineYIntercept.getData()[0];

	const GeoDoublearray& rightLineSlopeIn = m_pPipeInRightLineSlope->read(m_oCounter);
	const Doublearray rightLineSlope = rightLineSlopeIn.ref();
	m_rightLineSlope = rightLineSlope.getData()[0];

	const GeoDoublearray& rightLineYInterceptIn = m_pPipeInRightLineYIntercept->read(m_oCounter);
	const Doublearray rightLineYIntercept = rightLineYInterceptIn.ref();
	m_rightLineYIntercept = rightLineYIntercept.getData()[0];



	geo2d::Doublearray oOutConcavity;
	geo2d::Doublearray oOutConvexity;
	geo2d::Doublearray oOutConcavityPosX;
	geo2d::Doublearray oOutConvexityPosX;
	geo2d::Doublearray oOutHeightDiff;

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
		(leftLineSlope.getData().size() <= 0) ||
		(leftLineYIntercept.getData().size() <= 0) ||
		(rightLineSlope.getData().size() <= 0) ||
		(rightLineYIntercept.getData().size() <= 0) ||
		(m_rGeoDoubleArrayAngle.ref().getData().size() <= 0)
		)
	{
		oOutConcavity.getData().push_back(0);
		oOutConcavity.getRank().push_back(0);
		oOutConcavityPosX.getData().push_back(0);
		oOutConcavityPosX.getRank().push_back(0);
		oOutConvexity.getData().push_back(0);
		oOutConvexity.getRank().push_back(0);
		oOutConvexityPosX.getData().push_back(0);
		oOutConvexityPosX.getRank().push_back(0);
		      oOutHeightDiff.getData().push_back(0);
		      oOutHeightDiff.getRank().push_back(0);

		const GeoDoublearray &rGeoConcave = GeoDoublearray( rLaserLineIn.context(), oOutConcavity, rLaserLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rGeoConcavePosX = GeoDoublearray(rLaserLineIn.context(), oOutConcavityPosX, rLaserLineIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rGeoConvex = GeoDoublearray( rLaserLineIn.context(), oOutConvexity, rLaserLineIn.analysisResult(), interface::NotPresent );
		const GeoDoublearray &rGeoConvexPosX = GeoDoublearray(rLaserLineIn.context(), oOutConvexityPosX, rLaserLineIn.analysisResult(), interface::NotPresent);
		const GeoDoublearray &rGeoHeightDiff = GeoDoublearray( rLaserLineIn.context(), oOutHeightDiff, rLaserLineIn.analysisResult(), interface::NotPresent );

		m_oSpTrafo = nullptr;  // Trafo zu null setzen, so dass keine Ausgabe erfolgt. Wir fuehren hier also keine Rechnung durch, also wollen wir auch nichts ausgeben.

        preSignalAction();
        m_pPipeOutConcavity->signal( rGeoConcave );
		m_pPipeOutConcavityPosX->signal(rGeoConcavePosX);
        m_pPipeOutConvexity->signal(rGeoConvex);
		m_pPipeOutConvexityPosX->signal(rGeoConvexPosX);
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
			oOutConcavityPosX.getData().push_back(0);
			oOutConcavityPosX.getRank().push_back(0);
			oOutConvexity.getData().push_back(0);
			oOutConvexity.getRank().push_back(0);
			oOutConvexityPosX.getData().push_back(0);
			oOutConvexityPosX.getRank().push_back(0);
			         oOutHeightDiff.getData().push_back(0);
			         oOutHeightDiff.getRank().push_back(0);

			const GeoDoublearray &rGeoConcave = GeoDoublearray(rLaserLineIn.context(), oOutConcavity, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoConcavePosX = GeoDoublearray(rLaserLineIn.context(), oOutConcavityPosX, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoConvex = GeoDoublearray(rLaserLineIn.context(), oOutConvexity, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoConvexPosX = GeoDoublearray(rLaserLineIn.context(), oOutConvexityPosX, rLaserLineIn.analysisResult(), interface::NotPresent);
			const GeoDoublearray &rGeoHeightDiff = GeoDoublearray(rLaserLineIn.context(), oOutHeightDiff, rLaserLineIn.analysisResult(), interface::NotPresent);

			m_oSpTrafo = nullptr;  // Trafo zu null setzen, so dass keine Ausgabe erfolgt.

			preSignalAction();
            m_pPipeOutConcavity->signal(rGeoConcave);
			m_pPipeOutConcavityPosX->signal(rGeoConcavePosX);
            m_pPipeOutConvexity->signal(rGeoConvex);
			m_pPipeOutConvexityPosX->signal(rGeoConvexPosX);
			m_pPipeOutHeightDifference->signal(rGeoHeightDiff);

			return; // RETURN
		}

		//int startXLeft, endXLeft, startXRight, endXRight;
		int middleSeam = (int)(0.5 + (seamLeft + seamRight) / 2);

		// left side
//		double slopeLeft, yInterceptionLeft;
//		int slopeRankLeft, yInterceptionRankLeft;
//		double slopeRight, yInterceptionRight;
//		int slopeRankRight, yInterceptionRankRight;


		// Die Behandlung im Fall der ersten Laserlinie kann gemeinsam nach den beiden Aufrufen des Linienfits durchgefuehrt werden:
		if (lineN == 0)
		{
			m_paintNumber = rLaserLineIn_Data.size();
			m_paintSlopeLeft = m_leftLineSlope;
			m_paintYInterceptLeft = m_leftLineYIntercept;

			m_paintSlopeRight = m_rightLineSlope;
			m_paintYInterceptRight = m_rightLineYIntercept;
		}

		// Data from left side
		Line2D lineLeft(m_leftLineSlope, m_leftLineYIntercept);
		double seamLeftY = lineLeft.getY(seamLeft);

		// Data from right side
		Line2D lineRight(m_rightLineSlope, m_rightLineYIntercept);
		double seamRightY = lineRight.getY(seamRight);

		double middleSeamY_LineLeft = lineLeft.getY(middleSeam);
		double middleSeamY_LineRight = lineRight.getY(middleSeam);
		double middleSeamY = 0.0;

        double heightDiff = 0;

		bool leftIsLower = seamLeftY < seamRightY;
		double maxConcavity = 0, maxConcavityPosX = 0;
		double maxConvexity = 0, maxConvexityPosX = 0;

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
					middleSeamY = middleSeamY_LineLeft;
				}

				if (trackingY > calcYPosRight)
				{
					curConcavity = std::abs(lineRight.calcDistance(xPos, trackingY, outX));
					outY = lineRight.getY(outX);
					middleSeamY = middleSeamY_LineRight;
				}
			}
			else // right is lower
			{
				if (calcYPosRight > trackingY)
				{
					curConvexity = std::abs(lineRight.calcDistance(xPos, trackingY, outX));
					outY = lineRight.getY(outX);
					middleSeamY = middleSeamY_LineRight;
				}

				if (trackingY > calcYPosLeft)
				{
					curConcavity = std::abs(lineLeft.calcDistance(xPos, trackingY, outX));
					outY = lineLeft.getY(outX);
					middleSeamY = middleSeamY_LineLeft;
				}
			}

			if (curConvexity > maxConvexity) // groessere Konvexitaet gefunden
			{
				m_hasConvex = true;
				maxConvexity = curConvexity;
				maxConvexityPosX = xPos - middleSeam;
				m_paintVexX = xPos;
				m_paintVexY = (int)(trackingY+0.5);
				m_paintVexLineX = (int)(outX + 0.5);
				m_paintVexLineY = (int)(outY + 0.5);
				m_paintVexMiddleY = (int)(middleSeamY + 0.5);
			}

			if (curConcavity > maxConcavity) // groessere Konkavitaet gefunden
			{
				m_hasConcave = true;
				maxConcavity = curConcavity;
				maxConcavityPosX = xPos - middleSeam;
				m_paintCavX = xPos;
				m_paintCavY = (int)(trackingY+0.5);
				m_paintCavLineX = (int)(outX + 0.5);
				m_paintCavLineY = (int)(outY + 0.5);
				m_paintCavMiddleY = (int)(middleSeamY + 0.5);
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

				// Calculate Distance of concav to seam middle
				const TPoint<double> pointCavMiddle(middleSeam, m_paintCavMiddleY);
				const TPoint<double> sensorPosCavMiddle(pointCavMiddle + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				auto distance = rCalib.distFrom2D(sensorPosCavMiddle.x, sensorPosCavMiddle.y, sensorPosCavLine.x, sensorPosCavLine.y);
				maxConcavityPosX = distance;
				if (sensorPosCavLine.x < sensorPosCavMiddle.x)
					maxConcavityPosX *= -1;
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

                // Calculate Distance of convex to seam middle
				const TPoint<double> pointVexMiddle(middleSeam, m_paintVexMiddleY);
				const TPoint<double> sensorPosVexMiddle(pointVexMiddle + oGlobalPos + hwRoi);	//	Offset Bildkoordinaten -> Sensorkoordinaten
				auto distance = rCalib.distFrom2D(sensorPosVexMiddle.x, sensorPosVexMiddle.y, sensorPosVexLine.x, sensorPosVexLine.y);
				maxConvexityPosX = distance;
				if (sensorPosVexLine.x < sensorPosVexMiddle.x)
					maxConvexityPosX *= -1;

            }
		}

		//HeightDiff = foundOne? stat4HeightDiff.getCurrentMean() : 0;

		if (m_oCalcType != CalcType::eNormal)
		{
			double d = maxConcavity;
			maxConcavity = maxConvexity;
			maxConvexity = d;

			d = maxConcavityPosX;
			maxConcavityPosX = maxConvexityPosX;
			maxConvexityPosX = d;

            //bool b = m_hasConcave;
			//m_hasConcave = m_hasConvex;
			//m_hasConvex = b;


		}

		oOutConcavity.getData().push_back(maxConcavity);
		oOutConcavityPosX.getData().push_back(maxConcavityPosX);
		oOutConvexity.getData().push_back(maxConvexity);
		oOutConvexityPosX.getData().push_back(maxConvexityPosX);
		      oOutHeightDiff.getData().push_back(heightDiff);

		int rank = (foundOne) ? 255 : 0;
		oOutConcavity.getRank().push_back(rank);
		oOutConcavityPosX.getRank().push_back(rank);
		oOutConvexity.getRank().push_back(rank);
		oOutConvexityPosX.getRank().push_back(rank);
		rank = (foundMiddleSeam) ? 255 : 0;
		      oOutHeightDiff.getRank().push_back(rank);
	} // over lines

	interface::ResultType oGeoAnalysisResult = rLaserLineIn.analysisResult();

	const interface::GeoDoublearray oGeoDoubleOutConvexity( rLaserLineIn.context(), oOutConvexity, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutConvexityPosX(rLaserLineIn.context(), oOutConvexityPosX, oGeoAnalysisResult, 1.0);
	const interface::GeoDoublearray oGeoDoubleOutConcavity( rLaserLineIn.context(), oOutConcavity, oGeoAnalysisResult, 1.0 );
	const interface::GeoDoublearray oGeoDoubleOutConcavityPosX(rLaserLineIn.context(), oOutConcavityPosX, oGeoAnalysisResult, 1.0);
	const interface::GeoDoublearray oGeoDoubleOutHeightDiff( rLaserLineIn.context(), oOutHeightDiff, oGeoAnalysisResult, 1.0 );
	// send the data out ...
	preSignalAction();
	m_pPipeOutConvexity->signal( oGeoDoubleOutConvexity );
	m_pPipeOutConvexityPosX->signal(oGeoDoubleOutConvexityPosX);
	m_pPipeOutConcavity->signal( oGeoDoubleOutConcavity );
	m_pPipeOutConcavityPosX->signal(oGeoDoubleOutConcavityPosX);
	m_pPipeOutHeightDifference->signal( oGeoDoubleOutHeightDiff );
} // proceedGroup

/*// Fast alle Argumente koennten je nach Situation sowohl Eingabe als auch Ausgabe sein:
void CavvexSimple::applyResultsFromOneSideToOther(const std::vector<double, std::allocator<double>> &p_rLaserLineIn_Data, const std::vector<int, std::allocator<int>> p_rLaserLineIn_Rank, double& p_rSlopeLeft, int& p_rSlopeRankLeft, double& p_rYInterceptionLeft, int& p_rYInterceptionRankLeft, double& p_rSlopeRight, int& p_rSlopeRankRight, double& p_rYInterceptionRight, int& p_rYInterceptionRankRight)
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
}*/

/*virtual*/ void
CavvexSimple::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

double CavvexSimple::calc2dDistance(double x1, double y1, double x2, double y2)
{
	double distance;
	distance  = (x1 - x2) * (x1 - x2);
	distance += (y1 - y2) * (y1 - y2);
	return sqrt(distance);
}

/* void CavvexSimple::calcOneLine(const std::vector<double, std::allocator<double>> &rLaserLineIn_Data, const std::vector<int, std::allocator<int>> rLaserLineIn_Rank,
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
} */



} // namespace precitec
} // namespace filter
