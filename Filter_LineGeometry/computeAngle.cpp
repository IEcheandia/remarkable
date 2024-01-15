/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Christian Duchow (Duw)
 * 	@date		2015
 * 	@brief
 */

// Diese Header sind auch in hough.cpp verwendet worden. Sehr interessant:
// Diese Zeilen muessen hier als erstes kommen! Das liegt daran, dass <cmath> wohl bereits an anderer Stelle
// inkludiert wird, aber ohne _USE_MATH_DEFINES. Aber weil <cmath> natuerlich Inkludeguards hat,
// hat eine zweite Einbindung weiter unten keinen Effekt - also muss das hier als erstes erfolgen!
#define _USE_MATH_DEFINES						/// pi constant
#include <cmath>								/// trigonometry, pi constant


#include "computeAngle.h"

#include <cmath>

#include "Poco/SharedPtr.h"
#include "fliplib/Parameter.h"
#include "fliplib/SynchronePipe.h"

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "util/calibDataSingleton.h"

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
	using namespace interface;
	using namespace image;
	using namespace geo2d;
	namespace filter
	{

using Poco::SharedPtr;
using fliplib::SynchronePipe;
using fliplib::BaseFilterInterface;
using fliplib::BasePipe;
using fliplib::TransformFilter;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;


const std::string ComputeAngle::m_oFilterName = std::string("ComputeAngle");
const std::string ComputeAngle::m_oPipenameOut = std::string("Angle");
const std::string ComputeAngle::m_oFitintervalSizeName = std::string("FitIntervalSize");

ComputeAngle::ComputeAngle() :
TransformFilter(ComputeAngle::m_oFilterName, Poco::UUID{"6838F613-ACEB-4D52-BC8D-3835B9427F58"}),
	pipeInLineY_		( nullptr ),
	pipeInPosX_			( nullptr ),
	pipeOutAngle_(new SynchronePipe< GeoDoublearray >(this, ComputeAngle::m_oPipenameOut)),
	m_oFitintervalSize(10), // man sollte es sich angewoehnen, die Paramter grundsaetzlich im Konstruktor zu initialisieren.
	m_oTypeOfLaserLine(LaserLine::FrontLaserLine)
{
	// Parameter
	parameters_.add(m_oFitintervalSizeName, Parameter::TYPE_int, static_cast<int>(m_oFitintervalSize));
	parameters_.add("TypeOfLaserLine", Parameter::TYPE_int, int(m_oTypeOfLaserLine));

    setInPipeConnectors({{Poco::UUID("AF6FD348-8500-4D86-8B15-7DA6B8AEC7B0"), pipeInLineY_, "Laserline", 1, "line"},
    {Poco::UUID("725AC1AA-77C2-4678-B9C2-D755B5B6732D"), pipeInPosX_, "PositionX", 1, "position_x"}});
    setOutPipeConnectors({{Poco::UUID("51A1266B-079C-494D-993B-09C74763436F"), pipeOutAngle_, m_oPipenameOut, 0, ""}});
    setVariantID(Poco::UUID("C4968B22-D6C5-429A-A944-771101238D3F"));
}

ComputeAngle::~ComputeAngle()
{
	delete pipeOutAngle_;
}

void ComputeAngle::setParameter()
{
	TransformFilter::setParameter();

	// Parameter
	m_oFitintervalSize = parameters_.getParameter(m_oFitintervalSizeName).convert<int>();
	m_oTypeOfLaserLine = castToLaserLine(parameters_.getParameter("TypeOfLaserLine").convert<int>());


	if(m_oVerbosity >= eMedium)
	{
		wmLog(precitec::eDebug, "Filter '%s':.\n", m_oFilterName.c_str());
	}
}

void ComputeAngle::arm(const fliplib::ArmStateBase& state)
{
	int ArmState = state.getStateID();

	if(m_oVerbosity >= eMedium) wmLog(eDebug, "HeightDifference armstate=%d\n",ArmState);
} // arm

void ComputeAngle::paint() {
	if(m_oVerbosity < eLow || m_oSpTrafo.isNull()){
		return;
	} // if

	const Trafo		&rTrafo		( *m_oImageContext.trafo() );
	OverlayCanvas	&rCanvas	( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayer		( rCanvas.getLayerPosition() ); // layer 2: paint over position found before

	const Point oPoint1(m_oPos1X, m_oPos1Y);
	const Point oPoint2(m_oPos2X, m_oPos2Y);

	rLayer.add( new	OverlayCross(rTrafo(oPoint1), Color::Yellow())); // paint position in Yellow
	rLayer.add( new	OverlayCross(rTrafo(oPoint2), Color::Yellow())); // paint position in Yellow

} // paint

bool ComputeAngle::subscribe(BasePipe& pipe, int group)
{
	if ( pipe.type() == typeid(GeoVecDoublearray) )
		pipeInLineY_ = dynamic_cast< SynchronePipe <GeoVecDoublearray> * >(&pipe);
	else if ( pipe.type() == typeid(GeoDoublearray) ) {
		if (pipe.tag() == "position_x")
			pipeInPosX_  = dynamic_cast< fliplib::SynchronePipe < GeoDoublearray > * >(&pipe);
	} // if

	return BaseFilter::subscribe( pipe, group );
}


void ComputeAngle::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
	poco_assert_dbg(pipeInLineY_ != nullptr); // to be asserted by graph editor
	poco_assert_dbg(pipeInPosX_ != nullptr); // to be asserted by graph editor

	const GeoVecDoublearray& rGeoVecDoublearrayLineIn = pipeInLineY_->read(m_oCounter);
	m_oSpTrafo = rGeoVecDoublearrayLineIn.context().trafo();
	m_oImageContext = rGeoVecDoublearrayLineIn.context(); //context speichern

	m_oHwRoi.x = m_oImageContext.HW_ROI_x0;
	m_oHwRoi.y = m_oImageContext.HW_ROI_y0;

	const VecDoublearray& rVecDoublearrayLineIn = rGeoVecDoublearrayLineIn.ref();
	const Doublearray	&rDoubleArrayLineIn = rVecDoublearrayLineIn[0];
	const std::vector<double> &rInDataLine = rDoubleArrayLineIn.getData();
	const std::vector<int> &rInRankLine = rDoubleArrayLineIn.getRank();

	const GeoDoublearray& rGeoDoublearrayXIn = pipeInPosX_->read(m_oCounter);
	const Doublearray	&rDoubleArrayXIn = rGeoDoublearrayXIn.ref();
	const std::vector<double> &rInDataX = rDoubleArrayXIn.getData();
	//const std::vector<int> &rInRankX = rDoubleArrayXIn.getRank();

	if (inputIsInvalid(rGeoVecDoublearrayLineIn) || inputIsInvalid(rGeoDoublearrayXIn)) {
		const double			oRank		( interface::NotPresent );
		const GeoDoublearray	oGeoMismatchOut(rGeoDoublearrayXIn.context(), Doublearray(1, 0.0, eRankMin), rGeoDoublearrayXIn.analysisResult(), oRank);
		preSignalAction(); pipeOutAngle_->signal(oGeoMismatchOut);

		return; // RETURN
	}

	if (rInDataX.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDataX.size());
	}
	if (rGeoDoublearrayXIn.context() != m_oImageContext) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x value and laser line: '" << rGeoDoublearrayXIn.context() << "', '" << m_oImageContext << "'\n";
		wmLog(eWarning, oMsg.str());
	}

	// LineFit machen
	double oStartY = 0.0;
	double oEndY = 0.0;
	double oXIn = rInDataX[0];

	// Korrektur der eingehenden X-Koordinate entsprechend der korrekten Transformation in Bezug auf die Transformation der Linie:
	SmpTrafo oSmpTrafo(rGeoDoublearrayXIn.context().trafo());
	int oDiffX = oSmpTrafo->dx() - m_oSpTrafo->dx();      // ist null, falls Linie und X-Koordinate aus demselben ROI kommen
	oXIn = oXIn + oDiffX;

	double oEndX = oXIn + m_oFitintervalSize;
	bool oFitSuccessful = doLineFit(rInRankLine, rInDataLine, oXIn, oEndX, oStartY, oEndY);

	// Werte fuer die Visualisierung abspeichern:
	m_oPos1X = static_cast<int>(0.5 + oXIn);
	m_oPos2X = static_cast<int>(0.5 + oEndX);
	m_oPos1Y = static_cast<int>(rInDataLine[m_oPos1X]);
	m_oPos2Y = static_cast<int>(rInDataLine[m_oPos2X]);

	//Kalibrationsdaten holen
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

	const TPoint<double>   oPixPos1(oXIn, oStartY);
	const TPoint<double>   oPixPos2(oEndX, oEndY);

	//Achtung jeder Punkt im SW  ROI braucht die Korrektur Sw ROI und HW ROI
	const TPoint<double>	oGlobalPos			( m_oSpTrafo->dx(), m_oSpTrafo->dy() );	    //	Offset ROI Koordinaten -> Bildkoordinaten
	const TPoint<double>	oSensorPos1			( oPixPos1 + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten
	const TPoint<double>	oSensorPos2			( oPixPos2 + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten

	//Jetzt 3D Positionen
	const math::Vec3D oSensorCoord1 = rCalib.to3D(static_cast<int>(oSensorPos1.x), static_cast<int>(oSensorPos1.y), m_oTypeOfLaserLine);
	const math::Vec3D oSensorCoord2 = rCalib.to3D(static_cast<int>(oSensorPos2.x), static_cast<int>(oSensorPos2.y), m_oTypeOfLaserLine);

	math::Vec3D oDiffVector = oSensorCoord2 - oSensorCoord1;   // Weltkoordinate links minus Weltkoordinate rechts
	double oAlpha = atan(oDiffVector[2] / oDiffVector[0]);   // Z geteilt durch X

	// Berechne die Z-Komponente des um die X-Achse gedrehten Vektors:
	double oAngle = 180.0 * oAlpha / M_PI;

	if(m_oVerbosity >= eMedium)
	{
		wmLog(eDebug, "ComputeAngle: oAngle=%.2f \n", oAngle);
	}

	ValueRankType oResultingRank = filter::eRankMin;
	double oResultingGlobalRank = interface::NotPresent;
	ResultType oResult = AnalysisOK;
	if (oFitSuccessful)
	{
		oResultingRank = filter::eRankMax;
		oResultingGlobalRank = interface::Perfect;
	}

	// Wir geben nur einen Wert in den Ausgang:
	Doublearray oReadyForPipeAngle(1, oAngle, oResultingRank);

	// Create a new point, put the global context into the resulting profile and copy the rank over
	const auto oAnalysisResult = rGeoVecDoublearrayLineIn.analysisResult() == AnalysisOK ? oResult : rGeoVecDoublearrayLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const GeoDoublearray oGeoDoublearrayAngleOut(m_oImageContext, oReadyForPipeAngle, oAnalysisResult, oResultingGlobalRank); // full rank here, detailed rank in array

	// Resultat eintragen:
	preSignalAction(); pipeOutAngle_->signal(oGeoDoublearrayAngleOut);
}

bool doLineFit(const std::vector<int>& p_rInRankLine, const std::vector<double>& p_rInDataLine, double p_x0, double p_x1, double& p_ry0, double& p_ry1)
{
	double oXMean = 0.0f;
	double oYMean = 0.0f;
	int oInputCount = 0;  // wir muessen mitzaehlen, weil invalide Daten nicht betrachtet werden...

	int oX0 = static_cast<int>(p_x0 + 0.5);
	int oX1 = static_cast<int>(p_x1 + 0.5);
	for (int oIndex = oX0; oIndex <= oX1; ++oIndex)  // hier kleiner gleich, p_x1 soll mit betrachtet werden.
	{
		if (p_rInRankLine[oIndex] > eRankMin)
		{
			oXMean = oXMean + oIndex;  // der Index ist die X-Koordinate...
			oYMean = oYMean + p_rInDataLine[oIndex];

			oInputCount++;  // zaehle mit
		}
	}

	if (oInputCount < 3)
	{
		// Hierbei handelt es sich entweder um ein kurzes Stueck im allgemeinen, oder um ein mehr oder weniger kurzes, aber vor allem senkrechtes Stueck
		// ohne (validen) Daten oder um ein langes waagerechtes Stueck, welches hauptsaechlich aus invaliden Daten besteht. So ein Stueck
		// koennen wir verwerfen.
		return false;
	}

	oXMean = oXMean / static_cast<double>(oInputCount);
	oYMean = oYMean / static_cast<double>(oInputCount);

	double oSXX = 0.0f;
	double oSYY = 0.0f;
	double oSXY = 0.0f;
	for (int oIndex = oX0; oIndex <= oX1; ++oIndex)  // hier kleiner gleich, p_x1 soll mit betrachtet werden.
	{
		if (p_rInRankLine[oIndex] > eRankMin)
		{
			double oXTemp = oIndex - oXMean;
			double oYTemp = p_rInDataLine[oIndex] - oYMean;

			oSXX = oSXX + oXTemp * oXTemp;
			oSYY = oSYY + oYTemp * oYTemp;
			oSXY = oSXY + oXTemp * oYTemp;
		}
	}

	// Eine exakt senkrechte Gerade zeichnet sich aus durch oSXX = 0.
	// Allerdings sollte eine exakt senkrechte Gerade hier gar nicht vorkommen.
	// Die Eingangs-X-Koordinaten sind die Koordinaten zweier verschiedener Extrema
	// der Kruemmung. D.h. es liegen mindestens zwei Punkte vor, mit verschiedenen X-Koordinaten.
	// Tatsaechlich ist es sogar realistisch, dass wir eine exakt waagerechte Gerade vorliegen haben.
	// In dem Fall ist oSYY = 0.
	// Teste auf degenerierte Eingangsdaten und teste dabei sowohl X als auch Y:
	bool oDegenerate = (oSXX < 0.1f) && (oSYY < 0.1f);     // UND-verknuepft, nicht ODER-verknuepft ...
	if (oDegenerate)
	{
		// Diese Daten ergeben ganz einfach kein Liniensegment
		return false;
	}

	double oQ = 0.5f * (oSYY - oSXX);
	double oW = sqrt(oQ * oQ + oSXY * oSXY);
	double oQMinusW = oQ - oW;
	double oDenominator = sqrt(oQMinusW * oQMinusW + oSXY * oSXY);

	double oA = 0;
	double oB = 0;

	if (oDenominator == 0.0)
	{
		// Fuer den Fall, dass die Eingabepunkte aus 2 Punkten bestehen, welche exakt auf einer waagerechten oder senkrechten
		// Linie liegen, kann eine Loesung zwar bestimmt werden, aber dieser Loesungsweg geht dann nicht, weil der Denominator
		// zu null wird, und dann wird durch null geteilt und das ist bekanntlich nicht gut.

		// Falls Denominator gleich null und oSXX > 0, dann liegt eine waagerechte Gerade vor.
		// Der Normalenvektor zur Gerade ist also aufwaerts gerichtet.
		// Das Vorzeichen des Normalenvektors wird in der Folge noch korrigiert:
		if (oSXX > 0.1)   // Wert kann nicht negativ sein
		{
			oA = 0.0;
			oB = 1.0;
		}

		if (oSYY > 0.1)   // Wert kann nicht negativ sein
		{
			oA = 1.0;
			oB = 0.0;
		}
	}
	else
	{
		oA = oSXY / oDenominator;
		oB = oQMinusW / oDenominator;
	}

	// fA ist die X-Komponente des Normalenvektors, fB ist die Y-Komponente des Normalenvektors.
	// Es wird festgelegt, dass der Abstand der Gerade zum Ursprung in Richtung des Normalenvektors
	// stets positiv sein soll. D.h. der Normalenvektor ist entsprechend zu orientieren.
	double oDistanceTest = oXMean * oA + oYMean * oB;
	if (oDistanceTest < 0.0f)
	{
		oA = -oA;
		oB = -oB;
	}

	// Das Ergebnis dieser Methode sind die Y-Koordinaten vom Start und Ende des Liniensegments. D.h.
	// letztendlich ist es egal, wie wir den Normalenvektor der Geraden orientiert haben, wir muessen
	// nur die Y-Koordinaten korrekt erzeugen.
	// oB ist die Y-Komponente des Normalenvektors d.h. es ist die X-Komponente des Richtungsvektors
	// der Gerade. Die Steigung der Geraden berechnet sich dann aus A/B. Ausserdem
	// ist die Steigung negativ fuer den Fall, dass beide Komponenten des Normalenvektors positiv sind.
	double oSlope = -oA / oB;

	// Damit koennen wir jetzt die gewuenschten Y-Koordinaten bestimmen:
	double oXDelta0 = p_x0 - oXMean;    // negativ;    oXMean liegt moeglicherweise nicht in der Mitte des Intervalls !!
	double oXDelta1 = p_x1 - oXMean;    // positiv

	p_ry0 = oYMean + oSlope * oXDelta0;
	p_ry1 = oYMean + oSlope * oXDelta1;

	return true;
}

}  // namespace filter
}  // namespace precitec

