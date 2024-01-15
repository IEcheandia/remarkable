/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		Simon Hilsenbeck (HS)
 * 	@date		2014
 * 	@brief		Fliplib filter 'LineFeature' in component 'Filter_LineGeometry'.
 * 	@detail		Compares the shape of the input line with a user-defined line template.
 *				The form of the template consists of two connected straight line segments. Each segment is defined by a length and an angle.
 *				The result is the mean square error between the input line and the template at each point of the line.
 */

#define _USE_MATH_DEFINES
#include "lineRandkerbe3Inputs.h"

#include "module/moduleLogger.h"
#include "overlay/overlayPrimitive.h"	///< paint
#include "filter/algoArray.h"			///< Weighted mean
#include <filter/armStates.h>

#include "computeAngle.h"     // fuer TLS-Fit

#include <cmath>
#include <fliplib/TypeToDataTypeImpl.h>


using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace geo2d;
	using namespace image;
namespace filter {

const std::string LineRandkerbe3Inputs::m_oFilterName 	= "LineRandkerbe3Inputs";
const std::string LineRandkerbe3Inputs::m_oPipeOutName	= "Randkerbe";
const std::string LineRandkerbe3Inputs::m_oParameterLengthName = "TemplateLength";
const std::string LineRandkerbe3Inputs::m_oParameterLengthRightName = "TemplateLengthRight";
const std::string LineRandkerbe3Inputs::m_oParameterJumpName = "Jump";
const std::string LineRandkerbe3Inputs::m_oParameterThresholdName = "CorrelationThreshold";
const std::string LineRandkerbe3Inputs::m_oParameterAngleName = "Angle";
const std::string LineRandkerbe3Inputs::m_oParameterLineLengthName = "LineLength";
const std::string LineRandkerbe3Inputs::m_oParameterLineDeviationName = "Deviation";


LineRandkerbe3Inputs::LineRandkerbe3Inputs() :
TransformFilter(LineRandkerbe3Inputs::m_oFilterName, Poco::UUID{"0D72BD12-9410-4261-9AC1-94B5F93343C7"}),
	m_pPipeInLine		( nullptr ),
	m_pPipeInXLeft(nullptr),
	m_pPipeInXRight(nullptr),
	m_oPipeOutKerbe(this, m_oPipeOutName),
	m_oTemplateLength(10),
	m_oTemplateLengthRight(10),
	m_oTemplateLengthLeft(10),
	m_oJump(10),
	m_oThreshold(0.7),
	m_oLeftX(0),
	m_oRightX(1),
	m_oAngle(0.0),
	m_oLineLength(100),
	m_oLineDeviation(2.0)
{
	// Defaultwerte der Parameter setzen
	parameters_.add(m_oParameterLengthName, Parameter::TYPE_Int32, m_oTemplateLength);
	parameters_.add(m_oParameterLengthRightName, Parameter::TYPE_Int32, m_oTemplateLengthRight);
	parameters_.add(m_oParameterJumpName, Parameter::TYPE_Int32, m_oJump);
	parameters_.add(m_oParameterThresholdName, Parameter::TYPE_double, m_oThreshold);
	parameters_.add(m_oParameterAngleName, Parameter::TYPE_double, m_oAngle);
	parameters_.add(m_oParameterLineLengthName, Parameter::TYPE_Int32, m_oLineLength);
	parameters_.add(m_oParameterLineDeviationName, Parameter::TYPE_double, m_oLineDeviation);

    setInPipeConnectors({{Poco::UUID("AAD54B3A-671B-4093-8D35-BBF474943AB4"), m_pPipeInLine, "Line", 1, "Line"},
    {Poco::UUID("F157B9FF-9AB9-42CF-B549-2E430B4A9E69"), m_pPipeInXLeft, "xleft", 1, "xleft"},
    {Poco::UUID("7F9751E1-4BDC-4736-B09F-57AEC31A42BF"), m_pPipeInXRight, "xright", 1, "xright"}});
    setOutPipeConnectors({{Poco::UUID("EB5557F6-467B-4B20-99C6-F7306A8C1BC2"), &m_oPipeOutKerbe, m_oPipeOutName, 0, ""}});
    setVariantID(Poco::UUID("A1761CFF-0D88-437E-AA88-33166E3388D2"));
}

void LineRandkerbe3Inputs::setParameter() {
	TransformFilter::setParameter();
	m_oTemplateLength = parameters_.getParameter(m_oParameterLengthName);
	m_oTemplateLengthRight = parameters_.getParameter(m_oParameterLengthRightName);
	m_oJump = parameters_.getParameter(m_oParameterJumpName);
	m_oThreshold = parameters_.getParameter(m_oParameterThresholdName);
	double oAngleInDegrees = parameters_.getParameter(m_oParameterAngleName);
	m_oLineLength = parameters_.getParameter(m_oParameterLineLengthName);
	m_oLineDeviation = parameters_.getParameter(m_oParameterLineDeviationName);

	//m_oHalfLength = m_oTemplateLength / 2;  // TemplateLaenge soll eine gerade Zahl sein. Das wird hier umgesetzt. Im GUI kann vom Benutzer eine ungerade Zahl eingegeben werden. Das wird dann ignoriert und da wird hier eine gerade Zahl draus gemacht.
	//m_oTemplateLength = 2 * m_oHalfLength;    // Das GUI verlangt, dass das Template insgesamt mindestens 4 lang ist, d.h. die halbe Laenge ist mindestens 2

	// Zuerst gab es nur TemplateLength, jetzt rechts und links. Die alte Laenge wurde fuer links genommen, fuer rechts gab es eine neue Variable.
	// Jetzt muessen die Variablen entsprechend abgeaendert werden, damit alles stimm
	m_oTemplateLengthLeft = m_oTemplateLength; // alte wird zu links
	m_oTemplateLength = m_oTemplateLengthLeft + m_oTemplateLengthRight; // jetzt ist Lenght die Summe aus beiden Seiten

	if (m_oJump == 0)
	{
		// Wert von 0 macht keinen Sinn
		m_oJump = 1;
	}

	m_oAngle = M_PI / 180.0 * oAngleInDegrees;
	double oTanAlpha = tan(m_oAngle);
	m_oFeature.clear();   // leeren ... sollte sowieso leer sein.
	for (int oIndex = 0; oIndex < m_oTemplateLength; ++oIndex)
	{
		float oValue = static_cast<float>(oIndex * oTanAlpha);   // der genaue Wert ist eigentlich egal - Korrelationskoeffizient ist invariant gegenueber Skalierung und Offset
		if (oIndex >= m_oTemplateLengthLeft)  // z.B. wenn HalfLength = 3 dann ist FeatureLength = 6 also gehoeren Indices 0, 1, 2 zum ersten Segment
		{
			oValue = oValue + m_oJump;
		}

		m_oFeature.push_back(oValue);
	}

	// Im Folgenden berechnen wir die Zwischenergebnisse fuer die Berechnung des Kreuzkorrelationskoeffizienten, soweit
	// wir diese Zwischenergebnisse schon berechnen koennen.
	m_oMeanTemplate = 0.0f;  // initialisieren...
	for (int oIndex = 0; oIndex < (int)m_oFeature.size(); ++oIndex) {
		m_oMeanTemplate = m_oMeanTemplate + static_cast<float>(m_oFeature[oIndex]);
	} // for

	m_oMeanTemplate = m_oMeanTemplate / static_cast<float>(m_oFeature.size());

	m_oSumTemplateTemplate = 0.0f;
	for (int oIndex = 0; oIndex < (int)m_oFeature.size(); ++oIndex) {
		float oTempTemplate = static_cast<float>(m_oFeature[oIndex]) - m_oMeanTemplate;
		m_oSumTemplateTemplate = m_oSumTemplateTemplate + oTempTemplate * oTempTemplate;
	}
}

bool LineRandkerbe3Inputs::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.type() == typeid(GeoVecDoublearray))
	{
		m_pPipeInLine = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);
	}

	if (p_rPipe.type() == typeid(GeoDoublearray))
	{
		if (p_rPipe.tag() == "xleft")
		{
			m_pPipeInXLeft = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray >*>(&p_rPipe);
		}
		else if (p_rPipe.tag() == "xright")
		{
			m_pPipeInXRight = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray >*>(&p_rPipe);
		}
	}
	return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void LineRandkerbe3Inputs::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) {
	poco_assert_dbg(m_pPipeInLine != nullptr); // to be asserted by graph editor

	// get input data
	const GeoVecDoublearray &rGeoLinesIn = m_pPipeInLine->read(m_oCounter);
	const VecDoublearray& rLinesIn = rGeoLinesIn.ref();
	m_oSpTrafo = rGeoLinesIn.context().trafo();

	const GeoDoublearray &rXLeftIn = m_pPipeInXLeft->read(m_oCounter);
	const GeoDoublearray &rXRightIn = m_pPipeInXRight->read(m_oCounter);

	SmpTrafo oSmpTrafoLeft = rXLeftIn.context().trafo();
	SmpTrafo oSmpTrafoRight = rXRightIn.context().trafo();
	double oDiffLeft = static_cast<double>(oSmpTrafoLeft->dx() - m_oSpTrafo->dx());  // m_oSpTrafo ist die Trafo der Laserlinie
	double oDiffRight = static_cast<double>(oSmpTrafoRight->dx() - m_oSpTrafo->dx());

	bool validInput = rGeoLinesIn.ref().size()> 0 && !(inputIsInvalid(rGeoLinesIn))
		&& rXLeftIn.ref().size() >0 && rXRightIn.ref().size() > 0;
	if (validInput)
	{
		m_oLeftX = static_cast<int>(rXLeftIn.ref().getData()[0] + oDiffLeft);
		m_oRightX = static_cast<int>(rXRightIn.ref().getData()[0] + oDiffRight);
	}

	int oSizeLaserline = validInput?  rLinesIn[0].getData().size() : 0;

	if (m_oLeftX > m_oRightX)
	{
		// Eingangsdaten koennen je nach Anwendung durchaus vertauscht sein, also korrigiere das:
		int oTemp = m_oLeftX;
		m_oLeftX = m_oRightX;
		m_oRightX = oTemp;
	}

	// input validity check
	if ( !validInput || m_oLeftX < 0 || m_oRightX >= oSizeLaserline) {  // Ueberpruefe, ob die Eingangsdaten sinnvolle Werte haben. Wenn das nicht der Fall ist, koennen wir nicht arbeiten.
		m_oSpTrafo = nullptr;
		const GeoDoublearray geoIntarrayOut(rGeoLinesIn.context(), Doublearray(1, 0.0, eRankMin), rGeoLinesIn.analysisResult(), 0.0); // bad rank
		preSignalAction();
		m_oPipeOutKerbe.signal(geoIntarrayOut); // invoke linked filter(s)

		return; // RETURN
	} // if

	// Erst nach der Ueberpruefung frisieren wir die Eingangsdaten jetzt etwas. Sinn dieser Aktion ist folgende Tatsachenlage:
	// - Randkerben sind in der Naehe des Oberblechs
	// - die eindeutige Nahtbegrenzung ist typischerweise die auf dem Unterblech
	// Daraus folgt, dass die Nahtbegrenzung auf dem Unterblech relativ sicher erkannt wird, und daraus wird
	// dann die Position der Nahtbegrenzung auf dem Oberblech abgeleitet. Die Lokalisierung der
	// Nahtbegrenzung auf dem Oberblech ist relativ unsicher, weil das die weniger deutliche Nahtbegrenzung ist.
	// Das fuehrt dazu, dass eine moeglicherweise vorhandene Randkerbe ausserhalb des ueberprueften Bereiches ist!
	// Um das zu verhindern, verbreitern wir jetzt den zu ueberpruefenden Bereich:
	int oMargin = 40;
	m_oLeftX = m_oLeftX - oMargin;
	m_oRightX = m_oRightX + oMargin;

	//if (m_oLeftX < m_oHalfLength)
	if (m_oLeftX < m_oTemplateLengthLeft)
	{
		// So frueh koennen wir mit der Verarbeitung nicht starten:
		//m_oLeftX = m_oHalfLength;
		m_oLeftX = m_oTemplateLengthLeft;
	}

	//if (m_oRightX >(int)rLinesIn[0].getData().size() - m_oHalfLength)
	if (m_oRightX > oSizeLaserline - m_oTemplateLengthRight)
	{
		// So weit rechts koennen wir nicht verarbeiten:
		m_oRightX = oSizeLaserline - m_oTemplateLengthRight;
	}

	// process all lines in linevector
	double oResult = processFirstLine(rLinesIn);

	ValueRankType oResultingRank = filter::eRankMax;
	const auto	oNewGlobalRank			= rGeoLinesIn.rank();
	const auto	oAnalysisResult		= rGeoLinesIn.analysisResult() == AnalysisOK ? AnalysisOK : rGeoLinesIn.analysisResult(); // replace 2nd AnalysisOK by your result type
	const auto	oGeoOut = GeoDoublearray(rGeoLinesIn.context(), Doublearray(1, oResult, oResultingRank), oAnalysisResult, oNewGlobalRank);

	preSignalAction();  m_oPipeOutKerbe.signal(oGeoOut); // invoke linked filter(s)
}

