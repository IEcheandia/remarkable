/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2017
* 	@brief		This filter checks if a hough poor penetration candidate is a real poor penetration
*/

// project includes
#include "houghChecker.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "geo/array.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
#include <fliplib/TypeToDataTypeImpl.h>

#define OffsetFrame 20

namespace precitec {

	using namespace interface;
	using namespace geo2d;
	using namespace image;

	namespace filter {

		Poco::Mutex HoughChecker::m_oMutex;
		std::vector<geo2d::HoughPPCandidate> HoughChecker::_allPP;

		const std::string HoughChecker::m_oFilterName("HoughChecker");
		const std::string HoughChecker::m_oPipeOutName("DataOut");

		const std::string HoughChecker::m_oParamMinimumLength1("MinimumLength1");
		const std::string HoughChecker::m_oParamMinimumLength2("MinimumLength2");
		const std::string HoughChecker::m_oParamMaximumLineDistance("MaximumLineDistance");
		const std::string HoughChecker::m_oParamMaximumDistanceRoiMiddle("MaximumDistanceRoiMiddle");
		const std::string HoughChecker::m_oParamMaximumJump("MaximumJump");
		const std::string HoughChecker::m_oParamMinimumSumPixelOnLine("MinimumSumPixelOnLine");
		const std::string HoughChecker::m_oParamMaximumLineInterruption("MaximumLineInterruption");
		const std::string HoughChecker::m_oParamMaximumBrightness("MaximumBrightness");
		const std::string HoughChecker::m_oParamCheckIntersection("CheckIntersection");

		int HoughChecker::_currentImage = -1;

		HoughChecker::HoughChecker() : TransformFilter(HoughChecker::m_oFilterName, Poco::UUID{"F64833B9-A49D-4B62-BD49-537FAB860F01"}),
			m_pPipeInData(nullptr), m_oPipeOutData(this, HoughChecker::m_oPipeOutName),

			m_oMinimumLength1(70),
			m_oMinimumLength2(50),
			m_oMaximumLineDistance(15),
			m_oMaximumDistanceRoiMiddle(100),
			m_oMaximumJump(20),
			m_oMinimumSumPixelOnLine(50),
			m_oMaximumLineInterruption(50),
			m_oMaximumBrightness(50),
			m_oCheckIntersection(true)

