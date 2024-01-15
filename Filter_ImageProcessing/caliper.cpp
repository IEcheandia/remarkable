#include "caliper.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include "tinyexpr.h"

#include <cstddef>
#include <cstdint>
#include <opencv2/opencv.hpp>
#include <cmath>

#define FILTER_ID                 "43ccecb9-b1eb-41ea-9ed3-01899d014419"
#define VARIANT_ID                "c8598781-dae1-4a2d-bc36-87ac297ba732"

#define PIPE_ID_IMAGEIN           "b540bc61-9089-4595-8088-0c2e23dd70a8"
#define PIPE_ID_CENTERXIN         "3d16880f-30cb-45fd-9f1b-b83717feedbf"
#define PIPE_ID_CENTERYIN         "ceb60147-5554-4166-bad5-d05ae8ce2f6b"
#define PIPE_ID_WIDTHIN           "02e2818d-1665-4d0b-bb11-75761bde6509"
#define PIPE_ID_HEIGHTIN          "0fa78f9d-2710-4fb9-b93b-37479c3e9fc1"
#define PIPE_ID_ANGLEIN           "6698b35b-ff83-43c1-bc48-03c24cdeab6a"

#define PIPE_ID_X1OUT             "39a8e98f-a605-4fa4-8ad0-0a6fda14cfc1"
#define PIPE_ID_Y1OUT             "eba14e91-28cd-4206-8afc-4e619d5d07ba"
#define PIPE_ID_X2OUT             "301fb792-abde-4b01-8d31-f3017cd103cb"
#define PIPE_ID_Y2OUT             "9a93b654-ea4e-4011-b27f-b2a073a64a11"
#define PIPE_ID_ANGLEOUT          "61fcaccc-279d-4fdc-bf99-abcc11bc2a34"
#define PIPE_ID_SCOREOUT          "32955cf7-18af-4656-ac26-7b05451023c1"

void rotatedROI(const uint8_t *image, int height, int width, std::ptrdiff_t stride, uint8_t *dst, double x, double y, int roiHeight, int roiWidth, double angle)
{
    cv::Mat srcImage(height, width, CV_8UC1, (void *)image, stride);
    cv::Mat dstImage(roiHeight, roiWidth, CV_8UC1, (void *)dst);

    if (height < 1 || width < 1)
    {
        dstImage = 0;
        return;
    }

    if (roiHeight < 0)
    {
        roiHeight = 0;
    }

    if (roiWidth < 0)
    {
        roiWidth = 0;
    }

    cv::Mat affineMatrix = cv::Mat::zeros(2, 3, CV_64FC1);

    // Define the values of the affine matrix
    const double c = std::cos(angle * M_PI / 180);
    const double s = std::sin(angle * M_PI / 180);

    const double cx = (roiWidth - 1) / 2.0;
    const double cy = (roiHeight - 1) / 2.0;

    affineMatrix.at<double>(0, 0) = c;
    affineMatrix.at<double>(0, 1) = -s;
    affineMatrix.at<double>(0, 2) = x - c * cx + s * cy;
    affineMatrix.at<double>(1, 0) = s;
    affineMatrix.at<double>(1, 1) = c;
    affineMatrix.at<double>(1, 2) = y - s * cx - c * cy;

    cv::warpAffine(srcImage, dstImage, affineMatrix, cv::Size(roiWidth, roiHeight), cv::INTER_LINEAR | cv::WARP_INVERSE_MAP);
}

cv::Mat angleProfile(const uint8_t *image, int height, int width, std::ptrdiff_t stride, double x, double y, int roiHeight, int roiWidth, double angle)
{
    cv::Mat cvRoi(roiHeight, roiWidth,  CV_8UC1);
    rotatedROI(image, height, width, stride, (uint8_t *)cvRoi.data, x, y, roiHeight, roiWidth, angle);

    cv::Mat cvProfile;
    cv::reduce(cvRoi, cvProfile, 0, cv::REDUCE_AVG, CV_32F);
    cvProfile = cvProfile / 255;

    return cvProfile;
}

