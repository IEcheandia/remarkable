#include "metrology.h"
#include "module/moduleLogger.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <unistd.h>

#include "opencv2/opencv.hpp"

#include <algorithm>
#include <limits>
#include <utility>

#include <regex>

namespace precitec
{
namespace filter
{

Metrology::Metrology()
    : TransformFilter("Metrology", Poco::UUID("21100504-17f0-4545-b57f-dd2b9225f03f"))
    , m_imageFrameIn(nullptr)
    , m_contourIn(nullptr)
    , m_positionXOut(this, "PositionX")
    , m_positionYOut(this, "PositionY")
    , m_angleOut(this, "Angle")
    , m_contourSampleNumber(0)
    , m_angleStart(0.0)
    , m_angleExtent(0.0)
    , m_positionX(0.0)
    , m_positionY(0.0)
    , m_angle(0.0)
    , m_trafo(nullptr)
{
    parameters_.add("ContourSampleNumber", fliplib::Parameter::TYPE_uint, m_contourSampleNumber);
    parameters_.add("AngleStart", fliplib::Parameter::TYPE_double, m_angleStart);
    parameters_.add("AngleExtent", fliplib::Parameter::TYPE_double, m_angleExtent);
    setInPipeConnectors({
        {Poco::UUID("904dfd48-7363-47aa-8f47-42828370950c"), m_imageFrameIn, "ImageIn", 1, "ImageIn"},
        {Poco::UUID("69d81b2b-e3ce-4f91-957c-a9314ad416fc"), m_contourIn, "ContourIn", 1, "ContourIn"},
    });
    setOutPipeConnectors({
        {Poco::UUID("fc421c18-68db-40aa-88c5-d078f342e957"), &m_positionXOut, "PositionXOut", 1, "PositionXOut"},
        {Poco::UUID("f8d9eb62-e60b-4526-bb40-606fb390356a"), &m_positionYOut, "PositionYOut", 1, "PositionYOut"},
        {Poco::UUID("4d890856-6585-4fd0-bf5f-85e4126533f4"), &m_angleOut, "AngleOut", 1, "AngleOut"}
    });
    setVariantID(Poco::UUID("58c61058-a85b-4268-995e-957728ab89a6"));
}

void Metrology::setParameter()
{
    TransformFilter::setParameter();

    m_contourSampleNumber = parameters_.getParameter("ContourSampleNumber").convert<unsigned int>();

    const double DEG2RAD = M_PI / 180;
    m_angleStart = parameters_.getParameter("AngleStart").convert<double>() * DEG2RAD;
    m_angleExtent = parameters_.getParameter("AngleExtent").convert<double>() * DEG2RAD;
}

void Metrology::paint()
{
    if (m_trafo.isNull() || m_oVerbosity == eNone)
    {
        return;
    }

    image::OverlayCanvas &currentCanvas = canvas<image::OverlayCanvas>(m_oCounter);
    image::OverlayLayer &contourLayer = currentCanvas.getLayerContour();

    std::vector<cv::Point2f> rotatedContour = rotateContour(m_contour, m_angle);

    for (size_t i = 1; i < rotatedContour.size(); i++)
    {
        const double x1 = rotatedContour[i-1].x + m_positionX + m_trafo->dx();
        const double x2 = rotatedContour[i].x + m_positionX + m_trafo->dx();
        const double y1 = rotatedContour[i-1].y + m_positionY + m_trafo->dy();
        const double y2 = rotatedContour[i].y + m_positionY + m_trafo->dy();
        contourLayer.add<image::OverlayLine>(x1, y1, x2, y2, image::Color::Yellow());
    }
}

bool Metrology::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_imageFrameIn = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    else if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void Metrology::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contour = m_contourIn->read(m_oCounter);
    const auto &contourIn = contour.ref().at(0);

    const auto &contourPoints = contourIn.getData();
    m_contour.clear();
    for (const auto &point : contourPoints)
    {
        m_contour.emplace_back(point.x, point.y);
    }

    if (m_contour.empty())
    {
        wmLog(eError, "Metrology: Invalid contour, please check contour input");
        m_contour.emplace_back(0.0, 0.0);
    }

    if (m_contourSampleNumber != 0)
    {
        m_sampledContour = sampleContour(m_contour, m_contourSampleNumber);
    }
    else
    {
        m_sampledContour = m_contour;
    }

    const auto &frame = m_imageFrameIn->read(m_oCounter);
    const auto &imageIn = frame.data();
    m_trafo = frame.context().trafo();
    m_angle = m_angleStart + m_angleExtent * 0.5;

    cv::Mat image(imageIn.height(), imageIn.width(), CV_8UC1, (void*)(imageIn.begin()), imageIn.stride());

    const double DEG2RAD = M_PI / 180;

    //calculate the distance transform of inverted image.
    //expected image has non-zero edges and zero elsewhere.
    //inverting the input image results in edge pixels being
    //zero, and non-edge pixels being 1.
    //The output type of imageTransform is 32-bit float single
    //channel image
    cv::Mat imageDistance;
    cv::distanceTransform(~image, imageDistance, cv::DIST_L2, 3);

