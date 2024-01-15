/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		OS
* 	@date		2016
* 	@brief		This filter checks if a poor penetration candidate is a real poor penetration
*/

// project includes
#include "poorPenetrationChecker.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "geo/array.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {

	using namespace interface;
	using namespace geo2d;
	using namespace image;

	namespace filter {

		Poco::Mutex PoorPenetrationChecker::m_oMutex;
		std::vector<geo2d::PoorPenetrationCandidate> PoorPenetrationChecker::_allPP;

		const std::string PoorPenetrationChecker::m_oFilterName("PoorPenetrationChecker");
		const std::string PoorPenetrationChecker::m_oPipeOutName("DataOut");

		const std::string PoorPenetrationChecker::m_oParamActiveParams("ActiveParams");
		const std::string PoorPenetrationChecker::m_oParamMinWidth("MinWidth");
		const std::string PoorPenetrationChecker::m_oParamMaxWidth("MaxWidth");
		const std::string PoorPenetrationChecker::m_oParamMinLength("MinLength");
		const std::string PoorPenetrationChecker::m_oParamMaxLength("MaxLength");

		const std::string PoorPenetrationChecker::m_oParamMinGradient("MinGradient");
		const std::string PoorPenetrationChecker::m_oParamMaxGradient("MaxGradient");
		const std::string PoorPenetrationChecker::m_oParamMinGreyvalGap("MinGreyvalGap");
		const std::string PoorPenetrationChecker::m_oParamMaxGreyvalGap("MaxGreyvalGap");

		const std::string PoorPenetrationChecker::m_oParamMinRatioInnerOuter("MinRatioInnerOuter");
		const std::string PoorPenetrationChecker::m_oParamMaxRatioInnerOuter("MaxRatioInnerOuter");
		const std::string PoorPenetrationChecker::m_oParamMinStandardDeviation("MinStandardDeviation");
		const std::string PoorPenetrationChecker::m_oParamMaxStandardDeviation("MaxStandardDeviation");
		const std::string PoorPenetrationChecker::m_oParamMinDevelopedLength("MinDevelopedLength");
		const std::string PoorPenetrationChecker::m_oParamMaxDevelopedLength("MaxDevelopedLength");

		int PoorPenetrationChecker::_currentImage = -1;

		PoorPenetrationChecker::PoorPenetrationChecker() : TransformFilter(PoorPenetrationChecker::m_oFilterName, Poco::UUID{"A1111481-DD01-40AF-A825-580AFB952B49"}),
			m_pPipeInData(nullptr), m_oPipeOutData(this, PoorPenetrationChecker::m_oPipeOutName),
			m_oActiveParams(7), m_oMinWidth(5), m_oMaxWidth(35), m_oMinLength(200), m_oMaxLength(400),
			m_oMinGradient(80), m_oMaxGradient(250), m_oMinGreyvalGap(70), m_oMaxGreyvalGap(120),
			m_oMinRatioInnerOuter(15), m_oMaxRatioInnerOuter(25), m_oMinStandardDeviation(25), m_oMaxStandardDeviation(35),
			m_oMinDevelopedLength(500), m_oMaxDevelopedLength(1000)
		{
			parameters_.add(m_oParamActiveParams, fliplib::Parameter::TYPE_int, m_oActiveParams);

			parameters_.add(m_oParamMinWidth, fliplib::Parameter::TYPE_int, m_oMinWidth);
			parameters_.add(m_oParamMaxWidth, fliplib::Parameter::TYPE_int, m_oMaxWidth);
			parameters_.add(m_oParamMinLength, fliplib::Parameter::TYPE_int, m_oMinLength);
			parameters_.add(m_oParamMaxLength, fliplib::Parameter::TYPE_int, m_oMaxLength);

			parameters_.add(m_oParamMinGradient, fliplib::Parameter::TYPE_int, m_oMinGradient);
			parameters_.add(m_oParamMaxGradient, fliplib::Parameter::TYPE_int, m_oMaxGradient);
			parameters_.add(m_oParamMinGreyvalGap, fliplib::Parameter::TYPE_int, m_oMinGreyvalGap);
			parameters_.add(m_oParamMaxGreyvalGap, fliplib::Parameter::TYPE_int, m_oMaxGreyvalGap);

			parameters_.add(m_oParamMinRatioInnerOuter, fliplib::Parameter::TYPE_int, m_oMinRatioInnerOuter);
			parameters_.add(m_oParamMaxRatioInnerOuter, fliplib::Parameter::TYPE_int, m_oMaxRatioInnerOuter);
			parameters_.add(m_oParamMinStandardDeviation, fliplib::Parameter::TYPE_int, m_oMinStandardDeviation);
			parameters_.add(m_oParamMaxStandardDeviation, fliplib::Parameter::TYPE_int, m_oMaxStandardDeviation);

			parameters_.add(m_oParamMinDevelopedLength, fliplib::Parameter::TYPE_int, m_oMinDevelopedLength);
			parameters_.add(m_oParamMaxDevelopedLength, fliplib::Parameter::TYPE_int, m_oMaxDevelopedLength);

            setInPipeConnectors({{Poco::UUID("CE0DBE08-4661-4E9C-9381-0FAA0FC1DB81"), m_pPipeInData, "DataIn", 0, ""}});
            setOutPipeConnectors({{Poco::UUID("4A6F5062-6BB8-48B7-B9F6-A677080C09BF"), &m_oPipeOutData, m_oPipeOutName, 0, ""}});
            setVariantID(Poco::UUID("BCCA1B29-8B24-4F18-ABC1-E338D41A1554"));
		}

		PoorPenetrationChecker::~PoorPenetrationChecker()
		{
		}

		void PoorPenetrationChecker::setParameter()
		{
			TransformFilter::setParameter();

			m_oActiveParams = parameters_.getParameter(PoorPenetrationChecker::m_oParamActiveParams).convert<int>();

			m_oMinWidth = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinWidth).convert<int>();
			m_oMaxWidth = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxWidth).convert<int>();
			m_oMinLength = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinLength).convert<int>();
			m_oMaxLength = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxLength).convert<int>();

			m_oMinGradient = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinGradient).convert<int>();
			m_oMaxGradient = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxGradient).convert<int>();
			m_oMinGreyvalGap = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinGreyvalGap).convert<int>();
			m_oMaxGreyvalGap = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxGreyvalGap).convert<int>();

			m_oMinRatioInnerOuter = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinRatioInnerOuter).convert<int>();
			m_oMaxRatioInnerOuter = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxRatioInnerOuter).convert<int>();
			m_oMinStandardDeviation = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinStandardDeviation).convert<int>();
			m_oMaxStandardDeviation = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxStandardDeviation).convert<int>();

			m_oMinDevelopedLength = parameters_.getParameter(PoorPenetrationChecker::m_oParamMinDevelopedLength).convert<int>();
			m_oMaxDevelopedLength = parameters_.getParameter(PoorPenetrationChecker::m_oParamMaxDevelopedLength).convert<int>();
		}

		bool PoorPenetrationChecker::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
		{
			m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoPoorPenetrationCandidatearray > * >(&p_rPipe);

			return BaseFilter::subscribe(p_rPipe, p_oGroup);
		}

		void PoorPenetrationChecker::paint()
		{
			if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
			{
				return;
			}

			const Trafo		&rTrafo(*m_oSpTrafo);
			OverlayCanvas	&rCanvas(canvas<OverlayCanvas>(m_oCounter));
			OverlayLayer	&rLayerContour(rCanvas.getLayerContour());

            std::vector<PoorPenetrationRectangle> rectangleList = _overlay.getRectangleContainer();

			for (size_t i = 0; i < rectangleList.size(); i++)
			{
				PoorPenetrationRectangle rectangle = rectangleList[i];
				rLayerContour.add(new OverlayRectangle(rTrafo(Rect(rectangle.x, rectangle.y, rectangle.width, rectangle.height)), rectangle.color));
			}

			OverlayLayer	&rLayerText(rCanvas.getLayerText());
			OverlayLayer	&rLayerInfoBox(rCanvas.getLayerInfoBox());

			const interface::GeoPoorPenetrationCandidatearray &rGeoPoorPenetrationCandidateArrayIn = m_pPipeInData->read(m_oCounter);
			unsigned int oSizeOfArray = rGeoPoorPenetrationCandidateArrayIn.ref().size();

			for (unsigned int oIndex = 0; oIndex < oSizeOfArray; oIndex++)
			{
				// get the data
				geo2d::PoorPenetrationCandidate oInCandidate = std::get<eData>(rGeoPoorPenetrationCandidateArrayIn.ref()[oIndex]);

				int numberToDraw;
				int numberForLocation;

				{
					Poco::Mutex::ScopedLock oLock(m_oMutex);

					numberToDraw = _allPP.size(); // Start bei 0
					numberForLocation = getNumberOfStoredPP(oInCandidate); // Start bei 0

					_allPP.push_back(oInCandidate);
				}

				std::vector<InfoLine> _infoLinesPerPP = _allInfoLines[oIndex];

				const auto		oBoundingBoxStart = Point(oInCandidate.xmin, oInCandidate.ymin);
				const auto		oBoundingBoxEnd = Point(oInCandidate.xmax, oInCandidate.ymax);
				const auto		oBoundingBox = Rect(oBoundingBoxStart, oBoundingBoxEnd);

				std::vector<OverlayText> oFeatureLines;

				std::ostringstream		oMsg1;
				std::ostringstream		oMsg2;

				oFeatureLines = std::vector<OverlayText>(_infoLinesPerPP.size()+1);

				rLayerText.add(new OverlayText(convertIntToString(numberToDraw + 1), Font(16),
					rTrafo(Rect(
					Point(oInCandidate.xmin - 20, oInCandidate.ymin + numberForLocation * 20),
					Point(oInCandidate.xmin, oInCandidate.ymin + 20 + numberForLocation * 20))),
					Color::Yellow()));

				oFeatureLines[0] = OverlayText(id().toString() + ":FILTERGUID:0", Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), Color::Black(), 0);

				for (size_t i = 0; i < _infoLinesPerPP.size(); i++)
				{
					oFeatureLines[i+1] = OverlayText(_infoLinesPerPP[i].getLine(), Font(12, true, false, "Courier New"), rTrafo(Rect(10, 10, 100, 200)), _infoLinesPerPP[i]._color, i+1);
				}

				rLayerInfoBox.add(new OverlayInfoBox(image::ePoorPenetration, numberToDraw, std::move(oFeatureLines), rTrafo(oBoundingBox)));
			}
		}

		void PoorPenetrationChecker::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
		{
			poco_assert_dbg(m_pPipeInData != nullptr); // to be asserted by graph editor

			// data
			const interface::GeoPoorPenetrationCandidatearray &rGeoPoorPenetrationCandidateArrayIn = m_pPipeInData->read(m_oCounter);

			const auto&					rContext = m_pPipeInData->read(m_oCounter).context();
			m_oSpTrafo = rContext.trafo();

			_overlay.reset();

			// ist das ein neues Bild?
			// nur noetig fuer die Ausgabe der Nummern im Overlay. Es wird statisch gespeichert, welche Candidates schon in anderen Instanzen vorkamen.
			int imageNumber = rContext.imageNumber();
			if (_currentImage != imageNumber)
			{
				Poco::Mutex::ScopedLock oLock(m_oMutex);
				_currentImage = imageNumber;
				_allPP.clear();
			}

			// operation
			geo2d::Doublearray oOut;
			geo2d::PoorPenetrationCandidate oInCandidate;
			_allInfoLines.clear();

			//double oInValue = 0.0;
			//double lastValue = 0.0, oOutValue = 0.0;
			//int lastRank = eRankMax, oInRank = eRankMax;
			int oOutRank = eRankMax;
			unsigned int oSizeOfArray = rGeoPoorPenetrationCandidateArrayIn.ref().size();
			oOut.assign(1);

			int totalErrorCounter = 0;
			int singleErrorCounter = 0;
			int valueToCheck, min, max;

			Color color = Color::Black();
			Color darkGreen = Color(0, 150, 0);

			int result = 0;

			// ab hier alle Werte durchgehen, auf Grenzen ueberpruefen
			for (unsigned int oIndex = 0; oIndex < oSizeOfArray; oIndex++)
			{
				std::vector<InfoLine> _infoLinesPerPP;
                singleErrorCounter = 0;
                result = 0;

				// get the data
				oInCandidate = std::get<eData>(rGeoPoorPenetrationCandidateArrayIn.ref()[oIndex]);
				// oInRank = std::get<eRank>(rGeoPoorPenetrationCandidateArrayIn.ref()[oIndex]);

				// 1. Check Length
				color = darkGreen;
				valueToCheck = oInCandidate.m_oLength;
				min = m_oMinLength;
				max = m_oMaxLength;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 1;
				}
				_infoLinesPerPP.push_back(InfoLine(1, valueToCheck, color));

				// 2. Check Width
				color = darkGreen;
				valueToCheck = oInCandidate.m_oWidth;
				min = m_oMinWidth;
				max = m_oMaxWidth;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 2;
				}
				_infoLinesPerPP.push_back(InfoLine(2, valueToCheck, color));

				// 3. Check Gradient
				color = darkGreen;
				valueToCheck = oInCandidate.m_oGradient;
				min = m_oMinGradient;
				max = m_oMaxGradient;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 3;
				}
				_infoLinesPerPP.push_back(InfoLine(3, valueToCheck, color));

				// 4. Check GreyvalGap
				color = darkGreen;
				valueToCheck = oInCandidate.m_oGreyvalGap;
				min = m_oMinGreyvalGap;
				max = m_oMaxGreyvalGap;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 4;
				}
				_infoLinesPerPP.push_back(InfoLine(4, valueToCheck, color));

				// 5. Check RatioInnerOuter
				color = darkGreen;
				if (oInCandidate.m_oGreyvalInner > 0)
					valueToCheck = (int)(0.5 + 10 * (((double)(oInCandidate.m_oGreyvalOuter)) / ((double)(oInCandidate.m_oGreyvalInner))));
				else
					valueToCheck = 0;
				min = m_oMinRatioInnerOuter;
				max = m_oMaxRatioInnerOuter;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 5;
				}
				_infoLinesPerPP.push_back(InfoLine(5, oInCandidate.m_oGreyvalInner, color));
				_infoLinesPerPP.push_back(InfoLine(6, oInCandidate.m_oGreyvalOuter, color));

				// 6. Standard deviation
				color = darkGreen;
				valueToCheck = oInCandidate.m_oStandardDeviation;
				min = m_oMinStandardDeviation;
				max = m_oMaxStandardDeviation;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 6;
				}
				_infoLinesPerPP.push_back(InfoLine(7, valueToCheck, color));

				// 7. Check DevelopedLength
				// changed to the mean value
				color = darkGreen;
				valueToCheck = (oInCandidate.m_oDevelopedLengthLeft + oInCandidate.m_oDevelopedLengthRight)/2;
				min = m_oMinDevelopedLength;
				max = m_oMaxDevelopedLength;
				if ((valueToCheck>=min) && (valueToCheck<=max))
				{
					singleErrorCounter++;
					color = Color::Red();
				}
				else if (result == 0)
				{
					result = 7;
				}
				_infoLinesPerPP.push_back(InfoLine(8, oInCandidate.m_oDevelopedLengthLeft, color));
				_infoLinesPerPP.push_back(InfoLine(9, oInCandidate.m_oDevelopedLengthRight, color));

				color = darkGreen;
				if (singleErrorCounter >= m_oActiveParams)
				{
					color = Color::Red();
					totalErrorCounter++;
					_overlay.addRectangle(oInCandidate.xmin, oInCandidate.ymin, oInCandidate.xmax - oInCandidate.xmin, oInCandidate.ymax - oInCandidate.ymin, Color::Red());
				}

				_infoLinesPerPP.push_back(InfoLine(10, result, color));

				_allInfoLines.push_back(_infoLinesPerPP);

			}

			oOut[0] = std::tie(totalErrorCounter, oOutRank);

			const interface::GeoDoublearray oGeoDoubleOut(rGeoPoorPenetrationCandidateArrayIn.context(), oOut,
				rGeoPoorPenetrationCandidateArrayIn.analysisResult(), rGeoPoorPenetrationCandidateArrayIn.rank());

			preSignalAction();
			m_oPipeOutData.signal(oGeoDoubleOut);
		}

		std::string PoorPenetrationChecker::convertIntToString(int i)
		{ // es gab mit der to_string()-Methode Probleme. Daher eine eigene Implementierung.
			std::ostringstream strstream;
			strstream << i;
			return strstream.str();
		}

		std::string PoorPenetrationChecker::convertUuidToString(Poco::UUID id)
		{
			std::ostringstream strstream;
			strstream << id;
			return strstream.str();
		}

		int PoorPenetrationChecker::getNumberOfStoredPP(geo2d::PoorPenetrationCandidate cand)
		{
			// gibt zurueck, wie viele PPCandidates bereits mit denselben Rechteck-Massen vorhanden waren.
			// Ist noetig, um zu entscheiden, wo die Nummer im Overlay hinsoll.
			int count = 0;

			for (int i = 0; i < (int)_allPP.size(); i++)
			{
				PoorPenetrationCandidate storedCand = _allPP[i];
				if ((storedCand.xmin == cand.xmin) && (storedCand.ymin == cand.ymin) && (storedCand.xmax == cand.xmax) && (storedCand.ymax == cand.ymax)) count++;
			}
			return count;
		}
	}
}
