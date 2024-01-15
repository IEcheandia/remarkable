#pragma once

#include <fliplib/Fliplib.h>
#include <fliplib/TransformFilter.h>
#include <geo/geo.h>
#include <common/frame.h>
#include <opencv2/opencv.hpp>

namespace precitec
{
namespace filter
{

class FILTER_API Stencil: public fliplib::TransformFilter
{

public:

    Stencil();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    void generateMask();
    image::BImage imageMasking(const image::BImage& image) const;
    const fliplib::SynchronePipe<interface::ImageFrame>* m_pipeInImageFrame;
    fliplib::SynchronePipe<interface::ImageFrame> m_pipeOutImageFrame;
    static const std::string m_filterName;
    static const std::string m_imageFrameInName;
    static const std::string m_imageFrameOutName;
    std::string m_maskFileName;
    image::BImage m_mask;
    image::BImage m_imageOut;
    interface::SmpTrafo m_trafo;
};

}
}
