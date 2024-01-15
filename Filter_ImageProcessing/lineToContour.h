#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API LineToContour : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contourIn;
    const fliplib::SynchronePipe<interface::GeoVecDoublearray>* m_lineIn;
    fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> m_contourOut;

    bool m_closeContour;

    std::vector<std::pair<double, double>> m_contour;
    std::vector<std::pair<double, double>> m_sampledContour;
    std::vector<std::pair<double, double>> m_sampledContourNormal;
    geo2d::TAnnotatedArray<precitec::geo2d::TPoint<double>> m_contourTransformed;

public:
    LineToContour();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
