/*!
 *  @copyright:		Precitec GmbH & Co. KG
 *  @author			DUW
 *  @date			2015
 *  @file
 *  @brief			Fliplib filter 'LineEstimator' in component 'Filter_ImageProcessing'. Estimates a line given a set of points.
 */

// Diese Header sind auch in hough.cpp verwendet worden. Sehr interessant:
// Diese Zeilen muessen hier als erstes kommen! Das liegt daran, dass <cmath> wohl bereits an anderer Stelle
// inkludiert wird, aber ohne _USE_MATH_DEFINES. Aber weil <cmath> natuerlich Inkludeguards hat,
// hat eine zweite Einbindung weiter unten keinen Effekt - also muss das hier als erstes erfolgen!
#define _USE_MATH_DEFINES						/// pi constant
#include <cmath>								/// trigonometry, pi constant

#include "lineEstimator.h"

#include "module/moduleLogger.h"
#include "filter/algoArray.h"
#include "math/2D/LineEquation.h"
#include "overlay/overlayPrimitive.h"	///< Overlay

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {

const std::string LineEstimator::m_oFilterName = std::string("LineEstimator");
const std::string LineEstimator::PIPENAMEX = std::string("MeanX");
const std::string LineEstimator::PIPENAMEY = std::string("MeanY");
const std::string LineEstimator::PIPENAMEBETA = std::string("Beta");
const std::string LineEstimator::PIPENAMELINE = std::string("LineEquation");


LineEstimator::LineEstimator() :
	TransformFilter			( LineEstimator::m_oFilterName, Poco::UUID{"3068C8A9-41D6-46DA-B317-90180EB8B9A1"} ),
	m_pPipeInDoubleX0(nullptr),
	m_pPipeInDoubleY0(nullptr),
	m_pPipeInDoubleX1(nullptr),
	m_pPipeInDoubleY1(nullptr),
	m_pPipeInDoubleX2(nullptr),
	m_pPipeInDoubleY2(nullptr),
	m_oPipeOutDoubleX(this, LineEstimator::PIPENAMEX),
	m_oPipeOutDoubleY(this, LineEstimator::PIPENAMEY),
	m_oPipeOutDoubleBeta(this, LineEstimator::PIPENAMEBETA),
	m_oPipeOutLineEquation(this, LineEstimator::PIPENAMELINE),
	m_oXMean(0.0),
	m_oYMean(0.0),
	m_oBeta(0.0)
{
	// Defaultwerte der Parameter setzen
    setInPipeConnectors({{Poco::UUID("836A81A5-307A-4181-84D5-6CF6D4EFF10B"), m_pPipeInDoubleX0, "ScalarX0", 1, "position_x_0"},
    {Poco::UUID("9E7D9E25-8CD4-4910-A8F7-ED8A270DD4A8"), m_pPipeInDoubleY0, "ScalarY0", 1, "position_y_0"},
    {Poco::UUID("D373C052-864C-4EAC-9147-A82395805119"), m_pPipeInDoubleX1, "ScalarX1", 1, "position_x_1"},
    {Poco::UUID("9F3D506D-23E8-489F-923A-81AF342B22AB"), m_pPipeInDoubleY1, "ScalarY1", 1, "position_y_1"},
    {Poco::UUID("4E6AFECF-1376-43BE-9875-FF0A2F1AAB8F"), m_pPipeInDoubleX2, "ScalarX2", 1, "position_x_2"},
    {Poco::UUID("4151027F-9EBA-41C3-9619-3AE4CDCEEBC7"), m_pPipeInDoubleY2, "ScalarY2", 1, "position_y_2"}});
    setOutPipeConnectors({{Poco::UUID("9B549E34-0938-4A2B-BD9C-D7D724A736BF"), &m_oPipeOutDoubleX, "MeanX", 0, "MeanX"},
    {Poco::UUID("D52DD2A6-B324-4D45-B393-2BD287DB2C18"), &m_oPipeOutDoubleY, "MeanY", 0, "MeanY"},
    {Poco::UUID("2DC34EF7-E649-41F0-AB83-B03E6E064090"), &m_oPipeOutDoubleBeta, "Beta", 0, "Beta"},
    {Poco::UUID("9E515A1E-1477-4F04-A00F-DD9480857181"), &m_oPipeOutLineEquation, "LineEquation", 0, ""}});
    setVariantID(Poco::UUID("2F5E81E9-1F32-4883-8969-54E993BD86D8"));
}



void LineEstimator::setParameter() {
	//set verbosity
	TransformFilter::setParameter();
} // setParameter

void LineEstimator::paint() {
	if (m_oVerbosity < eLow){
		return;
	} // if

	OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer	&rLayerLine(rCanvas.getLayerLine());

	// Die Y-Achse geht nach unten im Bild, fuer Beta = positiv geht die Gerade von oben links im Bild nach unten rechts.
	// Urspruenglich hat die Visualisierung der gefundenen Gerade ein mehr oder weniger zufaellig gewaehltes Stueck der Geraden
	// gezeichnet. Joachim hat vorgeschlagen, dass das visualisierte Stueck der Geraden mit dem Stueck zwischen den beiden
	// eingegebenen Punkten zusammen fallen sollte. Ich will tatsaechlich die berechnete Gerade und nicht einfach die Verbindungsstrecke
	// zwischen den eingegebenen Punkten visualisieren - die Visualisierung soll schliesslich auch als Werkzeug zur Analyse
	// dienen, und das funktioniert nicht, wenn wir einfach nur die Eingabedaten zeichnen. Die Visualisierung muss
	// schon das Ergebnis der Berechnung zeigen.
	// Zu diesem Zweck werden die Eingabedaten auf die Gerade projeziert, und so ermitteln wir das anzuzeigende Geradenstueck.
	//
	// Der folgende Code nimmt nun also doch an, dass wir 3 Eingabepunkte erhalten - diese Annahme ist jedoch gerechtfertigt,
	// weil dass durch das Filter strikt vorgegeben ist.
	std::vector<double> oLambdas;
	int oCount = m_oInputX.size();

	// Tatsaechlich ist es so, dass paint() wohl schon waehrend der Initialisierung des Filters aufgerufen wird,
	// wenn die Pipes wohl noch nicht mit dem Filter verbunden sind. Fuer diesen Fall fordern wir, dass
	// die Groesse nicht 0 sein darf.
	if (oCount == 0)
	{
		return;
	}

	// Wenn wir bis hierhin kommen, ist die Initialisierung wohl abgeschlossen. Es sollten alle Eingaenge vorhanden sein.
	// Wenn das nicht der Fall ist, ist etwas anderes komisch:
	assert(oCount == 3);
	for (int oIndex = 0; oIndex < oCount; ++oIndex)
	{
		double oLambda = (m_oInputX[oIndex] - m_oXMean) * sin(m_oBeta) + (m_oInputY[oIndex] - m_oYMean) * cos(m_oBeta);
		oLambdas.push_back(oLambda);
	}

	// sortieren:
	std::sort(oLambdas.begin(), oLambdas.end());
	int oXStart = static_cast<int>(m_oXMean + oLambdas[0] * sin(m_oBeta));
	int oYStart = static_cast<int>(m_oYMean + oLambdas[0] * cos(m_oBeta));
	int oXEnd = static_cast<int>(m_oXMean + oLambdas[2] * sin(m_oBeta));
	int oYEnd = static_cast<int>(m_oYMean + oLambdas[2] * cos(m_oBeta));

	rLayerLine.add(new OverlayLine(oXStart, oYStart, oXEnd, oYEnd, Color::Green()));

	int oXMean = static_cast<int> (m_oXMean);
	int oYMean = static_cast<int> (m_oYMean);
	rLayerLine.add(new OverlayCircle(oXMean, oYMean, 4, Color::Red()));

	for (int oIndex = 0; oIndex < oCount; oIndex++)
	{
		oXStart = static_cast<int> (m_oInputX[oIndex]);
		oYStart = static_cast<int> (m_oInputY[oIndex]);

		rLayerLine.add(new OverlayCircle(oXStart, oYStart, 4, Color::Yellow()));
	}
} // paint


/*virtual*/ void
LineEstimator::arm (const fliplib::ArmStateBase& state) {
	//std::cout << "\nFilter '" << m_oFilterName << "' armed at armstate " << state.getStateID() << std::endl;
} // arm


using fliplib::SynchronePipe;

bool LineEstimator::subscribe(fliplib::BasePipe& rPipe, int p_oGroup) {
	if (rPipe.type() == typeid(GeoDoublearray)) {
		if (rPipe.tag() == "position_x_0")
			m_pPipeInDoubleX0 = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&rPipe);
		else if (rPipe.tag() == "position_y_0")
			m_pPipeInDoubleY0 = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&rPipe);
		else if (rPipe.tag() == "position_x_1")
			m_pPipeInDoubleX1 = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&rPipe);
		else if (rPipe.tag() == "position_y_1")
			m_pPipeInDoubleY1 = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&rPipe);
		else if (rPipe.tag() == "position_x_2")
			m_pPipeInDoubleX2 = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&rPipe);
		else if (rPipe.tag() == "position_y_2")
			m_pPipeInDoubleY2 = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray > *>(&rPipe);
	}

	return BaseFilter::subscribe( rPipe, p_oGroup );
} // subscribe


