#include "templateMatching.h"
#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoImage.h"

#include <unistd.h>

#include "opencv2/opencv.hpp"

static std::pair<double, double> rotatedRectBoundingBoxSize(int height, int width, double angle)
{
    const auto sinA = std::sin(angle / 180 * M_PI);
    const auto cosA = std::cos(angle / 180 * M_PI);

    const auto p0x = 0.0;
    const auto p0y = 0.0;

    const auto p1x = width * cosA;
    const auto p1y = width * sinA;

    const auto p2x = -height * sinA;
    const auto p2y = height * cosA;

    const auto p3x = p1x + p2x;
    const auto p3y = p1y + p2y;

    const auto xmin = std::min({p0x, p1x, p2x, p3x});
    const auto xmax = std::max({p0x, p1x, p2x, p3x});

    const auto ymin = std::min({p0y, p1y, p2y, p3y});
    const auto ymax = std::max({p0y, p1y, p2y, p3y});

    return {ymax - ymin, xmax - xmin};
}

static void copyMakeConstBorder8u(const uint8_t *src, int width, int height, std::ptrdiff_t srcStride, uint8_t *dst, std::ptrdiff_t dstStride, int topBorder, int bottomBorder, int leftBorder, int rightBorder, uint8_t borderValue)
{
    auto dstInner = dst + dstStride * topBorder + leftBorder;
    for (int j = 0; j < height; ++j, dstInner += dstStride, src += srcStride)
    {
        std::memset(dstInner - leftBorder, 0, leftBorder);
        std::memset(dstInner + width, 0, rightBorder);
        std::memcpy(dstInner, src, width);
    }

    for (int j = 0; j < topBorder; ++j)
    {
        std::memset(dst + j * dstStride, 0, width + leftBorder + rightBorder);
    }

    const auto dstLower = dst + dstStride * (height + topBorder);
    for (int j = 0; j < bottomBorder; ++j)
    {
        std::memset(dstLower + j * dstStride, 0, width + leftBorder + rightBorder);
    }
}

static void setZeroToOne(uint8_t *src, int width, int height, std::ptrdiff_t srcStride)
{
    for (int j = 0; j < height; ++j, src += srcStride)
    {
        for (int i = 0; i < width; ++i)
        {
            if (src[i] == 0)
            {
                src[i] = 1;
            }
        }
    }
}