double LineRandkerbe3Inputs::processFirstLine(const geo2d::VecDoublearray &p_rLineIn)
{
	const Doublearray	&rLineIn = p_rLineIn[0];
	return detektiereKerbe(rLineIn);
}

double LineRandkerbe3Inputs::detektiereKerbe(const geo2d::TArray<double> &p_rArrayIn)
{
	poco_assert_dbg(p_rArrayIn.size() != 0);

	const std::vector<double> &rInDataLine = p_rArrayIn.getData();
	const std::vector<int> &rInRankLine = p_rArrayIn.getRank();

	std::vector<float> oCorrelationCoefficients;   // intern koennen wir ruhig mit floats rechnen...
	oCorrelationCoefficients.assign(rInDataLine.size(), -2.0);  // initialisieren zu negativen Werten...

	for (int oCol = m_oLeftX; oCol <= m_oRightX; ++oCol) { // loop over line
		oCorrelationCoefficients[oCol] = berechneKorrelationsKoeffizient(rInDataLine, rInRankLine, oCol);
	} // for

	// Im Zusammenhang der Maximumsuche werden wir den Rank nicht mehr betrachten. Wir haben den Rank bereits bei der Berechnung
	// des Korrelationskoeffizienten berechnet.
	m_oMaxima.clear();
	findeMaxima(rInDataLine, oCorrelationCoefficients, m_oMaxima);

	m_oFinalMaxima.clear();
	fordereOberblech(rInDataLine, rInRankLine, m_oMaxima);

	if (m_oFinalMaxima.size() > 0)
	{
		return 1.0;  // Fehlerwert
	}

	// OK-Fall
	double returnVal = 0.1 * m_oMaxima.size();
	return (returnVal >= 1) ? 0.9 : returnVal;  // Gibt bis max 0.9 zurueck. Dann sieht man an der Ergebniskurve, wie viele Kandidaten infrage kamen.
}