/// die eigentliche Filterfunktion
bool LineEstimator::estimateLine(double p_X0, double p_Y0, double p_X1, double p_Y1, double p_X2, double p_Y2) {

	// Schaetzung der Geraden aus den Eingabedaten. Die Schaetzung minimiert den quadrierten absoluten Fehler (englisch total least squares),
	// was besser ist als eine Minimierung der Fehler in y alleine. Siehe den Artikel "Ausgleichsgeraden in der Ebene", Technisches
	// Messen 71 (2004).
	//
	// Der Code, der die Implementierung minimiert, soll allgemein gehalten sein d.h. soll nicht auf den Fall mit 3 Eingangspunkten
	// beschraenkt sein. Um das zu erreichen, werden die Eingangspunkte zunaechst in std::vector gegeben, und diese werden dann in die
	// Verarbeitung gefuettert. Zusaetzlich werden die Daten im Vektor auch gleich fuer die Visualisierung verwendet.
	m_oInputX.clear();
	m_oInputY.clear();  // wichtig: Inhalte loeschen...

	m_oInputX.push_back( p_X0 );
	m_oInputX.push_back( p_X1 );
	m_oInputX.push_back( p_X2 );

	m_oInputY.push_back( p_Y0 );
	m_oInputY.push_back( p_Y1 );
	m_oInputY.push_back( p_Y2 );

	// der weitere Code ist allgemein gueltig:
	int oInputCount = m_oInputX.size();
	m_oXMean = 0.0f;
	m_oYMean = 0.0f;
	for (int oIndex = 0; oIndex < oInputCount; ++oIndex)
	{
		m_oXMean = m_oXMean + m_oInputX[oIndex];
		m_oYMean = m_oYMean + m_oInputY[oIndex];
	}
	m_oXMean = m_oXMean / static_cast<double>(oInputCount);
	m_oYMean = m_oYMean / static_cast<double>(oInputCount);

	double oSXX = 0.0f;
	double oSYY = 0.0f;
	double oSXY = 0.0f;
	for (int oIndex = 0; oIndex < oInputCount; ++oIndex)
	{
		double oXTemp = m_oInputX[oIndex] - m_oXMean;
		double oYTemp = m_oInputY[oIndex] - m_oYMean;

		oSXX = oSXX + oXTemp * oXTemp;
		oSYY = oSYY + oYTemp * oYTemp;
		oSXY = oSXY + oXTemp * oYTemp;
	}

	// Die Eingabedaten sind laut Joachim Pixel. D.h. eigentlich sollte sich wenigstens einer
	// der 3 Eingabepunkte um einen Wert von mindestens 1 von den beiden anderen unterscheiden (wir
	// haben abgemacht, dass 2 der 3 Punkte gleich sein duerfen).
	// Uebrigens: Damit der Test auf degenerierte Daten sinnvoll ist, muessen wir SXX und SYY beide untersuchen!
	// Schliesslich koennen die Eingabepunkte ja sehr wohl auf einer exakt senkrechten Geraden liegen - dann
	// ist SXX gleich null, obwohl der Fall nicht degeneriert ist!!
	bool oDegenerate = (oSXX < 0.1f) && (oSYY < 0.1f);     // UND-verknuepft, nicht ODER-verknuepft ...
	if ( oDegenerate )
	{
		// Degenerierter Fall: 3 Identische Eingabepunkte

		// Plausibilitaets-Enum setzen laut Diskussion mit Joachim setzen.

		// Ausgabe
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Degenerate input to line estimator.\n";
		wmLog(eError, oMsg.str());

		// globalen Rank auf null setzen
		return false;
	}

	double oQ = 0.5f * (oSYY - oSXX);
	double oW = sqrt(oQ * oQ + oSXY * oSXY);
	double oQMinusW = oQ - oW;
	double oDenominator = sqrt(oQMinusW * oQMinusW + oSXY * oSXY);

	double oA = oSXY / oDenominator;
	double oB = oQMinusW / oDenominator;

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

	// Diskussion mit Joachim am 1. Juni 2015:
	// Ausgabewert soll der Winkel zur Vertikalen sein. Dabei wird festgelegt, dass das Vorzeichen entsprechend
	// der mathematischen Konvention gewaehlt wird.
	//
	//             ^ Y
	//           + |
	//         \<--|
	//          \  |
	//           \ |
	//            \|
	//
	// fA ist die X-Komponente des Normalenvektors, fB ist die Y-Komponente des Normalenvektors.
	// Es wird festgelegt, dass der Abstand der Gerade zum Ursprung in Richtung des Normalenvektors
	// stets positiv sein soll. D.h. der Normalenvektor ist entsprechend zu orientieren.
	//
	// Allerdings muss beruecksichtigt werden, dass die Y-Achse nach unten geht und nicht nach oben wie in obiger
	// Skizze. Wenn man das nicht beruecksichtigt, entsteht ein Fehler !!
	double oDistanceTest = m_oXMean * oA + m_oYMean * oB;
	if (oDistanceTest < 0.0f)
	{
		oA = -oA;
		oB = -oB;

		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Line representation was changed.\n";
		wmLog(eInfo, oMsg.str());
	}

	// Der Winkel der Gerade zur Vertikalen ist gleich dem Winkel des Normalenvektors zur Horizontalen.
	// Hier ist der Winkel noch im Bogenmass.
	m_oBeta = -atan(oB / oA);  // atan2() ist eigentlich nicht noetig, die Funktionalitaet von atan() sollte genuegen.

	return true;
}



