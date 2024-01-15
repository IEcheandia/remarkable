#include "removeBackground.h"

#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <opencv2/opencv.hpp>

#define FILTER_ID                 "ab8bdb29-7305-4783-bf29-d834e9642b45"
#define VARIANT_ID                "bfbbe454-007d-453b-b550-411782d1ac0e"

#define PIPE_ID_IMAGEIN           "1e799c54-a213-4e7b-9fb2-c07f9bb17698"
#define PIPE_ID_IMAGEOUT          "a44de287-f486-4a81-bf82-233f84814f19"

void backgroundFilter(const uint8_t *imageSrc, uint8_t *imageDst, int height, int width, std::ptrdiff_t strideSrc, std::ptrdiff_t strideDst, int contrast, int minArea, int maxArea, bool convexHull)
{
    if (height * width == 0)
    {
        return;
    }

    cv::Mat cvImage(height, width, CV_8UC1, (void *)imageSrc, strideSrc);
    cv::Mat cvEdge;

    if (contrast <= 0)
    {
        const auto upperThreshold = cv::threshold(cvImage, cvEdge, 0, 255, cv::THRESH_BINARY | cv::THRESH_OTSU);
        cv::Canny(cvImage, cvEdge, upperThreshold * 0.5, upperThreshold);
    }
    else
    {
        cv::Canny(cvImage, cvEdge, contrast, contrast * 1.5);
    }

    cv::Mat cvMask;
    cv::Mat cvMorphElement = cv::getStructuringElement(cv::MORPH_RECT, {5, 5});

    cv::dilate(cvEdge, cvMask, cvMorphElement);

    std::vector<std::vector<cv::Point>> contours;
    cv::findContours(cvMask, contours, cv::RETR_TREE, cv::CHAIN_APPROX_SIMPLE);

    if (convexHull)
    {
        for (std::size_t i = 0; i < contours.size(); ++i)
        {
            const auto contour = contours[i]; //copy
            cv::convexHull(contour, contours[i]);

        }
    }

    std::vector<double> areas(contours.size());

    for (std::size_t i = 0; i < contours.size(); ++i)
    {
        areas[i] = cv::contourArea(contours[i]);
    }

    cv::Mat cvCropMask = cv::Mat::zeros(height, width, CV_8UC1);

    for (std::size_t i = 0; i < contours.size(); ++i)
    {
        if ((maxArea <= 0 || areas[i] < maxArea) &&
            (minArea <= 0 || areas[i] > minArea))
        {
            cv::drawContours(cvCropMask, contours, i, 255, cv::FILLED);
        }
    }

    cv::erode(cvCropMask, cvCropMask, cvMorphElement);
    cv::dilate(cvCropMask, cvCropMask, cvMorphElement);
    cv::medianBlur(cvCropMask, cvCropMask, 5);

    cv::Mat cvImageDst(height, width, CV_8UC1, (void *)imageDst, strideDst);
    cv::bitwise_and(cvCropMask, cvImage, cvImageDst);
}

namespace precitec
{
namespace filter
{

RemoveBackground::RemoveBackground()
    : TransformFilter("RemoveBackground", Poco::UUID(FILTER_ID))
    , m_imageIn(nullptr)
    , m_imageOut(this, "ImageOut")
    , m_contrast(0)
    , m_minArea(0)
    , m_maxArea(0)
    , m_convexHull(false)
{
    parameters_.add("Contrast", fliplib::Parameter::TYPE_int, m_contrast);
    parameters_.add("MinArea", fliplib::Parameter::TYPE_int, m_minArea);
    parameters_.add("MaxArea", fliplib::Parameter::TYPE_int, m_maxArea);
    parameters_.add("ConvexHull", fliplib::Parameter::TYPE_bool, m_convexHull);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_IMAGEIN), m_imageIn, "ImageIn", 1, "ImageIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_IMAGEOUT), &m_imageOut, "ImageOut", 1, "ImageOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void RemoveBackground::setParameter()
{
    TransformFilter::setParameter();

    m_contrast = parameters_.getParameter("Contrast").convert<int>();
    m_minArea = parameters_.getParameter("MinArea").convert<int>();
    m_maxArea = parameters_.getParameter("MaxArea").convert<int>();
    m_convexHull = parameters_.getParameter("ConvexHull").convert<bool>();
}

void RemoveBackground::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas &rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &imageLayer(rCanvas.getLayerImage());

    imageLayer.add<image::OverlayImage>(geo2d::Point{m_originX, m_originY}, m_image, image::OverlayText());
}

bool RemoveBackground::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_imageIn = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void RemoveBackground::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &frame = m_imageIn->read(m_oCounter);
    auto &imageIn = frame.data();

    m_originX = frame.context().getTrafoX();
    m_originY = frame.context().getTrafoY();

    m_image.resize(image::Size2d{imageIn.width(), imageIn.height()});

    if (imageIn.isValid())
    {
        backgroundFilter(imageIn.begin(), m_image.begin(), imageIn.height(), imageIn.width(), imageIn.stride(), m_image.stride(), m_contrast, m_minArea, m_maxArea, m_convexHull);
    }

    const auto frameOut = interface::ImageFrame(frame.context(), m_image, frame.analysisResult());

    preSignalAction();
    m_imageOut.signal(frameOut);
}

} //namespace filter
} //namespace precitec
