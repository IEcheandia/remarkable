#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API ContourProfile : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::ImageFrame>* m_imageIn;
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contourIn;
    fliplib::SynchronePipe<interface::ImageFrame> m_contourProfile;

    unsigned int m_samples;
    unsigned int m_sampleLength;

    std::vector<std::pair<double, double>> m_contour;
    std::vector<std::pair<double, double>> m_sampledContour;
    std::vector<std::pair<double, double>> m_sampledContourNormal;
    image::BImage m_line;

public:
    ContourProfile();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