    cv::Point2f position;

    const float angleResolution = 2 * DEG2RAD;
    const unsigned int sampleDistance = 5;

    auto pose = fastFit(imageDistance, m_sampledContour, m_angleStart, m_angleExtent, sampleDistance, angleResolution);

    position = pose.first;
    m_angle = pose.second;
    cv::Rect2f reducedSearchRegion =
    cv::Rect2f
    (
        position - cv::Point2f(2 * sampleDistance, 2 * sampleDistance),
        position + cv::Point2f(2 * sampleDistance, 2 * sampleDistance)
    );

    reducedSearchRegion = reducedSearchRegion & cv::Rect2f(0, 0, imageDistance.cols - 1, imageDistance.rows - 1);

    pose = preciseFit(imageDistance, m_sampledContour, reducedSearchRegion, m_angleStart, m_angleExtent, 1, 0);
    position = pose.first;
    m_angle = pose.second;

    m_positionX = position.x;
    m_positionY = position.y;

    const double rank = interface::Limit; //1.0
    const interface::GeoDoublearray geoOutX(frame.context(), geo2d::TArray<double>(1, m_positionX, rank), interface::ResultType::AnalysisOK, rank);
    const interface::GeoDoublearray geoOutY(frame.context(), geo2d::TArray<double>(1, m_positionY, rank), interface::ResultType::AnalysisOK, rank);
    const interface::GeoDoublearray geoOutAngle(frame.context(), geo2d::TArray<double>(1, m_angle / DEG2RAD, rank), interface::ResultType::AnalysisOK, rank);

    preSignalAction();
    m_positionXOut.signal(geoOutX);
    m_positionYOut.signal(geoOutY);
    m_angleOut.signal(geoOutAngle);

}

float stringToFloat(const std::string& input) {
    std::stringstream ss;
    float result;
    static std::locale uselocale("C");
    ss.imbue(uselocale);
    ss << input;
    ss >> result;
    return result;
}

std::vector<cv::Point2f> parseArray(const std::string& array)
{
    std::vector <cv::Point2f> contour;
    const std::regex regexBracket("^ *\\[([0-9. ;+-]*)\\] *$");
    const std::regex regexSemicolonDelimit("[^;]+");
    const std::regex regexNumeric("[-+]?[0-9]+\\.?[0-9]*");

    std::smatch match;

    std::regex_search(array, match, regexBracket);
    if (match.size() != 2)
    {
        return std::vector<cv::Point2f>();
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
            return std::vector<cv::Point2f>();
        }
        contour.push_back(
            cv::Point2f
            (
                stringToFloat(values[0]),
                stringToFloat(values[1])
            )
        );
    }
    return contour;
}

std::vector<float> integrateContour(const std::vector<cv::Point2f>& contour)
{
    if (contour.empty())
    {
        return std::vector<float>();
    }

    std::vector<float> lengths(contour.size(), 0);

    for (size_t i = 0; i < contour.size() - 1; i++)
    {
        const cv::Point2f point1 = contour.at(i);
        const cv::Point2f point2 = contour.at(i + 1);

        const float dx = point2.x - point1.x;
        const float dy = point2.y - point1.y;
        lengths.at(i + 1) = lengths.at(i) + std::sqrt(dx * dx + dy * dy);
    }

    return lengths;
}

std::vector<cv::Point2f> sampleContour(const std::vector<cv::Point2f>& contour, const unsigned int nSample)
{
    std::vector<float> lengths = integrateContour(contour);
    std::vector<cv::Point2f> sampledContour(nSample);

    unsigned int j = 1;
    for (unsigned int i = 0; i < nSample; i++)
    {
        float c = (i + 0.5) * lengths.back() / nSample;
        for (; j < contour.size(); j++)
        {
            if (c < lengths[j])
            {
                const cv::Point2f& u = contour[j - 1];
                const cv::Point2f& v = contour[j];
                const float fraction = (c - lengths[j - 1]) / (lengths[j] - lengths[j - 1]);
                sampledContour[i] = cv::Point2f
                (
                    u.x + (v.x - u.x) * fraction,
                    u.y + (v.y - u.y) * fraction
                );
                break;
            }
        }
    }

    return sampledContour;
}

std::vector<cv::Point2f> rotateContour(const std::vector<cv::Point2f>& contour, float angle)
{
    std::vector<cv::Point2f> contourRotated(contour.size());

    const float cosine = cos(angle);
    const float sine = sin(angle);

    for (size_t i = 0; i < contour.size(); i++)
    {
        contourRotated[i].x = cosine * contour[i].x - sine * contour[i].y;
        contourRotated[i].y = sine * contour[i].x + cosine * contour[i].y;
    }

    return contourRotated;
}

