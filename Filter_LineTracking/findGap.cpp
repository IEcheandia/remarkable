/**
 * @file
 * @copyright   Precitec Vision GmbH & Co. KG
 * @author      MM
 * @date        2022
 * @brief       Laserline tracking filter, finds gap in laseline. If there is no gap, than returns the end of the line.
 */

#include "findGap.h"
#include "module/moduleLogger.h"
#include <filter/armStates.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <filter/algoArray.h>


namespace precitec
{
namespace filter
{

const std::string FindGap::m_filterName = std::string("FindGap");

FindGap::FindGap()
    : TransformFilter(FindGap::m_filterName, Poco::UUID{"9B3BAAD9-12C8-45CE-A95C-F7980E7AE322"})
    , m_pipeInLine(nullptr)
    , m_pipePositionX(nullptr)
    , m_pipePositionY(nullptr)
    , m_searchDirection(SearchDirType::eFromLeft)
    , m_numberValuesForAveraging(15)
    , m_gapWidth(5)
    , m_maxJumpDownY(5)
    , m_maxJumpUpY(20)
    , m_maxJumpX(20)
    , m_color(0, 255, 0)
{
    m_pipePositionX = new fliplib::SynchronePipe<interface::GeoDoublearray>(this, "posX");
    m_pipePositionY = new fliplib::SynchronePipe<interface::GeoDoublearray>(this, "posY");

    parameters_.add("SearchDirection", fliplib::Parameter::TYPE_int, (int)m_searchDirection);
    parameters_.add("MaxJumpDownY", fliplib::Parameter::TYPE_int, m_maxJumpDownY);
    parameters_.add("MaxJumpUpY", fliplib::Parameter::TYPE_int, m_maxJumpUpY);
    parameters_.add("MaxJumpX", fliplib::Parameter::TYPE_int, m_maxJumpX);
    parameters_.add("GapWidth", fliplib::Parameter::TYPE_int, m_gapWidth);
    parameters_.add("NumberValuesForAveraging", fliplib::Parameter::TYPE_int, m_numberValuesForAveraging);
    parameters_.add("rgbRed", fliplib::Parameter::TYPE_int, m_color.red);
    parameters_.add("rgbGreen", fliplib::Parameter::TYPE_int, m_color.green);
    parameters_.add("rgbBlue", fliplib::Parameter::TYPE_int, m_color.blue);

    setInPipeConnectors({{Poco::UUID("98DECC42-CEF8-4052-A61E-08E337B95148"), m_pipeInLine, "Line", 0, "line"}});
    setOutPipeConnectors({{Poco::UUID("FBB1E0B3-3C5D-46C5-B421-4F44A85CBE14"), m_pipePositionX, "posX", 1, ""}, {Poco::UUID("EEC2106D-0DA7-4712-A493-5719724CC903"), m_pipePositionY, "posY", 1, ""}});
    setVariantID(Poco::UUID("B47B81BF-1B9D-49CD-9779-54CAF77775B7"));
}


FindGap::~FindGap() = default;

void FindGap::setParameter()
{
    TransformFilter::setParameter();

    m_searchDirection = static_cast<SearchDirType>(parameters_.getParameter("SearchDirection").convert<int>());
    m_maxJumpDownY = parameters_.getParameter("MaxJumpDownY");
    m_maxJumpUpY = parameters_.getParameter("MaxJumpUpY");
    m_maxJumpX = parameters_.getParameter("MaxJumpX");
    m_gapWidth = parameters_.getParameter("GapWidth");
    m_numberValuesForAveraging = parameters_.getParameter("NumberValuesForAveraging");
    m_color.red = parameters_.getParameter("rgbRed");
    m_color.green = parameters_.getParameter("rgbGreen");
    m_color.blue = parameters_.getParameter("rgbBlue");
}


void FindGap::paint()
{
    if(m_oVerbosity < VerbosityType::eLow || m_smpTrafo.isNull())
    {
        return;
    }

    const interface::Trafo& rTrafo (*m_smpTrafo);
    image::OverlayCanvas& rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer& rLayerContour (rCanvas.getLayerContour());

    rLayerContour.add<image::OverlayCross>(rTrafo(m_result), m_color);
}

bool FindGap::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "line")
    {
        m_pipeInLine = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}


void FindGap::proceed(const void* sender, fliplib::PipeEventArgs& e)
{
    const auto& lineIn = m_pipeInLine->read(m_oCounter);
    m_smpTrafo = lineIn.context().trafo();
    m_result = geo2d::Point(0, 0);
    m_outX = geo2d::Doublearray();
    m_outY = geo2d::Doublearray();

    if (!lineIn.isValid() || lineIn.ref().empty())
    {
        const auto& geoPositionOut = interface::GeoDoublearray(lineIn.context(), geo2d::Doublearray(), lineIn.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_pipePositionX->signal(geoPositionOut);
        m_pipePositionY->signal(geoPositionOut);

        return;
    }

    const auto& lines = lineIn.ref();
    const int numberLines(lines.size());
    const int profileLine(0);

    for (auto lineN = 0; lineN < numberLines; lineN++)
    {
        const auto& line = lines.at(lineN).getData();
        const int lineSize(line.size());
        if (lineSize < m_numberValuesForAveraging)
        {
            if (m_oVerbosity == VerbosityType::eMax)
            {
                wmLog(eDebug, "%s: Line %d is smaller than number of values for averaging and will be skipped because of that.\n", m_filterName, lineN);
            }
            setDefaultOut(line);
            continue;
        }

        geo2d::Point lastLinePos;
        double mean(0);
        std::deque<int> meanValuesIndices{}; ///< Indices of the values which are used to compute mean. Ensures to compute a correct mean, because not every point of line has to be used to compute mean.
        uint counter(0);
        if (m_searchDirection == SearchDirType::eFromLeft)
        {
            lastLinePos = geo2d::Point(0, line.front());
            int counterUsedValues(0);
            while (counterUsedValues < m_numberValuesForAveraging && counter < line.size())
            {
                if (lines.at(lineN).getRank().at(counter) == eRankMax)
                {
                    mean += line.at(counter) / m_numberValuesForAveraging;
                    meanValuesIndices.push_back(counter);
                    counterUsedValues++;
                }
                counter++;
            }
        }
        else
        {
            lastLinePos = geo2d::Point(lineSize - 1, line.back());
            int counterUsedValues(0);
            while (counterUsedValues < m_numberValuesForAveraging && counter < line.size())
            {
                if (lines.at(lineN).getRank().at(lineSize - 1 - counter) == eRankMax)
                {
                    mean += line.at(lineSize - 1 - counter) / m_numberValuesForAveraging;
                    meanValuesIndices.push_back(lineSize - 1 - counter);
                    counterUsedValues++;
                }
                counter++;
            }
        }

        if (counter == line.size())
        {
            if (m_oVerbosity == VerbosityType::eMax)
            {
                wmLog(eDebug, "%s: Line %d will not proceed because of bad rank.\n", m_filterName, lineN);
            }
            setDefaultOut(line);
            continue;
        }

        int gapWidth(0);
        const int increment = m_searchDirection == SearchDirType::eFromLeft ? 1 : -1;
        const int start = m_searchDirection == SearchDirType::eFromLeft ? counter : lineSize - counter - 1;
        const int end = m_searchDirection == SearchDirType::eFromLeft ? line.size() - 1 : 0;
        for (auto i = start; i != end; i += increment)
        {
            double expectedAfterGapValue = 0;
            if (gapWidth > m_gapWidth && gapWidth <= m_maxJumpX && m_numberValuesForAveraging > 0)
            {
                // skip some bigger bumps or smaller gaps, if they seem not to be the gap between the blanks. Based on that the blanks are on a different height.
                // Really simple line fitting, if something more robust is needed use LineFitter (in LineGeometry) or something like that.
                if (m_numberValuesForAveraging > 1)
                {
                    const int x1 = meanValuesIndices.front();
                    const int x2 = meanValuesIndices.back();
                    const double y1 = line.at(x1);
                    const double y2 = line.at(x2);
                    const double a = (y1 - y2) / (x1 - x2);
                    const double b = y1 - a * x1;
                    expectedAfterGapValue = a * (i + increment) + b;
                }
                else
                {
                    expectedAfterGapValue = line.at(meanValuesIndices.back());
                }
            }
            const double diff = expectedAfterGapValue ? line.at(i + increment) - expectedAfterGapValue : line.at(i + increment) - mean;
            const bool noJump = (diff >= 0 && diff <= m_maxJumpDownY) || (diff < 0 && std::abs(diff) <= m_maxJumpUpY);
            if (noJump && lines.at(lineN).getRank().at(i + increment) == eRankMax)
            {
                meanValuesIndices.push_back(i + increment);
                mean = mean - line.at(meanValuesIndices.front()) / m_numberValuesForAveraging + line.at(meanValuesIndices.back()) / m_numberValuesForAveraging;
                meanValuesIndices.pop_front();
                gapWidth = 0;
                lastLinePos = geo2d::Point(i + increment, line.at(i + increment));
                continue;
            }
            gapWidth++;
            if (gapWidth > m_gapWidth && (gapWidth > m_maxJumpX || m_numberValuesForAveraging == 1))
            {
                break;
            }
        }
        if (lineN == profileLine)
        {
            m_result = lastLinePos;
        }
        m_outX.getData().push_back(lastLinePos.x);
        m_outX.getRank().push_back(interface::Limit);
        m_outY.getData().push_back(lastLinePos.y);
        m_outY.getRank().push_back(interface::Limit);
    }

    const interface::GeoDoublearray& geoOutPositionX = interface::GeoDoublearray(lineIn.context(), m_outX, lineIn.analysisResult(), interface::Limit);
    const interface::GeoDoublearray& geoOutPositionY = interface::GeoDoublearray(lineIn.context(), m_outY, lineIn.analysisResult(), interface::Limit);
    preSignalAction();
    m_pipePositionX->signal(geoOutPositionX);
    m_pipePositionY->signal(geoOutPositionY);
}

void FindGap::setDefaultOut(const std::vector<double>& line)
{
    const uint lineSize(line.size());
    if (lineSize == 0u)
    {
        m_outX.getData().push_back(0);
        m_outY.getData().push_back(0);
    }
    else if (m_searchDirection == SearchDirType::eFromLeft)
    {
        m_outX.getData().push_back(lineSize - 1);
        m_outY.getData().push_back(line.back());
    }
    else
    {
        m_outX.getData().push_back(0);
        m_outY.getData().push_back(line.front());
    }
    m_outX.getRank().push_back(interface::NotPresent);
    m_outY.getRank().push_back(interface::NotPresent);
}

void FindGap::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        m_smpTrafo = nullptr;
    }
}

}
}
