#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

#include "shapeMatchingImpl.h"

#include <vector>

#include "opencv2/opencv.hpp"

namespace precitec
{
namespace filter
{

class FILTER_API TemplateMatching : public fliplib::TransformFilter
{
public:
    TemplateMatching();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceed(const void* sender, fliplib::PipeEventArgs& event) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    const fliplib::SynchronePipe<interface::ImageFrame>* m_pipeInImageFrame;
    const fliplib::SynchronePipe<interface::GeoDoublearray>* m_pipeAngleStart;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipePosX;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_PipePosY;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeAngle;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_pipeScore;

    enum class CropMode
    {
        Crop = 0,
        Keep = 1,
    };

    std::string m_templateFileName;
    double m_angleStart;
    unsigned int m_countStep;
    double m_angleStep;
    unsigned int m_pyramidLevels;
    CropMode m_cropMode;

    std::vector<image::BImage> m_templates;
    std::vector<cv::Mat> m_templateScaled;

    MatchCandidate m_matchResult;
    interface::SmpTrafo m_trafo;

    void generateTemplate();
};

void rotateImage(uint8_t *src, uint8_t *dst, size_t height,
  size_t width, size_t srcStride, size_t dstStride, double centerCol,
  double centerRow, double angle);

uint64_t normL2Squared(uint8_t *src, size_t height, size_t width,
    ptrdiff_t srcStride);

void crossCorrelation(const uint8_t *src, size_t srcHeight, size_t srcWidth,
    ptrdiff_t srcStride, const uint8_t *tmpl, size_t tmplHeight,
    size_t tmplWidth, ptrdiff_t tmplStride, uint64_t *dst, ptrdiff_t dstStride);

/*
Given a rectangle that has been rotated by 'angle' (in
degrees), computes the width and height of the largest possible
axis-aligned rectangle (maximal area) within the rotated rectangle.
*/
void rotatedRectWithMaxArea(double width, double height, double angle,
    double& cropWidth, double& cropHeight);

} //namespace filter
} //namespace precitec
