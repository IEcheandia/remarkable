/**
* @file
* @copyright    Precitec Vision GmbH & Co. KG
* @author       MM
* @date         2021
* @brief        This filter checks if a poor penetration candidate is a real poor penetration, analogue to PoorPenetrationChecker but all parameters were tripled.
*/

// project includes
#include "poorPenetrationCheckerTriple.h"
#include <module/moduleLogger.h>
#include <filter/armStates.h>
#include <event/resultType.h>
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "geo/array.h"
#include "module/moduleLogger.h"
#include "common/defines.h"
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{
PoorPenetrationCheckerTriple::PoorPenetrationCheckerTriple() : TransformFilter("PoorPenetrationCheckerTriple", Poco::UUID{"7f461f6e-1de3-4569-8966-1989dd855711"})
    , m_pPipeInData(nullptr)
    , m_oPipeOutData(this, "DataOut")
    , m_oActiveParamSets(1)
    , m_oActiveParams(7)
    , m_oMinWidth(5)
    , m_oMaxWidth(35)
    , m_oMinLength(200)
    , m_oMaxLength(400)
    , m_oMinGradient(80)
    , m_oMaxGradient(250)
    , m_oMinGreyvalGap(70)
    , m_oMaxGreyvalGap(120)
    , m_oMinRatioInnerOuter(15)
    , m_oMaxRatioInnerOuter(25)
    , m_oMinStandardDeviation(25)
    , m_oMaxStandardDeviation(35)
    , m_oMinDevelopedLength(500)
    , m_oMaxDevelopedLength(1000)
    , m_oActiveParams1(7)
    , m_oMinWidth1(5)
    , m_oMaxWidth1(35)
    , m_oMinLength1(200)
    , m_oMaxLength1(400)
    , m_oMinGradient1(80)
    , m_oMaxGradient1(250)
    , m_oMinGreyvalGap1(70)
    , m_oMaxGreyvalGap1(120)
    , m_oMinRatioInnerOuter1(15)
    , m_oMaxRatioInnerOuter1(25)
    , m_oMinStandardDeviation1(25)
    , m_oMaxStandardDeviation1(35)
    , m_oMinDevelopedLength1(500)
    , m_oMaxDevelopedLength1(1000)
    , m_oActiveParams2(7)
    , m_oMinWidth2(5)
    , m_oMaxWidth2(35)
    , m_oMinLength2(200)
    , m_oMaxLength2(400)
    , m_oMinGradient2(80)
    , m_oMaxGradient2(250)
    , m_oMinGreyvalGap2(70)
    , m_oMaxGreyvalGap2(120)
    , m_oMinRatioInnerOuter2(15)
    , m_oMaxRatioInnerOuter2(25)
    , m_oMinStandardDeviation2(25)
    , m_oMaxStandardDeviation2(35)
    , m_oMinDevelopedLength2(500)
    , m_oMaxDevelopedLength2(1000)
{
    parameters_.add("ActiveParamSets", fliplib::Parameter::TYPE_int, m_oActiveParamSets);

    parameters_.add("ActiveParams", fliplib::Parameter::TYPE_int, m_oActiveParams);
    parameters_.add("MinWidth", fliplib::Parameter::TYPE_int, m_oMinWidth);
    parameters_.add("MaxWidth", fliplib::Parameter::TYPE_int, m_oMaxWidth);
    parameters_.add("MinLength", fliplib::Parameter::TYPE_int, m_oMinLength);
    parameters_.add("MaxLength", fliplib::Parameter::TYPE_int, m_oMaxLength);
    parameters_.add("MinGradient", fliplib::Parameter::TYPE_int, m_oMinGradient);
    parameters_.add("MaxGradient", fliplib::Parameter::TYPE_int, m_oMaxGradient);
    parameters_.add("MinGreyvalGap", fliplib::Parameter::TYPE_int, m_oMinGreyvalGap);
    parameters_.add("MaxGreyvalGap", fliplib::Parameter::TYPE_int, m_oMaxGreyvalGap);
    parameters_.add("MinRatioInnerOuter", fliplib::Parameter::TYPE_int, m_oMinRatioInnerOuter);
    parameters_.add("MaxRatioInnerOuter", fliplib::Parameter::TYPE_int, m_oMaxRatioInnerOuter);
    parameters_.add("MinStandardDeviation", fliplib::Parameter::TYPE_int, m_oMinStandardDeviation);
    parameters_.add("MaxStandardDeviation", fliplib::Parameter::TYPE_int, m_oMaxStandardDeviation);
    parameters_.add("MinDevelopedLength", fliplib::Parameter::TYPE_int, m_oMinDevelopedLength);
    parameters_.add("MaxDevelopedLength", fliplib::Parameter::TYPE_int, m_oMaxDevelopedLength);

    parameters_.add("ActiveParams1", fliplib::Parameter::TYPE_int, m_oActiveParams1);
    parameters_.add("MinWidth1", fliplib::Parameter::TYPE_int, m_oMinWidth1);
    parameters_.add("MaxWidth1", fliplib::Parameter::TYPE_int, m_oMaxWidth1);
    parameters_.add("MinLength1", fliplib::Parameter::TYPE_int, m_oMinLength1);
    parameters_.add("MaxLength1", fliplib::Parameter::TYPE_int, m_oMaxLength1);
    parameters_.add("MinGradient1", fliplib::Parameter::TYPE_int, m_oMinGradient1);
    parameters_.add("MaxGradient1", fliplib::Parameter::TYPE_int, m_oMaxGradient1);
    parameters_.add("MinGreyvalGap1", fliplib::Parameter::TYPE_int, m_oMinGreyvalGap1);
    parameters_.add("MaxGreyvalGap1", fliplib::Parameter::TYPE_int, m_oMaxGreyvalGap1);
    parameters_.add("MinRatioInnerOuter1", fliplib::Parameter::TYPE_int, m_oMinRatioInnerOuter1);
    parameters_.add("MaxRatioInnerOuter1", fliplib::Parameter::TYPE_int, m_oMaxRatioInnerOuter1);
    parameters_.add("MinStandardDeviation1", fliplib::Parameter::TYPE_int, m_oMinStandardDeviation1);
    parameters_.add("MaxStandardDeviation1", fliplib::Parameter::TYPE_int, m_oMaxStandardDeviation1);
    parameters_.add("MinDevelopedLength1", fliplib::Parameter::TYPE_int, m_oMinDevelopedLength1);
    parameters_.add("MaxDevelopedLength1", fliplib::Parameter::TYPE_int, m_oMaxDevelopedLength1);

    parameters_.add("ActiveParams2", fliplib::Parameter::TYPE_int, m_oActiveParams2);
    parameters_.add("MinWidth2", fliplib::Parameter::TYPE_int, m_oMinWidth2);
    parameters_.add("MaxWidth2", fliplib::Parameter::TYPE_int, m_oMaxWidth2);
    parameters_.add("MinLength2", fliplib::Parameter::TYPE_int, m_oMinLength2);
    parameters_.add("MaxLength2", fliplib::Parameter::TYPE_int, m_oMaxLength2);
    parameters_.add("MinGradient2", fliplib::Parameter::TYPE_int, m_oMinGradient2);
    parameters_.add("MaxGradient2", fliplib::Parameter::TYPE_int, m_oMaxGradient2);
    parameters_.add("MinGreyvalGap2", fliplib::Parameter::TYPE_int, m_oMinGreyvalGap2);
    parameters_.add("MaxGreyvalGap2", fliplib::Parameter::TYPE_int, m_oMaxGreyvalGap2);
    parameters_.add("MinRatioInnerOuter2", fliplib::Parameter::TYPE_int, m_oMinRatioInnerOuter2);
    parameters_.add("MaxRatioInnerOuter2", fliplib::Parameter::TYPE_int, m_oMaxRatioInnerOuter2);
    parameters_.add("MinStandardDeviation2", fliplib::Parameter::TYPE_int, m_oMinStandardDeviation2);
    parameters_.add("MaxStandardDeviation2", fliplib::Parameter::TYPE_int, m_oMaxStandardDeviation2);
    parameters_.add("MinDevelopedLength2", fliplib::Parameter::TYPE_int, m_oMinDevelopedLength2);
    parameters_.add("MaxDevelopedLength2", fliplib::Parameter::TYPE_int, m_oMaxDevelopedLength2);

    setInPipeConnectors({{Poco::UUID("dd7ad10f-4865-407c-9de9-c868183c2210"), m_pPipeInData, "DataIn", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("bd526229-62b6-4554-ab5c-45246abfefc3"), &m_oPipeOutData, "DataOut", 0, ""}});
    setVariantID(Poco::UUID("249025ae-615c-4582-8beb-8dfe90e5f249"));
}

PoorPenetrationCheckerTriple::~PoorPenetrationCheckerTriple() = default;

void PoorPenetrationCheckerTriple::setParameter()
{
    TransformFilter::setParameter();

    m_oActiveParamSets = parameters_.getParameter("ActiveParamSets").convert<int>();

    m_oActiveParams = parameters_.getParameter("ActiveParams").convert<int>();
    m_oMinWidth = parameters_.getParameter("MinWidth").convert<int>();
    m_oMaxWidth = parameters_.getParameter("MaxWidth").convert<int>();
    m_oMinLength = parameters_.getParameter("MinLength").convert<int>();
    m_oMaxLength = parameters_.getParameter("MaxLength").convert<int>();
    m_oMinGradient = parameters_.getParameter("MinGradient").convert<int>();
    m_oMaxGradient = parameters_.getParameter("MaxGradient").convert<int>();
    m_oMinGreyvalGap = parameters_.getParameter("MinGreyvalGap").convert<int>();
    m_oMaxGreyvalGap = parameters_.getParameter("MaxGreyvalGap").convert<int>();
    m_oMinRatioInnerOuter = parameters_.getParameter("MinRatioInnerOuter").convert<int>();
    m_oMaxRatioInnerOuter = parameters_.getParameter("MaxRatioInnerOuter").convert<int>();
    m_oMinStandardDeviation = parameters_.getParameter("MinStandardDeviation").convert<int>();
    m_oMaxStandardDeviation = parameters_.getParameter("MaxStandardDeviation").convert<int>();
    m_oMinDevelopedLength = parameters_.getParameter("MinDevelopedLength").convert<int>();
    m_oMaxDevelopedLength = parameters_.getParameter("MaxDevelopedLength").convert<int>();

    m_oActiveParams1 = parameters_.getParameter("ActiveParams1").convert<int>();
    m_oMinWidth1 = parameters_.getParameter("MinWidth1").convert<int>();
    m_oMaxWidth1 = parameters_.getParameter("MaxWidth1").convert<int>();
    m_oMinLength1 = parameters_.getParameter("MinLength1").convert<int>();
    m_oMaxLength1 = parameters_.getParameter("MaxLength1").convert<int>();
    m_oMinGradient1 = parameters_.getParameter("MinGradient1").convert<int>();
    m_oMaxGradient1 = parameters_.getParameter("MaxGradient1").convert<int>();
    m_oMinGreyvalGap1 = parameters_.getParameter("MinGreyvalGap1").convert<int>();
    m_oMaxGreyvalGap1 = parameters_.getParameter("MaxGreyvalGap1").convert<int>();
    m_oMinRatioInnerOuter1 = parameters_.getParameter("MinRatioInnerOuter1").convert<int>();
    m_oMaxRatioInnerOuter1 = parameters_.getParameter("MaxRatioInnerOuter1").convert<int>();
    m_oMinStandardDeviation1 = parameters_.getParameter("MinStandardDeviation1").convert<int>();
    m_oMaxStandardDeviation1 = parameters_.getParameter("MaxStandardDeviation1").convert<int>();
    m_oMinDevelopedLength1 = parameters_.getParameter("MinDevelopedLength1").convert<int>();
    m_oMaxDevelopedLength1 = parameters_.getParameter("MaxDevelopedLength1").convert<int>();

    m_oActiveParams2 = parameters_.getParameter("ActiveParams2").convert<int>();
    m_oMinWidth2 = parameters_.getParameter("MinWidth2").convert<int>();
    m_oMaxWidth2 = parameters_.getParameter("MaxWidth2").convert<int>();
    m_oMinLength2 = parameters_.getParameter("MinLength2").convert<int>();
    m_oMaxLength2 = parameters_.getParameter("MaxLength2").convert<int>();
    m_oMinGradient2 = parameters_.getParameter("MinGradient2").convert<int>();
    m_oMaxGradient2 = parameters_.getParameter("MaxGradient2").convert<int>();
    m_oMinGreyvalGap2 = parameters_.getParameter("MinGreyvalGap2").convert<int>();
    m_oMaxGreyvalGap2 = parameters_.getParameter("MaxGreyvalGap2").convert<int>();
    m_oMinRatioInnerOuter2 = parameters_.getParameter("MinRatioInnerOuter2").convert<int>();
    m_oMaxRatioInnerOuter2 = parameters_.getParameter("MaxRatioInnerOuter2").convert<int>();
    m_oMinStandardDeviation2 = parameters_.getParameter("MinStandardDeviation2").convert<int>();
    m_oMaxStandardDeviation2 = parameters_.getParameter("MaxStandardDeviation2").convert<int>();
    m_oMinDevelopedLength2 = parameters_.getParameter("MinDevelopedLength2").convert<int>();
    m_oMaxDevelopedLength2 = parameters_.getParameter("MaxDevelopedLength2").convert<int>();
}

bool PoorPenetrationCheckerTriple::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    m_pPipeInData = dynamic_cast< fliplib::SynchronePipe < interface::GeoPoorPenetrationCandidatearray > * >(&p_rPipe);

    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void PoorPenetrationCheckerTriple::paint()
{
    if (m_oVerbosity < eLow || m_oSpTrafo.isNull())
    {
        return;
    }

    const interface::Trafo	&rTrafo(*m_oSpTrafo);
    image::OverlayCanvas	&rCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer	    &rLayerContour(rCanvas.getLayerContour());

    const std::vector<PoorPenetrationRectangle>& rectangleList = m_overlay.getRectangleContainer();

    for (size_t i = 0; i < rectangleList.size(); i++)
    {
        const PoorPenetrationRectangle& rectangle = rectangleList[i];
        rLayerContour.add<image::OverlayRectangle>(rTrafo(geo2d::Rect(rectangle.x, rectangle.y, rectangle.width, rectangle.height)), rectangle.color);
    }

    image::OverlayLayer	&rLayerText(rCanvas.getLayerText());
    image::OverlayLayer	&rLayerInfoBox(rCanvas.getLayerInfoBox());

    const interface::GeoPoorPenetrationCandidatearray &rGeoPoorPenetrationCandidateArrayIn = m_pPipeInData->read(m_oCounter);
    const size_t sizeOfArray = rGeoPoorPenetrationCandidateArrayIn.ref().size();

    const image::Font fontCourierNew = image::Font(12, true, false, "Courier New");
    std::vector<geo2d::PoorPenetrationCandidate> allPP;

    for (size_t i = 0; i < sizeOfArray; i++)
    {
        // get the data
        geo2d::PoorPenetrationCandidate oInCandidate = std::get<eData>(rGeoPoorPenetrationCandidateArrayIn.ref()[i]);

        int numberForLocation = getNumberOfStoredPP(oInCandidate, allPP);
        allPP.push_back(oInCandidate);

        const auto oBoundingBoxStart = geo2d::Point(oInCandidate.xmin, oInCandidate.ymin);
        const auto oBoundingBoxEnd = geo2d::Point(oInCandidate.xmax, oInCandidate.ymax);
        const auto oBoundingBox = geo2d::Rect(oBoundingBoxStart, oBoundingBoxEnd);

        rLayerText.add(new image::OverlayText(std::to_string(i + 1), image::Font(16),
            rTrafo(geo2d::Rect(
            geo2d::Point(oInCandidate.xmin - 20, oInCandidate.ymin + numberForLocation * 20),
            geo2d::Point(oInCandidate.xmin, oInCandidate.ymin + 20 + numberForLocation * 20))),
            image::Color::Yellow()));

        std::vector<image::OverlayText> oFeatureLines;
        std::vector<InfoLine> _infoLinesPerPP;

        for (int j = 0; j < m_oActiveParamSets; j++)
        {
            oFeatureLines.clear();
            oFeatureLines.push_back(image::OverlayText(id().toString() + ":FILTERGUID:0", fontCourierNew, rTrafo(geo2d::Rect(10, 10, 100, 200)), image::Color::Black(), 0));
            _infoLinesPerPP = m_allInfoLines[i + j * sizeOfArray];
            for (size_t k = 0; k < _infoLinesPerPP.size(); k++)
            {
                oFeatureLines.push_back(image::OverlayText(_infoLinesPerPP[k].getLine(), fontCourierNew, rTrafo(geo2d::Rect(10, 10, 100, 200)), _infoLinesPerPP[k]._color, k+1));
            }
            // ID is shown in the GUI. Last digit correlates to parameter set j, first digit(s) correlates to the poor penetration candidate i.
            rLayerInfoBox.add<image::OverlayInfoBox>(image::ePoorPenetration, ((i + 1) * 10 + j), std::move(oFeatureLines), rTrafo(oBoundingBox));
        }
    }
}

void PoorPenetrationCheckerTriple::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rE)
{
    poco_assert_dbg(m_pPipeInData != nullptr); // to be asserted by graph editor

    // data
    const interface::GeoPoorPenetrationCandidatearray &rGeoPoorPenetrationCandidateArrayIn = m_pPipeInData->read(m_oCounter);

    const auto&	rContext = m_pPipeInData->read(m_oCounter).context();
    m_oSpTrafo = rContext.trafo();

    m_overlay.reset();
    m_allInfoLines.clear();

    int totalErrorCounter = 0;

    // check all values for limits
    if (m_oActiveParamSets > 0)
    {
        totalErrorCounter += checkForErrors(rGeoPoorPenetrationCandidateArrayIn, m_oMinLength, m_oMaxLength, m_oMinWidth, m_oMaxWidth, m_oMinGradient, m_oMaxGradient,
                                        m_oMinGreyvalGap, m_oMaxGreyvalGap, m_oMinRatioInnerOuter, m_oMaxRatioInnerOuter, m_oMinStandardDeviation, m_oMaxStandardDeviation,
                                        m_oMinDevelopedLength, m_oMaxDevelopedLength, m_oActiveParams);
        if (m_oActiveParamSets > 1)
        {
            totalErrorCounter += 10 * checkForErrors(rGeoPoorPenetrationCandidateArrayIn, m_oMinLength1, m_oMaxLength1, m_oMinWidth1, m_oMaxWidth1, m_oMinGradient1, m_oMaxGradient1,
                                                m_oMinGreyvalGap1, m_oMaxGreyvalGap1, m_oMinRatioInnerOuter1, m_oMaxRatioInnerOuter1, m_oMinStandardDeviation1, m_oMaxStandardDeviation1, m_oMinDevelopedLength1, m_oMaxDevelopedLength1, m_oActiveParams1);
            if (m_oActiveParamSets > 2)
            {
                totalErrorCounter += 100 * checkForErrors(rGeoPoorPenetrationCandidateArrayIn, m_oMinLength2, m_oMaxLength2, m_oMinWidth2, m_oMaxWidth2, m_oMinGradient2, m_oMaxGradient2,
                                                m_oMinGreyvalGap2, m_oMaxGreyvalGap2, m_oMinRatioInnerOuter2, m_oMaxRatioInnerOuter2, m_oMinStandardDeviation2, m_oMaxStandardDeviation2, m_oMinDevelopedLength2, m_oMaxDevelopedLength2, m_oActiveParams2);
            }
        }
    }

    geo2d::Doublearray oOut;
    int oOutRank = eRankMax;
    oOut.assign(1);
    oOut[0] = std::tie(totalErrorCounter, oOutRank);

    const interface::GeoDoublearray oGeoDoubleOut(rGeoPoorPenetrationCandidateArrayIn.context(), oOut,
        rGeoPoorPenetrationCandidateArrayIn.analysisResult(), rGeoPoorPenetrationCandidateArrayIn.rank());

    preSignalAction();
    m_oPipeOutData.signal(oGeoDoubleOut);
}

