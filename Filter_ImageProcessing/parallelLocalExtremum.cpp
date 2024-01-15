#include "parallelLocalExtremum.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <cmath>

#include <opencv2/opencv.hpp>

#define FILTER_ID                 "e63bce05-084f-4523-b18b-a70c32d611dd"
#define VARIANT_ID                "cf65f47f-92dc-4d35-b37e-42062051e043"

#define PIPE_ID_IMAGEIN           "1a122168-972d-4fbf-a8a7-6cce2febeabd"
#define PIPE_ID_EXTREMUMLOCATION  "588ba43b-714c-4697-9abe-03056409b1c8"

/**
For each row in the image, determine the local maxima and minima. Local maxima
is a pixel where the left and right neighbors are less than or equal to the
pixel and at least one neighbor is less than the pixel. Local minima is a
pixel where the left and right neighbors are greater than or equal to the pixel
and at least one neighbor is greater than the pixel.
**/
static void preprocessMinMax(const uint8_t *image, int height, int width, std::ptrdiff_t stride, uint8_t *minMax)
{
    const cv::Mat cvImage(height, width, CV_8UC1, (void *)(image), static_cast<std::size_t>(stride));
    cv::Mat cvMinMax{height, width, CV_8UC1, static_cast<void *>(minMax)};

    const auto element = cv::getStructuringElement(cv::MORPH_RECT, {3, 1}); //[1 1 1] structuring element

    cv::Mat dilateImage;
    cv::Mat erodeImage;
    cv::dilate(cvImage, dilateImage, element);
    cv::erode(cvImage, erodeImage, element);

    // ((D == I) | (E == I)) & (D != E)
    cv::bitwise_and(
        (dilateImage == cvImage) | (erodeImage == cvImage),
        dilateImage != erodeImage, cvMinMax);
}

static std::vector<int> localMinimum(const int *minMaxIndex, const uint8_t *minMaxValue, int minMaxCount, int thresholdLeft, int thresholdRight)
{
    std::vector<uint8_t> minValid(minMaxCount, 1);

    if (minMaxCount > 0)
    {
        minValid[0] = 0;
        minValid[minMaxCount - 1] = 0;
    }

    for (int i = 0; i < minMaxCount; ++i)
    {
        if (!minValid[i])
        {
            continue;
        }

        int j = i + 1;
        for (; j < minMaxCount; ++j)
        {
            if (minMaxValue[j] > minMaxValue[i] + thresholdRight)
            {
                for (int k = i + 1; k <= j; ++k)
                {
                    minValid[k] &= (minMaxValue[k] < minMaxValue[i]);
                    minValid[i] &= (minMaxValue[k] >= minMaxValue[i]);
                }
                break;
            }
        }

        if (j == minMaxCount)
        {
            minValid[i] = 0;
        }

        if (!minValid[i])
        {
            continue;
        }

        j = i - 1;
        for (; j >= 0; --j)
        {
            if (minMaxValue[j] > minMaxValue[i] + thresholdLeft)
            {
                for (int k = i - 1; k >= j; --k)
                {
                    minValid[k] &= (minMaxValue[k] <= minMaxValue[i]);
                    minValid[i] &= (minMaxValue[k] > minMaxValue[i]);
                }
                break;
            }
        }

        if (j == -1)
        {
            minValid[i] = 0;
        }
    }

    std::vector<int> minIndex;

    for (int i = 0; i < minMaxCount; ++i)
    {
        if (minValid[i])
        {
            minIndex.emplace_back(minMaxIndex[i]);
        }
    }

    return minIndex;
}