void LineRandkerbe3Inputs::fordereOberblech(const std::vector<double>& p_rInDataLine, const std::vector<int>& p_rInRankLine, const std::vector<int>& p_rMaxima)
{
	// Wir haben das Problem, dass die Kriterien, die wir bis jetzt verwenden, nicht fuer eine saubere Trennung ausreichen
	// zwischen den Klassen "Randkerbe" und "Nicht-Randkerbe". Die Stufenform (Kriterium Korrelationskoeffizient) und Sprunghoehe (Kriterium Sprunghoehe)
	// reichen nicht aus - einzelne Abschnitte in der Naht koennen durchaus die Form einer Stufe haben, und so gross ist der Sprung,
	// der an einer Randkerbe vorliegt, moeglicherweise gar nicht - daher reichen die bisher angewendeten Merkmale nicht vollstaendig
	// aus, um Randkerben eindeutig zu identifizieren. Diese Methode soll das loesen, indem explizit die unmittelbare Nachbarschaft
	// zum Oberblech gefordert wird. D.h. wir untersuchen die Nachbarschaft eines Randkerbenkandidaten darauf, ob sich hier das Oberblech
	// anschliesst. Wenn das der Fall ist, dann ist es eine Randkerbe, ansonsten nicht.
	//
	// Das Oberblech kann sich schliesslich nur an einen Kandidaten unmittelbar anschliessen - nicht an mehrere.
	// Wenn wir innerhalb einer Naht gleich zwei Kandidaten haben, dann ist klar, dass mindestens einer davon in der
	// Naht ist (und nicht am Rand, und dann ist es definitionsgemaess keine Randkerbe). Angenommen, wir haetten 3 Kandidaten,
	// und wir wuessten nicht, wo das Oberblech ist (links oder rechts). Selbst dann koennte man noch argumentieren, dass wir
	// den Kandidaten in der Mitte sowieso schon mal aussortieren koennen - unter der Annahme, dass eventuell der erste und
	// der letzte Kandidat am jeweiligen Rand sitzen, kann der mittlere Kandidat jedenfalls nicht am Rand sitzen.
	// D.h. aufgrund dieser Argumentation koennte man den mittleren Kandidaten sofort verwerfen.
	// ==> Dieser Argumentation wollen wir nicht folgen. Moeglicherweise sind die ersten und letzten Detektionen auf dem
	//     Blech oder auf der Spannvorrichtung. D.h. es soll bewusst jeder Kandidat untersucht werden.
	//
	// Weil diese Ueberpruefung arbeitsintensiv ist, wird sie erst am Ende der Verarbeitung durchgefuehrt, in der Hoffnung,
	// dass es dann nicht viele Kandidaten sind, die diese Ueberpruefung durchlaufen muessen. Diese Ueberpruefung hier ist
	// tatsaechlich ein starkes Merkmal und sie wuerde wohl einige FalschPositive aussortieren. Es ist Zweck der beiden
	// anderen Kriterien, dass nur wenige Kandidaten bis zu dieser Stufe der Verarbeitung kommen, so dass das hier nicht
	// zu viel Rechenzeit verschwendet.

#ifdef _DEBUG_DUCHOW
	// fuer Debug-Zwecke abspeichern:
	m_oPositionsLineStartDebug.clear();
	m_oPositionsLineEndDebug.clear();
#endif

	int oMargin = 7;  // Abstand zwischen Randkerbe und Oberblech erlauben, das ist praktisch relevant.
	for (int oCandidate = 0; oCandidate < (int)p_rMaxima.size(); oCandidate++)
	{
		double oYLinks = 0.0;
		double oYRechts = 0.0;
		int oXLinks = -1;
		int oXRechts = -1;

		// Das folgende waehlt den zu betrachtenden Bereich der Eingangsdaten:
		if (m_oJump < 0)  // Jump == 0 ist nicht zugelassen d.h. braucht nicht behandelt zu werden.
		{
			// Jump ist negativ d.h. die Stufe geht nach oben im Bild - Oberblech ist rechts
			oXLinks = p_rMaxima[oCandidate] + oMargin;  // vermuteter Start des Oberblechs nach rechts
			if (oXLinks >= (int)p_rInDataLine.size())
			{
				// Wegen der Margin kann es jetzt vorkommen, dass oXLinks groesser ist als die Anzahl der vorliegenden Daten. Das sollte
				// zwar kaum vorkommen, aber wir wollen natuerlich absolut sicher sein:
				oXLinks = p_rInDataLine.size() - 1;  // erlaubter Maximalwert
			}
			oXRechts = oXLinks + m_oLineLength;
			if (oXRechts >= (int)p_rInDataLine.size())
			{
				// So viele Daten haben wir nicht, d.h. wir muessen uns beschraenken bezueglich der zu betrachtenden Daten.
				// Im schlimmsten Fall hat uns der Benutzer die Liniendaten vom Blech nicht gegeben, und jetzt untersuchen wir einen
				// Kandidaten, der in der Naht liegt, ohne dass wir das feststellen koennen, dass er in der Naht liegt, weil
				// uns nicht genuegend Daten zur Verfuegung stehen.
				// Das ist ein Fehler des Benutzers - dann muss er damit leben, dass wir zu einem falschen Schluss kommen.
				oXRechts = (int)p_rInDataLine.size() - 1;  // rechter Index soll betrachtet werden d.h. darf nicht den Wert der Groesse des Vektors annehmen.
			}
		}
		else
		{
			// Jump ist positiv d.h. die Stufe geht nach unten im Bild - Oberblech ist links
			oXRechts = p_rMaxima[oCandidate] - oMargin;  // vermuteter Start des Oberblechs nach links
			if (oXRechts < 0)
			{
				// Wegen der Margin muessen wir das jetzt ueberpruefen.
				oXRechts = 0;
			}

			oXLinks = oXRechts - m_oLineLength;
			if (oXLinks < 0)
			{
				// So viele Daten haben wir nicht, d.h. wir muessen uns beschraenken bezueglich der zu betrachtenden Daten.
				// Im schlimmsten Fall hat uns der Benutzer die Liniendaten vom Blech nicht gegeben, und jetzt untersuchen wir einen
				// Kandidaten, der in der Naht liegt, ohne dass wir das feststellen koennen, dass er in der Naht liegt, weil
				// uns nicht genuegend Daten zur Verfuegung stehen.
				// Das ist ein Fehler des Benutzers - dann muss er damit leben, dass wir zu einem falschen Schluss kommen.
				oXLinks = 0;
			}
		} // Ende Wahl des zu betrachtenden Bereiches

		// Fuehre Verarbeitung durch:
		bool oFitSuccessful = doLineFit(p_rInRankLine, p_rInDataLine, static_cast<double>(oXLinks), static_cast<double>(oXRechts), oYLinks, oYRechts);

		if (oFitSuccessful)
		{
#ifdef _DEBUG_DUCHOW
			// fuer Debug-Zwecke abspeichern:
			m_oPositionsLineStartDebug.push_back(geo2d::Point(oXLinks, static_cast<int>(oYLinks)));
			m_oPositionsLineEndDebug.push_back(geo2d::Point(oXRechts, static_cast<int>(oYRechts)));
#endif
			// Betrachte die Fehler:
			bool oIsALine = true;  // gehe zunaechst davon aus, dass eine Linie vorliegt...
			double oTemp = static_cast<double>(oXRechts - oXLinks);
			double oM = (oYRechts - oYLinks) / oTemp;
			for (int oCol = oXLinks; oCol <= oXRechts; oCol++)  // Test auf <= weil oXRechts betrachtet werden soll
			{
				if (p_rInRankLine[oCol] > eRankMin)
				{
					// Sinnvoller Wert
					double oExpectedValue = oYLinks + (oCol - oXLinks) * oM;
					double oError = oExpectedValue - p_rInDataLine[oCol];

					if (fabs(oError) > m_oLineDeviation)
					{
						oIsALine = false;  // doch keine Linie
						break;  // brich die Schleife ab
					}
				}
			} // Ende Ueberpruefung Fehlerwerte

			if (oIsALine)
			{
				// Ist tatsaechlich eine Linie, d.h. der Kandidat muss uebernommen werden als endgueltig als Randkerbe erkannter Defekt:
				m_oFinalMaxima.push_back(m_oMaxima[oCandidate]);
			}
		}  // Falls Fit Successful war
	}  // Ende Schleife ueber alle Kandidaten
}  // Ende Methode

