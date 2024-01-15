#include "lineModelToLaserline.h"
#include "filter/algoArray.h"
#include "overlay/overlayPrimitive.h"
// #include <fliplib/Parameter.h>
#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec
{
namespace filter
{

const std::string LineModelToLaserline::m_filterName = std::string("LineModelToLaserline");
const std::string LineModelToLaserline::m_startInName = std::string("start");
const std::string LineModelToLaserline::m_endInName = std::string("end");

LineModelToLaserline::LineModelToLaserline()
    : TransformFilter(m_filterName, Poco::UUID{"16a3ef15-25e5-42cc-aa7b-df17807ca4ff"})
    , m_pipeEquationIn(nullptr)
    , m_pipeStartIn(nullptr)
    , m_pipeEndIn(nullptr)
    , m_pipeLineOut(this, "LineOut")
    , m_paintColor(0, 255, 0)
{
    parameters_.add("rgbRed", fliplib::Parameter::TYPE_UInt32, static_cast<unsigned int>(m_paintColor.red));
    parameters_.add("rgbGreen", fliplib::Parameter::TYPE_UInt32, static_cast<unsigned int>(m_paintColor.green));
    parameters_.add("rgbBlue", fliplib::Parameter::TYPE_UInt32, static_cast<unsigned int>(m_paintColor.blue));
    parameters_.add("Strategy", fliplib::Parameter::TYPE_int, int(m_strategy));

    setInPipeConnectors({{Poco::UUID("47D76F03-AD99-4599-9E2D-BD9BA403225D"), m_pipeEquationIn, "Line", 1, "line"},
                        {Poco::UUID("DCA0486E-72ED-4BB0-8F97-045572F5AD92"), m_pipeStartIn, "Start", 1, m_startInName},
                        {Poco::UUID("BBF34340-3578-424B-A53B-1844B0E042B5"), m_pipeEndIn, "End", 1, m_endInName}});
    setOutPipeConnectors({{Poco::UUID("99F86633-C71C-4691-8978-C9AF5C6653C8"), &m_pipeLineOut, "LineOut", 0, ""}});
    setVariantID(Poco::UUID("F3FE69AA-7BA9-4F4F-AB0C-14E3B585B440"));
}

void LineModelToLaserline::setParameter()
{
    TransformFilter::setParameter();

    m_paintColor.red = parameters_.getParameter("rgbRed").convert<byte>();
    m_paintColor.green = parameters_.getParameter("rgbGreen").convert<byte>();
    m_paintColor.blue = parameters_.getParameter("rgbBlue").convert<byte>();
    m_strategy = static_cast<Strategy>(parameters_.getParameter("Strategy").convert<int>());
}

bool LineModelToLaserline::subscribe(fliplib::BasePipe & p_rPipe, int p_oGroup)
{
    if (p_rPipe.type() == typeid(interface::GeoLineModelarray))
    {
        m_pipeEquationIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoLineModelarray>*>(&p_rPipe);
    }
    else if (p_rPipe.type() == typeid(interface::GeoDoublearray))
    {
        if (p_rPipe.tag() == m_startInName)
        {
            m_pipeStartIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
        }
        else if (p_rPipe.tag() == m_endInName)
        {
            m_pipeEndIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&p_rPipe);
        }
    }
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void LineModelToLaserline::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    const auto& lineModelIn = m_pipeEquationIn->read(m_oCounter);
    const auto& lineModel = lineModelIn.ref();
    const auto& startIn = m_pipeStartIn->read(m_oCounter).ref();
    const auto& endIn = m_pipeEndIn->read(m_oCounter).ref();

    if (lineModelIn.rank() == interface::NotPresent || lineModelIn.ref().empty())
    {
        wmLog(eDebug, "%s: Bad rank or input empty.", m_filterName);
        m_spTrafo = nullptr;
        geo2d::VecDoublearray nullOutput(0);
        const interface::GeoVecDoublearray geoDoublearrayOut(lineModelIn.context(), nullOutput, lineModelIn.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_pipeLineOut.signal(geoDoublearrayOut);

        return;
    }

    geo2d::VecDoublearray outX;
    outX.resize(lineModel.size());

    const auto paintProfileLineIndex = 0;
    const auto numberLines = lineModel.getData().size();

    const auto& start = startIn.getData();
    const bool useAlwaysFirstStart = !processEntireProfileLines(numberLines, start.size(), m_filterName, m_startInName);
    const auto& end = endIn.getData();
    const bool useAlwaysFirstEnd = !processEntireProfileLines(numberLines, end.size(), m_filterName, m_endInName);

    MultipleIndicesStrategy strategy = MultipleIndicesStrategy::UseFirstIndexes;
    switch (m_strategy)
    {
        case Strategy::UseFirstIndexes:
            strategy = MultipleIndicesStrategy::UseFirstIndexes;
            break;
        case Strategy::ExtractShortestLine:
            strategy = MultipleIndicesStrategy::ExtractShortestLine;
            break;
        case Strategy::ExtractLongestLine:
            strategy = MultipleIndicesStrategy::ExtractLongestLineWithoutPadding;
            break;
    }
    auto globalStart = useAlwaysFirstStart ? start.at(0) : (int)chooseStartIndex(startIn, m_filterName, strategy);
    auto globalEnd = useAlwaysFirstEnd ? end.at(0) : (int)chooseEndIndex(endIn, m_filterName, strategy);

    if (globalEnd <= globalStart)
    {
        wmLog(eDebug, "%s: end is smaller than start\n", m_filterName);
        m_spTrafo = nullptr;
        geo2d::VecDoublearray nullOutput(0);
        const interface::GeoVecDoublearray geoDoublearrayOut(lineModelIn.context(), nullOutput, lineModelIn.analysisResult(), interface::NotPresent);
        preSignalAction();
        m_pipeLineOut.signal(geoDoublearrayOut);

        return;
    }

    for (auto lineN = 0u; lineN < numberLines; lineN++)
    {
        const auto [a, b, c] = lineModel.getData().at(lineN).getCoefficients();
        std::vector<double>& lineOutData(outX.at(lineN).getData());
        lineOutData.clear();
        std::vector<int>& lineOutRank(outX.at(lineN).getRank());
        if (b == 0)
        {
            // vertical line
            lineOutData = {0};
            lineOutRank = {eRankMin};
            continue;
        }
        auto equation = math::LineEquation(a, b, c);
        for (auto i = globalStart; i <= globalEnd; i++)
        {
            lineOutData.push_back(equation.getY(i));
        }
        lineOutRank.assign(lineOutData.size(), lineModel.getRank()[lineN]);
    }

    m_paintLine = outX.at(paintProfileLineIndex).getData();

    m_spTrafo = new interface::LinearTrafo(lineModelIn.context().getTrafoX() + (int)globalStart, lineModelIn.context().getTrafoY());
    interface::ImageContext outContext(lineModelIn.context(), m_spTrafo);

    const interface::GeoVecDoublearray geoDoublearrayOut(outContext, outX, lineModelIn.analysisResult(), lineModelIn.rank());
    preSignalAction();
    m_pipeLineOut.signal(geoDoublearrayOut);
}


void LineModelToLaserline::paint()
{
    if (m_oVerbosity < eLow  || m_spTrafo.isNull() || m_paintLine.empty())
    {
        return;
    }

    image::OverlayCanvas&rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer& rLayerContour(rCanvas.getLayerContour());

    const auto& start = geo2d::Point(m_spTrafo->dx(), m_spTrafo->dy());
    rLayerContour.add<image::OverlayPointList>(start, m_paintLine, m_paintColor, true);
}


}
}
