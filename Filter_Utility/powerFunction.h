#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

#include <string>

namespace precitec
{
namespace filter
{

class FILTER_API PowerFunction : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contourIn;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_contourOut;

    double m_stepSize;
    std::string m_channel1Power;
    std::string m_channel2Power;

public:
    PowerFunction();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec

