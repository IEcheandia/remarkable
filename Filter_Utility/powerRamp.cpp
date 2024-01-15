#include "powerRamp.h"

#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <utility>
#include <cmath>

#define FILTER_ID                 "fdc7a6fa-7a83-4e1d-961e-448a8073b8f1"
#define VARIANT_ID                "59b99f78-6b31-47f6-bb88-fb0236716112"

#define PIPE_ID_CONTOURIN         "59b99f78-6b31-47f6-bb88-fb0236716112"
#define PIPE_ID_CONTOUROUT        "24215c45-c42e-49e8-8373-c161346facce"

typedef std::vector<precitec::geo2d::DPoint> ContourType;

static ContourType repeatContourLength(const ContourType &contourIn, double repeatLength)
{
    ContourType contourOut = contourIn;

    if (repeatLength <= 0.0)
    {
        return contourOut;
    }

    double l = 0.0;

    for (std::size_t i = 1; i < contourIn.size(); ++i)
    {
        const auto ux = contourIn[i - 1].x;
        const auto uy = contourIn[i - 1].y;
        const auto vx = contourIn[i].x;
        const auto vy = contourIn[i].y;

        const auto dx = vx - ux;
        const auto dy = vy - uy;
        const auto d = std::sqrt(dx * dx + dy * dy);

        if (l + d < repeatLength)
        {
            contourOut.emplace_back(contourIn[i]);
            l = l + d;
        }
        else
        {
            contourOut.emplace_back(
                ux + dx / d * (repeatLength - l),
                uy + dy / d * (repeatLength - l)
            );

            break;
        }
    }

    return contourOut;
}

static ContourType discretizeContour(const ContourType &contourIn, double rampLength, double rampStep)
{
    if (contourIn.empty() || rampStep <= 0.0)
    {
        return contourIn;
    }

    ContourType contourOut;

    contourOut.emplace_back(contourIn[0]);

    double l = 0.0;
    std::size_t i = 1;

    while (l < rampLength && i < contourIn.size())
    {
        const auto ux = contourOut.back().x;
        const auto uy = contourOut.back().y;
        const auto vx = contourIn[i].x;
        const auto vy = contourIn[i].y;
        const auto dx = vx - ux;
        const auto dy = vy - uy;

        const auto d = std::sqrt(dx * dx + dy * dy);

        if (d == 0.0) // u == v, do not insert v (remove duplicate)
        {
            ++i;
        }
        else if (d < rampStep)
        {
            contourOut.emplace_back(contourIn[i]);
            l = l + d;
            ++i;
        }
        else
        {
            contourOut.emplace_back(
                ux + dx / d * rampStep,
                uy + dy / d * rampStep
            );
            l = l + rampStep;
        }
    }

    for (; i < contourIn.size(); ++i)
    {
        contourOut.emplace_back(contourIn[i]);
    }

    return contourOut;
}

static std::vector<double> assignRamp(const ContourType &contourIn, double y0, double y1, double rampInLength, double rampOutLength)
{
    std::vector<double> ramp(contourIn.size(), y1);

    if (contourIn.empty())
    {
        return ramp;
    }

    const auto size = contourIn.size();

    auto ux = contourIn[0].x;
    auto uy = contourIn[0].y;

    double l = 0.0;

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto vx = contourIn[i].x;
        const auto vy = contourIn[i].y;
        const auto dx = vx - ux;
        const auto dy = vy - uy;

        l = l + std::sqrt(dx * dx + dy * dy);

        if (l >= rampInLength)
        {
            break;
        }

        ramp[i] = l / rampInLength * (y1 - y0) + y0;
        ux = vx;
        uy = vy;
    }

    ux = contourIn.back().x;
    uy = contourIn.back().y;

    l = 0.0;

    for (int i = static_cast<int>(size) - 1; i >= 0; --i)
    {
        const auto vx = contourIn[i].x;
        const auto vy = contourIn[i].y;
        const auto dx = vx - ux;
        const auto dy = vy - uy;

        l = l + std::sqrt(dx * dx + dy * dy);

        if (l >= rampOutLength)
        {
            break;
        }

        ramp[i] = l / rampOutLength * (y1 - y0) + y0;
        ux = vx;
        uy = vy;
    }

    return ramp;
}

namespace precitec
{
namespace filter
{

PowerRamp::PowerRamp()
    : TransformFilter("PowerRamp", Poco::UUID(FILTER_ID))
    , m_contourIn(nullptr)
    , m_contourOut(this, "ContourOut")
    , m_repeatMode(Mode::RepeatWhenClosed)
    , m_channel1StartPower(0.0)
    , m_channel1EndPower(0.0)
    , m_channel2StartPower(0.0)
    , m_channel2EndPower(0.0)
    , m_rampInLength(0.0)
    , m_rampOutLength(0.0)
{
    parameters_.add("RepeatMode", fliplib::Parameter::TYPE_int, static_cast<int>(m_repeatMode));
    parameters_.add("Channel1StartPower", fliplib::Parameter::TYPE_double, static_cast<double>(m_channel1StartPower));
    parameters_.add("Channel1EndPower", fliplib::Parameter::TYPE_double, static_cast<double>(m_channel1EndPower));
    parameters_.add("Channel2StartPower", fliplib::Parameter::TYPE_double, static_cast<double>(m_channel2StartPower));
    parameters_.add("Channel2EndPower", fliplib::Parameter::TYPE_double, static_cast<double>(m_channel2EndPower));
    parameters_.add("RampInLength", fliplib::Parameter::TYPE_double, static_cast<double>(m_rampInLength));
    parameters_.add("RampOutLength", fliplib::Parameter::TYPE_double, static_cast<double>(m_rampOutLength));

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_contourIn, "ContourIn", 1, "ContourIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUROUT), &m_contourOut, "ContourOut", 1, "ContourOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void PowerRamp::setParameter()
{
    TransformFilter::setParameter();

    m_repeatMode = static_cast<Mode>(parameters_.getParameter("RepeatMode").convert<int>());
    m_channel1StartPower = parameters_.getParameter("Channel1StartPower").convert<double>() / 100;
    m_channel1EndPower = parameters_.getParameter("Channel1EndPower").convert<double>() / 100;
    m_channel2StartPower = parameters_.getParameter("Channel2StartPower").convert<double>() / 100;
    m_channel2EndPower = parameters_.getParameter("Channel2EndPower").convert<double>() / 100;
    m_rampInLength = parameters_.getParameter("RampInLength").convert<double>();
    m_rampOutLength = parameters_.getParameter("RampOutLength").convert<double>();

    if (m_channel1StartPower < 0 || m_channel1EndPower < 0)
    {
        m_channel1StartPower = -1;
        m_channel1EndPower = -1;
    }

    if (m_channel2StartPower < 0 || m_channel2EndPower < 0)
    {
        m_channel2StartPower = -1;
        m_channel2EndPower = -1;
    }
}

void PowerRamp::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }
}

bool PowerRamp::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void PowerRamp::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contourInArray = m_contourIn->read(m_oCounter);

    interface::GeoVecAnnotatedDPointarray contourOutArray(contourInArray.context(), {}, contourInArray.analysisResult(), contourInArray.rank());

    for (const auto &contourIn : contourInArray.ref())
    {
        ContourType contour;

        if (m_repeatMode == Mode::RepeatWhenClosed)
        {
            const auto &contourInData = contourIn.getData();
            if (!contourIn.getData().empty() && (distance(contourInData.back(), contourInData.front()) < 1e-6))
            {
                contour = repeatContourLength(contourInData, m_rampOutLength);
            }
            else
            {
                contour = contourIn.getData();
            }
        }
        else if (m_repeatMode == Mode::NoRepeat)
        {
            contour = contourIn.getData();
        }
        else
        {
            contour = repeatContourLength(contourIn.getData(), m_rampOutLength);
        }

        const double rampStep = 40e-3; // 40 um
        std::reverse(contour.begin(), contour.end());
        contour = discretizeContour(contour, m_rampOutLength, rampStep);
        std::reverse(contour.begin(), contour.end());
        contour = discretizeContour(contour, m_rampInLength, rampStep);

        const auto channel1Power = assignRamp(contour, m_channel1StartPower, m_channel1EndPower, m_rampInLength, m_rampOutLength);
        const auto channel2Power = assignRamp(contour, m_channel2StartPower, m_channel2EndPower, m_rampInLength, m_rampOutLength);

        geo2d::TAnnotatedArray<geo2d::DPoint> contourOut;
        contourOut.getData() = contour;
        contourOut.getRank() = std::vector<int>(contour.size(), eRankMax);
        contourOut.getScalarData(geo2d::AnnotatedDPointarray::Scalar::LaserVelocity) = std::vector<double>(contour.size(), -1);
        contourOut.getScalarData(geo2d::AnnotatedDPointarray::Scalar::LaserPower) = channel1Power;
        contourOut.getScalarData(geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing) = channel2Power;

        contourOutArray.ref().emplace_back(contourOut);

        //debug
        if (m_oVerbosity > eLow)
        {
            for (std::size_t i = 0; i < contour.size(); ++i)
            {
                wmLog(eInfo, "(x: %f, y: %f)  (P1: %f, P2: %f)\n", contour[i].x, contour[i].y, channel1Power[i], channel2Power[i]);
            }
        }
    }

    preSignalAction();
    m_contourOut.signal(contourOutArray);
}

} //namespace filter
} //namespace precitec