float profileScore(cv::InputArray& profile)
{
    int width = profile.total();

    if (profile.total() == 0)
    {
        return 0;
    }

    std::vector<float> kernel = {-1, 0, 1};

    cv::Mat result;
    cv::filter2D(profile, result, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

    return result.dot(result) / width;
}

double optimizeAngle(const uint8_t* image, int height, int width, std::ptrdiff_t stride, double x, double y, int roiHeight, int roiWidth, double angleStart, int angleSteps, double stepAngle, std::vector<double>& profileOut)
{
    if (angleSteps < 1)
    {
        angleSteps = 1;
    }

    if (roiHeight < 1 || roiWidth < 1)
    {
        return angleStart;
    }

    float optScore = -1.0f;
    cv::Mat cvOptProfile;
    double optAngle = angleStart;

    for (int i = 0; i < angleSteps; ++i)
    {
        const auto angle = angleStart + i * stepAngle;
        const auto cvProfile = angleProfile(image, height, width, stride, x, y, roiHeight, roiWidth, angle);

        const auto score = profileScore(cvProfile);

        if (score > optScore)
        {
            optScore = score;
            cvOptProfile = cvProfile;
            optAngle = angle;
        }
    }

    cvOptProfile.convertTo(profileOut, CV_64F);

    return optAngle;
}

std::vector<double> normalizeVectorMinMax(const std::vector<double>& vector)
{
    if (vector.empty())
    {
        return {};
    }

    double minValue = *std::min_element(vector.begin(), vector.end());
    double maxValue = *std::max_element(vector.begin(), vector.end());

    if (minValue == maxValue)
    {
        minValue = 0.0;
        maxValue = 1.0;
    }

    std::vector<double> normalizedVector(vector.size());

    for (std::size_t i = 0; i < vector.size(); ++i)
    {
        normalizedVector[i] = (vector[i] - minValue) / (maxValue - minValue);
    }

    return normalizedVector;
}

std::vector<double> invertedGaussianKernel(double sigma)
{
    int kernelHalfSize = std::max(1, (int)std::ceil(sigma * 3));
    int kernelSize = kernelHalfSize * 2 + 1;

    std::vector<double> kernel(kernelSize);

    double abssum = 0.0;

    for (int i = 0; i < kernelSize; ++i)
    {
        int x = i - kernelHalfSize;
        const auto a = std::exp(-x * x * 0.5 / sigma / sigma);
        kernel[i] = x < 0 ? -a : x > 0 ? a : 0.0;

        abssum += std::abs(kernel[i]);
    }

    for (int i = 0; i < kernelSize; ++i)
    {
        kernel[i] = kernel[i] / abssum * 2;
    }

    return kernel;
}

std::vector<double> convolve1D(const std::vector<double>& data, const std::vector<double>& kernel)
{
    if (data.empty())
    {
        return {};
    }

    std::vector<double> result;
    cv::filter2D(data, result, -1, kernel, cv::Point(-1, -1), 0, cv::BORDER_DEFAULT);

    return result;
}

std::vector<double> cumsumNorm(const std::vector<double>& input)
{
    std::vector<double> output = input;

    for (std::size_t i = 1; i < output.size(); ++i)
    {
        output[i] += output[i - 1];
    }

    if (!output.empty() && output.back() != 0.0)
    {
        for (std::size_t i = 0; i < output.size(); ++i)
        {
            output[i] = output[i] / output.back();
        }
    }

    return output;
}

std::vector<PeakType> getPeakType(const std::vector<double>& data)
{
    if (data.empty())
    {
        return {};
    }

    cv::Mat D;
    cv::Mat E;

    cv::Mat element = cv::getStructuringElement(cv::MORPH_RECT, {3, 1});

    cv::dilate(data, D, element);
    cv::erode(data, E, element);

    std::vector<PeakType> output(data.size(), NONPEAK);

    for (std::size_t i = 1;i < data.size() - 1; ++i)
    {
        const auto Di = D.at<double>(i);
        const auto Ei = E.at<double>(i);
        const auto x = data[i];

        if (x == Di && Di != Ei && x > 0)
        {
            output[i] = MAX_POSITIVE_PEAK;
        }
        else if (x == Ei && Di != Ei && x < 0)
        {
            output[i] = MIN_NEGATIVE_PEAK;
        }
        else if (x == Di && Di != Ei && x < 0)
        {
            output[i] = MAX_NEGATIVE_PEAK;
        }
        else if (x == Ei && Di != Ei && x > 0)
        {
            output[i] = MIN_POSITIVE_PEAK;
        }
    }

    return output;
}

std::vector<PeakCandidate> getPeaks(std::vector<PeakType> data, PeakType type)
{
    const int N = data.size();

    std::vector<PeakCandidate> output;

    for (int i = 0; i < N; ++i)
    {
        if (data[i] == type)
        {
            output.push_back({i, i, 0.0});
        }
    }

    return output;
}

std::vector<PeakCandidate> getPeakPairs(std::vector<PeakType> data, PeakType firstType, PeakType secondType)
{
    const int N = data.size();

    std::vector<PeakCandidate> output;

    for (int i = 0; i < N; ++i)
    {
        for (int j = i + 1; j < N; ++j)
        {
            if (data[i] == firstType && data[j] == secondType)
            {
                 output.push_back({i, j, 0.0});
            }
        }
    }

    return output;
}

double fuzzy(double x, double x0, double x1, double x2, double x3)
{
    // x0 <= x1 <= x2 <= x3
    if (x1 < x0)
    {
        x1 = x0;
    }
    if (x2 < x1)
    {
        x2 = x1;
    }
    if (x3 < x2)
    {
        x3 = x2;
    }

    double y;

    if (x < x0)
    {
        y = 0.0;
    }
    else if (x < x1)
    {
        y = x1 != x0 ? (x - x0) / (x1 - x0) : 1.0;
    }
    else if (x < x2)
    {
        y = 1.0;
    }
    else
    {
        y = x2 != x3 ? (x - x3) / (x2 - x3) : 0.0;
    }

    if (y > 1.0)
    {
        y = 1.0;
    }
    else if (y < 0.0)
    {
        y = 0.0;
    }

    return y;
}

namespace precitec
{
namespace filter
{

Caliper::Caliper()
    : TransformFilter("Caliper", Poco::UUID(FILTER_ID))
    , m_imageIn(nullptr)
    , m_centerXIn(nullptr)
    , m_centerYIn(nullptr)
    , m_widthIn(nullptr)
    , m_heightIn(nullptr)
    , m_angleIn(nullptr)
    , m_x1Out(this, "X1Out")
    , m_y1Out(this, "Y1Out")
    , m_x2Out(this, "X2Out")
    , m_y2Out(this, "Y2Out")
    , m_angleOut(this, "AngleOut")
    , m_scoreOut(this, "ScoreOut")
    , m_angleRange(0.0)
    , m_angleSteps(1)
    , m_normalizeProfileMinMax(true)
    , m_sigma(3)
    , m_minContrast(0.05)
    , m_peakSearchType(MAX_P_TO_MIN_N)
    , m_scoreFunction("")
    , m_trafo(nullptr)
    , m_centerX(0.0)
    , m_centerY(0.0)
    , m_width(0)
    , m_height(0)
    , m_angle(0.0)
    , m_profile()
    , m_convolvedProfile()
    , m_peak()
    , m_candidate()
{
    parameters_.add("AngleRange", fliplib::Parameter::TYPE_double, m_angleRange);
    parameters_.add("AngleSteps", fliplib::Parameter::TYPE_int, m_angleSteps);
    parameters_.add("NormalizeProfileMinMax", fliplib::Parameter::TYPE_bool, m_normalizeProfileMinMax);
    parameters_.add("Sigma", fliplib::Parameter::TYPE_double, m_sigma);
    parameters_.add("MinContrast", fliplib::Parameter::TYPE_double, m_minContrast);
    parameters_.add("PeakSearchType", fliplib::Parameter::TYPE_int, static_cast<int>(m_peakSearchType));
    parameters_.add("ScoreFunction", fliplib::Parameter::TYPE_string, m_scoreFunction);

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_IMAGEIN), m_imageIn, "ImageIn", 1, "ImageIn"},
        {Poco::UUID(PIPE_ID_CENTERXIN), m_centerXIn, "CenterXIn", 1, "CenterXIn"},
        {Poco::UUID(PIPE_ID_CENTERYIN), m_centerYIn, "CenterYIn", 1, "CenterYIn"},
        {Poco::UUID(PIPE_ID_WIDTHIN), m_widthIn, "WidthIn", 1, "WidthIn"},
        {Poco::UUID(PIPE_ID_HEIGHTIN), m_heightIn, "HeightIn", 1, "HeightIn"},
        {Poco::UUID(PIPE_ID_ANGLEIN), m_angleIn, "AngleIn", 1, "AngleIn"},
    });
    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_X1OUT), &m_x1Out, "X1Out", 1, "X1Out"},
        {Poco::UUID(PIPE_ID_Y1OUT), &m_y1Out, "Y1Out", 1, "Y1Out"},
        {Poco::UUID(PIPE_ID_X2OUT), &m_x2Out, "X2Out", 1, "X2Out"},
        {Poco::UUID(PIPE_ID_Y2OUT), &m_y2Out, "Y2Out", 1, "Y2Out"},
        {Poco::UUID(PIPE_ID_ANGLEOUT), &m_angleOut, "AngleOut", 1, "AngleOut"},
        {Poco::UUID(PIPE_ID_SCOREOUT), &m_scoreOut, "ScoreOut", 1, "ScoreOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void Caliper::setParameter()
{
    TransformFilter::setParameter();

    m_angleRange = parameters_.getParameter("AngleRange").convert<double>();
    m_angleSteps = parameters_.getParamValue("AngleSteps").convert<int>();
    m_normalizeProfileMinMax = parameters_.getParamValue("NormalizeProfileMinMax").convert<bool>();
    m_sigma = parameters_.getParamValue("Sigma").convert<double>();
    m_peakSearchType = static_cast<PeakSearchType>(parameters_.getParamValue("PeakSearchType").convert<int>());
    m_minContrast = parameters_.getParamValue("MinContrast").convert<double>();
    m_scoreFunction = parameters_.getParamValue("ScoreFunction").convert<std::string>();
}

void Caliper::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas& rCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer& layer(rCanvas.getLayerContour());
    const interface::Trafo& trafo(*m_trafo);
    const auto origin = trafo(geo2d::Point(0, 0));

    // draw original axis
    const auto ox = origin.x + m_centerX;
    const auto oy = origin.y + m_centerY;

    const auto c = std::cos(m_angle * M_PI / 180);
    const auto s = std::sin(m_angle * M_PI / 180);
    double x1x = c * m_width / 2;
    double x1y = s * m_width / 2;
    double x2x = -x1x;
    double x2y = -x1y;
    double y1x = -s * m_height / 2;
    double y1y = c * m_height / 2;
    double y2x = -y1x;
    double y2y = -y1y;

    layer.add<image::OverlayLine>(ox + x1x + y1x, oy + x1y + y1y, ox + x2x + y1x, oy + x2y + y1y, image::Color::Cyan());
    layer.add<image::OverlayLine>(ox + x1x + y2x, oy + x1y + y2y, ox + x2x + y2x, oy + x2y + y2y, image::Color::Cyan());
    layer.add<image::OverlayLine>(ox + x1x + y1x, oy + x1y + y1y, ox + x1x + y2x, oy + x1y + y2y, image::Color::Cyan());
    layer.add<image::OverlayLine>(ox + x2x + y1x, oy + x2y + y1y, ox + x2x + y2x, oy + x2y + y2y, image::Color::Cyan());

    if (!m_candidate.empty())
    {
        const auto& candidate = m_candidate[0];
        const int i = candidate.first;
        const int j = candidate.second;

        // plot the edges
        {
            const double x1 = i - m_width * 0.5;
            const double y1 = m_height * 0.5;
            const double x2 = i - m_width * 0.5;
            const double y2 = -m_height * 0.5;

            layer.add<image::OverlayLine>(
                ox + c * x1 + s * y1,
                oy + s * x1 - c * y1,
                ox + c * x2 + s * y2,
                oy + s * x2 - c * y2,
                image::Color::Green());
        }
        {
            const double x1 = j - m_width * 0.5;
            const double y1 = m_height * 0.5;
            const double x2 = j - m_width * 0.5;
            const double y2 = -m_height * 0.5;

            layer.add<image::OverlayLine>(
                ox + c * x1 + s * y1,
                oy + s * x1 - c * y1,
                ox + c * x2 + s * y2,
                oy + s * x2 - c * y2,
                image::Color::Green());
        }
    }

    if (m_oVerbosity <= eLow)
    {
        return;
    }

    layer.add<image::OverlayLine>(ox + x1x, oy + x1y, ox + x2x, oy + x2y, image::Color::Cyan());
    layer.add<image::OverlayLine>(ox + y1x, oy + y1y, ox + y2x, oy + y2y, image::Color::Cyan());

    std::vector<double> convolvedProfileX(m_profile.size());
    std::vector<double> convolvedProfileY(m_profile.size());

    const double yScaling = m_height > 100 ? m_height : 100;

    for (std::size_t i = 0; i < m_profile.size(); ++i)
    {
        const double x = i - m_width / 2.0;
        const double y = m_convolvedProfile[i] * yScaling * 0.5;

        convolvedProfileX[i] = c * x + s * y;
        convolvedProfileY[i] = s * x - c * y;
    }

    for (std::size_t i = 1; i < m_profile.size(); ++i)
    {
        layer.add<image::OverlayLine>(
            origin.x + m_centerX + convolvedProfileX[i - 1],
            origin.y + m_centerY + convolvedProfileY[i - 1],
            origin.x + m_centerX + convolvedProfileX[i],
            origin.y + m_centerY + convolvedProfileY[i],
            image::Color::Magenta());
    }

    for (std::size_t i = 0; i < m_profile.size(); ++i)
    {
        if (m_peak[i] == MAX_POSITIVE_PEAK || m_peak[i] == MIN_NEGATIVE_PEAK)
        {
            layer.add<image::OverlayCircle>(
                ox + convolvedProfileX[i],
                oy + convolvedProfileY[i],
                5,
                image::Color::Green()
            );
        }
    }

    // plot candidates
    if (!m_candidate.empty())
    {
        const auto& candidate = m_candidate[0];
        const int i = candidate.first;
        const int j = candidate.second;

        layer.add<image::OverlayCircle>(
                ox + convolvedProfileX[i],
                oy + convolvedProfileY[i],
                2.5,
                image::Color::Green());
        layer.add<image::OverlayCircle>(
                ox + convolvedProfileX[j],
                oy + convolvedProfileY[j],
                2.5,
                image::Color::Green());
    }

    if (m_oVerbosity <= eMedium)
    {
        return;
    }

    std::vector<double> profileX(m_profile.size());
    std::vector<double> profileY(m_profile.size());

    for (std::size_t i = 0; i < m_profile.size(); ++i)
    {
        const double x = i - m_width / 2.0;
        const double y = m_profile[i] * yScaling - yScaling * 0.5;

        profileX[i] = c * x + s * y;
        profileY[i] = s * x - c * y;
    }

    std::vector<double> sumProfileX(m_profile.size());
    std::vector<double> sumProfileY(m_profile.size());

    for (std::size_t i = 0; i < m_profile.size(); ++i)
    {
        const double x = i - m_width / 2.0;
        const double y = m_sumProfile[i] * yScaling - yScaling * 0.5;

        sumProfileX[i] = c * x + s * y;
        sumProfileY[i] = s * x - c * y;
    }

    for (std::size_t i = 1; i < m_profile.size(); ++i)
    {
        layer.add<image::OverlayLine>(
            origin.x + m_centerX + profileX[i - 1],
            origin.y + m_centerY + profileY[i - 1],
            origin.x + m_centerX + profileX[i],
            origin.y + m_centerY + profileY[i],
            image::Color::Yellow());
        layer.add<image::OverlayLine>(
            origin.x + m_centerX + convolvedProfileX[i - 1],
            origin.y + m_centerY + convolvedProfileY[i - 1],
            origin.x + m_centerX + convolvedProfileX[i],
            origin.y + m_centerY + convolvedProfileY[i],
            image::Color::Magenta());
        if (i % 3 == 0)
        {
            layer.add<image::OverlayPoint>(
                origin.x + m_centerX + sumProfileX[i - 1],
                origin.y + m_centerY + sumProfileY[i - 1],
                image::Color::Orange());
        }
    }
}

