#include "shapeMatching.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <unistd.h>

#define FILTER_ID            "fbffebf9-a495-4623-ad4d-459158779d39"
#define VARIANT_ID           "a1f93d33-3d3f-4479-940c-7cbdafe92a3f"

#define PIPE_ID_IMAGEIN      "fbe8df8f-12fd-4cf4-8c7f-b7b6c90b389c"
#define PIPE_ID_POSITIONX    "b630f69c-6fcf-42e4-931b-46074084119d"
#define PIPE_ID_POSITIONY    "bd146949-5664-409d-8c74-c864e6954ea6"
#define PIPE_ID_ANGLE        "8b04f761-3f02-41b8-926a-7ffbe4300697"
#define PIPE_ID_SCORE        "2de91cd8-119e-11ed-861d-0242ac120002"

namespace precitec
{
namespace filter
{
ShapeMatching::ShapeMatching()
    : TransformFilter("ShapeMatching", Poco::UUID(FILTER_ID))
    , m_pipeInImageFrame(nullptr)
    , m_pipePosX(this, "PositionX")
    , m_PipePosY(this, "PositionY")
    , m_pipeAngle(this, "Angle")
    , m_pipeScore(this, "Score")
    , m_templateFileName("")
    , m_blur(3)
    , m_contrast(60)
    , m_pyramidLevels(3)
    , m_angleStart(0.0)
    , m_angleExtent(0.0)
    , m_minScore(0.7)
    , m_greediness(0.9)
    , m_maxOverlap(0.5)
    , m_maxMatches(0)
    , m_sortMethod(CandidateSortMethod::ScoreHighestToLowest)
{
    parameters_.add("TemplateFileName", fliplib::Parameter::TYPE_string, m_templateFileName);
    parameters_.add("Blur", fliplib::Parameter::TYPE_double, m_blur);
    parameters_.add("Contrast", fliplib::Parameter::TYPE_UInt32, m_contrast);
    parameters_.add("PyramidLevels", fliplib::Parameter::TYPE_UInt32, m_pyramidLevels);
    parameters_.add("AngleStart", fliplib::Parameter::TYPE_double, m_angleStart);
    parameters_.add("AngleExtent", fliplib::Parameter::TYPE_double, m_angleExtent);
    parameters_.add("MinScore", fliplib::Parameter::TYPE_double, m_minScore);
    parameters_.add("Greediness", fliplib::Parameter::TYPE_double, m_greediness);
    parameters_.add("MaxOverlap", fliplib::Parameter::TYPE_double, m_maxOverlap);
    parameters_.add("MaxMatches", fliplib::Parameter::TYPE_UInt32, m_maxMatches);
    parameters_.add("SortMethod", fliplib::Parameter::TYPE_int, static_cast<int>(m_sortMethod));

    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_IMAGEIN), m_pipeInImageFrame, "ImageIn", 1, "ImageIn"},
    });
    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_POSITIONX), &m_pipePosX, "PositionX", 1, "PositionX"},
        {Poco::UUID(PIPE_ID_POSITIONY), &m_PipePosY, "PositionY", 1, "PositionY"},
        {Poco::UUID(PIPE_ID_ANGLE), &m_pipeAngle, "Angle", 1, "Angle"},
        {Poco::UUID(PIPE_ID_SCORE), &m_pipeScore, "Score", 1, "Score"},
    });
    setVariantID(Poco::UUID(VARIANT_ID));
}

