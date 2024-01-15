#include "lineArithmetic.h"
#include "fliplib/TypeToDataTypeImpl.h"
#include "filter/algoArray.h"

namespace precitec
{
namespace filter
{

LineArithmetic::LineArithmetic()
: TransformFilter("LineArithmetic", Poco::UUID("01E33417-10FA-49B5-A769-7BD618DC1D8F") )
    , m_pipeLineIn(nullptr)
    , m_pipeValue(nullptr)
    , m_pipeLineOut(this, "Result")
    , m_operation(Operation::Addition)
{
    parameters_.add("Operation", fliplib::Parameter::TYPE_int, int(m_operation));
    setInPipeConnectors({
        {Poco::UUID("BE2697B8-6693-47CE-A09E-A52E79BA2307"), m_pipeLineIn},
                        {Poco::UUID("A1B6FEEA-DF80-4244-AFCC-D49A062000CE"), m_pipeValue},
    });
    setOutPipeConnectors({{Poco::UUID("3347EBF9-CE4F-4DE5-B3B2-231043E9DF50"), &m_pipeLineOut}});
    setVariantID(Poco::UUID("688C3924-539B-4F2D-9101-E1619B6D0BFE"));
}

void LineArithmetic::setParameter()
{
    TransformFilter::setParameter();
    m_operation = static_cast<Operation>(parameters_.getParameter("Operation").convert<int>());
}

bool LineArithmetic::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.type() == typeid(interface::GeoVecDoublearray))
    {
        m_pipeLineIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&pipe);
    }
    else if (pipe.type() == typeid(interface::GeoDoublearray))
    {
        m_pipeValue = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void LineArithmetic::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& eventArg)
{
    poco_assert_dbg(m_pipeLineIn != nullptr);
    poco_assert_dbg(m_pipeValue != nullptr);
    const auto& geoLineIn = m_pipeLineIn->read(m_oCounter);
    const auto& valueIn = m_pipeValue->read(m_oCounter).ref();
    if (valueIn.empty())
    {
        auto output = geoLineIn;
        preSignalAction();
        m_pipeLineOut.signal(output);
        return;
    }

    int numberLines = geoLineIn.ref().size();
    interface::GeoVecDoublearray output(geoLineIn.context(), std::vector<geo2d::Doublearray>(numberLines), interface::AnalysisOK, 1.0);
    auto alwaysUseFirstValue = !processEntireProfileLines(numberLines, valueIn.size(), name(), "Value");
    for (int i = 0; i < numberLines; i++)
    {
        auto& inputLine = geoLineIn.ref()[i];
        auto& inputValue = valueIn.getData()[alwaysUseFirstValue ? 0 : i];
        auto& outputLine = output.ref()[i];
        outputLine.resize(inputLine.size());
        outputLine.getRank() = inputLine.getRank();
        switch (m_operation)
        {
        case Operation::Addition:
            std::transform(inputLine.getData().begin(), inputLine.getData().end(), outputLine.getData().begin(),
                           [inputValue](auto y)
                           { return y + inputValue; });
            break;
        case Operation::Multiplication:
            std::transform(inputLine.getData().begin(), inputLine.getData().end(), outputLine.getData().begin(),
                           [inputValue](auto y)
                           { return y * inputValue; });
            break;
        case Operation::Minimum:
            std::transform(inputLine.getData().begin(), inputLine.getData().end(), outputLine.getData().begin(),
                           [inputValue](auto y)
                           { return std::min(y, inputValue); });
            break;
        case Operation::Maximum:
            std::transform(inputLine.getData().begin(), inputLine.getData().end(), outputLine.getData().begin(),
                           [inputValue](auto y)
                           { return std::max(y, inputValue); });
            break;
        case Operation::AbsoluteDifference:
            std::transform(inputLine.getData().begin(), inputLine.getData().end(), outputLine.getData().begin(),
                           [inputValue](auto y)
                           { return std::abs(y - inputValue); });
            break;
        }
    }
    preSignalAction();
    m_pipeLineOut.signal(output);
}

}
}