using fliplib::PipeEventArgs;

void LineEstimator::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) {
	poco_assert_dbg(m_pPipeInDoubleX0 != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleY0 != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleX1 != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleY1 != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleX2 != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInDoubleY2 != nullptr); // to be asserted by graph editor

	// hole const referenz aus pipe
	const GeoDoublearray	&rGeoInDoubleX0(m_pPipeInDoubleX0->read(m_oCounter));
	const GeoDoublearray	&rGeoInDoubleY0(m_pPipeInDoubleY0->read(m_oCounter));
	const GeoDoublearray	&rGeoInDoubleX1(m_pPipeInDoubleX1->read(m_oCounter));
	const GeoDoublearray	&rGeoInDoubleY1(m_pPipeInDoubleY1->read(m_oCounter));
	const GeoDoublearray	&rGeoInDoubleX2(m_pPipeInDoubleX2->read(m_oCounter));
	const GeoDoublearray	&rGeoInDoubleY2(m_pPipeInDoubleY2->read(m_oCounter));

	// Uebernommen aus Mismatch1.cpp:
	if ( inputIsInvalid(rGeoInDoubleX0) || inputIsInvalid(rGeoInDoubleY0) || inputIsInvalid(rGeoInDoubleX1) || inputIsInvalid(rGeoInDoubleY1) || inputIsInvalid(rGeoInDoubleX2) || inputIsInvalid(rGeoInDoubleX2)) {
		const double			oRank(interface::NotPresent);
		const GeoDoublearray	oGeoMismatchOut(rGeoInDoubleX0.context(), Doublearray(1, 0.0, eRankMin), rGeoInDoubleX0.analysisResult(), oRank);
		GeoLineModelarray oLineOut(rGeoInDoubleX0.context(),
			LineModelarray(1, geo2d::LineModel(0, 0, 0, 0, 0), eRankMin), rGeoInDoubleX0.analysisResult(), oRank);

		preSignalAction();
		m_oPipeOutDoubleX.signal(oGeoMismatchOut);
		m_oPipeOutDoubleY.signal(oGeoMismatchOut);
		m_oPipeOutDoubleBeta.signal(oGeoMismatchOut);
		m_oPipeOutLineEquation.signal(oLineOut);

		return; // RETURN
	}


	const Doublearray		&rInDoubleX0(rGeoInDoubleX0.ref());
	const Doublearray		&rInDoubleY0(rGeoInDoubleY0.ref());
	const Doublearray		&rInDoubleX1(rGeoInDoubleX1.ref());
	const Doublearray		&rInDoubleY1(rGeoInDoubleY1.ref());
	const Doublearray		&rInDoubleX2(rGeoInDoubleX2.ref());
	const Doublearray		&rInDoubleY2(rGeoInDoubleY2.ref());

	if (rInDoubleX0.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDoubleX0.size());
	}
	if (rInDoubleY0.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDoubleY0.size());
	}
	if (rInDoubleX1.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDoubleX1.size());
	}
	if (rInDoubleY1.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDoubleY1.size());
	}
	if (rInDoubleX2.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDoubleX2.size());
	}
	if (rInDoubleY2.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rInDoubleY2.size());
	}

	//internally, bring the points to the same reference system (adjust for different contexts)

	double oX0 = rInDoubleX0.getData().front() + rGeoInDoubleX0.context().trafo()->dx();
	double oY0 = rInDoubleY0.getData().front() + rGeoInDoubleY0.context().trafo()->dy();

	double oX1 = rInDoubleX1.getData().front() + rGeoInDoubleX1.context().trafo()->dx();
	double oY1 = rInDoubleY1.getData().front() + rGeoInDoubleY1.context().trafo()->dy();

	double oX2 = rInDoubleX2.getData().front() + rGeoInDoubleX2.context().trafo()->dx();
	double oY2 = rInDoubleY2.getData().front() + rGeoInDoubleY2.context().trafo()->dy();

	// die eigentiche Filterfunktion
	bool oSuccess = estimateLine( oX0, oY0, oX1, oY1, oX2, oY2);

	ValueRankType oResultingRank = filter::eRankMax;
	double oResultingGlobalRank = interface::Perfect;
	if (!oSuccess)
	{
		oResultingRank = filter::eRankMin;
		oResultingGlobalRank = interface::NotPresent;
	}

	// Frame-/Contextverwaltung
	const auto & oOutputContext = rGeoInDoubleX0.context();
	double oXMeanWithContext = m_oXMean - oOutputContext.trafo()->dx();
	double oYMeanWithContext = m_oYMean - oOutputContext.trafo()->dy();

	math::LineEquation oLineEquation(oXMeanWithContext, oYMeanWithContext, m_oBeta,
		math::angleUnit::eRadians, false);
	double oCoeffA, oCoeffB, oCoeffC;
	oLineEquation.getCoefficients(oCoeffA, oCoeffB, oCoeffC);

	geo2d::Doublearray outputX(1, oXMeanWithContext, oResultingRank);
	geo2d::Doublearray outputY(1, oYMeanWithContext, oResultingRank);
	geo2d::Doublearray outputBeta(1, m_oBeta * 180.0 / M_PI, oResultingRank);  // Audi hat sich gewuenscht, dass die Ausgabe des Filters Grad sein soll. Das rechnen wir erst hier um, weil wir es selber fuer die Visualisierung brauchen - dann muss die nicht geaendert werden.
	geo2d::LineModelarray outputLine(1, geo2d::LineModel(oXMeanWithContext, oYMeanWithContext, oCoeffA, oCoeffB, oCoeffC), oResultingRank);

	preSignalAction();
	m_oPipeOutDoubleX.signal(interface::GeoDoublearray(oOutputContext, outputX, rGeoInDoubleX0.analysisResult(), oResultingGlobalRank));
	m_oPipeOutDoubleY.signal(interface::GeoDoublearray(oOutputContext, outputY, rGeoInDoubleX0.analysisResult(), oResultingGlobalRank));
	m_oPipeOutDoubleBeta.signal(interface::GeoDoublearray(oOutputContext, outputBeta, rGeoInDoubleX0.analysisResult(), oResultingGlobalRank));
	m_oPipeOutLineEquation.signal(interface::GeoLineModelarray(oOutputContext, outputLine, rGeoInDoubleX0.analysisResult(), oResultingGlobalRank));

} // proceed



} // namespace filter
} // namespace precitec

