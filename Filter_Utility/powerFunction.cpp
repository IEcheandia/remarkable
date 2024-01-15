#include "powerFunction.h"

#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <utility>
#include <cmath>
#include <regex>

#define FILTER_ID                 "b55c4fa0-09cd-49e5-bf62-9ee3ee230c1f"
#define VARIANT_ID                "581fdd2d-4a6a-4409-89ad-ef8ae141ab70"

#define PIPE_ID_CONTOURIN         "cc588737-4fc7-4c7f-a925-7fb1e64d8b68"
#define PIPE_ID_CONTOUROUT        "5e99b7c6-bf72-4427-bca1-3f25658217b9"

typedef std::vector<precitec::geo2d::DPoint> ContourType;
typedef std::vector<std::pair<double, double>> ArrayType;

static double stringToDouble(const std::string& input)
{
    std::stringstream ss;
    double result;
    static std::locale useLocale("C");
    ss.imbue(useLocale);
    ss << input;
    ss >> result;
    return result;
}

static ArrayType parseArray(const std::string& arrayString)
{
    ArrayType array;
    const std::regex regexBracket("^ *\\[([0-9. ;+-]*)\\] *$");
    const std::regex regexSemicolonDelimit("[^;]+");
    const std::regex regexNumeric("[-+]?[0-9]+\\.?[0-9]*");

    std::smatch match;

    std::regex_search(arrayString, match, regexBracket);
    if (match.size() != 2)
    {
        return {};
    }
    std::string s1 = match[1].str();

    for (std::sregex_iterator i = std::sregex_iterator(s1.begin(), s1.end(), regexSemicolonDelimit); i != std::sregex_iterator(); ++i)
    {
        std::smatch m = *i;
        std::string s2 = m.str();

        std::vector<std::string> values;
        for (std::sregex_iterator j = std::sregex_iterator(s2.begin(), s2.end(), regexNumeric); j != std::sregex_iterator(); ++j)
        {
            std::smatch n = *j;
            values.push_back(n.str());
        }
        if (values.size() != 2)
        {
            return {};
        }
        array.emplace_back(stringToDouble(values[0]), stringToDouble(values[1]));
    }
    return array;
}

void validateFunction(const ArrayType& f)
{
    bool valid = true;
    for (std::size_t i = 0; i < f.size(); ++i)
    {
        valid &= f[i].first >= 0;
    }

    if (!valid)
    {
        throw std::invalid_argument("validateFunction: received negative value");
    }

    for (std::size_t i = 1; i < f.size(); ++i)
    {
        valid &= f[i].first >= f[i - 1].first;
    }

    if (!valid)
    {
        throw std::invalid_argument("validateFunction: x values must steadily increase");
    }

    valid &= f.size() > 1;

    if (!valid)
    {
        throw std::invalid_argument("validateFunction: not a valid function. Either wrong format or contains less than 2 points");
    }
}

static std::vector<double> integrateContour(const ContourType &contourIn)
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

static ContourType repeatContour(const ContourType &contourIn, double n)
{
    if (contourIn.size() < 2 || n < 0)
    {
        return contourIn;
    }

    double nWhole;
    const auto nFractional = std::modf(n, &nWhole);

    ContourType contourOut;
    contourOut.reserve(contourIn.size() * std::ceil(n));

    for (int i = 0; i < nWhole; ++i)
    {
        contourOut.insert(std::end(contourOut), std::begin(contourIn), std::end(contourIn));
    }

    if (nFractional == 0.0)
    {
        return contourOut;
    }

    contourOut.emplace_back(contourIn.front());

    const auto contourLength = integrateContour(contourIn).back();
    const auto repeatLength = nFractional * contourLength;

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

static ContourType discretizeContour(const ContourType &contourIn, double step)
{
    if (contourIn.empty() || step <= 0.0)
    {
        return contourIn;
    }

    ContourType contourOut;

    contourOut.emplace_back(contourIn[0]);

    std::size_t i = 1;

    while (i < contourIn.size())
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
        else if (d < step)
        {
            contourOut.emplace_back(contourIn[i]);
            ++i;
        }
        else
        {
            contourOut.emplace_back(
                ux + dx / d * step,
                uy + dy / d * step
            );
        }
    }

    return contourOut;
}

static std::vector<double> assignFunction(const std::vector<double> &s, const ArrayType &p)
{
    std::vector<double> f(s.size(), 0);

    std::size_t u = 0;

    for (std::size_t i = 0; i < s.size(); ++i)
    {
        for (; u < p.size() - 2; ++u)
        {
            if (s[i] >= p[u].first &&
                s[i] < p[u + 1].first)
            {
                break;
            }
        }

        const auto v = u + 1;
        const auto xu = p[u].first;
        const auto yu = p[u].second;
        const auto xv = p[v].first;
        const auto yv = p[v].second;

        f[i] = xv == xu ? // only occurs when x[end] == x[end - 1]
               yv :
               yu + (s[i] - xu) / (xv - xu) * (yv - yu);
    }

    return f;
}

