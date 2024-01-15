#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

#include <vector>

#include "opencv2/opencv.hpp"

namespace precitec
{
namespace filter
{

class FILTER_API Metrology : public fliplib::TransformFilter
{
public:
    Metrology();

    void setParameter() override;
    void paint() override;

    bool subscribe(fliplib::BasePipe& pipe, int group) override;
    void proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event) override;

private:
    //in pipes
    const fliplib::SynchronePipe<interface::ImageFrame>* m_imageFrameIn;
    const fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>* m_contourIn;
    //out pipes
    fliplib::SynchronePipe<interface::GeoDoublearray> m_positionXOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_positionYOut;
    fliplib::SynchronePipe<interface::GeoDoublearray> m_angleOut;

    // parameter
    unsigned int m_contourSampleNumber;
    double m_angleStart;
    double m_angleExtent;

    //measured object pose that goes to out pipe
    double m_positionX;
    double m_positionY;
    double m_angle;

    //metrology contour
    std::vector<cv::Point2f> m_contour;
    std::vector<cv::Point2f> m_sampledContour;

    //ROI position for painting fitted contour
    interface::SmpTrafo m_trafo;
};

float stringToFloat(const std::string& input);
std::vector<cv::Point2f> parseArray(const std::string& array);
std::vector<float> integrateContour(const std::vector<cv::Point2f>& contour);
std::vector<cv::Point2f> sampleContour(const std::vector<cv::Point2f>& contour, const unsigned int nSample);
std::vector<cv::Point2f> rotateContour(const std::vector<cv::Point2f>& contour, float angle);
cv::Rect2f contourBoundingBox(const std::vector<cv::Point2f>& contour);
std::pair<cv::Point2f, float> preciseFit(const cv::Mat& distanceImage, const std::vector<cv::Point2f>& contour, const cv::Rect2f& searchRegion,
  const float angleStart = 0, const float angleExtent = 0, const unsigned int subsampleDistance = 1, float subsampleAngle = 0);
std::pair<cv::Point2f, float> fastFit(const cv::Mat& distanceImage, const std::vector<cv::Point2f>& contour,
    const float angleStart, const float angleExtent, const unsigned int subsampleDistance, float subsampleAngle);
} //namespace filter
} //namespace precitec