		{
			parameters_.add(m_oParamMinimumLength1, fliplib::Parameter::TYPE_double, m_oMinimumLength1);
			parameters_.add(m_oParamMinimumLength2, fliplib::Parameter::TYPE_double, m_oMinimumLength2);
			parameters_.add(m_oParamMaximumLineDistance, fliplib::Parameter::TYPE_int, m_oMaximumLineDistance);
			parameters_.add(m_oParamMaximumDistanceRoiMiddle, fliplib::Parameter::TYPE_int, m_oMaximumDistanceRoiMiddle);
			parameters_.add(m_oParamMaximumJump, fliplib::Parameter::TYPE_int, m_oMaximumJump);
			parameters_.add(m_oParamMinimumSumPixelOnLine, fliplib::Parameter::TYPE_double, m_oMinimumSumPixelOnLine);
			parameters_.add(m_oParamMaximumLineInterruption, fliplib::Parameter::TYPE_int, m_oMaximumLineInterruption);
			parameters_.add(m_oParamMaximumBrightness, fliplib::Parameter::TYPE_int, m_oMaximumBrightness);
			parameters_.add(m_oParamCheckIntersection, fliplib::Parameter::TYPE_bool, m_oCheckIntersection);

            setInPipeConnectors({{Poco::UUID("6575D39B-68DA-49FD-B30A-CC4CA31931B6"), m_pPipeInData, "DataIn", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("464C8856-1EF1-4D83-9766-CC0C5E88FFB4"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
            setVariantID(Poco::UUID("151CE170-7306-49FB-9C0E-09C2A54B4BE0"));
		}

		HoughChecker::~HoughChecker()
		{
		}

		void HoughChecker::setParameter()
		{
			TransformFilter::setParameter();

			m_oMinimumLength1 = parameters_.getParameter(HoughChecker::m_oParamMinimumLength1).convert<int>();
			m_oMinimumLength2 = parameters_.getParameter(HoughChecker::m_oParamMinimumLength2).convert<int>();
			m_oMaximumLineDistance = parameters_.getParameter(HoughChecker::m_oParamMaximumLineDistance).convert<int>();
			m_oMaximumDistanceRoiMiddle = parameters_.getParameter(HoughChecker::m_oParamMaximumDistanceRoiMiddle).convert<int>();
			m_oMaximumJump = parameters_.getParameter(HoughChecker::m_oParamMaximumJump).convert<int>();
			m_oMinimumSumPixelOnLine = parameters_.getParameter(HoughChecker::m_oParamMinimumSumPixelOnLine).convert<int>();
			m_oMaximumLineInterruption = parameters_.getParameter(HoughChecker::m_oParamMaximumLineInterruption).convert<int>();
			m_oMaximumBrightness = parameters_.getParameter(HoughChecker::m_oParamMaximumBrightness).convert<int>();
			m_oCheckIntersection = parameters_.getParameter(HoughChecker::m_oParamCheckIntersection).convert<int>();
		} // setParameter.

		bool HoughChecker::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoHoughPPCandidatearray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		} // subscribe

		void HoughChecker::paint()
		{
			if (m_oVerbosity < eLow)
			{
				return;
			}

			if (m_oSpTrafo.isNull())
            {
                return;
            }


			const Trafo		&rTrafo(*m_oSpTrafo);
			OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
			OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

			std::vector<HoughPPRectangle> rectangleList = _overlay.getRectangleContainer();

			for (int i = 0; i < (int)rectangleList.size(); i++)
			{
				HoughPPRectangle rectangle = rectangleList[i];
				rLayerContour.add<OverlayRectangle>(rTrafo(Rect(rectangle.x, rectangle.y, rectangle.width, rectangle.height)), rectangle.color);
			}


			OverlayLayer	&rLayerInfoBox(rCanvas.getLayerInfoBox());

			const interface::GeoHoughPPCandidatearray &rGeoHoughPPCandidateArrayIn = m_pPipeInData->read(m_oCounter);
			unsigned int oSizeOfArray = rGeoHoughPPCandidateArrayIn.ref().size();

			for (unsigned int oIndex = 0; oIndex < oSizeOfArray; oIndex++)
			{
				// get the data
				geo2d::HoughPPCandidate oInCandidate = std::get<eData>(rGeoHoughPPCandidateArrayIn.ref()[oIndex]);

				std::vector<HoughPPInfoLine> _infoLinesPerPP = _allInfoLines[oIndex];

				const auto		oBoundingBoxStart = Point(oInCandidate.m_oPosition1 - OffsetFrame, 2);
				const auto		oBoundingBoxEnd = Point(oInCandidate.m_oPosition2 + OffsetFrame, oInCandidate.m_oHeightRoi - 2);
				const auto		oBoundingBox = Rect(oBoundingBoxStart, oBoundingBoxEnd);

				std::vector<OverlayText> oFeatureLines;

				std::ostringstream		oMsg1;
				std::ostringstream		oMsg2;

				oFeatureLines = std::vector<OverlayText>(_infoLinesPerPP.size() + 1);

				oFeatureLines[0] = OverlayText(id().toString() + ":FILTERGUID:0", Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), Color::Black(), 0);

				for (int i = 0; i < (int)_infoLinesPerPP.size(); i++)
				{
					oFeatureLines[i + 1] = OverlayText(_infoLinesPerPP[i].getLine(), Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), _infoLinesPerPP[i]._color, i + 1);
				}

				rLayerInfoBox.add<OverlayInfoBox>(image::ePoorPenetrationHough, 1, std::move(oFeatureLines), rTrafo(oBoundingBox));
			}
		}

		void HoughChecker::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInData != nullptr); // to be asserted by graph editor

			 m_hasCandidates = false;

			// data
			const interface::GeoHoughPPCandidatearray &rGeoHoughPPCandidateArrayIn = m_pPipeInData->read(m_oCounter);

			const auto&					rContext = m_pPipeInData->read(m_oCounter).context();
			m_oSpTrafo = rContext.trafo();