cv::Rect2f contourBoundingBox(const std::vector<cv::Point2f>& contour)
{
    float top = contour.empty() ? 0 : contour.at(0).y;
    float bottom = top;
    float left = contour.empty() ? 0 : contour.at(0).x;
    float right = left;

    for (const auto& point : contour)
    {
        if (point.y < top)
        {
            top = point.y;
        }
        if (point.y > bottom)
        {
            bottom = point.y;
        }
        if (point.x < left)
        {
            left = point.x;
        }
        if (point.x > right)
        {
            right = point.x;
        }
    }

    return cv::Rect2f
    (
        cv::Point2f(left, top),
        cv::Point2f(right, bottom)
    );
}

std::pair<cv::Point2f, float> preciseFit(const cv::Mat& distanceImage, const std::vector<cv::Point2f>& contour, const cv::Rect2f& searchRegion,
    const float angleStart, const float angleExtent, const unsigned int subsampleDistance, float subsampleAngle)
{
    if (subsampleAngle <= 0)
    {
        //calculate the angle so that when the contour is rotated, the
        //furthest contour point from rotation center is moved by one pixel
        cv::Rect2f box = contourBoundingBox(contour);
        const auto boxXMax = std::max(std::abs(box.tl().x), std::abs(box.br().x));
        const auto boxYMax = std::max(std::abs(box.tl().y), std::abs(box.br().y));
        const auto boxMaxDistance = std::max(1.0f, std::sqrt(boxXMax * boxXMax + boxYMax * boxYMax));
        subsampleAngle = std::asin(0.5 / boxMaxDistance);
    }

    int iMin = searchRegion.x;
    int jMin = searchRegion.y;
    auto angleMin = angleStart;
    auto sumMin = std::numeric_limits<float>::infinity();

    const int angleStep = angleExtent / subsampleAngle + 1;

    for (int j = searchRegion.y; j < searchRegion.y + searchRegion.height; j += subsampleDistance)
    {
        for (int i = searchRegion.x; i < searchRegion.x + searchRegion.width; i += subsampleDistance)
        {
            for (int step = 0; step < angleStep; ++step)
            {
                const auto angle = angleStart + step * subsampleAngle;

                float sum = 0;
                auto contourRotated = rotateContour(contour, angle);

                for (const auto& point : contourRotated)
                {
                    int x = point.x + i;
                    int y = point.y + j;
                    x = x < 0 ? 0 : (x > distanceImage.cols - 1 ? distanceImage.cols - 1 : x);
                    y = y < 0 ? 0 : (y > distanceImage.rows - 1 ? distanceImage.rows - 1 : y);

                    const auto pixel = distanceImage.at<float>(y, x);
                    sum = sum + pixel * pixel;
                }

                if (sum < sumMin)
                {
                    sumMin = sum;
                    iMin = i;
                    jMin = j;
                    angleMin = angle;
                }
            }
        }
    }

    return std::make_pair(cv::Point2f(iMin, jMin), angleMin);
}

std::pair<cv::Point2f, float> fastFit(const cv::Mat& distanceImage, const std::vector<cv::Point2f>& contour,
    const float angleStart, const float angleExtent, const unsigned int subsampleDistance, float subsampleAngle)
{
    if (subsampleAngle <= 0)
    {
        //calculate the angle so that when the contour is rotated, the
        //furthest contour point from rotation center is moved by one pixel
        cv::Rect2f box = contourBoundingBox(contour);
        const auto boxXMax = std::max(std::abs(box.tl().x), std::abs(box.br().x));
        const auto boxYMax = std::max(std::abs(box.tl().y), std::abs(box.br().y));
        const auto boxMaxDistance = std::max(1.0f, std::sqrt(boxXMax * boxXMax + boxYMax * boxYMax));
        subsampleAngle = std::asin(0.5 / boxMaxDistance);
    }

    int iMin = 0;
    int jMin = 0;
    auto angleMin = angleStart;
    auto sumMin = std::numeric_limits<float>::infinity();

    const int angleStep = angleExtent / subsampleAngle + 1;

    for (int step = 0; step < angleStep; ++step)
    {
        const auto angle = angleStart + step * subsampleAngle;
        const auto contourRotated = rotateContour(contour, angle);
        const auto box = contourBoundingBox(contourRotated);
        const auto iStart = -box.tl().x;
        const auto iEnd = distanceImage.cols - box.br().x;
        const auto jStart = -box.tl().y;
        const auto jEnd = distanceImage.rows - box.br().y;
        for (int j = jStart; j < jEnd; j += subsampleDistance)
        {
            for (int i = iStart; i < iEnd; i += subsampleDistance)
            {
                float sum = 0;
                for (const auto& point : contourRotated)
                {
                    int x = point.x + i;
                    int y = point.y + j;
                    x = x < 0 ? 0 : (x > distanceImage.cols - 1 ? distanceImage.cols - 1 : x);
                    y = y < 0 ? 0 : (y > distanceImage.rows - 1 ? distanceImage.rows - 1 : y);

                    const auto pixel = distanceImage.at<float>(y, x);
                    sum = sum + pixel * pixel;
                }

                if (sum < sumMin)
                {
                    sumMin = sum;
                    iMin = i;
                    jMin = j;
                    angleMin = angle;
                }
            }
        }
    }

    return std::make_pair(cv::Point2f(iMin, jMin), angleMin);
}

} //namespace filter
} //namespace precitec
