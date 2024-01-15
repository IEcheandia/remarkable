#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API StitchContour : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contour1;
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contour2;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_contourOut;

    bool m_stitchBothEnds;

public:
    StitchContour();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
