#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API PowerRamp : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contourIn;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_contourOut;

    enum class Mode {
        RepeatWhenClosed = 0,
        NoRepeat = 1,
        Repeat = 2,
    };

    Mode m_repeatMode;
    double m_channel1StartPower;
    double m_channel1EndPower;
    double m_channel2StartPower; //Ring power
    double m_channel2EndPower;
    double m_rampInLength;
    double m_rampOutLength;

public:
    PowerRamp();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
