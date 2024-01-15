#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

namespace precitec
{
namespace filter
{

class FILTER_API RemoveBackground : public fliplib::TransformFilter
{
private:
    const fliplib::SynchronePipe<interface::ImageFrame>* m_imageIn;
    fliplib::SynchronePipe<interface::ImageFrame> m_imageOut;

    int m_contrast;
    int m_minArea;
    int m_maxArea;
    bool m_convexHull;

    // paint info
    image::BImage m_image;
    int m_originX;
    int m_originY;

public:
    RemoveBackground();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;
};

} //namespace filter
} //namespace precitec