/**
Given a line function, return indices of all local minima whose prominence is
greater than the threshold. "thresholdLeft" and "thresholdRight" parameters are
provided for asymmetric prominence filtering. If no local minima exists, the
function returns an empty vector. The "minMax" input comes from the
preprocessMinMax function, which processes all rows at once.
**/
static std::vector<int> localMinimum(const uint8_t *line, const uint8_t *minMax, int width, int thresholdLeft, int thresholdRight)
{
    auto minMaxIndex = new int [width];
    auto minMaxValue = new uint8_t [width];
    int minMaxCount = 0;

    for (int i = 0; i < width; ++i)
    {
        if (minMax[i] > 0)
        {
            minMaxIndex[minMaxCount] = i;
            minMaxValue[minMaxCount] = line[i];
            ++minMaxCount;
        }
    }

    const auto minIndex = localMinimum(minMaxIndex, minMaxValue, minMaxCount, thresholdLeft, thresholdRight);

    delete[] minMaxIndex;
    delete[] minMaxValue;

    return minIndex;
}

static std::vector<int> localMaximum(const uint8_t *line, const uint8_t *minMax, int width, int thresholdLeft, int thresholdRight)
{
    auto minMaxIndex = new int [width];
    auto minMaxValue = new uint8_t [width];
    int minMaxCount = 0;

    for (int i = 0; i < width; ++i)
    {
        if (minMax[i] > 0)
        {
            minMaxIndex[minMaxCount] = i;
            minMaxValue[minMaxCount] = ~line[i]; // flip data, bit inversion trick: 0 -> 255, 1 -> 254, ... , 255 -> 0
            ++minMaxCount;
        }
    }

    const auto minIndex = localMinimum(minMaxIndex, minMaxValue, minMaxCount, thresholdLeft, thresholdRight);

    delete[] minMaxIndex;
    delete[] minMaxValue;

    return minIndex;
}

/**
Given the peak index along a line function, approximate the subpixel position
of the peak by performing a paraoba fit around the peak position.
**/
static double subpixelPeak(const uint8_t *line, int width, int peakIndex)
{
    if (peakIndex <= 0 || peakIndex >= width - 1)
    {
        return peakIndex;
    }

    const auto y0 = line[peakIndex - 1];
    const auto y1 = line[peakIndex];
    const auto y2 = line[peakIndex + 1];

    return peakIndex + (y0 - y2) / (2.0 * (y0 - 2 * y1 + y2));
}

namespace precitec
{
namespace filter
{

ParallelLocalExtremum::ParallelLocalExtremum()
    : TransformFilter("ParallelLocalExtremum", Poco::UUID(FILTER_ID))
    , m_imageIn(nullptr)
    , m_extremumLocation(this, "ExtremumLocation")
    , m_extremumType(eMaximum)
    , m_searchFromRightToLeft(false)
    , m_extremumIndex(0)
    , m_minProminence(0)
    , m_subpixel(false)
    , m_offset(0.0)
{
    parameters_.add("ExtremumType", fliplib::Parameter::TYPE_int, static_cast<int>(m_extremumType));
    parameters_.add("SearchFromRightToLeft", fliplib::Parameter::TYPE_bool, m_searchFromRightToLeft);
    parameters_.add("ExtremumIndex", fliplib::Parameter::TYPE_int, m_extremumIndex);
    parameters_.add("MinProminence", fliplib::Parameter::TYPE_int, m_minProminence);
    parameters_.add("Subpixel", fliplib::Parameter::TYPE_bool, m_subpixel);
    parameters_.add("Offset", fliplib::Parameter::TYPE_double, m_offset);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_IMAGEIN), m_imageIn, "ImageIn", 1, "ImageIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_EXTREMUMLOCATION), &m_extremumLocation, "ExtremumLocation", 1, "ExtremumLocation"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void ParallelLocalExtremum::setParameter()
{
    TransformFilter::setParameter();

    m_extremumType = static_cast<ExtremumType>(parameters_.getParameter("ExtremumType").convert<int>());
    m_searchFromRightToLeft  = parameters_.getParameter("SearchFromRightToLeft").convert<bool>();
    m_extremumIndex = parameters_.getParameter("ExtremumIndex").convert<int>();
    m_minProminence = parameters_.getParameter("MinProminence").convert<int>();
    m_subpixel  = parameters_.getParameter("Subpixel").convert<bool>();
    m_offset = parameters_.getParameter("Offset").convert<double>();
}

void ParallelLocalExtremum::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas &rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &lineLayer(rCanvas.getLayerContour());