void ShapeMatching::setParameter()
{
    TransformFilter::setParameter();

    auto updateShapeModel = false;

    if (m_templateFileName != parameters_.getParameter("TemplateFileName").convert<std::string>())
    {
        m_templateFileName = parameters_.getParameter("TemplateFileName").convert<std::string>();
        updateShapeModel = true;
    }

    if (m_blur != parameters_.getParameter("Blur").convert<double>())
    {
        m_blur = parameters_.getParameter("Blur").convert<double>();
        updateShapeModel = true;
    }

    if (m_contrast != parameters_.getParameter("Contrast").convert<unsigned int>())
    {
        m_contrast = parameters_.getParameter("Contrast").convert<unsigned int>();
        updateShapeModel = true;
    }

    if (m_pyramidLevels != parameters_.getParameter("PyramidLevels").convert<unsigned int>())
    {
        m_pyramidLevels = parameters_.getParameter("PyramidLevels").convert<unsigned int>();
        updateShapeModel = true;
    }

    m_angleStart = parameters_.getParameter("AngleStart").convert<double>();
    m_angleExtent = parameters_.getParameter("AngleExtent").convert<double>();
    m_minScore = parameters_.getParameter("MinScore").convert<double>();
    m_greediness = parameters_.getParameter("Greediness").convert<double>();
    m_maxOverlap = parameters_.getParameter("MaxOverlap").convert<double>();
    m_maxMatches = parameters_.getParameter("MaxMatches").convert<unsigned int>();
    m_sortMethod = static_cast<CandidateSortMethod>(parameters_.getParameter("SortMethod").convert<int>());

    if (updateShapeModel == true)
    {
        generateShapeModel();
    }
}

