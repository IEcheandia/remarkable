#include "contourProfile.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <utility>
#include <cmath>

#define FILTER_ID                 "f74b6642-b341-4744-90ab-9bc4d923ee3d"
#define VARIANT_ID                "8915d15e-88c2-4820-94b6-9ad96352f891"

#define PIPE_ID_IMAGEIN           "a903ce7a-d601-459d-8374-385ab8da8c56"
#define PIPE_ID_CONTOURIN         "7c192b53-b2b5-455c-ba23-7e7beccfa8ec"
#define PIPE_ID_CONTOURPROFILE    "2e31d797-93fa-4384-824d-c4284872577d"

static std::vector<double> integrateContour(const std::vector<std::pair<double, double>>& contourIn)
{
    if (contourIn.empty())
    {
        return {0};
    }

    std::vector<double> length(contourIn.size(), 0);

    for (std::size_t i = 1; i < contourIn.size(); ++i)
    {
        const auto& p1 = contourIn[i - 1];
        const auto& p2 = contourIn[i];

        const auto dx = p2.first - p1.first;
        const auto dy = p2.second - p1.second;
        length[i] = length[i - 1] + std::sqrt(dx * dx + dy * dy);
    }
    return length;
}

static void sampleContour(const std::vector<std::pair<double, double>>& contourIn, std::vector<std::pair<double, double>>& contourOut, std::vector<std::pair<double, double>>& contourOutNormal, int nSample)
{
    const auto& length = integrateContour(contourIn);
    contourOut.resize(nSample);
    contourOutNormal.resize(nSample);

    std::size_t j = 1;

    for (int i = 0; i < nSample; ++i)
    {
        const double c = (i + 0.5) * length.back() / nSample;
        for(; j < contourIn.size(); ++j)
        {
            if (c < length[j])
            {
                const auto& u = contourIn[j - 1];
                const auto& v = contourIn[j];
                const double fraction = (c - length[j - 1]) / (length[j] - length[j - 1]);
                contourOut[i] =
                {
                    u.first + (v.first - u.first) * fraction,
                    u.second + (v.second - u.second) * fraction
                };

                const auto nx = v.second - u.second;
                const auto ny = u.first - v.first;
                const auto n = std::sqrt(nx * nx + ny * ny);
                contourOutNormal[i] =
                {
                    nx / n,
                    ny / n
                };
                break;
            }
        }
    }
}

static void sampleImageLine(const uint8_t *image, uint8_t *line, int height, int width, std::ptrdiff_t stride, double x0, double y0, double nx, double ny, int samples)
{
    std::vector<uint8_t> valid(samples, 255);

    auto x = x0;
    auto y = y0;

    for (int i = 0; i < samples; ++i, x += nx, y += ny)
    {
        const auto px = static_cast<int>(x);
        const auto py = static_cast<int>(y);

        if (px < 0 || px > width - 2 || py < 0 || py > height - 2)
        {
            valid[i] = 0;
            line[i] = 0;
        }
        else
        {
            const auto p = px + py * stride;
            const auto p1 = image[p];
            const auto p2 = image[p + 1];
            const auto p3 = image[p + stride];
            const auto p4 = image[p + 1 + stride];

            const auto tx = x - px;
            const auto ty = y - py;

            const auto a = p1 + tx * (p2 - p1);
            const auto b = p3 + tx * (p4 - p3);

            line[i] = a + ty * (b - a);
        }
    }

    for (int i = 1; i < samples; ++i)
    {
        if (valid[i] == 0)
        {
            line[i] = line[i - 1];
        }
    }

    for (int i = samples - 2; i >= 0; --i)
    {
        if (valid[i] == 0)
        {
            line[i] = line[i + 1];
        }
    }
}

