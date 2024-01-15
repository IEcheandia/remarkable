#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include <geo/geo.h>

namespace precitec
{
namespace filter
{

class LineTemporalImpl;

class FILTER_API LineTemporalFilter : public fliplib::TransformFilter
{
public:
    LineTemporalFilter();
    ~LineTemporalFilter();
    void setParameter() override;
private:
    void arm(const fliplib::ArmStateBase& state) override;
    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& e) override;
    void paint() override;

    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_pipeIn;
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_pipeOut;
    int m_lastProcessedImageNumber = -1;
    int m_startImage = 1;

    std::unique_ptr<LineTemporalImpl> m_impl;
    interface::SmpTrafo m_trafo;
};

}
}
