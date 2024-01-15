#include "contourPathLocation.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <utility>
#include <cmath>

#define FILTER_ID                 "11ae2c0a-bb99-468c-a202-f3af67c9a137"
#define VARIANT_ID                "80409a9a-818c-4a1b-b441-a01637c49b00"

#define PIPE_ID_CONTOURIN         "11fb6373-4546-4083-a96b-1895b626c1ff"
#define PIPE_ID_POSITIONX         "694c8f43-2d88-4361-a835-b2c5699da96c"
#define PIPE_ID_POSITIONY         "0af4b7ab-9fd0-4ba9-9751-1fb8722e7819"
#define PIPE_ID_NORMALX           "cd7f1713-b4d1-4e57-bb73-0b049f529411"
#define PIPE_ID_NORMALY           "412df61f-f704-4d7e-9b91-8464447a35ca"
#define PIPE_ID_ANGLE             "8b92ea21-b42e-40a3-88b6-2dda265f47cd"

template <class T>
static std::vector<double> integrateContour(const std::vector<T>& contourIn)
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

        const auto dx = p2.x - p1.x;
        const auto dy = p2.y - p1.y;
        length[i] = length[i - 1] + std::sqrt(dx * dx + dy * dy);
    }
    return length;
}

template <class T>
static void contourLocalInfo(const std::vector<T>& contourIn, double relativePath, double &x, double &y, double &nx, double &ny)
{
    x = 0.0;
    y = 0.0;
    nx = 1.0;
    ny = 0.0;

    const auto size = contourIn.size();

    if (size <= 1)
    {
        return;
    }

    const auto s = integrateContour(contourIn);

    auto c = relativePath * s[size - 1];
    if (c > s[size - 1])
    {
        c = s[size - 1];
    }
    else if (c < s[0])
    {
        c = s[0];
    }

    for (std::size_t j = 1; j < size; ++j)
    {
        if (c <= s[j])
        {
            const auto ux = contourIn[j - 1].x;
            const auto uy = contourIn[j - 1].y;
            const auto vx = contourIn[j].x;
            const auto vy = contourIn[j].y;

            const double f = (c - s[j - 1]) / (s[j] - s[j - 1]);

            x = ux + (vx - ux) * f;
            y = uy + (vy - uy) * f;

            nx = vy - uy;
            ny = ux - vx;

            const auto n = std::sqrt(nx * nx + ny * ny);

            nx = nx / n;
            ny = ny / n;

            return;
        }
    }
}

namespace precitec
{
namespace filter
{

ContourPathLocation::ContourPathLocation()
    : TransformFilter("ContourPathLocation", Poco::UUID(FILTER_ID))
    , m_pipeContourIn(nullptr)
    , m_pipePositionX(this, "PositionX")
    , m_pipePositionY(this, "PositionY")
    , m_pipeNormalX(this, "NormalX")
    , m_pipeNormalY(this, "NormalY")
    , m_pipeAngle(this, "Angle")
    , m_relativePath(0.0)
{
    parameters_.add("RelativePath", fliplib::Parameter::TYPE_double, m_relativePath);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_pipeContourIn, "ContourIn", 1, "ContourIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_POSITIONX), &m_pipePositionX, "PositionX", 1, "PositionX"},
        {Poco::UUID(PIPE_ID_POSITIONY), &m_pipePositionY, "PositionY", 1, "PositionY"},
        {Poco::UUID(PIPE_ID_NORMALX), &m_pipeNormalX, "NormalX", 1, "NormalX"},
        {Poco::UUID(PIPE_ID_NORMALY), &m_pipeNormalY, "NormalY", 1, "NormalY"},
        {Poco::UUID(PIPE_ID_ANGLE), &m_pipeAngle, "Angle", 1, "Angle"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void ContourPathLocation::setParameter()
{
    TransformFilter::setParameter();

    m_relativePath = parameters_.getParameter("RelativePath").convert<double>();
}

void ContourPathLocation::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }
}

bool ContourPathLocation::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_pipeContourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void ContourPathLocation::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contourInArray = m_pipeContourIn->read(m_oCounter);
    const auto &contourIn = contourInArray.ref().front().getData();

    const auto context = contourInArray.context();

    double x;
    double y;
    double nx;
    double ny;

    contourLocalInfo(contourIn, m_relativePath, x, y, nx, ny);

    const double angle = std::atan2(ny, nx) * 180 / M_PI;

    const auto geoRank = interface::Limit; //1.0
    const auto arrayRank = 255;
    const interface::GeoDoublearray geoPositionX(context,
        geo2d::TArray<double>(1, x, arrayRank),
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoPositionY(context,
        geo2d::TArray<double>(1, y, arrayRank),
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoNormalX(context,
        geo2d::TArray<double>(1, nx, arrayRank),
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoNormalY(context,
        geo2d::TArray<double>(1, ny, arrayRank),
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoAngle(context,
        geo2d::TArray<double>(1, angle, arrayRank),
        interface::ResultType::AnalysisOK, geoRank);

    if (m_oVerbosity > eLow)
    {
        wmLog(eInfo, "[ContourPathLocation] x:%f, y:%f, nx:%f, ny:%f, angle:%f", x, y, nx, ny, angle);
    }

    preSignalAction();
    m_pipePositionX.signal(geoPositionX);
    m_pipePositionY.signal(geoPositionY);
    m_pipeNormalX.signal(geoNormalX);
    m_pipeNormalY.signal(geoNormalY);
    m_pipeAngle.signal(geoAngle);
}


} //namespace filter
} //namespace precitec