namespace precitec
{
namespace filter
{

ContourProfile::ContourProfile()
    : TransformFilter("ContourProfile", Poco::UUID(FILTER_ID))
    , m_imageIn(nullptr)
    , m_contourIn(nullptr)
    , m_contourProfile(this, "ContourProfile")
    , m_samples(0)
    , m_sampleLength(0)
{
    parameters_.add("Samples", fliplib::Parameter::TYPE_uint, m_samples);
    parameters_.add("SampleLength", fliplib::Parameter::TYPE_uint, m_sampleLength);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_IMAGEIN), m_imageIn, "ImageIn", 1, "ImageIn"},
        {Poco::UUID(PIPE_ID_CONTOURIN), m_contourIn, "ContourIn", 1, "ContourIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURPROFILE), &m_contourProfile, "ContourProfile", 1, "ContourProfile"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void ContourProfile::setParameter()
{
    TransformFilter::setParameter();

    m_samples = parameters_.getParameter("Samples").convert<unsigned int>();
    m_sampleLength = parameters_.getParameter("SampleLength").convert<unsigned int>();
}

void ContourProfile::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas &currentCanvas = canvas<image::OverlayCanvas>(m_oCounter);
    image::OverlayLayer &imageLayer = currentCanvas.getLayerLine();

    imageLayer.add<image::OverlayImage>(geo2d::Point(0, 0), m_line, image::OverlayText());

    image::OverlayLayer &contourLayer = currentCanvas.getLayerContour();

    for (std::size_t i = 1; i < m_contour.size(); ++i)
    {
        const double x1 = std::round(m_contour[i-1].first);
        const double x2 = std::round(m_contour[i].first);
        const double y1 = std::round(m_contour[i-1].second);
        const double y2 = std::round(m_contour[i].second);
        contourLayer.add<image::OverlayLine>(x1, y1, x2, y2, image::Color::Yellow());
    }

    for (std::size_t i = 0; i < m_sampledContour.size(); ++i)
    {
        const auto nx = m_sampledContourNormal[i].first;
        const auto ny = m_sampledContourNormal[i].second;
        const auto x1 = std::round(m_sampledContour[i].first - (m_sampleLength / 2) * nx);
        const auto y1 = std::round(m_sampledContour[i].second - (m_sampleLength / 2) * ny);
        const auto x2 = std::round(m_sampledContour[i].first + (m_sampleLength / 2) * nx);
        const auto y2 = std::round(m_sampledContour[i].second + (m_sampleLength / 2) * ny);
        contourLayer.add<image::OverlayLine>(x1, y1, x2, y2, image::Color::Red());
    }
}

bool ContourProfile::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_imageIn = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    else if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void ContourProfile::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &frame = m_imageIn->read(m_oCounter);
    const auto &imageIn = frame.data();
    const auto &contour = m_contourIn->read(m_oCounter);
    const auto &contourIn = contour.ref().at(0);

    const auto &contourPoints = contourIn.getData();
    m_contour.clear();
    m_contour.reserve(contourPoints.size());
    for (const auto &point : contourPoints)
    {
        m_contour.emplace_back(point.x, point.y);
    }

    sampleContour(m_contour, m_sampledContour, m_sampledContourNormal, m_samples);

    m_line.resize(precitec::image::Size2d(m_sampleLength, m_samples));

    for (unsigned int i = 0; i < m_samples; ++i)
    {
        const auto nx = m_sampledContourNormal[i].first;
        const auto ny = m_sampledContourNormal[i].second;
        sampleImageLine(imageIn.begin(), m_line[i],
                        imageIn.height(), imageIn.width(), imageIn.stride(),
                        m_sampledContour[i].first - nx * (m_sampleLength / 2),
                        m_sampledContour[i].second - ny * (m_sampleLength / 2),
                        nx, ny, m_sampleLength);
    }

    const auto frameOut = interface::ImageFrame(frame.context(), m_line, frame.analysisResult());

    preSignalAction();
    m_contourProfile.signal(frameOut);
}

} //namespace filter
} //namespace precitec
