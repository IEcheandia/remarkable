#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

#include "filter/parameterEnums.h"

namespace precitec
{
namespace filter
{

class FILTER_API ParallelLocalExtremum : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::ImageFrame>* m_imageIn;
    fliplib::SynchronePipe<interface::GeoVecDoublearray> m_extremumLocation;

    ExtremumType m_extremumType; // min or max search
    bool m_searchFromRightToLeft;
    int m_extremumIndex; // which peak to select
    int m_minProminence;
    bool m_subpixel;
    double m_offset;

    geo2d::VecDoublearray m_lineOut;
    std::vector<std::pair<int, double>> m_allDetectedExtremum;

public:
    ParallelLocalExtremum();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