void ShapeMatching::paint()
{
    if (m_oVerbosity < eMedium)
    {
        return;
    }

    image::OverlayCanvas &rCanvas (canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer &shapeModelLayer(rCanvas.getLayerContour());
    const interface::Trafo &trafo (*m_trafo);
    const auto origin = trafo(geo2d::Point(0, 0));

    for (std::size_t i = 0; i < m_matchCandidate.size(); ++i)
    {
        const ShapeModel rotatedModel(m_shapeModelScaled[0], m_matchCandidate[i].angle);
        const auto& shape = rotatedModel.position();

        for (const auto& p : shape)
        {
            shapeModelLayer.add<image::OverlayPoint>(image::OverlayPoint(
                p.x + m_matchCandidate[i].x + origin.x,
                p.y + m_matchCandidate[i].y + origin.y,
                image::Color::Yellow()));
        }

        // draw axis
        const double axisLength = 15;
        const auto angle = m_matchCandidate[i].angle / 180 * M_PI;
        const Point position(m_matchCandidate[i].x + origin.x, m_matchCandidate[i].y + origin.y);
        const Point xAxis(axisLength * cos(angle), axisLength * sin(angle));
        const Point yAxis(axisLength * -sin(angle), axisLength * cos(angle));
        shapeModelLayer.add<image::OverlayLine>(position.x, position.y,
            position.x + xAxis.x, position.y + xAxis.y, image::Color::Blue());
        shapeModelLayer.add<image::OverlayLine>(position.x, position.y,
            position.x + yAxis.x, position.y + yAxis.y, image::Color::Red());

        // display score
        const auto tl = rotatedModel.tl();
        const auto yShift = -24;
        std::ostringstream scoreOutput;
        scoreOutput << i << ", score: " << std::fixed << std::setprecision(2) << m_matchCandidate[i].score;
        shapeModelLayer.add<image::OverlayText>(scoreOutput.str(), image::Font(16),
            geo2d::Point(tl.x + position.x, tl.y  + position.y + yShift), image::Color::Cyan());
    }
}

bool ShapeMatching::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ImageIn")
    {
        m_pipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<interface::ImageFrame>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void ShapeMatching::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto contrast = m_contrast;
    const auto pyramidLevels = m_pyramidLevels;
    const auto angleStart = m_angleStart;
    const auto angleExtent = m_angleExtent;
    const auto minScore = m_minScore;
    const auto greediness = m_greediness;
    const auto maxOverlap = m_maxOverlap;
    const auto maxMatches = m_maxMatches;
    const auto sortMethod = m_sortMethod;
    auto &candidates = m_matchCandidate;

    const auto &frame = m_pipeInImageFrame->read(m_oCounter);
    const auto &imageIn = frame.data();
    m_trafo = frame.context().trafo();

    const cv::Mat image(imageIn.height(), imageIn.width(), CV_8UC1,
                        (void*)(imageIn.begin()), imageIn.stride());

    //TODO: reduce openCV dependency regarding pyramid and gradient calculation
    std::vector<cv::Mat> imagePyramid;
    imagePyramid.push_back(image.clone());
    for (auto i = 0u; i < pyramidLevels; ++i)
    {
        imagePyramid.push_back(cv::Mat());
        cv::Size size(imagePyramid[i].cols / 2, imagePyramid[i].rows / 2);
        cv::pyrDown(imagePyramid[i], imagePyramid[i + 1], size);
    }

    candidates.clear();
    auto previousAngleStep = 360.0;

    for (int i = pyramidLevels; i >= 0; --i)
    {
        cv::Mat gradX;
        cv::Mat gradY;
        cv::spatialGradient(imagePyramid.at(i), gradX, gradY);

        const auto height = gradX.rows;
        const auto width = gradX.cols;
        const auto stride = gradX.step / sizeof(int16_t); // 16-bit

        cv::Mat gx(height, width, CV_32FC1);
        cv::Mat gy(height, width, CV_32FC1);
        cv::Mat g(height, width, CV_16SC1);
        cv::Mat gMask(height, width, CV_8UC1);

        normalizeGradient((int16_t*)gradX.data, (int16_t*)gradY.data,
(float*)gx.data, (float*)gy.data, (int16_t*)g.data, (uint8_t*)gMask.data,
contrast / 2, height, width, stride, gMask.step);

        const auto angleStep = m_shapeModelScaled.at(i).optimalAngleStep();

        if (candidates.empty())
        {
            const auto relaxScore = greediness * minScore;

            candidates = matchShapeAll(m_shapeModelScaled.at(i), (float*)gx.data,
(float*)gy.data, (uint8_t*)gMask.data, height, width, width, relaxScore,
angleStart, angleStep, angleExtent);

            filterCandidate(candidates, m_shapeModelScaled.at(i), maxOverlap);
        }
        else
        {
            for (auto c = candidates.begin(); c != candidates.end(); ++c)
            {
                c->x *= 2;
                c->y *= 2;

                auto c_angleStart = c->angle - previousAngleStep;
                if (c_angleStart < angleStart)
                {
                    c_angleStart = angleStart;
                }

                auto c_angleStop = c->angle + previousAngleStep;
                if (c_angleStop > angleStart + angleExtent)
                {
                    c_angleStop = angleStart + angleExtent;
                }

                const auto c_engleExtent = c_angleStop - c_angleStart;

                const auto candidateOut = matchShapeOne(m_shapeModelScaled.at(i),
*c, (float*)gx.data, (float*)gy.data, (uint8_t*)gMask.data, height, width,
width, c_angleStart, angleStep, c_engleExtent);
               *c = candidateOut;
            }
        }

        previousAngleStep = angleStep;
    }

    auto it = std::remove_if(candidates.begin(), candidates.end(),
        [minScore](auto candidate)->bool
        {
            return candidate.score < minScore;
        }
    );
    candidates.erase(it, candidates.end());

    filterCandidate(candidates, m_shapeModelScaled.at(0), maxOverlap, maxMatches);

    // sort candidate
    switch(sortMethod)
    {
    case CandidateSortMethod::ScoreHighestToLowest:
    {
        std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs)
              {
                  return lhs.score > rhs.score;
              });
        break;
    }
    case CandidateSortMethod::ClosestToCenter:
    {
        std::sort(candidates.begin(), candidates.end(),
              [&](const auto& lhs, const auto& rhs)
              {
                  const double centerX = imageIn.width() / 2;
                  const double centerY = imageIn.height() / 2;
                  const double lx = lhs.x - centerX;
                  const double ly = lhs.y - centerY;
                  const double rx = rhs.x - centerX;
                  const double ry = rhs.y - centerY;
                  return lx * lx + ly * ly < rx * rx + ry * ry;
              });
        break;
    }
    case CandidateSortMethod::LeftToRight:
    {
        std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs)
              {
                  return lhs.x < rhs.x;
              });
        break;
    }
    case CandidateSortMethod::TopToBottom:
    {
        std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs)
              {
                  return lhs.y < rhs.y;
              });
        break;
    }
    default:
        break;
    }

    const auto geoRank = interface::Limit; //1.0
    const auto arrayRank = eRankMax;

    geo2d::TArray<double> cmax(candidates.size(), 0.0, arrayRank);
    geo2d::TArray<double> rmax(candidates.size(), 0.0, arrayRank);
    geo2d::TArray<double> angle(candidates.size(), 0.0, arrayRank);
    geo2d::TArray<double> score(candidates.size(), 0.0, arrayRank);

    for (std::size_t i = 0; i < candidates.size(); ++i)
    {
        cmax.getData()[i] = candidates[i].x;
        rmax.getData()[i] = candidates[i].y;
        angle.getData()[i] = candidates[i].angle;
        score.getData()[i] = candidates[i].score;
    }

    if (candidates.empty())
    {
        cmax.assign(1, 0, eRankMin);
        rmax.assign(1, 0, eRankMin);
        angle.assign(1, 0, eRankMin);
        score.assign(1, 0, eRankMin);
    }

    // debug
    if (m_oVerbosity > eLow)
    {
        for (std::size_t i = 0; i < candidates.size(); ++i)
        {
            wmLog(eInfo, "[ShapeMatching] row: %f, col: %f, angle: %f, score: %f", candidates[i].y, candidates[i].x, candidates[i].angle, candidates[i].score);
        }
    }

    const interface::GeoDoublearray geoOutX(frame.context(),
        cmax,
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoOutY(frame.context(),
        rmax,
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoOutAngle(frame.context(),
        angle,
        interface::ResultType::AnalysisOK, geoRank);
    const interface::GeoDoublearray geoOutScore(frame.context(),
        score,
        interface::ResultType::AnalysisOK, geoRank);

    preSignalAction();
    m_pipePosX.signal(geoOutX);
    m_PipePosY.signal(geoOutY);
    m_pipeAngle.signal(geoOutAngle);
    m_pipeScore.signal(geoOutScore);
}

void ShapeMatching::generateShapeModel()
{
    m_shapeModelScaled.clear();
    std::string filePath;
    filePath = (getenv("WM_BASE_DIR") ? std::string(getenv("WM_BASE_DIR")) : "")
               + "/" + m_templateFileName;

    if (access(filePath.c_str(), F_OK) == -1)
    {
        wmLog(eError, "ShapeMatching: cannot access template file: " + filePath + "\n");
        return;
    }

    fileio::Bitmap bmp(filePath);
    image::BImage image(geo2d::Size(bmp.width(), bmp.height()));
    if (!bmp.load(image.begin()))
    {
        std::ostringstream oss;
        oss << "ShapeMatching: error in loading bitmap file "
            << filePath << std::endl;
        oss << "Bitmap header: " << bmp << std::endl;
        wmLog(eError, oss.str());
        return;
    }

    wmLog(eInfo, "ShapeMatching: read template from " + filePath + " successful!\n");

    const cv::Mat imageCv(image.height(), image.width(), CV_8UC1, (void*)(image.begin()));
    cv::Mat tmpl = imageCv.clone(); // assure memory integrity
    const auto kernelSize = 9;
    cv::GaussianBlur(tmpl, tmpl, cv::Size(kernelSize, kernelSize), m_blur);
    m_shapeModelScaled.push_back(ShapeModel(tmpl, m_contrast));
    for (auto i = 0u; i < m_pyramidLevels; ++i)
    {
        // increasing contrast for upper pyramid levels, more analysis needed on
        // this to determine the optimal factor
        auto const contrastScaled = m_contrast * sqrt((i + 1) * 3);
        cv::pyrDown(tmpl, tmpl, cv::Size(tmpl.cols/2, tmpl.rows/2));
        m_shapeModelScaled.push_back(ShapeModel(tmpl, contrastScaled));
    }
}

} //namespace filter
} //namespace precitec
