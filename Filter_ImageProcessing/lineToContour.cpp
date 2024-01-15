#include "lineToContour.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <utility>
#include <cmath>

#define FILTER_ID                 "4c46aa05-3484-40d4-93e1-63f91ce3d615"
#define VARIANT_ID                "1cf8ed3e-86bc-4361-a607-81b7f3f77f05"

#define PIPE_ID_CONTOURIN         "5ffc124a-5f22-4ea7-be0e-c8ece2a9ab55"
#define PIPE_ID_LINEIN            "56a3da9d-5d21-4f5a-9b0d-be1bc54eaa2f"
#define PIPE_ID_CONTOUROUT        "2983e282-52d2-40c4-98f2-cd02e9c8f4c1"

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

namespace precitec
{
namespace filter
{

LineToContour::LineToContour()
    : TransformFilter("LineToContour", Poco::UUID(FILTER_ID))
    , m_contourIn(nullptr)
    , m_lineIn(nullptr)
    , m_contourOut(this, "ContourOut")
    , m_closeContour(true)
{
    parameters_.add("CloseContour", fliplib::Parameter::TYPE_bool, m_closeContour);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_contourIn, "ContourIn", 1, "ContourIn"},
        {Poco::UUID(PIPE_ID_LINEIN), m_lineIn, "LineIn", 1, "LineIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUROUT), &m_contourOut, "ContourOut", 1, "ContourOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void LineToContour::setParameter()
{
    TransformFilter::setParameter();

    m_closeContour = parameters_.getParameter("CloseContour").convert<bool>();
}

void LineToContour::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas &currentCanvas = canvas<image::OverlayCanvas>(m_oCounter);
    image::OverlayLayer &layer = currentCanvas.getLayerContour();

    const auto &contourTransformedPoints = m_contourTransformed.getData();

    for (std::size_t i = 0; i < contourTransformedPoints.size(); ++i)
    {
        const auto& point = contourTransformedPoints[i];
        layer.add<image::OverlayCross>(geo2d::Point(std::round(point.x + 0.5), std::round(point.y + 0.5)), 5, image::Color::Cyan());
    }
}

bool LineToContour::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }
    else if (pipe.tag() == "LineIn")
    {
        m_lineIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecDoublearray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void LineToContour::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contourInArray = m_contourIn->read(m_oCounter);
    const auto &contourIn = contourInArray.ref().at(0);
    const auto &lineInArray = m_lineIn->read(m_oCounter);
    const auto &lineIn = lineInArray.ref().front().getData();

    const auto dx = lineInArray.context().getTrafoX(); // half of original line width

    const auto &contourPoints = contourIn.getData();
    m_contour.clear();
    m_contour.reserve(contourPoints.size());
    for (const auto &point : contourPoints)
    {
        m_contour.emplace_back(point.x, point.y);
    }

    sampleContour(m_contour, m_sampledContour, m_sampledContourNormal, lineIn.size());

    m_contourTransformed.resize(lineIn.size());
    auto &contourTransformedPoints = m_contourTransformed.getData();
    auto &contourTransformedRank = m_contourTransformed.getRank();

    for (std::size_t i = 0; i < lineIn.size(); ++i)
    {
        const auto nx = m_sampledContourNormal[i].first;
        const auto ny = m_sampledContourNormal[i].second;
        contourTransformedPoints[i] =
        {
            m_sampledContour[i].first + nx * (lineIn[i] - dx),
            m_sampledContour[i].second + ny * (lineIn[i] - dx)
        };

        contourTransformedRank[i] = eRankMax;
    }

    if (m_closeContour)
    {
        contourTransformedPoints.emplace_back(contourTransformedPoints.front());
        contourTransformedRank.emplace_back(eRankMax);
    }

    interface::GeoVecAnnotatedDPointarray geoContourOut(
        contourInArray.context(),
        {m_contourTransformed},
        contourInArray.analysisResult(),
        contourInArray.rank());

    preSignalAction();
    m_contourOut.signal(geoContourOut);
}


} //namespace filter
} //namespace precitec
