#include "lineSelectLocalExtremum.h"
#include "filter/algoArray.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include <fliplib/Parameter.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {
namespace filter {



const std::string LineSelectLocalExtremum::m_oFilterName 	= std::string("LineSelectLocalExtremum");
const std::string LineSelectLocalExtremum::PIPENAME_OUT1  	= std::string("PositionX");
const std::string LineSelectLocalExtremum::PIPENAME_OUT2	    = std::string("PositionY");

//constants for the mask
static const int INVALID_MASK_ELEMENT = -3;
static const int UNASSIGNED_MASK_ELEMENT = -2;
static const int DISCARDED_PEAK = -1;
static const auto isUnassignedElement = [](int maskElement ){ return maskElement == UNASSIGNED_MASK_ELEMENT;};


LineSelectLocalExtremum::LineSelectLocalExtremum()
    :TransformFilter(m_oFilterName, Poco::UUID{"9E26E6F6-88B5-414D-AA6E-5AD562B5400A"}),
    m_pPipeLineIn(nullptr),
    m_oPipePositionXOut	(this, PIPENAME_OUT1),
    m_oPipePositionYOut	(this, PIPENAME_OUT2),
    m_oPipeStartPeakX (this, "StartPeakX"),
    m_oPipeEndPeakX (this, "EndPeakX"),
    m_oExtremumType(eMaximum),
    m_oExtremumNumber(1),
    m_oExtremumDistance  (1),
    m_oExtremumDifference(0.)
{
    parameters_.add("ExtremumType", fliplib::Parameter::TYPE_int, static_cast<int>(m_oExtremumType));
    parameters_.add("ExtremumNumber", fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oExtremumNumber));
    parameters_.add("ExtremumDistance", fliplib::Parameter::TYPE_uint, static_cast<unsigned int>(m_oExtremumDistance));
    parameters_.add("ExtremumDifference", fliplib::Parameter::TYPE_double, static_cast<double>(m_oExtremumDifference));

    setInPipeConnectors({{Poco::UUID("409373e8-e132-4502-b10f-9e24e818d199"), m_pPipeLineIn, "Line", 1, ""}});
    setOutPipeConnectors({
        {Poco::UUID("a75fba00-8908-4240-8f1e-5ccd4ca6de06"), &m_oPipePositionXOut, PIPENAME_OUT1, 0, ""},
        {Poco::UUID("46093401-1e83-42ae-950a-cb294affcf44"), &m_oPipePositionYOut, PIPENAME_OUT2, 0, ""},
        {Poco::UUID("9e15374a-34c9-457d-933d-07ebc4a46f79"), &m_oPipeStartPeakX},
        {Poco::UUID("1042d6b4-16e9-4b1b-96d4-6338692642d2"), &m_oPipeEndPeakX}
    });
    setVariantID(Poco::UUID("4f844cf8-e7c9-4f2b-a94b-2e3fa91478c8"));
}

void LineSelectLocalExtremum::setParameter()
{
    TransformFilter::setParameter();
    m_oExtremumType = static_cast<ExtremumType>(parameters_.getParameter("ExtremumType").convert<int>());
    m_oExtremumNumber = static_cast<unsigned int>(parameters_.getParameter("ExtremumNumber").convert<unsigned int>());
    m_oExtremumDistance = static_cast<unsigned int>(parameters_.getParameter("ExtremumDistance").convert<unsigned int>());
    m_oExtremumDifference = static_cast<double>(parameters_.getParameter("ExtremumDifference").convert<double>());
    if (m_oExtremumDifference < 0)
    {
        wmLog(eWarning, "ExtremumDifference can't be negative, set to 0 \n");    //it should never happen, min value in attributes.json is 0
        m_oExtremumDifference = 0;
    }
}


int LineSelectLocalExtremum::indexMaxUnassignedArrayElement(const std::vector<double> & rLine, std::vector<int> mask)
{
    double yMax = 0.0;
    int x = -1;

    auto lineLength = rLine.size();
    std::size_t xValidEnd = 0;
    std::size_t xValidStart = xValidEnd;
    while (xValidStart < lineLength)
    {
        std::tie(xValidStart, xValidEnd) = searchFirstValidArrayRange(mask, xValidStart,  isUnassignedElement);
        if (xValidStart >= lineLength)
        {
            break;
        }

        auto itMax = std::max_element(rLine.begin() + xValidStart, rLine.begin() + xValidEnd);

        if (*itMax > yMax || x == -1)
        {
            yMax = *itMax;
            x = static_cast<int>(std::distance(rLine.begin(), itMax));
        }

        xValidStart = xValidEnd;
    }
    return x;
}