float LineRandkerbe3Inputs::berechneKorrelationsKoeffizient(const std::vector<double>& p_rInput, const std::vector<int>& p_rInRankLine, int p_oCol)
{
	// Im Rahmen dieser Methode besteht weiteres Optimierungspotential in dem Sinne, dass
	// die Zwischenergebnisse hier fuer aufeinanderfolgende Aufrufe der Methode aehnlich sind - d.h.
	// wenn man die Struktur des Codes aendern wuerde, dann koennte man die Berechnung der Zwischenergebnisse optimieren.
	float oMeanSignal = 0.0f;
	for (int oIndex = 0; oIndex < (int)m_oFeature.size(); ++oIndex) {
		//int oIndexInSignal = p_oCol - m_oHalfLength + oIndex;  // oIndexInSignal geht von      p_oCol - m_oHalfLength    bis     p_oCol + m_oHalfLength - 1
		int oIndexInSignal = p_oCol - m_oTemplateLengthLeft + oIndex;  // oIndexInSignal geht von      p_oCol - m_oHalfLength    bis     p_oCol + m_oHalfLength - 1
		oMeanSignal = oMeanSignal + static_cast<float>(p_rInput[oIndexInSignal]);

		if (p_rInRankLine[oIndexInSignal] == eRankMin)
		{
			// wenn ein Datum fehlerhaft ist, dann gib zurueck:
			return -2.0f;  // Hierdurch ist klar, dass dieses Datum auf diese Weise entstanden ist.
		}
	} // for

	oMeanSignal = oMeanSignal / static_cast<float>(m_oFeature.size());

	float oSumSignalSignal = 0.0f;
	float oSumSignalTemplate = 0.0;

	for (int oIndex = 0; oIndex < (int)m_oFeature.size(); ++oIndex) {
		float oTempTemplate = static_cast<float>(m_oFeature[oIndex]) - m_oMeanTemplate;

		//int oIndexInSignal = p_oCol - m_oHalfLength + oIndex;  // oIndexInSignal geht von      p_oCol - m_oHalfLength    bis     p_oCol + m_oHalfLength - 1
		int oIndexInSignal = p_oCol - m_oTemplateLengthLeft + oIndex;  // oIndexInSignal geht von      p_oCol - m_oHalfLength    bis     p_oCol + m_oHalfLength - 1
		float oTempSignal = static_cast<float>(p_rInput[oIndexInSignal]) - oMeanSignal;
		oSumSignalSignal = oSumSignalSignal + oTempSignal * oTempSignal;

		oSumSignalTemplate = oSumSignalTemplate + oTempTemplate * oTempSignal;
	} // for

	float oDenominator = sqrt((double)oSumSignalSignal * m_oSumTemplateTemplate);

	float oResult;
	if (oDenominator == 0.0f)
	{
		oResult = 0.0f;
	}
	else
	{
		oResult = oSumSignalTemplate / oDenominator;
	}

	return oResult;
}