static std::vector<double> maxNormalize(const std::vector<double> &f, double maxNorm = 1.0)
{
    const auto max = f.back(); //max is at the end of function!

    if (max <= 0.0)
    {
        return f;
    }

    std::vector<double> fNorm(f.size());

    for (std::size_t i = 0; i < f.size(); ++i)
    {
        fNorm[i] = f[i] / max * maxNorm;
    }

    return fNorm;
}

namespace precitec
{
namespace filter
{

PowerFunction::PowerFunction()
    : TransformFilter("PowerFunction", Poco::UUID(FILTER_ID))
    , m_contourIn(nullptr)
    , m_contourOut(this, "ContourOut")
    , m_stepSize(500e-3)
    , m_channel1Power("[0 0; 1 0]")
    , m_channel2Power("[0 0; 1 0]")
{
    parameters_.add("StepSize", fliplib::Parameter::TYPE_double, m_stepSize);
    parameters_.add("Channel1Power", fliplib::Parameter::TYPE_string, m_channel1Power);
    parameters_.add("Channel2Power", fliplib::Parameter::TYPE_string, m_channel2Power);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_contourIn, "ContourIn", 1, "ContourIn"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUROUT), &m_contourOut, "ContourOut", 1, "ContourOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void PowerFunction::setParameter()
{
    TransformFilter::setParameter();

    m_stepSize = parameters_.getParameter("StepSize").convert<double>();
    m_channel1Power = parameters_.getParameter("Channel1Power").convert<std::string>();
    m_channel2Power = parameters_.getParameter("Channel2Power").convert<std::string>();
}

void PowerFunction::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }
}

bool PowerFunction::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void PowerFunction::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contourInArray = m_contourIn->read(m_oCounter);

    interface::GeoVecAnnotatedDPointarray contourOutArray(contourInArray.context(), {}, contourInArray.analysisResult(), contourInArray.rank());

    auto p1 = parseArray(m_channel1Power);
    auto p2 = parseArray(m_channel2Power);

    validateFunction(p1);
    validateFunction(p2);

    for (auto &p : p1)
    {
        p.second = p.second / 100;
    }

    for (auto &p : p2)
    {
        p.second = p.second / 100;
    }

    const auto p1xMax = p1.back().first;
    const auto p2xMax = p2.back().first;

    const auto pxMax = std::max({p1xMax, p2xMax, 1.0});

    if (p1.front().first > 0)
    {
        if (p1.front().second != 0.0)
        {
            p1.insert(p1.begin(), {p1.front().first, 0.0});
        }
        p1.insert(p1.begin(), {0.0, 0.0});
    }

    if (p2.front().first > 0)
    {
        if (p2.front().second != 0.0)
        {
            p2.insert(p2.begin(), {p2.front().first, 0.0});
        }
        p2.insert(p2.begin(), {0.0, 0.0});
    }

    if (p1.back().first < pxMax)
    {
        if (p1.back().second != 0.0)
        {
            p1.emplace_back(p1.back().first, 0.0);
        }
        p1.emplace_back(pxMax, 0.0);
    }

    if (p2.back().first < pxMax)
    {
        if (p2.back().second != 0.0)
        {
            p2.emplace_back(p2.back().first, 0.0);
        }
        p2.emplace_back(pxMax, 0.0);
    }

    for (const auto &contourIn : contourInArray.ref())
    {
        ContourType contour;

        const auto &contourInData = contourIn.getData();

        contour = repeatContour(contourInData, pxMax);
        contour = discretizeContour(contour, m_stepSize);

        auto contourIntegral = integrateContour(contour);
        const auto integralNorm = maxNormalize(contourIntegral, pxMax);

        const auto f1 = assignFunction(integralNorm, p1);
        const auto f2 = assignFunction(integralNorm, p2);

        geo2d::TAnnotatedArray<geo2d::DPoint> contourOut;
        contourOut.getData() = contour;
        contourOut.getRank() = std::vector<int>(contour.size(), eRankMax);
        contourOut.getScalarData(geo2d::AnnotatedDPointarray::Scalar::LaserVelocity) = std::vector<double>(contour.size(), -1);
        contourOut.getScalarData(geo2d::AnnotatedDPointarray::Scalar::LaserPower) = f1;
        contourOut.getScalarData(geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing) = f2;

        //debug
        if (m_oVerbosity > eNone)
        {
            wmLog(eInfo, "Output Contour Length: %f\n", contourIntegral.back());
            wmLog(eInfo, "Output Contour Total Points: %f\n", contour.size());
            wmLog(eInfo, "Maximum x value: %f\n", pxMax);
        }

        if (m_oVerbosity > eLow)
        {
            for (std::size_t i = 0; i < contour.size(); ++i)
            {
                wmLog(eInfo, "(x: %f, y: %f)  (P1: %f pct., P2: %f pct.) (l: %f)\n", contour[i].x, contour[i].y, f1[i] * 100, f2[i] * 100, integralNorm[i]);
            }
        }

        contourOutArray.ref().emplace_back(contourOut);
    }

    preSignalAction();
    m_contourOut.signal(contourOutArray);
}

} //namespace filter
} //namespace precitec
