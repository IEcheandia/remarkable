#include "lineTemporalFilter.h"

#include <module/moduleLogger.h>
#include <geo/array.h>
#include <filter/armStates.h>
#include <filter/algoArray.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <overlay/overlayCanvas.h>
#include <overlay/overlayPrimitive.h>

namespace precitec
{
namespace filter
{

// assumption: the input lines always have the same length, but linesBlock size is not checked
class LineTemporalImpl
{
public:
    enum Operation
    {
        eMean,
        eVariance,
        eValidity
    };
    LineTemporalImpl(Operation operation, std::size_t numBlocks)
    {
        setParameters(operation, numBlocks);
    }
    ~LineTemporalImpl() = default;

    bool isResultValid() const
    {
        return m_validResult;
    }
    const geo2d::Doublearray& getResult() const
    {
        return m_result;
    }

    std::size_t getNumBlocks() const
    {
        return m_numBlocks;
    }

    Operation getOperation() const
    {
        return m_operation;
    }

    void setParameters(Operation operation, std::size_t numBlocks)
    {
        m_numBlocks = numBlocks;
        m_operation = operation;
        reset();
    }
    void reset()
    {
        m_validResult = false;
        m_lineBlocksBuffer.clear();
        resetSubTotals(m_lineLength);
    }

    bool updateResult(const std::vector<geo2d::Doublearray>& linesBlock)
    {
        bool valid = true;

        // remove the old blocks from buffer
        while (m_lineBlocksBuffer.size() > (m_numBlocks - 1))
        {
            m_lineBlocksBuffer.pop_front();
        }

        // let's not reset everything if we get an empty block, but if the line length has changed we need to do something
        auto currentLineLength = linesBlock.size() > 0 ? linesBlock.front().size() : m_lineLength;

        if (currentLineLength != m_lineLength)
        {
            valid = m_lineBlocksBuffer.empty();
            // we can't combine line with different lengths, we compute something but with bad rank
        }
        resetSubTotals(currentLineLength);

        // process the old blocks and the new one
        assert(m_lineBlocksBuffer.size() <= m_numBlocks - 1);
        for (const auto& linesBlockFromBuffer : m_lineBlocksBuffer)
        {
            valid &= addToSubtotals(linesBlockFromBuffer);
        }
        valid &= addToSubtotals(linesBlock);

        // if needed, add the current block to the buffer
        if (m_numBlocks > 1)
        {
            m_lineBlocksBuffer.push_back(linesBlock);
        }

        // compute the result
        switch (m_operation)
        {
        case Operation::eMean:
            computeMean(m_result);
            break;
        case Operation::eVariance:
            computeVariance(m_result);
            break;
        case Operation::eValidity:
            computeValidity(m_result);
            break;
        default:
            return false;
        }
        m_validResult = valid;
        return m_validResult;
    }


private:
    void resetSubTotals(std::size_t lineLength)
    {
        m_validResult = false;
        m_lineLength = lineLength;
        m_counter.assign(m_lineLength, 0);
        m_validCounter.assign(m_lineLength, 0);
        m_sum.assign(m_lineLength, 0.0);
        m_sumSquare.assign(m_lineLength, 0.0);
    }

    bool addToSubtotals(const std::vector<geo2d::Doublearray>& linesBlock)
    {
        for (const auto& line : linesBlock)
        {
            if (line.size() != m_lineLength)
            {
                return false;
            }
            for (auto x = 0u; x < m_lineLength; x++)
            {
                if (line.getRank()[x] > 0)
                {
                    auto value = line.getData()[x];
                    m_sum[x] += value;
                    m_sumSquare[x] += (value * value);
                    m_validCounter[x]++;
                }
                m_counter[x]++;
            }
        }
        return true;
    }

    void computeMean(geo2d::Doublearray& meanLine) const
    {
        meanLine.resize(m_lineLength);
        for (auto x = 0u; x < m_lineLength; x++)
        {
            if (m_validCounter[x] > 0)
            {
                auto mean = m_sum[x] / m_validCounter[x];
                meanLine.getData()[x] = mean;
                meanLine.getRank()[x] = eRankMax;
            }
            else
            {
                meanLine.getData()[x] = 0.0;
                meanLine.getRank()[x] = eRankMin;
            }
        }
    }

    void computeVariance(geo2d::Doublearray& varianceLine) const
    {
        varianceLine.resize(m_lineLength);
        for (auto x = 0u; x < m_lineLength; x++)
        {
            if (m_validCounter[x] > 1)
            {
                auto mean = m_sum[x] / m_validCounter[x];
                auto variance = (m_sumSquare[x] - mean * m_sum[x]) / (m_validCounter[x] - 1);

                varianceLine.getData()[x] = variance;
                varianceLine.getRank()[x] = eRankMax;
            }
            else
            {
                varianceLine.getData()[x] = 0.0;
                varianceLine.getRank()[x] = eRankMin;
            }
        }
    }

    void computeValidity(geo2d::Doublearray& validityLine) const
    {
        validityLine.resize(m_lineLength);

        //compute the ratio of valid points
        for (auto x = 0u; x < m_lineLength; x++)
        {
            auto totPoints = m_counter[x];
            if (totPoints > 0)
            {
                validityLine[x] = std::make_tuple(m_validCounter[x] / double(totPoints), eRankMax);
            }
            else
            {
                validityLine[x] = std::make_tuple(0.0, eRankMin);
            }
        }
    }

    bool m_validResult = false;
    geo2d::Doublearray m_result;

    // parameters
    Operation m_operation;
    std::size_t m_numBlocks;
    // buffer info
    std::deque<geo2d::VecDoublearray> m_lineBlocksBuffer; //buffer containing the received blocks, 1 block for every image