    const auto& line = m_lineOut.front().getData();

    for (std::size_t i = 1; i < line.size(); ++i)
    {
        lineLayer.add<image::OverlayLine>(
            std::round(line[i-1]),
            i-1,
            std::round(line[i]),
            i,
            image::Color::Cyan());
    }
}

bool ParallelLocalExtremum::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_imageIn = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void ParallelLocalExtremum::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &frame = m_imageIn->read(m_oCounter);
    const auto &imageIn = frame.data();

    m_lineOut.resize(1); // 1 line
    m_lineOut.front().assign(imageIn.height(), 0.0, filter::eRankMax);

    if (!imageIn.isValid())
    {
        m_lineOut.front().assign(imageIn.height(), 0.0, filter::eRankMin);
        const interface::GeoVecDoublearray geoLineOut(frame.context(), m_lineOut, frame.analysisResult(), 1.0);
        preSignalAction();
        m_extremumLocation.signal(geoLineOut);
        return;
    }

    auto& line = m_lineOut.front().getData();

    if (m_extremumType == ExtremumType::eMaximum)
    {
        image::BImage minMax(imageIn.size());
        preprocessMinMax(imageIn.begin(), imageIn.height(), imageIn.width(), imageIn.stride(), minMax.begin());

        const int index = m_searchFromRightToLeft ? -m_extremumIndex : m_extremumIndex - 1;

        for (int i = 0; i < imageIn.height(); ++i)
        {
            const auto localPeak = localMaximum(imageIn[i], minMax[i], imageIn.width(), m_minProminence, m_minProminence);

            const int n = localPeak.size();

            if (n == 0 || n < std::abs(m_extremumIndex))
            {
                line[i] = 0.0;
                continue;
            }

            if (m_subpixel)
            {
                line[i] = subpixelPeak(imageIn[i], imageIn.width(), localPeak[(index % n + n) % n]) + m_offset;
            }
            else
            {
                line[i] = localPeak[(index % n + n) % n] + m_offset;
            }
        }
    }
    else if (m_extremumType == ExtremumType::eMinimum)
    {
        image::BImage minMax(imageIn.size());
        preprocessMinMax(imageIn.begin(), imageIn.height(), imageIn.width(), imageIn.stride(), minMax.begin());

        const int index = m_searchFromRightToLeft ? -m_extremumIndex : m_extremumIndex - 1;

        for (int i = 0; i < imageIn.height(); ++i)
        {
            const auto localPeak = localMinimum(imageIn[i], minMax[i], imageIn.width(), m_minProminence, m_minProminence);

            const int n = localPeak.size();

            if (n == 0 || n < std::abs(m_extremumIndex))
            {
                line[i] = 0.0;
                continue;
            }

            if (m_subpixel)
            {
                line[i] = subpixelPeak(imageIn[i], imageIn.width(), localPeak[(index % n + n) % n]) + m_offset;
            }
            else
            {
                line[i] = localPeak[(index % n + n) % n] + m_offset;
            }
        }
    }

    const auto trafo = frame.context().trafo();

    // send half of image width to recover line position for other filter (see LineToContour Filter)
    const interface::ImageContext contextOut(frame.context(), new interface::LinearTrafo(trafo->dx() + (imageIn.width() / 2), trafo->dy()));

    const interface::GeoVecDoublearray geoLineOut(contextOut, m_lineOut, frame.analysisResult(), 1.0);

    preSignalAction();
    m_extremumLocation.signal(geoLineOut);
}

} //namespace filter
} //namespace precitec