void LineRandkerbe3Inputs::findeMaxima(const std::vector<double>& rInput, const std::vector<float>& p_rCorrCoefficient, std::vector<int>& p_rMaxima)
{
	//int oQuarterLength = static_cast<int>(0.5 * m_oHalfLength + 0.5);
	int oQuarterLength = static_cast<int>(0.25 * m_oTemplateLength + 0.5);
	float oHeightDifferenceNoJump = static_cast<float>(oQuarterLength * 2 * tan(m_oAngle)); // ausserhalb der Schleife, um Rechenzeit zu sparen.
	double oDiffCurrent = 0.0;
	double oDiffLast = p_rCorrCoefficient[m_oLeftX] - p_rCorrCoefficient[m_oLeftX - 1];  // m_oStartX ist mindestens 2 d.h. hier ist erlaubt
	assert(m_oLeftX > 1);
	for (int oIndex = m_oLeftX; oIndex < m_oRightX; oIndex++)
	{
		oDiffCurrent = p_rCorrCoefficient[oIndex] - p_rCorrCoefficient[oIndex - 1];

		double oProduct = oDiffCurrent * oDiffLast;
		if (oProduct < 0.0)
		{
			// Ableitung des Eingangssignals hat einen Nulldurchgang, d.h. hier ist ein Extremum:
			// Verlange zusaetzlich, dass das Eingangssignal selber einen kleinen Betrag hat.
			// Ausserdem interessieren wir uns tatsaechlich nur fuer Minima. Teste das also ab:
			bool oIsMaximum = oDiffCurrent < oDiffLast;  // Wir wissen, dass es ein Extremum ist, d.h. die beiden haben verschiedene Vorzeichen. Falls oDiffCurrent negativ, dann ist es ein Maximum.
			bool oThresholdHit = p_rCorrCoefficient[oIndex - 1] > m_oThreshold;

			float oValueLeft = static_cast<float>(rInput[oIndex - 1 - oQuarterLength]);  // auch das hier erfordert tatsaechlich, dass m_oStartX groesser 1 sein muss...
			float oValueRight = static_cast<float>(rInput[oIndex - 1 + oQuarterLength]);  // m_oEndX hat genuegend Abstand vom Endwert, so dass das hier erlaubt ist.
			bool oJumpHighEnough;
			if (m_oJump > 0)
			{
				oJumpHighEnough = ((oValueRight - oValueLeft - oHeightDifferenceNoJump) >= m_oJump);   // bei Sprung von oben nach unten ist links oben und das ist der kleinere Wert !!
			}
			else
			{
				oJumpHighEnough = ((oValueRight - oValueLeft - oHeightDifferenceNoJump) <= m_oJump);  // bei negativem Sprung ist die linke Laserlinie unter der rechten d.h. y-Werte sind links hoeher. Die Differenz hat einen negativen Wert und soll noch negativer sein als der Sprung.
			}
			if (oIsMaximum && oThresholdHit && oJumpHighEnough)
			{
				p_rMaxima.push_back(oIndex - 1);
			}
		}
		else if ((oDiffCurrent == 0.0) && (p_rCorrCoefficient[oIndex - 1] > m_oThreshold))   // mache diese Extra-Aktion, wenn oDiffCurrent gleich null. oDiffLast ist dann noch ungleich null.
		{
			// Das ist gleich in einem der ersten Tests passiert: Die Kruemmungsdaten, die reingekommen sind, waren in
			// zwei aufeinanderfolgenden Eintragungen identisch, und diese identischen Eintraege stellten obendrein
			// auch noch ein echtes Extremum dar. Diesen Fall muessen wir behandeln koennen.
			// Also: falls das Produkt null ist, und falls die Daten so gross sind, dass wir sie als Extremum akzeptieren wuerden,
			// dann veranstalte diesen Extra-Zirkus. Den Test auf den Betrag der Daten machen wir extra jetzt schon,
			// weil wir diesen Zirkus nur dann machen wollen, wenn es auch wirklich notwendig ist.
			double oDiffCurrentTemp = 0.0; // Wert der Initialisierung ist egal.
			int oMaxStep = 5;  // nur ein paar Pixel, wir wollen das nicht ewig machen
			int oStep = 0;   // Wert der Initialisierung ist egal
			for (oStep = 1; oStep < oMaxStep; ++oStep)  // Schleife laeuft ab 1
			{
				unsigned int oColTemp = oIndex + oStep;
				if (oColTemp >= p_rCorrCoefficient.size()) // Falls wir jetzt den gueltigen Bereich verlassen, dann mache nichts. Wichtig ist der Test auf groesser und auf Gleichheit.
				{
					// Brich ab; damit bleibt sinnvolle Information in oStep erhalten.
					break;
				}

				oDiffCurrentTemp = p_rCorrCoefficient[oColTemp] - p_rCorrCoefficient[oColTemp - 1];

				double oProductTemp = oDiffCurrentTemp * oDiffLast;  // oDiffLast wird hier bewusst nicht aktualisiert.
				if (oProductTemp < 0.0)
				{
					bool oIsMaximum = oDiffCurrentTemp < oDiffLast;

					float oValueLeft = static_cast<float>(rInput[oIndex - 1 - oQuarterLength]);  // auch das hier erfordert tatsaechlich, dass m_oStartX groesser 1 sein muss...
					float oValueRight = static_cast<float>(rInput[oIndex - 1 + oQuarterLength]);  // m_oEndX hat genuegend Abstand vom Endwert, so dass das hier erlaubt ist.

					bool oJumpHighEnough;
					if (m_oJump > 0)
					{
						oJumpHighEnough = ((oValueRight - oValueLeft - oHeightDifferenceNoJump) >= m_oJump);   // bei Sprung von oben nach unten ist links oben und das ist der kleinere Wert !!
					}
					else
					{
						oJumpHighEnough = ((oValueRight - oValueLeft - oHeightDifferenceNoJump) <= m_oJump);  // bei negativem Sprung ist die linke Laserlinie unter der rechten d.h. y-Werte sind links hoeher. Die Differenz hat einen negativen Wert und soll noch negativer sein als der Sprung.
					}

					if (oIsMaximum && oJumpHighEnough)
					{
						p_rMaxima.push_back(oIndex - 1);
					}
					break;  // Brich auch hier ab: Wir haben ein Extremum gefunden und koennen die Extra-Aktion beenden.
				}
			}  // Ende Schleife

			oIndex = oIndex + oStep;  // inkrementiere oCol entsprechend der Aktion
			oDiffCurrent = oDiffCurrentTemp;  // Damit die Information ueber das aktuelle Vorzeichen in oDiffLast transportiert wird.
		}   // Ende Extra-Aktion. Nach Ende der Extra-Aktion ist der Code wieder in einem definierten Zustand und die Verarbeitung sollte problemlos weiterlaufen.

		oDiffLast = oDiffCurrent;
	}
}