int PoorPenetrationCheckerTriple::checkForErrors(const interface::GeoPoorPenetrationCandidatearray &candidates, int minLength, int maxLength,
    int minWidth, int maxWidth, int minGradient, int maxGradient, int minGreyvalGap, int maxGreyvalGap, int minRatioInnerOuter,
    int maxRatioInnerOuter, int minStandardDeviation, int maxStandardDeviation, int minDevelopedLength, int maxDevelopedLength, int activParams
)
{
    geo2d::PoorPenetrationCandidate oInCandidate;

    auto color = image::Color::Black();
    const auto darkGreen = image::Color(0, 150, 0);

    int valueToCheck, min, max;
    int singleErrorCounter = 0, totalErrorCounter = 0, result = 0;

    for (size_t i = 0; i < candidates.ref().size(); i++)
    {
        std::vector<InfoLine> _infoLinesPerPP;
        singleErrorCounter = 0;
        result = 0;

        // get the data
        oInCandidate = std::get<eData>(candidates.ref()[i]);

        // 1. Check Length
        color = darkGreen;
        valueToCheck = oInCandidate.m_oLength;
        min = minLength;
        max = maxLength;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
        }
        else if (result == 0)
        {
            result = 1;
        }
        _infoLinesPerPP.push_back(InfoLine(1, valueToCheck, color));

        // 2. Check Width
        color = darkGreen;
        valueToCheck = oInCandidate.m_oWidth;
        min = minWidth;
        max = maxWidth;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
        }
        else if (result == 0)
        {
            result = 2;
        }
        _infoLinesPerPP.push_back(InfoLine(2, valueToCheck, color));

        // 3. Check Gradient
        color = darkGreen;
        valueToCheck = oInCandidate.m_oGradient;
        min = minGradient;
        max = maxGradient;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
        }
        else if (result == 0)
        {
            result = 3;
        }
        _infoLinesPerPP.push_back(InfoLine(3, valueToCheck, color));

        // 4. Check GreyvalGap
        color = darkGreen;
        valueToCheck = oInCandidate.m_oGreyvalGap;
        min = minGreyvalGap;
        max = maxGreyvalGap;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
        }
        else if (result == 0)
        {
            result = 4;
        }
        _infoLinesPerPP.push_back(InfoLine(4, valueToCheck, color));

        // 5. Check RatioInnerOuter
        color = darkGreen;
        if (oInCandidate.m_oGreyvalInner > 0)
        {
            valueToCheck = (int) std::round((10.0 * oInCandidate.m_oGreyvalOuter) / oInCandidate.m_oGreyvalInner);
        }
        else
        {
            valueToCheck = 0;
        }
        min = minRatioInnerOuter;
        max = maxRatioInnerOuter;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
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
        min = minStandardDeviation;
        max = maxStandardDeviation;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
        }
        else if (result == 0)
        {
            result = 6;
        }
        _infoLinesPerPP.push_back(InfoLine(7, valueToCheck, color));

        // 7. Check DevelopedLength
        // changed to the mean value
        color = darkGreen;
        valueToCheck = (oInCandidate.m_oDevelopedLengthLeft + oInCandidate.m_oDevelopedLengthRight) * 0.5;
        min = minDevelopedLength;
        max = maxDevelopedLength;
        if ((valueToCheck >= min) && (valueToCheck <= max))
        {
            singleErrorCounter++;
            color = image::Color::Red();
        }
        else if (result == 0)
        {
            result = 7;
        }
        _infoLinesPerPP.push_back(InfoLine(8, oInCandidate.m_oDevelopedLengthLeft, color));
        _infoLinesPerPP.push_back(InfoLine(9, oInCandidate.m_oDevelopedLengthRight, color));

        color = darkGreen;
        if (singleErrorCounter >= activParams)
        {
            color = image::Color::Red();
            totalErrorCounter++;
            m_overlay.addRectangle(oInCandidate.xmin, oInCandidate.ymin, oInCandidate.xmax - oInCandidate.xmin, oInCandidate.ymax - oInCandidate.ymin, image::Color::Red());
        }
        _infoLinesPerPP.push_back(InfoLine(10, result, color));

        m_allInfoLines.push_back(_infoLinesPerPP);
    }

    return totalErrorCounter;
}

int PoorPenetrationCheckerTriple::getNumberOfStoredPP(const geo2d::PoorPenetrationCandidate& cand, const std::vector<geo2d::PoorPenetrationCandidate> allPP)
{
    int count = 0;

    for (auto storedCand : allPP)
    {
        if ((storedCand.xmin == cand.xmin) && (storedCand.ymin == cand.ymin) && (storedCand.xmax == cand.xmax) && (storedCand.ymax == cand.ymax))
        {
            count++;
        }
    }

    return count;
}
}
}