int LineSelectLocalExtremum::indexMinUnassignedArrayElement(const std::vector<double> & rLine, std::vector<int> mask)
{
    double yMin = 0.0;
    int x = -1;

    auto lineLength = rLine.size();
    std::size_t xValidEnd = 0;
    std::size_t xValidStart = xValidEnd;
    while (xValidStart < lineLength)
    {
        std::tie(xValidStart, xValidEnd) = searchFirstValidArrayRange(mask, xValidStart, isUnassignedElement);
        if (xValidStart >= lineLength)
        {
            break;
        }

        auto itMin = std::min_element(rLine.begin() + xValidStart, rLine.begin() + xValidEnd);

        if (*itMin < yMin || x == -1)
        {
            yMin = *itMin;
            x = static_cast<int>(std::distance(rLine.begin(), itMin));
        }

        xValidStart = xValidEnd;
    }
    return x;
}

template<ExtremumType extremumType>
std::tuple<std::vector<geo2d::DPoint>, double, double> LineSelectLocalExtremum::computePeaks(const std::vector<double> & rLine, std::vector<int> & mask, bool maskCurrentPeak) const
{

    static_assert(extremumType == ExtremumType::eMaximum || extremumType == ExtremumType::eMinimum, "");

    const auto & numPeaks = m_oExtremumNumber;

    std::vector<geo2d::DPoint> peaks;
    peaks.reserve(numPeaks);
    assert(mask.size() == rLine.size());

    auto isOnSlope = [&rLine, &mask](int x, int deltaX, double oldY, double yTolerance)
    {
        assert(yTolerance >= 0);

        int xStart = (deltaX > 0) ? x : x + deltaX;
        int xEnd = (deltaX > 0) ? x + deltaX : x;
        assert(xStart < xEnd);
        assert(xStart >= 0 && "calling code hasn't checked minXLeft");
        assert(xEnd < int(mask.size()) && "calling code hasn't checked maxXRight");

        double ySum = 0.0;
        int yCount = 0;
        for (int x = xStart; x < xEnd; x++)
        {
            if (mask[x] == UNASSIGNED_MASK_ELEMENT)
            {
                ySum += rLine[x];
                yCount++;
            }
        }
        if (yCount == 0)
        {
            return false;
        }

        double yAvg = ySum / yCount;

        switch (extremumType)
        {
        case ExtremumType::eMaximum:
            return (yAvg <= (oldY + yTolerance));     //from the local maximum, values go down
        case ExtremumType::eMinimum:
            return (yAvg >= (oldY - yTolerance));     //from the local minimum, values go up
        }
    };


    int xPeakIndex = -1;
    int maxXRight = (rLine.size() - m_oExtremumDistance);
    int minXLeft = m_oExtremumDistance;
    auto isPeakN = false;
    int leftIndexPeakN = xPeakIndex;
    int rightIndexPeakN = xPeakIndex;
    while (peaks.size() < numPeaks)
    {
        if (extremumType == ExtremumType::eMaximum)
        {
            xPeakIndex = indexMaxUnassignedArrayElement(rLine, mask);
        }
        else
        {
            xPeakIndex = indexMinUnassignedArrayElement(rLine, mask);
        }

        if (xPeakIndex == -1)
        {
            //all points are invalid or assigned to a peak
            break;
        }

        auto yPeak = rLine[xPeakIndex];

        //in case of flat line, refine the peak position
        int leftXPeakIndex = xPeakIndex - 1;
        int rightXPeakIndex = xPeakIndex + 1;
        while (leftXPeakIndex >= 0 && std::abs(rLine[leftXPeakIndex] - yPeak) <= m_oExtremumDifference)
        {
            leftXPeakIndex --;
        }
        leftXPeakIndex++;
        while (rightXPeakIndex < static_cast<int>(rLine.size()) && std::abs(rLine[rightXPeakIndex] - yPeak) <= m_oExtremumDifference)
        {
            rightXPeakIndex++;
        }
        rightXPeakIndex--;

        double xPeak = 0.5 * (leftXPeakIndex + rightXPeakIndex);   //actual peak position (subpixel)

        //check if there is a slope on the left and the right of the current point
        bool isRealPeak = rightXPeakIndex < maxXRight
                          && leftXPeakIndex > minXLeft
                          && isOnSlope(rightXPeakIndex, m_oExtremumDistance, yPeak, m_oExtremumDifference)
                          && isOnSlope(leftXPeakIndex, -m_oExtremumDistance, yPeak, m_oExtremumDifference);

        int currentPeakLabel = peaks.size();
        assert(currentPeakLabel != INVALID_MASK_ELEMENT);
        assert(currentPeakLabel != UNASSIGNED_MASK_ELEMENT);

        if (isRealPeak)
        {
            peaks.emplace_back(xPeak, yPeak);

            // when using y tolerance, the x position can be shifted
            assert(m_oExtremumDifference != 0.0 || rLine[xPeak] == rLine[xPeakIndex]);
            assert(std::abs(rLine[xPeak] - yPeak) <= m_oExtremumDifference);

            isPeakN = (peaks.size() == numPeaks);
            if (isPeakN && !maskCurrentPeak)
            {
                //the desired peak has been found, do not mask the corresponding points if not requested
                break;
            }
        }
        else
        {
            currentPeakLabel = DISCARDED_PEAK;
        }

        //mask the points on the current peak
        for (int x = leftXPeakIndex; x <= rightXPeakIndex; x++)
        {
            mask[x] = currentPeakLabel;
        }

        //mask the points on the right of the current peak
        auto prevY = yPeak;
        int x = rightXPeakIndex + 1;
        while (x < maxXRight && isOnSlope(x, m_oExtremumDistance, prevY, m_oExtremumDifference))
        {
            mask[x] = currentPeakLabel;
            prevY = rLine[x];
            x++;
        }
        //fill the last window
        int lastX = std::min<int> (x + m_oExtremumDistance - 1, rLine.size() - 1);
        while (x < lastX && (mask[x] == UNASSIGNED_MASK_ELEMENT))
        {
            mask[x] = currentPeakLabel;
            x++;
        }
        if (isPeakN)
        {
            rightIndexPeakN = x - 1;
        }
        //mask the points on the left of the current peak
        prevY = yPeak;
        x = leftXPeakIndex - 1;
        while (x > minXLeft && isOnSlope(x, -m_oExtremumDistance, prevY, m_oExtremumDifference))
        {
            mask[x] = currentPeakLabel;
            prevY = rLine[x];
            x--;
        }
        //fill the last window
        lastX = std::max<int> (x - m_oExtremumDistance + 1, 0);
        while (x > lastX && (mask[x] == UNASSIGNED_MASK_ELEMENT))
        {
            mask[x] = currentPeakLabel;
            x--;
        }
        if (isPeakN)
        {
            leftIndexPeakN = x+1;
        }
    }
    return {peaks, leftIndexPeakN, rightIndexPeakN};
}

