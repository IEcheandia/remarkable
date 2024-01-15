
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

#include "filter/algoArray.h"	///< algorithmic interface for class TArray
#include "filter/algoImage.h"   ///< for applying trafo to Vec3D for 2d to 3d conversion
#include <filter/structures.h>
#include <filter/armStates.h>

#include<fliplib/TypeToDataTypeImpl.h>

// local includes
#include "seamGeometryTwb.h"
#include "util/calibDataSingleton.h"
#include "2D/avgAndRegression.h"

using namespace fliplib;

namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;

	namespace filter {

		const std::string SeamGeometryTwb::m_oFilterName = std::string("SeamGeometryTwb");
		const std::string SeamGeometryTwb::PIPENAME_WIDTH = std::string("Width");
		//const std::string SeamGeometryTwb::PIPENAME_HEIGHT = std::string("Height");
		const std::string SeamGeometryTwb::PIPENAME_ROUNDNESS_POS = std::string("RoundnessPos");
		const std::string SeamGeometryTwb::PIPENAME_ROUNDNESS_NEG = std::string("RoundnessNeg");
		const std::string SeamGeometryTwb::PIPENAME_AREA_POS = std::string("PositiveArea");
		const std::string SeamGeometryTwb::PIPENAME_AREA_NEG = std::string("NegativeArea");

		const std::string SeamGeometryTwb::m_oParamTypeOfLaserLine("TypeOfLaserLine"); ///< Parameter: Type of LaserLine (e.g. FrontLaserLine, BehindLaserLine)

		const double m_epsilon = 0.000001;
		const double m_bigSlope = 1000000.0;


		SeamGeometryTwb::SeamGeometryTwb() : TransformFilter(SeamGeometryTwb::m_oFilterName, Poco::UUID{"CC62BB0B-0EF0-4660-8C23-5028BB9E2737"}),
			m_pPipeInLaserline(nullptr),
			m_pPipeInXLeft(nullptr),
			m_pPipeInXRight(nullptr),
			m_pPipeInAngle(nullptr),
			m_oPipeOutWidth(this, SeamGeometryTwb::PIPENAME_WIDTH),
			//m_oPipeOutHeight(this, SeamGeometryTwb::PIPENAME_HEIGHT),
			m_oPipeOutRoundnessPos(this, SeamGeometryTwb::PIPENAME_ROUNDNESS_POS),
			m_oPipeOutRoundnessNeg(this, SeamGeometryTwb::PIPENAME_ROUNDNESS_NEG),
			m_oPipeOutAreaPos(this, SeamGeometryTwb::PIPENAME_AREA_POS),
			m_oPipeOutAreaNeg(this, SeamGeometryTwb::PIPENAME_AREA_NEG),
			m_oXLeft(-1),
			m_oXRight(-1),
			m_oHeight(0.0),
			m_oWidth(0.0),
			m_oAreaNeg(0.0),
			m_oAreaPos(0.0),
			m_oPaint(true),
			m_oTypeOfLaserLine(LaserLine::FrontLaserLine),
			m_oSwitchRoundness(false),
			m_oXPosMax(0),
			m_oYPosMax(0),
			m_oXPosMaxLine(0),
			m_oYPosMaxLine(0)
		{
			int oLaserLineTemp = static_cast<int>(m_oTypeOfLaserLine);
			parameters_.add(m_oParamTypeOfLaserLine, fliplib::Parameter::TYPE_int, oLaserLineTemp);  // Fuege den Parameter mit dem soeben initialisierten Wert hinzu.
			parameters_.add("SwitchRoundness", Parameter::TYPE_bool, m_oSwitchRoundness);

            setInPipeConnectors({{Poco::UUID("C5757232-CF4D-4103-B773-6EBEFAC6B0D6"), m_pPipeInLaserline, "Line", 1, "line"},
            {Poco::UUID("D514DFA7-911C-4990-B16B-0E0D52084C29"), m_pPipeInXLeft, "MarkerLeftX", 1, "xleft"},{Poco::UUID("4744D6CD-CDDC-48CB-93DF-6622446A8F05"), m_pPipeInXRight, "MarkerRightX", 1, "xright"},{Poco::UUID("79FDD5E3-66D2-4CBE-9ECB-4F40D1CEED8B"), m_pPipeInAngle, "Angle", 1, "angle"}});
            setOutPipeConnectors({{Poco::UUID("69434CA7-6896-4EDE-ACC0-E5F9B6128BF5"), &m_oPipeOutWidth, PIPENAME_WIDTH, 0, ""},
            {Poco::UUID("DD75950B-23AE-4245-881C-5F8FF0D19F20"), &m_oPipeOutRoundnessPos, PIPENAME_ROUNDNESS_POS, 0, ""},
            {Poco::UUID("C2EBAACB-B477-43D2-90B9-1F2DC447C807"), &m_oPipeOutRoundnessNeg, PIPENAME_ROUNDNESS_NEG, 0, ""},
            {Poco::UUID("BF78CDB9-F6CA-49BC-BF5F-862CFE14A7AA"), &m_oPipeOutAreaPos, PIPENAME_AREA_POS, 0, ""},
            {Poco::UUID("C0FE20F6-7ED9-4BBC-B871-9F04C505F820"), &m_oPipeOutAreaNeg, PIPENAME_AREA_NEG, 0, ""}});
            setVariantID(Poco::UUID("37ED23CF-2761-4119-85D5-D11CB3902EE0"));
		}

		void SeamGeometryTwb::setParameter()
		{
			TransformFilter::setParameter();

			int oTempLine = parameters_.getParameter(SeamGeometryTwb::m_oParamTypeOfLaserLine).convert<int>();
			m_oTypeOfLaserLine = static_cast<LaserLine>(oTempLine);

			m_oSwitchRoundness = static_cast<bool>(parameters_.getParameter("SwitchRoundness").convert<bool>());
		}

		void SeamGeometryTwb::paint() {
			if (!m_oPaint || m_oVerbosity < eLow || m_oSpTrafo.isNull() || (m_oYcoords.size() <= 0))
			{
				return;
			}

			const Trafo		&rTrafo(*m_oSpTrafo);
			OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
			OverlayLayer	&rLayerLine(rCanvas.getLayerLine());

			for (unsigned int i = 0; i < m_oYcoords.size() - 1; ++i)
			{
				if ((m_oYcoords[i][0] >= 0) && (m_oYcoords[i + 1][0] >= 0))
				{
					geo2d::Point oLineStart((int)m_oYcoords[i][0], (int)m_oYcoords[i][1]);
					geo2d::Point oLineEnd((int)m_oYcoords[i + 1][0], (int)m_oYcoords[i + 1][1]);
					rLayerLine.add<OverlayLine>(rTrafo(oLineStart), rTrafo(oLineEnd), Color::Yellow());
				}
			}

			// Roundness einzeichnen

			//rLayerLine.add(new OverlayCross(rTrafo(geo2d::Point(m_oXPosMax, m_oYPosMax)), 6, Color::Orange()));
			//rLayerLine.add(new OverlayCross(rTrafo(geo2d::Point(m_oXPosMaxLine, m_oYPosMaxLine)), 6, Color::Orange()));

			//geo2d::Point oFrom, oTo;
			//oFrom.x = (int)m_oXPosMax;
			//oFrom.y = (int)m_oYPosMax;
			//oTo.x = (int)m_oXPosMaxLine;
			//oTo.y = (int)m_oYPosMaxLine;

			//rLayerLine.add(new OverlayLine(rTrafo(oFrom), rTrafo(oTo), Color::Orange()));

			// pos. Roundness
			if (m_oXPosRoundnessMax != 0)
			{
				rLayerLine.add<OverlayCross>(rTrafo(geo2d::Point(m_oXPosRoundnessMax, m_oYPosRoundnessMax)), 6, Color::Green());
				rLayerLine.add<OverlayCross>(rTrafo(geo2d::Point(m_oXPosRoundnessMaxLine, m_oYPosRoundnessMaxLine)), 6, Color::Green());

				geo2d::Point oFrom, oTo;
				oFrom.x = (int)m_oXPosRoundnessMax;
				oFrom.y = (int)m_oYPosRoundnessMax;
				oTo.x = (int)m_oXPosRoundnessMaxLine;
				oTo.y = (int)m_oYPosRoundnessMaxLine;

				rLayerLine.add<OverlayLine>(rTrafo(oFrom), rTrafo(oTo), Color::Green());
			}

			// neg. Roundness
			if (m_oXNegRoundnessMax)
			{
				rLayerLine.add<OverlayCross>(rTrafo(geo2d::Point(m_oXNegRoundnessMax, m_oYNegRoundnessMax)), 6, Color::Red());
				rLayerLine.add<OverlayCross>(rTrafo(geo2d::Point(m_oXNegRoundnessMaxLine, m_oYNegRoundnessMaxLine)), 6, Color::Red());

				geo2d::Point oFrom, oTo;
				oFrom.x = (int)m_oXNegRoundnessMax;
				oFrom.y = (int)m_oYNegRoundnessMax;
				oTo.x = (int)m_oXNegRoundnessMaxLine;
				oTo.y = (int)m_oYNegRoundnessMaxLine;

				rLayerLine.add<OverlayLine>(rTrafo(oFrom), rTrafo(oTo), Color::Red());
			}

			rLayerLine.add<OverlayCross>(rTrafo(geo2d::Point(m_oXLeft, m_oYLeft)), 6, Color::Green());
			rLayerLine.add<OverlayCross>(rTrafo(geo2d::Point(m_oXRight, m_oYRight)), 6, Color::Green());
			rLayerLine.add<OverlayLine>(rTrafo(geo2d::Point(m_oXLeft, m_oYLeft)), rTrafo(geo2d::Point(m_oXRight, m_oYRight)), Color::Orange());
		} // paint

		/*virtual*/ void
			SeamGeometryTwb::arm(const fliplib::ArmStateBase& state) {
				if (state.getStateID() == eSeamStart)
				{
					m_oSpTrafo = nullptr;
				}
		} // arm

		bool SeamGeometryTwb::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			if (p_rPipe.type() == typeid(GeoVecDoublearray))
			{
				m_pPipeInLaserline = dynamic_cast< fliplib::SynchronePipe < GeoVecDoublearray >* >(&p_rPipe);
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
				else if ((p_rPipe.tag() == "angle"))
				{
					m_pPipeInAngle = dynamic_cast<fliplib::SynchronePipe < GeoDoublearray >*>(&p_rPipe);
				}
			}
			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		bool SeamGeometryTwb::get3DValues(const geo2d::Doublearray &p_rLine, const Vec3D &p_oStart, const Vec3D &p_oEnd)
		{
			// OS: Hier wird die eigentliche Berechnung gemacht. Es ist die Laserlinie bekannt sowie Punkt links Naht und rechts Naht (x/y/1).
			// Bemerkung: Die eingehenden "3D"-Punkte liegen tatsaechlich im Bild. Die liegen nicht in 3D.
			if (std::abs(p_oEnd[0] - p_oStart[0]) < math::eps)
			{
				// todo: handle case
				return false;
			}

			// in pixel: steigung und laenge
			double oSlopeScreen = (p_oEnd[1] - p_oStart[1]) / (p_oEnd[0] - p_oStart[0]); // Steigung der Verbindungsgeraden zwischen den Nahtpunkten
			int oLenScreen = int(p_oEnd[0] - p_oStart[0] + 1); // sowas wie die Nahtbreite in Pixeln

			m_oYcoords.resize(oLenScreen);

			int oIdx = -1;

			auto &oData = p_rLine.getData();
			auto &oRank = p_rLine.getRank();
			double oYValScreen(0.0);

			double diff = 0.0;
			double maxdiff = 0.0;
			double maxDiffNegRound = 0.0;
			double maxDiffPosRound = 0.0;

			double AreaPos = 0.0;
			double AreaNeg = 0.0;

			Line2D_ seamline(p_oStart[0], p_oStart[1], p_oEnd[0], p_oEnd[1]);

			m_oXPosMax = m_oYPosMax = m_oXPosMaxLine = m_oYPosMaxLine = 0;
			m_oXPosRoundnessMax = m_oYPosRoundnessMax = m_oXPosRoundnessMaxLine = m_oYPosRoundnessMaxLine = 0;
			m_oXNegRoundnessMax = m_oYNegRoundnessMax = m_oXNegRoundnessMaxLine = m_oYNegRoundnessMaxLine = 0;

			// traverse seam to find highest point
			for (int i = 0; i < oLenScreen; ++i)
			{
				const double oXVal(p_oStart[0] + i);  // current x position -- oYVal
				const int oXValInt = static_cast<int>(oXVal + 0.5);
				if (oRank[oXValInt] > eRankMin)
				{
					oYValScreen = oSlopeScreen*i + p_oStart[1];            // y Gerade
					m_oYcoords[i].set(oXVal, oYValScreen);                 // Punkte der Verbindungsgeraden

					//distanz senkrecht zur Verbindungsgeraden
					diff = seamline.calcDistance(oXValInt, oData[oXValInt]);

					if (diff < maxDiffNegRound) // neg. Roundness
					{
						maxDiffNegRound = diff;

						m_oXNegRoundnessMax = oXValInt;
						m_oYNegRoundnessMax = static_cast<int>(oData[oXValInt] + 0.5);

						m_oXNegRoundnessMaxLine = static_cast<int>(seamline.m_oInterceptX + 0.5);
						m_oYNegRoundnessMaxLine = static_cast<int>(seamline.m_oInterceptY + 0.5);
					}

					if (diff > maxDiffPosRound) // pos. Roundness
					{
						maxDiffPosRound = diff;

						m_oXPosRoundnessMax = oXValInt;
						m_oYPosRoundnessMax = static_cast<int>(oData[oXValInt] + 0.5);

						m_oXPosRoundnessMaxLine = static_cast<int>(seamline.m_oInterceptX + 0.5);
						m_oYPosRoundnessMaxLine = static_cast<int>(seamline.m_oInterceptY + 0.5);
					}

					if (std::abs(diff) > maxdiff)
					{
						oIdx = i;
						m_oXPosMax = oXValInt;
						m_oYPosMax = static_cast<int>(oData[oXValInt] + 0.5);

						m_oXPosMaxLine = static_cast<int>(seamline.m_oInterceptX + 0.5);
						m_oYPosMaxLine = static_cast<int>(seamline.m_oInterceptY + 0.5);

						maxdiff = std::abs(diff);
					}

					//Flaeche in Pixel
					if (oLenScreen > 1)
					{
						if (diff > 0) AreaPos += diff;

						if (diff < 0) AreaNeg += diff;
					}
				}
				else
				{
					m_oYcoords[i].set(-1, -1);
				}
			}

			m_oAreaPos = AreaPos;
			m_oAreaNeg = AreaNeg;

			m_oRoundnessPos = std::abs(maxDiffPosRound);
			m_oRoundnessNeg = std::abs(maxDiffNegRound);

			if (oIdx >= 0)
			{
				// Wenn ein gueltiges Ergebnis gefunden wurde, dann ist oIdx nicht negativ.
				return true;
			}
			else
			{
				// oIdx ist negativ d.h. es ist kein gueltiges Ergebnis gefunden worden.
				// Setze die relevanten Groessen nochmal zu null. Es ist zwar unklar, ob das notwendig ist,
				// denn sie werden zu null initialisiert, und gefunden haben wir schliesslich nichts,
				// aber es kann nichts schaden.
				m_oWidth = 0.0;
				//m_oHeight = 0.0;
				m_oRoundnessPos = 0.0;
				m_oRoundnessNeg = 0.0;
				m_oAreaPos = 0.0;
				m_oAreaNeg = 0.0;
				return false;
			}
		}

		bool SeamGeometryTwb::findExtremum(const geo2d::Doublearray &p_rLine)
		{
			auto &oData = p_rLine.getData();

			// Setze die relevanten Groessen zu null. Das wird hier gemacht und dann nicht nochmal in der Methode get3DValues().
			m_oWidth = 0.0;
			m_oHeight = 0.0;
			m_oAreaPos = 0.0;
			m_oAreaNeg = 0.0;

			// for paint
			m_oYcoords.resize(0);

			double oYLeft = oData[m_oXLeft];
			double oYRight = oData[m_oXRight];

			if ((oYLeft < 1) && (oYRight < 1)) // beide Murks
			{
				oYLeft = oYRight = 100;
			}
			else
			{
				if (oYLeft < 1) // nur links schlecht
				{
					oYLeft = oYRight;
				}
				if (oYRight < 1) // nur rechts schlecht
				{
					oYRight = oYLeft;
				}
			}

			Vec3D oStart(m_oXLeft, oYLeft, 1);
			Vec3D oEnd(m_oXRight, oYRight, 1);

			if (!get3DValues(p_rLine, oStart, oEnd))
				return false;

			return true;
		}


		void SeamGeometryTwb::signalSendInvalidResult(const ImageContext &p_rImgContext, ResultType p_oAnalysisResult)
		{
			// Diese Methode ist dafuer zustaendig, das Ergebnis zu versenden. In dem originalen Code zu dieser
			// Methode gab es frueher 3 Zustaende:
			// - alles okay, d.h. es konnte ein sinnvolles Ergebnis erzeugt werden
			// - Fehlerfall
			// - Fehlerfall
			//
			// Dabei war aber unklar, wieso es zwei verschiedene Fehlerfaelle gibt und was die verschiedenen Konsequenzen
			// dieser verschiedenen Fehlerfaelle sind.
			//
			// Wegen dieser Unklarheit gibt es ab jetzt nur noch zwei Zustaende: Entweder es ist alles okay, oder es liegt ein Fehlerfall vor. Dazwischen gibt es nichts.


				// Nichts ist okay - falls die Laserlinie schon ein fehlerhaftes AnalyseResultat hat, dann verwende das, ansonsten
				// setze ein fehlerhaftes AnalyseResultat. Wenn die Ausfuehrung hier ankommt, ist klar, dass ein Fehler
				// vorliegt, also ist hier nur ein fehlerhaftes AnalyseResultat sinnvoll.
				//std::cout << "****SeamGeometry::signalSend() AnalysisError -> interface::AnalysisErrNoBeadOrGap (1202)" << std::endl;

                // In Compact errors only from sum error
                //wmLog(eDebug, "****SeamGeometry::signalSend() AnalysisError -> interface::AnalysisErrNoBeadOrGap (1202)\n");
				//auto oAnalysisResult = rLine.analysisResult() != AnalysisOK ? rLine.analysisResult() : AnalysisErrNoBeadOrGap;

				preSignalAction();

				m_oPipeOutWidth.signal(GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent));
				//m_oPipeOutHeight.signal(GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), oAnalysisResult, interface::NotPresent));  // Ergebnis mit einem Datum von Wert 0 und Rank 0, globaler Rank ist NotPresent
				m_oPipeOutRoundnessPos.signal(GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent));  // Ergebnis mit einem Datum von Wert 0 und Rank 0, globaler Rank ist NotPresent
				m_oPipeOutRoundnessNeg.signal(GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent));  // Ergebnis mit einem Datum von Wert 0 und Rank 0, globaler Rank ist NotPresent
				m_oPipeOutAreaPos.signal(GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent));
				m_oPipeOutAreaNeg.signal(GeoDoublearray(p_rImgContext, Doublearray(1, 0.0, 0), p_oAnalysisResult, interface::NotPresent));
			}

		void SeamGeometryTwb::resizeOutArrays (unsigned int size)
		{
			//note: the arrays are only resized, not filled with zero
			m_oWidthArray.resize(size);
			m_oRoundnessPosArray.resize(size);
			m_oRoundnessNegArray.resize(size);
			m_oAreaPosArray.resize(size);
			m_oAreaNegArray.resize(size);
			m_oGeoOutRank = 1.0;
			assert(m_lastUpdatedArrayIndex == -1);
		}
		void SeamGeometryTwb::updateOutArrays (unsigned int lineN, bool p_oResultValid )
		{
			assert( lineN < m_oWidthArray.size() && m_oRoundnessPosArray.size() == m_oWidthArray.size() && m_oRoundnessNegArray.size() == m_oWidthArray.size()
				&& m_oAreaPosArray.size() == m_oWidthArray.size() && m_oAreaNegArray.size() == m_oWidthArray.size());

			assert((int) lineN == m_lastUpdatedArrayIndex + 1 && "proceedGroup has not processed all data sequentially");
			m_lastUpdatedArrayIndex = lineN;

			if (!p_oResultValid )
			{
				m_oWidthArray[lineN] = std::make_tuple(0.0,0);
				m_oRoundnessPosArray[lineN] = std::make_tuple(0.0,0);
				m_oRoundnessNegArray[lineN] = std::make_tuple(0.0,0);
				m_oAreaPosArray[lineN] = std::make_tuple(0.0,0);
				m_oAreaNegArray[lineN] = std::make_tuple(0.0,0);
			}
			else
			{
				clipDouble(m_oWidth, -9999, 9999);
				//clipDouble(m_oHeight, -9999, 9999);
				clipDouble(m_oRoundnessPos, -9999, 9999);
				clipDouble(m_oRoundnessNeg, -9999, 9999);
				clipDouble(m_oAreaPos, -9999, 9999);
				clipDouble(m_oAreaNeg, -9999, 9999);

				if (m_oSwitchRoundness)
				{
					double temp = m_oRoundnessPos;
					m_oRoundnessPos = m_oRoundnessNeg;
					m_oRoundnessNeg = temp;
				}
				m_oWidthArray[lineN] = std::make_tuple(m_oWidth, eRankMax);
				m_oRoundnessPosArray[lineN] = std::make_tuple(m_oRoundnessPos, eRankMax);
				m_oRoundnessNegArray[lineN] = std::make_tuple(m_oRoundnessNeg, eRankMax);
				m_oAreaPosArray[lineN] = std::make_tuple(m_oAreaPos, eRankMax);
				m_oAreaNegArray[lineN] = std::make_tuple(m_oAreaNeg, eRankMax);
			}

		}

		void SeamGeometryTwb::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInLaserline != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInXLeft != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInXRight != nullptr); // to be asserted by graph editor
			poco_assert_dbg(m_pPipeInAngle != nullptr); // to be asserted by graph editor

			// Get Coords
			const GeoVecDoublearray &rLine(m_pPipeInLaserline->read(m_oCounter));
			const GeoDoublearray &rXLeft(m_pPipeInXLeft->read(m_oCounter));
			const GeoDoublearray &rXRight(m_pPipeInXRight->read(m_oCounter));
			const GeoDoublearray &rAngle(m_pPipeInAngle->read(m_oCounter));

			geo2d::TPoint<double>	oPositionLaserInRoiRoundPos;
			geo2d::TPoint<double>	oPositionLaserInRoiRoundNeg;
			geo2d::TPoint<double>	oPositionLineInRoiRoundPos;
			geo2d::TPoint<double>	oPositionLineInRoiRoundNeg;
			geo2d::TPoint<double>	oPositionLeftPointInRoi;
			geo2d::TPoint<double>	oPositionRightPointInRoi;

			m_oSpTrafo = rLine.context().trafo();
			const ImageContext& rContext(rLine.context());

			geo2d::TPoint<double>	oRoiPos(m_oSpTrafo->dx(), m_oSpTrafo->dy());
			geo2d::TPoint<double>	oHwRoiPos(rContext.HW_ROI_x0, rContext.HW_ROI_y0);

			ResultType oAnalysisResult = rLine.analysisResult();

			if ((rXLeft.rank() == 0) || (rXRight.rank() == 0) || (rLine.rank() == 0) || (rAngle.rank() == 0)
				|| rXLeft.ref().size() == 0 || rXRight.ref().size() == 0 || rLine.ref().size() == 0 || rAngle.ref().size() == 0
			)
			{   // Rank ist mies => abbrechen ohne weitere Berechnunge
				m_oPaint = false; // suppress paint

				// Rank ist mies. Versende Resultat, Fehlerfall
				signalSendInvalidResult(rContext, oAnalysisResult);
				return;
			}
			// Rank erlaubt sinnvolle Berechnungen
			if (oAnalysisResult != AnalysisOK)
			{   // Analyse hat bereits einen Fehler => auch hier abbrechen
				m_oPaint = false;
				signalSendInvalidResult(rContext, oAnalysisResult); // Versende Resultat, Fehlerfall
				return;
			}

			assert((rXLeft.rank() != 0) && (rXRight.rank() != 0) && (rLine.rank() != 0) && (rAngle.rank() != 0) && (oAnalysisResult == AnalysisOK));
			// Rank gut, kein Analysefehler

			SmpTrafo oSmpTrafoLeft = rXLeft.context().trafo();
			SmpTrafo oSmpTrafoRight = rXRight.context().trafo();
			int oDiffLeft = oSmpTrafoLeft->dx() - m_oSpTrafo->dx();  // m_oSpTrafo entspricht der Transformation der Laserlinie
			int oDiffRight = oSmpTrafoRight->dx() - m_oSpTrafo->dx();

			auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

			const unsigned int oNbLines = rLine.ref().size();

			bool alwaysUseFirstXLeft = rXLeft.ref().size() != oNbLines;
			bool alwaysUseFirstXRight = rXRight.ref().size() != oNbLines;
			bool alwaysUseFirstAngle = rAngle.ref().size() != oNbLines;

			if (alwaysUseFirstXLeft)
			{
				wmLog(eDebug, "Filter '%s': Received %u XLeft values for %u laser lines. Only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rXLeft.ref().size(), oNbLines);
			}
			if (alwaysUseFirstXRight)
			{
				wmLog(eDebug, "Filter '%s': Received %u XRight values for %u laser lines. Only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rXRight.ref().size(), oNbLines);
			}
			if (alwaysUseFirstAngle)
			{
				wmLog(eDebug, "Filter '%s': Received %u Angle values for %u laser lines. Only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rAngle.ref().size(), oNbLines);
			}

			m_lastUpdatedArrayIndex = -1;
			resizeOutArrays (oNbLines);
			for (unsigned int lineN = 0; lineN < oNbLines; ++lineN)
			{
					auto &rData(rLine.ref()[lineN].getData());

					// Get start and end laser line points of run... hier knallt es ab und zu get geht aus dem ROI...
					m_oXLeft = (int)(std::get<eData>(rXLeft.ref()[(alwaysUseFirstXLeft ? 0 : lineN)])); // In-Pipe LeftX
					m_oXRight = (int)(std::get<eData>(rXRight.ref()[(alwaysUseFirstXRight ? 0 : lineN)])); // In-Pipe RightX

					// Es kann durchaus den Anwendungsfall geben, dass die eingehenden X-Koordinaten (links und rechts)
					// aus einem anderen ROI stammen, aus dem die Laserlinie kommt. In dem Fall muessen die X-Koordinaten korrekt
					// in Bezug gesetzt werden zur Laserlinie:
					m_oXLeft = m_oXLeft + oDiffLeft;  // fuer den Fall, dass alle Daten aus ein und demselben ROI kommen, ist das hier eine Null-Operation.
					m_oXRight = m_oXRight + oDiffRight;

					double oAngleIn = (int)(std::get<eData>(rAngle.ref()[(alwaysUseFirstAngle ? 0 : lineN)])); // In-Pipe RightX

					// if necessary, swap left and right points
					if (m_oXLeft > m_oXRight)  // falls links groesser rechts ... Gleichheit braucht hier nicht geprueft zu werden, weil vertauschen dann auch nichts bringt.
					{
						int oTmp = m_oXLeft;
						m_oXLeft = m_oXRight;
						m_oXRight = oTmp; // swap
					}
					const auto left = m_oXLeft;
					const auto right = m_oXRight;

					int oTempSize = static_cast<int>(rLine.ref()[0].size());  // in int wandeln. Wenn wir das nicht machen und die ungewandelte Groesse in der naechsten Zeile verwenden, und m_oXLeft ausserdem negativ ist, dann wandelt er m_oXLeft in ein unsigned Datum, und vergleicht die beiden Zahlen dann, und das ist Bloedsinn.
					if ((left < 0) || (right < 0) || (left == right))  // falls links kleiner null oder rechts kleiner null oder links gleich rechts:
					{
						m_oPaint = false; // suppress paint
						updateOutArrays (lineN, false);                  // Versende Resultat, Fehlerfall
					}
					else if (left >= oTempSize || right >= oTempSize)  // falls Daten nicht zum ROI passen...
					{
						m_oPaint = false; // suppress paint
						updateOutArrays (lineN, false);                  // Versende Resultat, Fehlerfall
					}
					else
					{
						m_oYLeft = int(rData[m_oXLeft]);
						m_oYRight = int(rData[m_oXRight]); // for painting

						// Start computation of data
						if (findExtremum(rLine.ref()[lineN]))
						{
							//Umrechnung 3D
							oPositionLaserInRoiRoundPos.x = m_oXPosRoundnessMax;
							oPositionLaserInRoiRoundPos.y = m_oYPosRoundnessMax;
							oPositionLineInRoiRoundPos.x = m_oXPosRoundnessMaxLine;
							oPositionLineInRoiRoundPos.y = m_oYPosRoundnessMaxLine;

							oPositionLaserInRoiRoundNeg.x = m_oXNegRoundnessMax;
							oPositionLaserInRoiRoundNeg.y = m_oYNegRoundnessMax;
							oPositionLineInRoiRoundNeg.x = m_oXNegRoundnessMaxLine;
							oPositionLineInRoiRoundNeg.y = m_oYNegRoundnessMaxLine;

							const TPoint<double>	oSensorPosLineRoundPos(oPositionLineInRoiRoundPos + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten
							const TPoint<double>	oSensorPosLaserRoundPos(oPositionLaserInRoiRoundPos + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten
							const TPoint<double>	oSensorPosLineRoundNeg(oPositionLineInRoiRoundNeg + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten
							const TPoint<double>	oSensorPosLaserRoundNeg(oPositionLaserInRoiRoundNeg + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten

							const math::Vec3D oSensorCoordLaserRoundPos = rCalib.to3D(static_cast<int>(oSensorPosLaserRoundPos.x + 0.5), static_cast<int>(oSensorPosLaserRoundPos.y + 0.5), m_oTypeOfLaserLine);
							const math::Vec3D oSensorCoordLineRoundPos = rCalib.to3D(static_cast<int>(oSensorPosLineRoundPos.x + 0.5), static_cast<int>(oSensorPosLineRoundPos.y + 0.5), m_oTypeOfLaserLine);
							const math::Vec3D oSensorCoordLaserRoundNeg = rCalib.to3D(static_cast<int>(oSensorPosLaserRoundNeg.x + 0.5), static_cast<int>(oSensorPosLaserRoundNeg.y + 0.5), m_oTypeOfLaserLine);
							const math::Vec3D oSensorCoordLineRoundNeg = rCalib.to3D(static_cast<int>(oSensorPosLineRoundNeg.x + 0.5), static_cast<int>(oSensorPosLineRoundNeg.y + 0.5), m_oTypeOfLaserLine);

							// Berechne die Z-Komponente des um die Y-Achse gedrehten Vektors:
							double oAngle = M_PI * oAngleIn / 180.0;
							math::Vec3D oDiffVectorRoundPos = oSensorCoordLaserRoundPos - oSensorCoordLineRoundPos;
							math::Vec3D oDiffVectorTransformedRoundPos = math::Calibration3DCoords::FromCalibratedToRotated(oDiffVectorRoundPos, oAngle);
							math::Vec3D oDiffVectorRoundNeg = oSensorCoordLaserRoundNeg - oSensorCoordLineRoundNeg;
							math::Vec3D oDiffVectorTransformedRoundNeg = math::Calibration3DCoords::FromCalibratedToRotated(oDiffVectorRoundNeg, oAngle);

							m_oRoundnessPos = std::abs(oDiffVectorTransformedRoundPos[2]);
							m_oRoundnessNeg = std::abs(oDiffVectorTransformedRoundNeg[2]);

							oPositionLeftPointInRoi.x = m_oXLeft;
							oPositionLeftPointInRoi.y = m_oYLeft;    // Y-Wert ist bereits gesetzt worden, siehe oben
							oPositionRightPointInRoi.x = m_oXRight;
							oPositionRightPointInRoi.y = m_oYRight;

							const TPoint<double>	oSensorPositionLeft(oPositionLeftPointInRoi + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten
							const TPoint<double>	oSensorPositionRight(oPositionRightPointInRoi + oRoiPos + oHwRoiPos);	//	Offset Bildkoordinaten -> Sensorkoordinaten

							const math::Vec3D oLeft3D = rCalib.to3D(static_cast<int>(oSensorPositionLeft.x + 0.5), static_cast<int>(oSensorPositionLeft.y + 0.5), m_oTypeOfLaserLine);
							const math::Vec3D oRight3D = rCalib.to3D(static_cast<int>(oSensorPositionRight.x + 0.5), static_cast<int>(oSensorPositionRight.y + 0.5), m_oTypeOfLaserLine);

							math::Vec3D oDiffVector = oLeft3D - oRight3D;  // oDiffVector wird hier nochmal verwendet.

							// Effekt vom Anstellwinkel beruecksichtigen:
							math::Vec3D oDiffVectorTransformed = math::Calibration3DCoords::FromCalibratedToRotated(oDiffVector, oAngle);    // oDiffVectorTransformed   wird hier nochmal verwendet

							// Y ist die Vorschubrichtung und eine Beruecksichtigung der Vorschubrichtung macht keinen Sinn;
							// Z ist die Vertikaldistanz und X ist der Abstand senkrecht zur Naht. Es ist die Frage, ob wir nur den
							// X-Anteil beruecksichtigen wollen oder auch den Z-Anteil. Wir beruecksichtigen X und Z:
							m_oWidth = sqrt(oDiffVectorTransformed[0] * oDiffVectorTransformed[0] + oDiffVectorTransformed[2] * oDiffVectorTransformed[2]);

							m_oPaint = true;
							updateOutArrays (lineN, true);;   // true: Erfolg
						}
						else
						{	// error
							m_oPaint = false; // suppress paint
							updateOutArrays (lineN, false);;
						}
					} // if (!m_oSend)
			}

			auto oGeoWidthArray = GeoDoublearray(rContext, m_oWidthArray, rLine.analysisResult(), m_oGeoOutRank);
			auto oGeoRoundnessPosArray = GeoDoublearray(rContext, m_oRoundnessPosArray, rLine.analysisResult(), m_oGeoOutRank);
			auto oGeoRoundnessNegArray = GeoDoublearray(rContext, m_oRoundnessNegArray, rLine.analysisResult(), m_oGeoOutRank);
			auto oGeoAreaPosArray = GeoDoublearray(rContext, m_oAreaPosArray, rLine.analysisResult(), m_oGeoOutRank);
			auto oGeoAreaNegArray = GeoDoublearray(rContext, m_oAreaNegArray, rLine.analysisResult(), m_oGeoOutRank);
			preSignalAction();

			m_oPipeOutWidth.signal(oGeoWidthArray);
			m_oPipeOutRoundnessPos.signal(oGeoRoundnessPosArray);
			m_oPipeOutRoundnessNeg.signal(oGeoRoundnessNegArray);
			m_oPipeOutAreaPos.signal(oGeoAreaPosArray);
			m_oPipeOutAreaNeg.signal(oGeoAreaNegArray);

		}

		void SeamGeometryTwb::clipDouble(double & d, double lowerLimit, double upperLimit)
		{
			if (d > upperLimit) d = upperLimit;
			if (d < lowerLimit) d = lowerLimit;
		}


		/////////////////////////
		// Class Line 2D
		/////////////////////////

		Line2D_::Line2D_(double slope, double yIntercept)
		{
			m_slope = slope;
			m_yIntercept = yIntercept;;
			m_isVertical = false;
			m_isValid = true;
		}

		Line2D_::Line2D_(double x, double y, double slope)
		{
			m_slope = slope;
			m_yIntercept = y - slope * x;
			m_isVertical = false;
			m_isValid = true;
		}


		Line2D_::Line2D_(double x1, double y1, double x2, double y2)
		{
			if ((std::abs(x2 - x1) < m_epsilon) && (std::abs(y2 - y1) < m_epsilon))
			{ // Punkte identisch => keine Gerade moeglich
				m_slope = 0;
				m_yIntercept = 0;
				m_isValid = false;
				m_isVertical = true;

				return;
			}

			if (std::abs(x2 - x1) < m_epsilon)
			{ // Punkte verschieden, aber Gerade senkrecht
				m_slope = m_bigSlope;
				m_yIntercept = x1;
				m_isValid = true;
				m_isVertical = true;
				return;
			}

			// Punkte hier ok

			m_slope = (y2 - y1) / (x2 - x1);
			m_yIntercept = y1 - m_slope * x1;
			m_isValid = true;
			m_isVertical = false;
			m_oInterceptX = 0.0;
			m_oInterceptY = 0.0;
		}

		double Line2D_::getSlope()
		{
			return m_slope;
		}

		double Line2D_::getYIntercept()
		{
			return m_yIntercept;
		}

		double Line2D_::getIsValid()
		{
			return m_isValid;
		}
		double Line2D_::getIsVertical()
		{
			return m_isVertical;
		}

		double Line2D_::getY(double x)
		{
			if (!m_isValid || m_isVertical) return 0;
			return x * m_slope + m_yIntercept;
		}

		double Line2D_::getOrthoSlope()
		{
			if (!m_isValid) return 0;
			if (m_isVertical) return 0;
			if (m_slope == 0) return m_bigSlope;
			return (-1.0 / m_slope);
		}

		double Line2D_::calcDistance(double x, double y)
		{
			// zunaechst bestimmen, ob Punkt ueber oder unter Gerade liegt, dann Factor setzen
			double calcLineY = getY(x);
			int fac = (y - calcLineY > 0) ? -1 : 1;

			// jetzt Abstand bestimmen
			Line2D_ newLine(x, y, getOrthoSlope());

			double intersectionX = getIntersectionX(newLine);
			double intersectionY = getY(intersectionX);

			m_oInterceptX = intersectionX;
			m_oInterceptY = intersectionY;

			double diffX = intersectionX - x;
			double diffY = intersectionY - y;

			return fac * sqrt(diffX * diffX + diffY * diffY);
		}

		double Line2D_::getIntersectionX(Line2D_ otherLine)
		{
			if (!m_isValid) return 0;
			if (!otherLine.getIsValid()) return 0;

			if (m_isVertical) return m_yIntercept;
			if (otherLine.getIsVertical()) return otherLine.getYIntercept();

			if (std::abs(m_slope - otherLine.getSlope()) < m_epsilon) return 0; // Steigungen zu aehnlich => parallel => nix Schnittpunkt

			return (otherLine.getYIntercept() - m_yIntercept) / (m_slope - otherLine.getSlope());
		}


	} // namespace precitec
} // namespace filter
