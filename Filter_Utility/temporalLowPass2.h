#pragma once

#include "temporalLowPassTemplate.h"

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"

namespace precitec
{
namespace filter
{

class FILTER_API TemporalLowPass2 : public fliplib::TransformFilter
{
public:
    TemporalLowPass2();
    void setParameter() override;
private:
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;

    void arm(const fliplib::ArmStateBase& state) override;
    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override;

    const pipe_scalar_t* m_pPipeIn1;
    const pipe_scalar_t* m_pPipeIn2;
    pipe_scalar_t m_oPipeOut1;
    pipe_scalar_t m_oPipeOut2;

    TemporalLowPassTemplate<2> m_calculator;
};


}
}
