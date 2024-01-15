#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include <geo/geo.h>

namespace precitec
{
namespace filter
{

class FILTER_API LineArithmetic : public fliplib::TransformFilter
{
    enum class Operation
    {
        Addition,
        Multiplication,
        Minimum,
        Maximum,
        AbsoluteDifference
    };

public:
    LineArithmetic();
    ~LineArithmetic() override = default;
    void setParameter() override;

protected:
    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& eventArg) override;

private:
    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_pipeLineIn;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeValue;
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_pipeLineOut;
    Operation m_operation;

};
}
}