			m_oNoErrorNr4Paint = 0;
			//std::string overlayString = "";

			_overlay.reset();

			// ist das ein neues Bild?
			// nur noetig fuer die Ausgabe der Nummern im Overlay. Es wird statisch gespeichert, welche Candidates schon in anderen Instanzen vorkamen.
			//int imageNumber = rContext.imageNumber();
			//if (_currentImage != imageNumber)
			//{
			//	Poco::Mutex::ScopedLock oLock(m_oMutex);
			//	_currentImage = imageNumber;
			//	_allPP.clear();
			//}

			// operation
			geo2d::Doublearray oOut;
			geo2d::HoughPPCandidate oInCandidate;
			_allInfoLines.clear();
			std::vector<HoughPPInfoLine> infoLinesPerPP;

			//double oInValue = 0.0;
			//double lastValue = 0.0, oOutValue = 0.0;
			//int lastRank = eRankMax, oInRank = eRankMax;
			int oOutRank = eRankMax;
			unsigned int oSizeOfArray = rGeoHoughPPCandidateArrayIn.ref().size();

			if (oSizeOfArray > 0) // Kandidat ist da
			{
				oInCandidate = std::get<eData>(rGeoHoughPPCandidateArrayIn.ref()[0]);
			}

			oOut.assign(1);

			int totalErrorCounter = 0;
			int noErrorNumber = 100;

			image::Color col = Color::Red();

			if (oInCandidate.m_oTwoLinesFound) //sinnvolles drin?
			{
                m_hasCandidates = true;
				noErrorNumber = 0;
				double bigLength = (oInCandidate.m_oNumberOfPixelOnLine1 > oInCandidate.m_oNumberOfPixelOnLine2) ?
					oInCandidate.m_oNumberOfPixelOnLine1 : oInCandidate.m_oNumberOfPixelOnLine2;
				double smallLength = (oInCandidate.m_oNumberOfPixelOnLine1 < oInCandidate.m_oNumberOfPixelOnLine2) ?
					oInCandidate.m_oNumberOfPixelOnLine1 : oInCandidate.m_oNumberOfPixelOnLine2;

				if (m_oCheckIntersection && !oInCandidate.m_oLineIntersection)
				{
					noErrorNumber = 1;
					//overlayString = "Intersec";
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(1, oInCandidate.m_oLineIntersection, col));

				col = Color::Red();
				if (bigLength < m_oMinimumLength1)
				{
					noErrorNumber = 2; // Linie 1 zu kurz
					//overlayString = "L1 " + toString(bigLength);
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(2, (int)(0.5+bigLength), col));

				col = Color::Red();
				if (smallLength < m_oMinimumLength2)
				{
					noErrorNumber = 3; // Linie 2 zu kurz
					//overlayString = "L2 " + toString(smallLength);
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(3, (int)(0.5+smallLength), col));

				col = Color::Red();
				if (bigLength + smallLength < m_oMinimumSumPixelOnLine)
				{
					noErrorNumber = 4; // Linien zusammen zu kurz
					//overlayString = "DiffSum " + toString(bigLength + smallLength);
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(4, (int)(0.5+bigLength + smallLength), col));

				col = Color::Red();
				if (std::abs((int)oInCandidate.m_oPosition1 - (int)oInCandidate.m_oPosition2) > m_oMaximumLineDistance)
				{
					noErrorNumber = 5;
					//overlayString = "Dist " + toString(std::abs((int)oInCandidate.m_oPosition1 - (int)oInCandidate.m_oPosition2));
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(5, std::abs((int)oInCandidate.m_oPosition1 - (int)oInCandidate.m_oPosition2), col));

				col = Color::Red();
				if (int(oInCandidate.m_oDiffToMiddle1) > m_oMaximumDistanceRoiMiddle)
				{
					noErrorNumber = 6;
					//overlayString = "DiffMid2 " + toString(oInCandidate.m_oDiffToMiddle2);
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(6, oInCandidate.m_oDiffToMiddle1, col));

				//
				//col = Color::Red();
				//infoLinesPerPP.push_back(HoughPPInfoLine(7, 0, Color::Green()));

				col = Color::Red();
				if (int(oInCandidate.m_oBiggestInterruption1) > m_oMaximumLineInterruption)
				{
					noErrorNumber = 9;
					//overlayString = "Inter1 " + toString(oInCandidate.m_oBiggestInterruption1);
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(8, oInCandidate.m_oBiggestInterruption1, col));


				col = Color::Red();
				if (int(oInCandidate.m_oMeanBrightness) > m_oMaximumBrightness)
				{
					noErrorNumber = 10;
					//overlayString = "Bright " + toString(oInCandidate.m_oBiggestInterruption1);
					col = Color::Green();
				}
				infoLinesPerPP.push_back(HoughPPInfoLine(9, oInCandidate.m_oMeanBrightness, col));

				_allInfoLines.push_back(infoLinesPerPP);

				//m_oNoErrorNr4Paint = noErrorNumber;
				//m_oOverlayString = overlayString;
			}
			else
            {
                col = image::Color::m_oAluminium1;
                infoLinesPerPP.push_back(HoughPPInfoLine(1, 0, col, false));
                infoLinesPerPP.push_back(HoughPPInfoLine(2, 0, col, false));
                infoLinesPerPP.push_back(HoughPPInfoLine(3, 0, col, false));
                infoLinesPerPP.push_back(HoughPPInfoLine(4, 0, col, false));
                infoLinesPerPP.push_back(HoughPPInfoLine(5, 0, col, false));
                infoLinesPerPP.push_back(HoughPPInfoLine(6, 0, col, false));
                // 7 is missing because it is also missing if there are candidates available.
                infoLinesPerPP.push_back(HoughPPInfoLine(8, 0, col, false));
                infoLinesPerPP.push_back(HoughPPInfoLine(9, 0, col, false));

                _allInfoLines.push_back(infoLinesPerPP);
            }