    //counter, subTotals, intermediate computations
    std::size_t m_lineLength = 800;
    std::vector<unsigned int> m_counter;
    std::vector<unsigned int> m_validCounter;
    std::vector<double> m_sum;
    std::vector<double> m_sumSquare;
};

LineTemporalFilter::LineTemporalFilter()
    : TransformFilter("LineTemporalFilter", Poco::UUID{"bc0e5f47-9018-4f31-bde3-51d6842b5b4e"})
    , m_pipeIn(nullptr)
    , m_pipeOut(this, "Result")
    , m_impl(std::make_unique<LineTemporalImpl>(LineTemporalImpl::Operation::eMean, 1))
{
    parameters_.add("Operation", fliplib::Parameter::TYPE_int, static_cast<int>(m_impl->getOperation()));
    parameters_.add("NumImages", fliplib::Parameter::TYPE_int, m_impl->getNumBlocks());
    parameters_.add("StartImage", fliplib::Parameter::TYPE_int, m_startImage);
    setInPipeConnectors({{Poco::UUID("1573c3c4-102d-48f5-a206-28bac50ea53e"), m_pipeIn, "input", 1}});
    setOutPipeConnectors({{Poco::UUID("A50A7E80-86AC-4E63-B6B6-791C67669760"), &m_pipeOut}});
    setVariantID(Poco::UUID{"68DEA6C4-30A3-4A2E-9B59-55F6A7483227"});
}

LineTemporalFilter::~LineTemporalFilter() = default;

bool LineTemporalFilter::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    m_pipeIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&p_rPipe);
    return BaseFilter::subscribe(p_rPipe, p_oGroup);
}

void LineTemporalFilter::setParameter()
{
    TransformFilter::setParameter();
    auto operation = static_cast<LineTemporalImpl::Operation>(parameters_.getParameter("Operation").convert<int>());
    auto numImages = parameters_.getParameter("NumImages").convert<int>();
    m_impl->setParameters(operation, numImages);
    m_startImage = parameters_.getParameter("StartImage").convert<int>();
}

void LineTemporalFilter::arm(const fliplib::ArmStateBase& armState)
{
    if (armState.getStateID() == eSeamStart)
    {
        m_impl->reset();
        m_lastProcessedImageNumber = -1;
    }
}

void LineTemporalFilter::paint()
{
    using namespace image;

    if (m_oVerbosity < eLow || m_trafo.isNull() || !m_impl->isResultValid())
    {
        return;
    }
    const auto& trafo(*m_trafo);
    auto& overlayCanvas = canvas<OverlayCanvas>(m_oCounter);
    auto& layerContour = overlayCanvas.getLayerContour();

    auto drawLine = [&layerContour, &trafo](const geo2d::Doublearray& line, Color lineColor)
    {
        const auto& lineData = line.getData();
        std::size_t xValidStart = 0;
        auto xValidEnd = xValidStart;
        std::tie(xValidStart, xValidEnd) = searchFirstValidArrayRange(line, xValidStart);
        if (xValidStart == 0 && xValidEnd == line.size())
        {
            //each point of the line has good rank
            layerContour.add<OverlayPointList>(trafo(geo2d::Point{static_cast<int>(xValidStart), 0}), lineData, lineColor, /*connected*/ true);
        }
        else
        {
            std::vector<int> yValuesInRange;
            while (xValidStart < line.size())
            {
                yValuesInRange.resize(xValidEnd - xValidStart);
                std::transform(lineData.begin() + xValidStart, lineData.begin() + xValidEnd, yValuesInRange.begin(), [](double y)
                               { return y + 0.5; });
                layerContour.add<OverlayPointList>(trafo(geo2d::Point{static_cast<int>(xValidStart), 0}), std::move(yValuesInRange), lineColor, /*connected*/ true);

                std::tie(xValidStart, xValidEnd) = searchFirstValidArrayRange(line, xValidEnd);
            }
        }
    };
    drawLine(m_impl->getResult(), Color::m_oPlum);
}

void LineTemporalFilter::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e)
{
    poco_assert_dbg(m_pipeIn != nullptr); // to be asserted by graph editor
    const auto& geoArrayIn = m_pipeIn->read(m_oCounter);
    auto& context = geoArrayIn.context();

    //start filtering only at startImage (startImage = 1 corresponds to imageNumber 0, startImage=0 means no filtering at all)
    if ( m_startImage == 0 || (m_startImage - 1) > context.imageNumber())
    {
        const interface::GeoVecDoublearray geoLineOut = geoArrayIn;
        preSignalAction();
        m_pipeOut.signal(geoLineOut);
        return;
    }

    //resend the last result if the same image is being processed (i.e when moving the profile line slider in simulation)
    if (context.imageNumber() == m_lastProcessedImageNumber)
    {
        const interface::GeoVecDoublearray geoLineOut(context, {m_impl->getResult()}, geoArrayIn.analysisResult(), m_impl->isResultValid() ? 1.0 : 0.0);
        preSignalAction();
        m_pipeOut.signal(geoLineOut);
        return;
    }
    m_lastProcessedImageNumber = context.imageNumber();
    m_trafo = context.trafo();

    if (geoArrayIn.rank() > 0.0)
    {
        m_impl->updateResult(geoArrayIn.ref());
    }
    else
    {
        //it's not clear what to do, let's just fill the buffer position with a null block
        m_impl->updateResult({});
    }

    const interface::GeoVecDoublearray geoLineOut(context, {m_impl->getResult()}, geoArrayIn.analysisResult(), m_impl->isResultValid() ? 1.0 : 0.0);
    preSignalAction();
    m_pipeOut.signal(geoLineOut);
}

}
}