namespace precitec
{
namespace filter
{

TemplateMatching::TemplateMatching()
    : TransformFilter("TemplateMatching", Poco::UUID("19fc6376-43e9-465d-ae87-116ddc4e5ae7"))
    , m_pipeInImageFrame(nullptr)
    , m_pipeAngleStart(nullptr)
    , m_pipePosX(this, "PositionX")
    , m_PipePosY(this, "PositionY")
    , m_pipeAngle(this, "Angle")
    , m_pipeScore(this, "Score")
    , m_templateFileName("")
    , m_angleStart(0.0)
    , m_countStep(1)
    , m_angleStep(0.0)
    , m_pyramidLevels(0)
    , m_cropMode(CropMode::Crop)
{
    parameters_.add("TemplateFileName", fliplib::Parameter::TYPE_string, m_templateFileName);
    parameters_.add("AngleStart", fliplib::Parameter::TYPE_double, m_angleStart);
    parameters_.add("CountStep", fliplib::Parameter::TYPE_uint, m_countStep);
    parameters_.add("AngleStep", fliplib::Parameter::TYPE_double, m_angleStep);
    parameters_.add("PyramidLevels", fliplib::Parameter::TYPE_UInt32, m_pyramidLevels);
    parameters_.add("CropMode", fliplib::Parameter::TYPE_int, static_cast<int>(m_cropMode));
    setInPipeConnectors({
        {Poco::UUID("da593725-957a-4cc2-99bd-15d52d02e531"), m_pipeInImageFrame, "ImageIn", 1, "ImageIn"},
        {Poco::UUID("9bbd9b7d-169c-4f57-a018-547917adbb65"), m_pipeAngleStart, "VariableAngleStart", 1, "VariableAngleStart", fliplib::PipeConnector::ConnectionType::Optional}
    });
    setOutPipeConnectors({
        {Poco::UUID("ab08b8f4-0970-48c7-a2e7-b6145f10c17d"), &m_pipePosX, "PositionX", 1, "PositionX"},
        {Poco::UUID("01679219-2b33-49b5-9b75-f237c317067c"), &m_PipePosY, "PositionY", 1, "PositionY"},
        {Poco::UUID("b002ac91-08b0-46f3-b9bf-c003328a9936"), &m_pipeAngle, "Angle", 1, "Angle"},
        {Poco::UUID("2d41964e-0f13-11ed-861d-0242ac120002"), &m_pipeScore, "Score", 1, "Score"}

    });
    setVariantID(Poco::UUID("00dd635e-f606-4eee-9b55-8c73849f7841"));
}

void TemplateMatching::setParameter()
{
    TransformFilter::setParameter();
    bool updateTemplate = false;
    if (m_templateFileName != parameters_.getParameter("TemplateFileName").convert<std::string>())
    {
        m_templateFileName = parameters_.getParameter("TemplateFileName").convert<std::string>();
        updateTemplate = true;
    }
    //use angleStart from parameter if the optional pipeAngleStart is not connected!
    if (m_pipeAngleStart == nullptr && m_angleStart != parameters_.getParameter("AngleStart").convert<double>())
    {
        m_angleStart = parameters_.getParameter("AngleStart").convert<double>();
        updateTemplate = true;
    }
    if (m_countStep != parameters_.getParameter("CountStep").convert<unsigned int>())
    {
        m_countStep = parameters_.getParameter("CountStep").convert<unsigned int>();
        updateTemplate = true;
    }
    if (m_angleStep != parameters_.getParameter("AngleStep").convert<double>())
    {
        m_angleStep = parameters_.getParameter("AngleStep").convert<double>();
        updateTemplate = true;
    }
    if (m_pyramidLevels != parameters_.getParameter("PyramidLevels").convert<unsigned int>())
    {
        m_pyramidLevels = parameters_.getParameter("PyramidLevels").convert<unsigned int>();
        updateTemplate = true;
    }
    if (m_cropMode != static_cast<CropMode>(parameters_.getParameter("CropMode").convert<int>()))
    {
        m_cropMode = static_cast<CropMode>(parameters_.getParameter("CropMode").convert<int>());
        updateTemplate = true;
    }

    if (updateTemplate == true)
    {
        generateTemplate();
    }
}

void TemplateMatching::paint()
{
    if (m_oVerbosity == eNone || m_templates.empty())
    {
        return;
    }

    image::OverlayCanvas &rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &imageLayer(rCanvas.getLayerImage());
    const interface::Trafo &trafo (*m_trafo);
    const auto origin = trafo(geo2d::Point(0, 0));
    const auto& candidate = m_matchResult;
    const int step = candidate.angle;
    const auto angle = (m_angleStart + step * m_angleStep) / 180 * M_PI;
    const geo2d::Point overlayPosition
    (
        origin.x + candidate.x - m_templates[step].width() / 2,
        origin.y + candidate.y - m_templates[step].height() / 2

    );

    if (m_oVerbosity == eMax)
    {
        auto title = image::OverlayText("Angle Start", image::Font(),
            geo2d::Rect(0, 0, m_templates.front().width(), m_templates.front().height()), image::Color::Green());
        imageLayer.add<image::OverlayImage>(geo2d::Point(0, 0), m_templates.front(), title);
        auto title2 = image::OverlayText("Angle End", image::Font(),
            geo2d::Rect(m_templates.front().width(), 0, m_templates.back().width() * 2, m_templates.back().height()), image::Color::Green());
        imageLayer.add<image::OverlayImage>(geo2d::Point(m_templates.front().width(), 0), m_templates.back(), title2);
    }

    if (m_oVerbosity >= eMedium)
    {
        imageLayer.add<image::OverlayImage>(overlayPosition, m_templates[step], image::OverlayText());
        imageLayer.add<image::OverlayRectangle>(overlayPosition.x, overlayPosition.y,  m_templates[step].width() + 1, m_templates[step].height() + 1, image::Color::Yellow());
        const auto yShift = -24;
        std::ostringstream scoreOutput;
        scoreOutput << "score: " << std::fixed << std::setprecision(2) << candidate.score;
        imageLayer.add<image::OverlayText>(scoreOutput.str(), image::Font(16),
            geo2d::Point(overlayPosition.x, overlayPosition.y + yShift), image::Color::Cyan());

        std::ostringstream angleOutput;
        angleOutput << "angle: " << std::fixed << std::setprecision(2) << m_angleStart + step * m_angleStep;
        imageLayer.add<image::OverlayText>(angleOutput.str(), image::Font(16),
            geo2d::Point(overlayPosition.x, overlayPosition.y + yShift * 2), image::Color::Green());
    }

    const double axisLength = 15;
    const Point position(candidate.x + origin.x, candidate.y + origin.y);
    const Point xAxis(axisLength * cos(angle), axisLength * sin(angle));
    const Point yAxis(axisLength * -sin(angle), axisLength * cos(angle));
    imageLayer.add<image::OverlayLine>(position.x, position.y,
        position.x + xAxis.x, position.y + xAxis.y, image::Color::Blue());
    imageLayer.add<image::OverlayLine>(position.x, position.y,
        position.x + yAxis.x, position.y + yAxis.y, image::Color::Red());
}

bool TemplateMatching::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_pipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    if (pipe.tag() == "VariableAngleStart")
    {
        m_pipeAngleStart = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void TemplateMatching::proceed(const void* sender, fliplib::PipeEventArgs& event)
{
    //backward compatibility for older graphs that still calls proceed
    //To be removed in the future
    fliplib::PipeGroupEventArgs dummyEvent(0, nullptr, 0);
    proceedGroup(sender, dummyEvent);
}

void TemplateMatching::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    //handle optional angleStart pipe if it is connected
    if (m_pipeAngleStart != nullptr)
    {
        const auto &geoAngleStart = m_pipeAngleStart->read(m_oCounter);
        const auto &angleStart = geoAngleStart.ref().getData().front();

        //update templates if angleStart changed
        if (m_angleStart != angleStart)
        {
            m_angleStart = angleStart;
            generateTemplate();
        }
    }

    const interface::ImageFrame &frame = m_pipeInImageFrame->read(m_oCounter);
    m_trafo = frame.context().trafo();

    unsigned int pyramidLevels = m_pyramidLevels;
    const image::TLineImage<byte> &imageIn = frame.data();

    // make sure template is not larger than the image, otherwise opencv will throw an exception.
    // Since all rotated templates have the same size, only the first template is checked.
    if (m_templates.at(0).width() > imageIn.width() || m_templates.at(0).height() > imageIn.height())
    {
        wmLog(eError, "[TemplateMatching] The size of template cannot be larger than the size of the image!\n");

        const auto geoRank = interface::Minimum;
        const interface::GeoDoublearray geoOutX(frame.context(), geo2d::TArray<double>(1, 0, eRankMin), interface::ResultType::AnalysisOK, geoRank);
        const interface::GeoDoublearray geoOutY(frame.context(), geo2d::TArray<double>(1, 0, eRankMin), interface::ResultType::AnalysisOK, geoRank);
        const interface::GeoDoublearray geoOutAngle(frame.context(), geo2d::TArray<double>(1, 0, eRankMin), interface::ResultType::AnalysisOK, geoRank);
        const interface::GeoDoublearray geoOutScore(frame.context(), geo2d::TArray<double>(1, 0, eRankMin), interface::ResultType::AnalysisOK, geoRank);

        preSignalAction();
        m_pipePosX.signal(geoOutX);
        m_PipePosY.signal(geoOutY);
        m_pipeAngle.signal(geoOutAngle);
        m_pipeScore.signal(geoOutScore);

        return;
    }

    cv::Mat image(imageIn.height(), imageIn.width(), CV_8UC1, (void*)(imageIn.begin()), imageIn.stride());
    cv::Mat imageScaled = image.clone();
    for (unsigned int i = 0; i < pyramidLevels; i++)
    {
        cv::pyrDown(imageScaled, imageScaled, cv::Size(imageScaled.cols/2, imageScaled.rows/2));
    }

    if (m_cropMode == CropMode::Keep)
    {
        // set zero to one to avoid zero division by cross correlation, which causes score to be infinity
        setZeroToOne((uint8_t *)imageScaled.data, imageScaled.cols, imageScaled.rows, imageScaled.step);
    }

    cv::Mat templates[m_templates.size()];
    for (size_t i = 0; i < m_templates.size(); i++)
    {
        templates[i] = cv::Mat(m_templates[i].height(), m_templates[i].width(), CV_8UC1, (void*)(m_templates[i].begin()));
    }

    //match with reduced resolution
    double max = 0;
    unsigned int step = 0;
    cv::Point location;
    for (size_t i = 0; i < m_templateScaled.size(); i++)
    {
        cv::Mat imageDst;

        if (m_cropMode == CropMode::Keep)
        {
            cv::matchTemplate(imageScaled, m_templateScaled[i], imageDst, cv::TM_CCORR_NORMED, m_templateScaled[i] != 0);
        }
        else
        {
            cv::matchTemplate(imageScaled, m_templateScaled[i], imageDst, cv::TM_CCORR_NORMED);
        }

        double minScore, maxScore;
        cv::Point minLocation, maxLocation;
        cv::minMaxLoc(imageDst, &minScore, &maxScore, &minLocation, &maxLocation);
        if (maxScore > max)
        {
            location = cv::Point(
                (maxLocation.x) * pow(2, pyramidLevels),
                (maxLocation.y) * pow(2, pyramidLevels)
            );
            max = maxScore;
            step = i;
        }
    }

    //match with full resolution and a smaller ROI around the first match location
    if (pyramidLevels > 0)
    {
        cv::Rect reducedROI = cv::Rect(
            cv::Point(
                (location.x) - pyramidLevels * 2,
                (location.y) - pyramidLevels * 2
            ),
            cv::Point(
                (location.x) + templates[0].cols + pyramidLevels * 2,
                (location.y) + templates[0].rows + pyramidLevels * 2
            )
        );

        //make share the reduced ROI is within the image
        if (reducedROI.x < 0)
        {
            reducedROI.x = 0;
        }
        if (reducedROI.y < 0)
        {
            reducedROI.y = 0;
        }
        if (reducedROI.width > image.cols)
        {
            reducedROI.width = image.cols;
        }
        if (reducedROI.height > image.rows)
        {
            reducedROI.height = image.rows;
        }
        if (reducedROI.x + reducedROI.width > image.cols)
        {
            reducedROI.x = image.cols - reducedROI.width;
        }
        if (reducedROI.y + reducedROI.height > image.rows)
        {
            reducedROI.y = image.rows - reducedROI.height;
        }
        cv::Mat imageReduced = cv::Mat(image, reducedROI);
        cv::Mat imageDst;

        //match around the optimal angle step determined in the first match
        max = 0;
        int bestStep = step;
        for (int i = bestStep - 1; i <= bestStep + 1; i++)
        {
            if (i < 0 || i >= (int)m_templates.size())
            {
                continue;
            }

            if (m_cropMode == CropMode::Keep)
            {
                cv::matchTemplate(imageReduced, templates[i], imageDst, cv::TM_CCORR_NORMED, templates[i] != 0);
            }
            else
            {
                cv::matchTemplate(imageReduced, templates[i], imageDst, cv::TM_CCORR_NORMED);
            }

            double minScore, maxScore;
            cv::Point minLocation, maxLocation;
            cv::minMaxLoc(imageDst, &minScore, &maxScore, &minLocation, &maxLocation);
            if (max < maxScore)
            {
                location = maxLocation + reducedROI.tl();
                max = maxScore;
                step = i;
            }
        }
    }

    double cmax, rmax;
    cmax = location.x + m_templates.at(0).width()/2;
    rmax = location.y + m_templates.at(0).height()/2;
    const auto angle = m_angleStart + step * m_angleStep;
    m_matchResult = MatchCandidate
        (
            cmax,
            rmax,
            max,
            step //use angle in step instead of angle in degree
        );

    const auto geoRank = interface::Limit; //1.0
    const auto arrayRank = 255;
    const interface::GeoDoublearray geoOutX(frame.context(), geo2d::TArray<double>(1, cmax, arrayRank), interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoOutY(frame.context(), geo2d::TArray<double>(1, rmax, arrayRank), interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoOutAngle(frame.context(), geo2d::TArray<double>(1, angle, arrayRank), interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoOutScore(frame.context(), geo2d::TArray<double>(1, max, arrayRank), interface::ResultType::AnalysisOK, geoRank);

    preSignalAction();
    m_pipePosX.signal(geoOutX);
    m_PipePosY.signal(geoOutY);
    m_pipeAngle.signal(geoOutAngle);
    m_pipeScore.signal(geoOutScore);
}

void TemplateMatching::generateTemplate()
{
    m_templates.clear();
    m_templateScaled.clear();

    std::string filePath;
    filePath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "") +
        "/" + m_templateFileName;
    if (access(filePath.c_str(), F_OK) == -1)
    {
        wmLog(eError, "TemplateMatching: cannot access template file: " + filePath + "\n");
        return;
    }
    fileio::Bitmap bmp(filePath);
    image::BImage image(geo2d::Size(bmp.width(), bmp.height()));
    if (!bmp.load(image.begin()))
    {
        std::ostringstream oss;
        oss << "TemplateMatching: error in loading bitmap file " << filePath << std::endl;
        oss << "Bitmap header: " << bmp << std::endl;
        wmLog(eError, oss.str());
        return;
    }
    wmLog(eInfo, "TemplateMatching: read template from "+ filePath + " successful!\n");

    m_templates.resize(m_countStep);
    m_templateScaled.resize(m_countStep);

    if (m_cropMode == CropMode::Crop)
    {
        //calculate template cropped size
        int newWidth = image.width();
        int newHeight = image.height();
        for (unsigned int i = 0; i < m_countStep; i++)
        {
            double cropWidth;
            double cropHeight;
            rotatedRectWithMaxArea(image.width(), image.height(),
                m_angleStart + i * m_angleStep, cropWidth, cropHeight);
            if (cropWidth < newWidth)
            {
                newWidth = cropWidth;
            }
            if (cropHeight < newHeight)
            {
                newHeight = cropHeight;
            }
        }

        for (unsigned int i = 0; i < m_countStep; i++)
        {
            image::BImage imageRotated(image.size());
            rotateImage(image.begin(), imageRotated.begin(), image.height(),
                image.width(), image.stride(), imageRotated.stride(),
                image.width() / 2.0, image.height() / 2.0, m_angleStart + i * m_angleStep);

            // crop off template black region caused by rotation
            int templateCenterRow = imageRotated.height() / 2;
            int templateCenterCol = imageRotated.width() / 2;
            int templateNewWidth = newWidth;
            int templateNewHeight = newHeight;

            imageRotated = image::BImage(imageRotated, geo2d::Rect(templateCenterCol - templateNewWidth / 2,
                templateCenterRow - templateNewHeight / 2, templateNewWidth, templateNewHeight), true);
            m_templates[i].resize(geo2d::Size(templateNewWidth, templateNewHeight));
            imageRotated.copyPixelsTo(m_templates[i]);

            m_templateScaled[i] = cv::Mat(m_templates[i].height(), m_templates[i].width(), CV_8UC1, (void*)(m_templates[i].begin())).clone();
            for(unsigned int j = 0; j < m_pyramidLevels; j++)
            {
                cv::pyrDown(m_templateScaled[i], m_templateScaled[i], cv::Size(m_templateScaled[i].cols / 2, m_templateScaled[i].rows / 2));
            }
        }
    }
    else
    {
        int verticalPadding = 0;
        int horizontalPadding = 0;

        int newWidth = 0;
        int newHeight = 0;

        for (unsigned int i = 0; i < m_countStep; i++)
        {
            const auto boxSize = rotatedRectBoundingBoxSize(image.height(), image.width(), m_angleStart + i * m_angleStep);
            if (boxSize.first > newHeight)
            {
                newHeight = boxSize.first;
            }
            if (boxSize.second > newWidth)
            {
                newWidth = boxSize.second;
            }
        }

        double d = std::sqrt(image.height() * image.height() + image.width() * image.width());
        verticalPadding = std::ceil((d - image.height()) * 0.5) + 1;
        horizontalPadding = std::ceil((d - image.width()) * 0.5) + 1;

        int paddedHeight = 2 * verticalPadding + image.height();
        int paddedWidth = 2 * horizontalPadding + image.width();

        image::BImage imagePadded(geo2d::Size(paddedWidth, paddedHeight));

        setZeroToOne(image.begin(), image.width(), image.height(), image.stride()); // if the original image contains zero values, set them to one so that the mask for cross correlation doesn't get affected by these pixels
        copyMakeConstBorder8u(image.begin(), image.width(), image.height(), image.stride(), imagePadded.begin(), imagePadded.stride(), verticalPadding, verticalPadding, horizontalPadding, horizontalPadding, 0);

        for (unsigned int i = 0; i < m_countStep; i++)
        {
            image::BImage imageRotated(imagePadded.size());
            rotateImage(imagePadded.begin(), imageRotated.begin(), imagePadded.height(),
                imagePadded.width(), imagePadded.stride(), imageRotated.stride(),
                imagePadded.width() / 2.0, imagePadded.height() / 2.0, m_angleStart + i * m_angleStep);

            int templateCenterRow = imageRotated.height() / 2;
            int templateCenterCol = imageRotated.width() / 2;
            int templateNewWidth = newWidth + 2;
            int templateNewHeight = newHeight + 2;

            imageRotated = image::BImage(imageRotated, geo2d::Rect(templateCenterCol - templateNewWidth / 2,
                templateCenterRow - templateNewHeight / 2, templateNewWidth, templateNewHeight), true);

            m_templates[i].resize(geo2d::Size(templateNewWidth, templateNewHeight));
            imageRotated.copyPixelsTo(m_templates[i]);
        }

        cv::Mat imageCv(image.height(), image.width(), CV_8UC1, image.begin());
        cv::Mat imageScaledCv = imageCv.clone();

        for(unsigned int j = 0; j < m_pyramidLevels; j++)
        {
            cv::pyrDown(imageScaledCv, imageScaledCv, cv::Size(imageScaledCv.cols / 2, imageScaledCv.rows / 2));
        }

        cv::Mat imageScaledPaddedCv;

        d = std::sqrt(imageScaledCv.rows * imageScaledCv.rows + imageScaledCv.cols * imageScaledCv.cols);
        verticalPadding = std::ceil((d - imageScaledCv.rows) * 0.5) + 1;
        horizontalPadding = std::ceil((d - imageScaledCv.cols) * 0.5) + 1;

        cv::copyMakeBorder(imageScaledCv, imageScaledPaddedCv, verticalPadding, verticalPadding, horizontalPadding, horizontalPadding, cv::BORDER_CONSTANT, 0);

        newWidth = 0;
        newHeight = 0;

        for (unsigned int i = 0; i < m_countStep; i++)
        {
            const auto boxSize = rotatedRectBoundingBoxSize(imageScaledCv.rows, imageScaledCv.cols, m_angleStart + i * m_angleStep);
            if (boxSize.first > newHeight)
            {
                newHeight = boxSize.first;
            }
            if (boxSize.second > newWidth)
            {
                newWidth = boxSize.second;
            }
        }

        for (unsigned int i = 0; i < m_countStep; i++)
        {
            cv::Mat imageRotatedCv(imageScaledPaddedCv.size(), CV_8UC1);
            rotateImage((uint8_t *)imageScaledPaddedCv.data, (uint8_t *)imageRotatedCv.data, imageScaledPaddedCv.rows,
                imageScaledPaddedCv.cols, imageScaledPaddedCv.step, imageRotatedCv.step,
                (imageScaledPaddedCv.cols - 1) / 2.0, (imageScaledPaddedCv.rows - 1) / 2.0, m_angleStart + i * m_angleStep);

            int templateCenterRow = imageRotatedCv.rows / 2;
            int templateCenterCol = imageRotatedCv.cols / 2;
            int templateNewWidth = newWidth + 2;
            int templateNewHeight = newHeight + 2;

            m_templateScaled[i] = cv::Mat(imageRotatedCv, cv::Rect(templateCenterCol - (templateNewWidth) / 2,
                templateCenterRow - (templateNewHeight) / 2, templateNewWidth, templateNewHeight)).clone();
        }
    }
}

void rotateImage(uint8_t *src, uint8_t *dst, size_t height,
  size_t width, size_t srcStride, size_t dstStride, double centerCol,
  double centerRow, double angle)
{
    const double DEG2RAD = 3.14159265358979323846 / 180;
    double cos_ = cos(angle * DEG2RAD);
    double sin_ = sin(angle * DEG2RAD);
    double Affine[2][3];
    Affine[0][0] = cos_;
    Affine[0][1] = sin_;
    Affine[1][0] = -sin_;
    Affine[1][1] = cos_;
    Affine[0][2] = (1 - cos_) * centerCol - sin_ * centerRow;
    Affine[1][2] = sin_ * centerCol + (1 - cos_) * centerRow;

    //col' = cos_*col + sin_*row + (1-cos_)*centerCol - sin_*centerRow
    //row' = -sin_*col + cos_*row + sin_*centerCol + (1-cos_)*centerRow;
    double a00c[width];
    double a10c[width];
    for (size_t c = 0; c < width; ++c)
    {
        a00c[c] = Affine[0][0] * c;
        a10c[c] = Affine[1][0] * c;
    }

    for (size_t r = 0; r < height; ++r, dst += dstStride)
    {
        double a01r_a02 = Affine[0][1] * r + Affine[0][2];
        double a11r_a12 = Affine[1][1] * r + Affine[1][2];
        for (size_t c = 0; c < width; ++c)
        {
            double c_ = std::round(a00c[c] + a01r_a02);
            double r_ = std::round(a10c[c] + a11r_a12);

            if (c_ < 0 || c_ >= width || r_ < 0 || r_ >= height)
            {
                dst[c] = 0;
            }
            else
            {
                size_t c_int = c_;
                size_t r_int = r_;

                dst[c] = src[c_int + srcStride * r_int];
            }
        }
    }
}

uint64_t normL2Squared(uint8_t *src, size_t height, size_t width,
    ptrdiff_t srcStride)
{
    uint64_t result = 0;
    for (size_t r = 0; r < height; ++r, src += srcStride)
    {
        for (size_t c = 0; c < width; ++c)
        {
            result += src[c] * src[c];
        }
    }

    return result;
}

void crossCorrelation(const uint8_t *src, size_t srcHeight, size_t srcWidth,
    ptrdiff_t srcStride, const uint8_t *tmpl, size_t tmplHeight,
    size_t tmplWidth, ptrdiff_t tmplStride, uint64_t *dst, ptrdiff_t dstStride)
{
    assert(srcHeight > tmplHeight);
    assert(srcWidth > tmplWidth);
    size_t dstHeight = srcHeight - tmplHeight + 1;
    size_t dstWidth = srcWidth - tmplWidth + 1;

    for (size_t r = 0; r < dstHeight; ++r, src += srcStride, dst += dstStride)
    {
        for (size_t c = 0; c < dstWidth; ++c)
        {
            uint64_t corr = 0;
            const uint8_t *p_tmpl = tmpl;
            const uint8_t *p_src2 = src + c;
            for (size_t u = 0; u < tmplHeight; ++u, p_src2 += srcStride, p_tmpl += tmplStride)
            {
                for (size_t v = 0; v < tmplWidth; ++v)
                {
                    corr += p_tmpl[v] * p_src2[v];
                }
            }
            dst[c] = corr;
        }
    }
}

void rotatedRectWithMaxArea(double width, double height, double angle,
    double& cropWidth, double& cropHeight)
{
    if (width <= 0 || height <= 0)
    {
        cropWidth = 0;
        cropHeight = 0;
        return;
    }

    bool widthIsLonger = width >= height;
    double sideLong = widthIsLonger ? width : height;
    double sideShort = widthIsLonger ? height : width;

    const double DEG2RAD = 3.14159265358979323846 / 180;
    double sin_a = std::abs(sin(angle * DEG2RAD));
    double cos_a = std::abs(cos(angle * DEG2RAD));

    if (sideShort <= 2 * sin_a * cos_a * sideLong || std::abs(sin_a - cos_a) < 1e-10)
    {
        if (widthIsLonger)
        {
            cropWidth = sideShort / sin_a / 2;
            cropHeight = sideShort /cos_a / 2;
        }
        else
        {
            cropWidth = sideShort / cos_a / 2;
            cropHeight = sideShort / sin_a / 2;
        }
    }
    else
    {
        double cos_2a = cos_a * cos_a - sin_a * sin_a;
        cropWidth = (width * cos_a - height * sin_a) / cos_2a;
        cropHeight = (height * cos_a - width * sin_a) / cos_2a;
    }
}


} //namespace filter
} //namespace precitec