			totalErrorCounter = (noErrorNumber == 0) ? 1 : 0;

			col = Color::Green();
			if (totalErrorCounter)
			{
				col = Color::Red();
				totalErrorCounter++;
				_overlay.addRectangle(oInCandidate.m_oPosition1 - OffsetFrame, 2, (oInCandidate.m_oPosition2 - oInCandidate.m_oPosition1) + 2 * OffsetFrame, oInCandidate.m_oHeightRoi - 2, Color::Red());
			}

			oOut[0] = std::tie(totalErrorCounter, oOutRank);

			const interface::GeoDoublearray oGeoDoubleOut(rGeoHoughPPCandidateArrayIn.context(), oOut,
				rGeoHoughPPCandidateArrayIn.analysisResult(), rGeoHoughPPCandidateArrayIn.rank());

			preSignalAction();
			m_oPipeOutData.signal(oGeoDoubleOut);
		} // proceedGroup

		std::string HoughChecker::convertIntToString(int i)
		{ // es gab mit der to_string()-Methode Probleme. Daher eine eigene Implementierung.
			std::ostringstream strstream;
			strstream << i;
			return strstream.str();
		}

		std::string HoughChecker::convertUuidToString(Poco::UUID id)
		{
			std::ostringstream strstream;
			strstream << id;
			return strstream.str();
		}

		int HoughChecker::getNumberOfStoredPP(geo2d::HoughPPCandidate cand)
		{
			// gibt zurueck, wie viele PPCandidates bereits mit denselben Rechteck-Massen vorhanden waren.
			// Ist noetig, um zu entscheiden, wo die Nummer im Overlay hinsoll.
			int count = 0;

			for (int i = 0; i < (int)_allPP.size(); i++)
			{
				HoughPPCandidate storedCand = _allPP[i];
				if ((storedCand.m_oWidth == cand.m_oWidth) && (storedCand.m_oWidth == cand.m_oWidth) && (storedCand.m_oWidth == cand.m_oWidth) && (storedCand.m_oWidth == cand.m_oWidth)) count++; // <---
			}
			return count;
		}

		////////////////////////////////////////////////
		// Klasse InfoLine
		// Haelt eine Zeile einer Overlay-TextBox-Ausgabe
		////////////////////////////////////////////////