bool LineSelectLocalExtremum::subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup)
{
    m_pPipeLineIn  = dynamic_cast< fliplib::SynchronePipe < interface::GeoVecDoublearray > * >(&p_rPipe);
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void LineSelectLocalExtremum::proceedGroup(const void * p_pSender, fliplib::PipeGroupEventArgs & p_rEventArg)
{
    poco_assert_dbg(m_pPipeLineIn != nullptr);    // to be asserted by graph editor

    const auto & rLineIn = m_pPipeLineIn->read(m_oCounter);
    if (rLineIn.rank() == interface::NotPresent || rLineIn.ref().empty())
    {
        m_oSpTrafo = nullptr;
        geo2d::Doublearray nullOutput(0);
        const interface::GeoDoublearray oGeoDoublearrayXOut(rLineIn.context(), nullOutput, rLineIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoDoublearrayYOut(rLineIn.context(), nullOutput, rLineIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoDoublearrayStartOut(rLineIn.context(), nullOutput, rLineIn.analysisResult(), interface::NotPresent);
        const interface::GeoDoublearray oGeoDoublearrayEndOut(rLineIn.context(), nullOutput, rLineIn.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_oPipePositionXOut.signal(oGeoDoublearrayXOut);
        m_oPipePositionYOut.signal(oGeoDoublearrayYOut);
        m_oPipeStartPeakX.signal(oGeoDoublearrayStartOut);
        m_oPipeEndPeakX.signal(oGeoDoublearrayEndOut);

        return;
    }

    m_oSpTrafo  = rLineIn.context().trafo();
    const auto & rArrayIn = rLineIn.ref();
    auto oNbLines = rArrayIn.size();

    geo2d::Doublearray outX, outY, outStart, outEnd;
    outX.reserve(oNbLines);
    outY.reserve(oNbLines);
    outStart.reserve(oNbLines);
    outEnd.reserve(oNbLines);

    unsigned int paintProfileLineIndex = 0;
    bool computePeakLimits = m_oPipeStartPeakX.linked() || m_oPipeEndPeakX.linked();
    bool paintMask = m_oVerbosity >= eMax;
    bool maskResultPeak = paintMask || computePeakLimits;

    for (auto lineN = 0u; lineN < oNbLines; lineN++)
    {
        const auto & rData = rArrayIn[lineN].getData();

        //initialize the mask (which will hold the peaks label) with the rank
        auto & rRank = rArrayIn[lineN].getRank();
        m_maskCache.resize(rData.size());
        std::transform(rRank.begin(), rRank.end(), m_maskCache.begin(), [](int rank)
        {
            return (rank == eRankMin) ? INVALID_MASK_ELEMENT : UNASSIGNED_MASK_ELEMENT ;
        });

        auto [peaks, leftIndexPeakN, rightIndexPeakN] = m_oExtremumType == ExtremumType::eMaximum ?
                     computePeaks<ExtremumType::eMaximum> (rData, m_maskCache, maskResultPeak)
                     : computePeaks<ExtremumType::eMinimum> (rData, m_maskCache, maskResultPeak);

        bool valid = peaks.size() >= m_oExtremumNumber;
        if (valid)
        {
            auto & rPeakN = peaks[m_oExtremumNumber - 1];
            outX.push_back(rPeakN.x, eRankMax);
            outY.push_back(rPeakN.y, eRankMax);
            if (computePeakLimits)
            {
#ifndef NDEBUG
                auto hasPeakLabel = [peakLabel = m_maskCache[rPeakN.x]](int label){return label == peakLabel;};
                assert(std::none_of(m_maskCache.begin(), m_maskCache.begin() + leftIndexPeakN, hasPeakLabel));
                assert(std::all_of(m_maskCache.begin() + leftIndexPeakN, m_maskCache.begin() + rightIndexPeakN, hasPeakLabel));
                assert(std::none_of(m_maskCache.begin() + rightIndexPeakN + 1, m_maskCache.end(), hasPeakLabel));
#endif
                outStart.push_back(leftIndexPeakN, eRankMax);
                outEnd.push_back(rightIndexPeakN, eRankMax);
            }
            else
            {
                outStart.push_back(rPeakN.x, eRankMin);
                outEnd.push_back(rPeakN.x, eRankMin);
            }

        }
        else
        {
            outX.push_back(0.0, eRankMin);
            outY.push_back(0.0, eRankMin);
            outStart.push_back(0.0, eRankMin);
            outEnd.push_back(0.0, eRankMin);
        }

        if (lineN == paintProfileLineIndex && m_oVerbosity >= eLow)
        {
            if (paintMask)
            {
                m_paintMask = m_maskCache;
                m_paintLine = rArrayIn[lineN].getData();
            }
            else
            {
                m_paintMask.clear();
                m_paintLine.clear();
            }
            m_paintPeaks.clear();
            for (auto peak : peaks)
            {
                m_paintPeaks.emplace_back(peak.x, peak.y);
            }
            m_paintValid = valid;
        }
    }

    const auto oAnalysisResult  = rLineIn.analysisResult() == interface::AnalysisOK ? interface::AnalysisOK : rLineIn.analysisResult(); // replace 2nd AnalysisOK by your result type
    const interface::GeoDoublearray oGeoDoublearrayXOut(rLineIn.context(), outX, oAnalysisResult, interface::Limit);    // full rank here, detailed rank in array
    const interface::GeoDoublearray oGeoDoublearrayYOut(rLineIn.context(), outY, oAnalysisResult, interface::Limit);    // full rank here, detailed rank in array
    const interface::GeoDoublearray oGeoDoublearrayStartOut(rLineIn.context(), outStart, oAnalysisResult, interface::Limit);    // full rank here, detailed rank in array
    const interface::GeoDoublearray oGeoDoublearrayEndOut(rLineIn.context(), outEnd, oAnalysisResult, interface::Limit);    // full rank here, detailed rank in array
    preSignalAction();
    m_oPipePositionXOut.signal(oGeoDoublearrayXOut);
    m_oPipePositionYOut.signal(oGeoDoublearrayYOut);
    m_oPipeStartPeakX.signal(oGeoDoublearrayStartOut);
    m_oPipeEndPeakX.signal(oGeoDoublearrayEndOut);
}


void LineSelectLocalExtremum::paint()
{
    using namespace image;

    if (m_oVerbosity < eLow  || m_oSpTrafo.isNull())
    {
        return;
    }

    const auto & rTrafo(*m_oSpTrafo);
    auto & rCanvas(canvas<OverlayCanvas> (m_oCounter));
    auto & rLayerPosition(rCanvas.getLayerPosition());
    auto & rLayerContour(rCanvas.getLayerContour());
    auto & rLayerText(rCanvas.getLayerText());

    auto peakColor = [](int label)
    {
        static const std::vector<Color> colorMap
        {
            Color::Red(),
            Color::Cyan(),
            Color::Yellow(),
            Color::Green(),
            Color::Magenta(),
            Color::m_oOrange,
            Color::m_oPlum
        };
        switch (label)
        {
        case UNASSIGNED_MASK_ELEMENT:
        case INVALID_MASK_ELEMENT:
            return Color::Blue();
        case DISCARDED_PEAK:
            return Color::m_oButter;
        default:
            return colorMap[label % colorMap.size()];
        }
    };

    auto isValidPoint = [](int label)
    {
        switch (label)
        {
        case UNASSIGNED_MASK_ELEMENT:
        case INVALID_MASK_ELEMENT:
        case DISCARDED_PEAK:
            return false;
        default:
            return true;
        }
    };

    if (!m_paintLine.empty() && m_paintMask.size() == m_paintLine.size())
    {
        int connectedLineLabel = m_paintMask[0];
        int connectedLineStart = 0;
        std::vector<int> connectedLineValues;
        connectedLineValues.reserve(m_paintLine.size());

        for (int x = 0, lastValidX = m_paintLine.size() - 1; x <= lastValidX; x++)
        {
            auto label = m_paintMask[x];
            if ((label != connectedLineLabel || x == lastValidX) && !connectedLineValues.empty())
            {
                //draw what has been accumulated so far
                bool connected = isValidPoint(connectedLineLabel);
                rLayerContour.add<OverlayPointList>(rTrafo(geo2d::Point(connectedLineStart, 0)), connectedLineValues, peakColor(connectedLineLabel), connected);
                connectedLineValues.clear();
            }
            if (connectedLineValues.empty())
            {
                connectedLineLabel = label;
                connectedLineStart = x;
            }
            assert(connectedLineLabel == label);
            connectedLineValues.push_back(m_paintLine[x]);
        }
    }


    for (auto i = 0u; i < m_paintPeaks.size(); i++)
    {
        auto & peak = m_paintPeaks[i];
        auto color = peakColor(i);
        int size = (i == (m_oExtremumNumber - 1)) ? 15 : 5;
        rLayerPosition.add<OverlayCross> (rTrafo(peak), size, color);
        if (m_oVerbosity >= eMedium)
        {
            auto txt = std::to_string(int (peak.y)) + " #" + std::to_string(i + 1);
            rLayerText.add<OverlayText> (txt, Font(14), rTrafo(geo2d::Rect(peak.x, peak.y, 200, 20)), color);
        }
    }

} // paint


}
}