void LineRandkerbe3Inputs::paint() {
	if(m_oVerbosity < eLow){
		return;
	} // if

	if (m_oSpTrafo.isNull())
	{
		return;
	}

	const auto&		rTrafo = *m_oSpTrafo;
	auto&			rCanvas			= canvas<OverlayCanvas>(m_oCounter);
	auto&			rLayerContour	= rCanvas.getLayerContour();
	//auto&			rLayerLine		= rCanvas.getLayerLine();
	auto&			rLayerText		= rCanvas.getLayerText();

	for (unsigned int oIndex = 0; oIndex < m_oFinalMaxima.size(); ++oIndex) {
		int oCol = m_oFinalMaxima[oIndex];
		rLayerContour.add(new OverlayCross(rTrafo(Point(oCol, 0)), Color::m_oScarletRed));  // zuerst das grosse Kreuz zeichnen - Standardgroesse ist 10
	} // for

	if (m_oVerbosity < eMedium){  return; }

	// ab hier Verbosity mind. eMedium

	// die folgende Ausgabe erfolgt fuer m_oVerbosity == eMedium oder hoeher; fuer den Fall eMedium wird nur die detektierte Randkerbendetektion und der eingegebene Bereich ausgegeben:
	rLayerContour.add(new OverlayCross(rTrafo(Point(m_oLeftX, 0)), 6, Color::m_oButter));  // zuerst das grosse Kreuz zeichnen - Standardgroesse ist 10
	rLayerContour.add(new OverlayCross(rTrafo(Point(m_oRightX, 0)), 6, Color::m_oButter));  // zuerst das grosse Kreuz zeichnen - Standardgroesse ist 10

	if (m_oVerbosity < eHigh){  return;	}

	// ab hier Verbosity mind. eHigh

	for (unsigned int oIndex = 0; oIndex < m_oMaxima.size(); ++oIndex) {
		int oCol = m_oMaxima[oIndex];
		rLayerContour.add(new OverlayCross(rTrafo(Point(oCol, 0)), 6, Color::Green()));
	} // for

#ifdef _DEBUG_DUCHOW
	assert(m_oPositionsLineStartDebug.size() == m_oPositionsLineEndDebug.size());
	for (unsigned int oIndex = 0; oIndex < m_oPositionsLineStartDebug.size(); ++oIndex)
	{
		Color oColors[3];
		oColors[0] = Color::m_oScarletRed;
		oColors[1] = Color::m_oButter;
		oColors[2] = Color::m_oOrange;
		Color& rColor = oColors[oIndex % 3];
		rLayerContour.add(new OverlayLine(rTrafo(m_oPositionsLineStartDebug[oIndex]), rTrafo(m_oPositionsLineEndDebug[oIndex]), rColor));
	}
#endif

	// Das folgende visualisiert das Feature.
	const auto	oY					= 100;
	const auto	oTextFont			= Font(16);
	const auto	oTextPosition		= rTrafo(Point(10, oY)); // paint info close to left roi border
	const auto	oTextSize			= Size(100, 20);
	const auto	oTextBox			= Rect(oTextPosition, oTextSize);
	const auto	oFeatureStart = Point(oTextPosition.x + oTextSize.width, oTextPosition.y + oTextSize.height );

	rLayerText.add(new OverlayText("Feature:", oTextFont, oTextBox, Color::m_oScarletRed));

	for (int oIndex = 0; oIndex < (int)m_oFeature.size(); ++oIndex)
	{
		const int oY = static_cast<int>(m_oFeature[oIndex] + 0.5);
		rLayerContour.add(new OverlayPoint(oFeatureStart + Point(oIndex, oY), Color::m_oScarletRed)); // rTrafo ist oben schon angewendet worden, nicht doppelt anwenden !!
	}

} // paint

/*virtual*/ void
LineRandkerbe3Inputs::arm(const fliplib::ArmStateBase& state) {
	if (state.getStateID() == eSeamStart)
	{
		m_oSpTrafo = nullptr;
	}
} // arm

} // namespace filter
} // namespace precitec