		HoughPPInfoLine::HoughPPInfoLine()
		{
			_number = 0;
			_value = 0;
			_color = Color::Black();
		}

		HoughPPInfoLine::HoughPPInfoLine(int number, int value, Color color, bool hasValue)
		{
			_number = number;
			_value = value;
			_color = color;
            m_hasValue = hasValue;
		}

		std::string HoughPPInfoLine::getLine()
        {
            if (m_hasValue && _number > 0 && _number < 10)
            {
                return getLabel() + convertIntToString(_value);
            }
            return getLabel();
        }

        std::string HoughPPInfoLine::getLabel()
        {
            switch (_number)
            {
            case 0:
                return "Unit.None:Unit.None:0";
                break;
            case 1:
                return "Intersection:Unit.None:";
                break;
            case 2:
                return "Maximum1:Unit.None:";
                break;
            case 3:
                return "Maximum2:Unit.None:";
                break;
            case 4:
                return "SumMax12:Unit.None:";
                break;
            case 5:
                return "Distance:Unit.Pixels:";
                break;
            case 6:
                return "CenterDistance:Unit.Pixels:";
                break;
            case 7:
                return "GapJump:Unit.Pixels:";
                break;
            case 8:
                return "Interruption:Unit.Pixels:";
                break;
            case 9:
                return "Brightness:Unit.None:";
                break;
            }
            return "Unit.None:Unit.None:0";
        }

		std::string HoughPPInfoLine::spaces(int i)
		{
			std::ostringstream strstream;
			for (int j = 0; j < i; j++) strstream << ".";
			return strstream.str();
		}

		std::string HoughPPInfoLine::convertIntToString(int i)
		{
			std::ostringstream strstream;
			strstream << i;
			return strstream.str();
		}

		//////////////////////////
		// Overlay
		//////////////////////////

		HoughPPOverlay::HoughPPOverlay()
		{
			reset();
		}

		void HoughPPOverlay::reset()
		{
			_pointContainer.clear();
			_lineContainer.clear();
			_rectangleContainer.clear();
		}

		std::vector<HoughPPPoint> HoughPPOverlay::getPointContainer()
		{
			return _pointContainer;
		}

		std::vector<HoughPPLine> HoughPPOverlay::getLineContainer()
		{
			return _lineContainer;
		}

		std::vector<HoughPPRectangle> HoughPPOverlay::getRectangleContainer()
		{
			return _rectangleContainer;
		}

		void HoughPPOverlay::addPoint(int x, int y, Color color)
		{
			_pointContainer.push_back(HoughPPPoint(x, y, color));
		}

		void HoughPPOverlay::addLine(int x1, int y1, int x2, int y2, Color color)
		{
			_lineContainer.push_back(HoughPPLine(x1, y1, x2, y2, color));
		}

		void HoughPPOverlay::addRectangle(int x, int y, int width, int height, Color color)
		{
			_rectangleContainer.push_back(HoughPPRectangle(x, y, width, height, color));
		}

		HoughPPPoint::HoughPPPoint()
		{
			x = 0;
			y = 0;
			color = Color::Black();
		}

		HoughPPPoint::HoughPPPoint(int x, int y, Color color)
		{
			this->x = x;
			this->y = y;
			this->color = color;
		}

		HoughPPLine::HoughPPLine()
		{
			x1 = 0;
			y1 = 0;
			x2 = 0;
			y2 = 0;
			color = Color::Black();
		}

		HoughPPLine::HoughPPLine(int x1, int y1, int x2, int y2, Color color)
		{
			this->x1 = x1;
			this->y1 = y1;
			this->x2 = x2;
			this->y2 = y2;
			this->color = color;
		}

		HoughPPRectangle::HoughPPRectangle()
		{
			x = 0;
			y = 0;
			width = 0;
			height = 0;
			color = Color::Black();
		}

		HoughPPRectangle::HoughPPRectangle(int x, int y, int width, int height, Color color)
		{
			this->x = x;
			this->y = y;
			this->width = width;
			this->height = height;
			this->color = color;
		}
	} // namespace filter
} // namespace precitec