bool Caliper::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_imageIn = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    else if (pipe.tag() =="CenterXIn")
    {
        m_centerXIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "CenterYIn")
    {
        m_centerYIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "WidthIn")
    {
        m_widthIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "HeightIn")
    {
        m_heightIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == "AngleIn")
    {
        m_angleIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void Caliper::proceedGroup(const void *sender, fliplib::PipeGroupEventArgs& event)
{
    const auto& frame = m_imageIn->read(m_oCounter);
    const auto& centerXArray = m_centerXIn->read(m_oCounter);
    const auto& centerYArray = m_centerYIn->read(m_oCounter);
    const auto& widthArray = m_widthIn->read(m_oCounter);
    const auto& heightArray = m_heightIn->read(m_oCounter);
    const auto& angleArray = m_angleIn->read(m_oCounter);

    if (centerXArray.ref().empty() || centerYArray.ref().empty() || widthArray.ref().empty() || heightArray.ref().empty() || angleArray.ref().empty())
    {
        wmLog(eError, "[Caliper] pipe input cannot be empty!");
    }

    const double centerX = centerXArray.ref().empty() ? 0.0 : centerXArray.ref().getData().front();
    const double centerY = centerYArray.ref().empty() ? 0.0 : centerYArray.ref().getData().front();
    const int width = widthArray.ref().empty() ? 0.0 : widthArray.ref().getData().front();
    const int height = heightArray.ref().empty() ? 0.0 : heightArray.ref().getData().front();
    const double angle = angleArray.ref().empty() ? 0.0 : angleArray.ref().getData().front();

    const auto& imageIn = frame.data();
    const auto& context = frame.context();
    m_trafo = context.trafo();
    m_centerX = centerX;
    m_centerY = centerY;
    m_width = width;
    m_height = height;

    const auto dx = m_trafo->dx();
    const auto dy = m_trafo->dy();

    if (m_oVerbosity == eMax)
    {
        wmLog(eDebug, "[Caliper] centerX: %f, centerY: %f, width: %d, height: %d, angle: %f\n", centerX, centerY, width, height, angle);
        wmLog(eDebug, "[Caliper] image.dx: %d, image.dy: %d\n", dx, dy);
        wmLog(eDebug, "[Caliper] angleRange: %f, angleSteps: %d\n", m_angleRange, m_angleSteps);
    }

    // 1. optimize angle
    const auto angleStart = angle - m_angleRange / 2;
    const double stepAngle = m_angleSteps == 1 ? 0.0 : m_angleRange / (m_angleSteps - 1);

    m_angle = optimizeAngle(imageIn.begin(), imageIn.height(), imageIn.width(), imageIn.stride(), centerX, centerY, height, width, angleStart, m_angleSteps, stepAngle, m_profile);

    // 1.1 normalize profile with minmax
    if (m_normalizeProfileMinMax)
    {
        m_profile = normalizeVectorMinMax(m_profile);
    }

    // 2. convolve profile with a kernel
    const auto kernel = invertedGaussianKernel(m_sigma);
    m_convolvedProfile = convolve1D(m_profile, kernel);

    // 3. calculate cumulative sum of profile
    m_sumProfile = cumsumNorm(m_profile);

    // 4. peak detection
    m_peak = getPeakType(m_convolvedProfile);

    // 4.1 peak contrast filtering
    for (std::size_t i = 0; i < m_peak.size(); ++i)
    {
        if (std::abs(m_convolvedProfile[i]) < m_minContrast)
        {
            m_peak[i] = NONPEAK;
        }
    }

    // 5. detect/filter peak pairs
    switch (m_peakSearchType)
    {
        case MAX_P_TO_MIN_N:
        {
            m_candidate = getPeakPairs(m_peak, MAX_POSITIVE_PEAK, MIN_NEGATIVE_PEAK);
            break;
        }
        case MIN_N_TO_MAX_P:
        {
            m_candidate = getPeakPairs(m_peak, MIN_NEGATIVE_PEAK, MAX_POSITIVE_PEAK);
            break;
        }
        case MAX_P_TO_MAX_P:
        {
            m_candidate = getPeakPairs(m_peak, MAX_POSITIVE_PEAK, MAX_POSITIVE_PEAK);
            break;
        }
        case MIN_N_TO_MIN_N:
        {
            m_candidate = getPeakPairs(m_peak, MIN_NEGATIVE_PEAK, MIN_NEGATIVE_PEAK);
            break;
        }
        case MAX_P:
        {
            m_candidate = getPeaks(m_peak, MAX_POSITIVE_PEAK);
            break;
        }
        case MIN_N:
        {
            m_candidate = getPeaks(m_peak, MIN_NEGATIVE_PEAK);
            break;
        }
        case MAX_P_OR_MIN_N:
        {
            m_candidate = getPeaks(m_peak, MAX_POSITIVE_PEAK);
            const auto negative = getPeaks(m_peak, MIN_NEGATIVE_PEAK);
            m_candidate.insert(m_candidate.end(), negative.begin(), negative.end());
            break;
        }
        default:
        {
            m_candidate.clear();
            break;
        }
    }

    // 6. calculate score
    evaluateCandidateScore();

    std::sort(m_candidate.begin(), m_candidate.end(),
        [](const PeakCandidate& a, const PeakCandidate& b){
            return a.score > b.score;
        });

    // Log candidate infos
    if (m_oVerbosity > eLow)
    {
        for (const auto& c : m_candidate)
        {
            int c1 = c.first;
            int c2 = c.second;

            const double P1 = c1;
            const double P2 = c2;
            const double P = (P1 + P2) / 2;
            const double D = std::abs(P2 - P1);
            const double C1 = std::abs(m_convolvedProfile[c1]);
            const double C2 = std::abs(m_convolvedProfile[c2]);
            const double C = (C1 + C2) / 2;
            const double I = m_sumProfile[c2] - m_sumProfile[c1];
            const double W = m_width;
            const double Pr = P - W / 2;

            std::stringstream ss;
            ss << std::setprecision(3);
            ss << "Score: " << c.score
               << ", P: " << P
               << ", Pr: " << Pr
               << ", P1: " << P1
               << ", P2: " << P2
               << ", D: " << D
               << ", C: " << C
               << ", C1: " << C1
               << ", C2: " << C2
               << ", I: " << I
               << ", W: " << W
               << "\n";

            wmLog(eInfo, ss.str());
        }
    }

    const auto geoRank = interface::Limit;
    const auto arrayRank = eRankMax;

    geo2d::TArray<double> x1Out(1, 0.0, arrayRank);
    geo2d::TArray<double> y1Out(1, 0.0, arrayRank);
    geo2d::TArray<double> x2Out(1, 0.0, arrayRank);
    geo2d::TArray<double> y2Out(1, 0.0, arrayRank);
    geo2d::TArray<double> angleOut(1, 0.0, arrayRank);
    geo2d::TArray<double> scoreOut(1, 0.0, arrayRank);

    if (m_candidate.empty())
    {
        x1Out.assign(1, 0.0, eRankMin);
        y1Out.assign(1, 0.0, eRankMin);
        x2Out.assign(1, 0.0, eRankMin);
        y2Out.assign(1, 0.0, eRankMin);
        angleOut.assign(1, m_angle, eRankMin);
        scoreOut.assign(1, 0.0, eRankMin);
    }
    else
    {
        const auto c = std::cos(m_angle * M_PI / 180);
        const auto s = std::sin(m_angle * M_PI / 180);

        const auto& candidate = m_candidate[0];
        const auto c1 = candidate.first;
        const auto c2 = candidate.second;

        const double rc1 = c1 - m_width * 0.5;
        const double rc2 = c2 - m_width * 0.5;

        const double x1 = centerX + c * rc1;
        const double y1 = centerY + s * rc1;
        const double x2 = centerX + c * rc2;
        const double y2 = centerY + s * rc2;

        x1Out.getData()[0] = x1;
        y1Out.getData()[0] = y1;
        x2Out.getData()[0] = x2;
        y2Out.getData()[0] = y2;
        angleOut.getData()[0] = m_angle;
        scoreOut.getData()[0] = candidate.score;
    }

    const interface::GeoDoublearray geoX1Out(context, x1Out, interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoY1Out(context, y1Out, interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoX2Out(context, x2Out, interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoY2Out(context, y2Out, interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoAngleOut(context, angleOut, interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoScoreOut(context, scoreOut, interface::ResultType::AnalysisOK, geoRank);

    preSignalAction();
    m_x1Out.signal(geoX1Out);
    m_y1Out.signal(geoY1Out);
    m_x2Out.signal(geoX2Out);
    m_y2Out.signal(geoY2Out);
    m_angleOut.signal(geoAngleOut);
    m_scoreOut.signal(geoScoreOut);
}

void Caliper::evaluateCandidateScore()
{
    if (m_scoreFunction == "")
    {
        for (auto& c : m_candidate)
        {
            c.score = 1.0;
        }
        return;
    }

    double P;  // Edge absolute position or absolute postion of edge pair center
    double Pr; // Edge position relative to ROI center or edge pair center position relative to ROI center
    double P1; // Edge 1 absolute position
    double P2; // Edge 2 absolute position
    double D;  // Distance between edge 1 and edge 2, aka P2 - P1
    double C;  // Edge contrast or average contrast of edge pair
    double C1; // Edge 1 contrast
    double C2; // Edge 2 contrast
    double I;  // Ratio of intensity sum between edge 1 and edge 2 over the total intensity sum
    double W;  // Width of ROI

    te_variable vars[] = {
        {"fuzzy", (const void *)fuzzy, TE_FUNCTION5},
        {"P", &P},
        {"Pr", &Pr},
        {"P1", &P1},
        {"P2", &P2},
        {"D", &D},
        {"C", &C},
        {"C1", &C1},
        {"C2", &C2},
        {"I", &I},
        {"W", &W},
    };
    const int varSize = 11; // sizeof(vars) / sizeof(te_variable)

    // compile validate score function, if score function invalid, (set all candidate scores to 1 and) return
    // here we are changing the locale temporarily!
    std::string savedLocale = std::setlocale(LC_NUMERIC, nullptr);
    std::setlocale(LC_NUMERIC, "C");
    int err;
    te_expr *n = te_compile(m_scoreFunction.c_str(), vars, varSize, &err);
    std::setlocale(LC_NUMERIC, savedLocale.c_str());

    if (!n)
    {
        std::stringstream ss;
        ss << "[Caliper] Score function is invalid!\n";
        ss << "\t" << m_scoreFunction << "\n";
        ss << "\t" << std::string(err - 1, ' ') << "^\n";
        ss << "Error near here\n";

        wmLog(eError, ss.str());

        for (auto& c : m_candidate)
        {
            c.score = 1.0;
        }
        return;
    }

    // for each candidate, set the varriables, evaluate the score function and set the corresponding score
    for (std::size_t i = 0; i < m_candidate.size(); ++i)
    {
        auto& c = m_candidate[i];
        int c1 = c.first;
        int c2 = c.second;

        C1 = std::abs(m_convolvedProfile[c1]);
        C2 = std::abs(m_convolvedProfile[c2]);
        C = (C1 + C2) / 2;
        P1 = c1;
        P2 = c2;
        P = (P1 + P2) / 2;
        D = std::abs(P2 - P1);
        I = m_sumProfile[c2] - m_sumProfile[c1];
        W = m_width;
        Pr = P - W / 2;

        const double score = te_eval(n);

        c.score = score;
    }

    te_free(n);
}

} //namespace filter
} //namespace precitec
