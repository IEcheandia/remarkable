#include "shapeMatchingImpl.h"

namespace precitec
{
namespace filter
{
/******************************************************************************/
Point::Point()
{
}
Point::Point(float x, float y)
    : x(x)
    , y(y)
{
}

std::ostream& operator<<(std::ostream& os, const Point& point)
{
    os << point.x << ", " << point.y;
    return os;
}

std::ostream& operator<<(std::ostream& os, const std::vector<Point>& points)
{
    for (const auto& point : points)
    {
        os << point << std::endl;
    }
    return os;
}
/******************************************************************************/
MatchCandidate::MatchCandidate()
{
}

MatchCandidate::MatchCandidate(float x, float y, float score, float angle)
    : x(x)
    , y(y)
    , score(score)
    , angle(angle)
{
}
/******************************************************************************/
ShapeModel::ShapeModel(const cv::Mat& tmpl, double contrast)
{
    const auto height = tmpl.rows;
    const auto width = tmpl.cols;

    cv::Mat imageCanny;
    cv::Canny(tmpl, imageCanny, contrast, contrast * 1.5, 3);

    // calculate positions relative to first pixel (0,0)
    m_position = findNonZero(imageCanny.data, imageCanny.rows,
imageCanny.cols, imageCanny.step);

    // remove boundary positions since they have invalid gradient
    for (auto i = m_position.begin(); i != m_position.end();)
    {
        if (i->x == 0 || i->x == width - 1 || i->y == 0 || i->y == height - 1)
        {
            m_position.erase(i);
        }
        else
        {
            ++i;
        }
    }

    cv::Mat gradX;
    cv::Mat gradY;
    // calculate image x-y-gradient with int16_t resolution
    cv::spatialGradient(tmpl, gradX, gradY);

    cv::Mat dstGx(height, width, CV_32FC1);
    cv::Mat dstGy(height, width, CV_32FC1);
    cv::Mat dstG(height, width, CV_16SC1);
    cv::Mat dstGMask(height, width, CV_8UC1);

    normalizeGradient((int16_t*)gradX.data, (int16_t*)gradY.data,
(float*)dstGx.data, (float*)dstGy.data, (int16_t*)dstG.data,
(uint8_t*)dstGMask.data, 0, height, width, width, width);

    const std::size_t size = m_position.size();
    m_direction.resize(size);

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto row = static_cast<int>(m_position[i].y);
        const auto col = static_cast<int>(m_position[i].x);
        const auto gx = dstGx.at<float>(row, col);
        const auto gy = dstGy.at<float>(row, col);
        m_direction[i] = Point(gx, gy);
    }

    // store positions relative to reference point
    const auto reference = Point((width - 1) / 2, (height - 1) / 2);
    for (auto& point : m_position)
    {
        point = Point(point.x - reference.x, point.y - reference.y);
    }

    updateBoundingBox();
}

ShapeModel::ShapeModel(const ShapeModel& model, double angle, double scale)
{
    const auto size = model.m_position.size();
    m_position.resize(size);
    m_direction.resize(size);

    const auto cosine = cos(angle * M_PI / 180.0);
    const auto sine = sin(angle * M_PI / 180.0);

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto x = model.m_position[i].x;
        const auto y = model.m_position[i].y;
        const auto dx = model.m_direction[i].x;
        const auto dy = model.m_direction[i].y;

        m_position[i] =
        Point(
            scale * (cosine * x - sine * y),
            scale * (sine * x + cosine * y)
        );

        m_direction[i] =
        Point(
            cosine * dx - sine * dy,
            sine * dx + cosine * dy
        );
    }

    updateBoundingBox();
}

ShapeModel& ShapeModel::rotate(double angle)
{
    const auto size = m_position.size();
    const auto cosine = cos(angle * M_PI / 180.0);
    const auto sine = sin(angle * M_PI / 180.0);

    for (std::size_t i = 0; i < size; ++i)
    {
        const auto x = m_position[i].x;
        const auto y = m_position[i].y;
        const auto dx = m_direction[i].x;
        const auto dy = m_direction[i].y;

        m_position[i] =
        Point(
            cosine * x - sine * y,
            sine * x + cosine * y
        );

        m_direction[i] =
        Point(
            cosine * dx - sine * dy,
            sine * dx + cosine * dy
        );
    }

    updateBoundingBox();

    return *this;
}

const std::vector<Point>& ShapeModel::position() const
{
    return m_position;
}

const std::vector<Point>& ShapeModel::direction() const
{
    return m_direction;
}

Point ShapeModel::tl() const
{
    return m_topLeft;
}

Point ShapeModel::br() const
{
    return m_bottomRight;
}

double ShapeModel::optimalAngleStep() const
{
    const auto _tl = tl();
    const auto _br = br();
    const auto _max = std::max({_tl.x, _tl.y, _br.x, _br.y});

    // calculate the angle such that _max is shifted one pixel when rotated
    // with that angle. General formula: phi = acos(1 - 0.5 * d^2 / l^2)
    return std::acos(1 - 0.5 / (_max * _max)) * 180 / M_PI;
}

void ShapeModel::updateBoundingBox()
{
    if (m_position.empty())
    {
        m_topLeft = Point(0, 0);
        m_bottomRight = Point(0, 0);
        return;
    }

    auto minX = m_position[0].x;
    auto minY = m_position[0].y;
    auto maxX = m_position[0].x;
    auto maxY = m_position[0].y;

    for (const auto& point : m_position)
    {
        if (point.x < minX)
        {
            minX = point.x;
        }
        if (point.y < minY)
        {
            minY = point.y;
        }
        if (point.x > maxX)
        {
            maxX = point.x;
        }
        if (point.y > maxY)
        {
            maxY = point.y;
        }
    }

    m_topLeft = Point(roundIndex(minX), roundIndex(minY));
    m_bottomRight = Point(roundIndex(maxX), roundIndex(maxY));
}

std::vector<Point> findNonZero(const uint8_t* src, std::size_t height,
std::size_t width, std::ptrdiff_t stride)
{
    std::vector<Point> indices;
    int * const buffer = new int[width];

    for (std::size_t j = 0; j < height; ++j, src += stride)
    {
        int count = 0;
        for (std::size_t i = 0; i < width; ++i)
        {
            if (src[i] != 0)
            {
                buffer[count++] = i;
            }
        }

        if (count > 0)
        {
            const auto size = indices.size();
            indices.resize(size + count);
            for (int i = 0; i < count; ++i)
            {
                indices[size + i] = Point(buffer[i],j);
            }
        }
    }

    delete[] buffer;

    return indices;
}

std::vector<std::ptrdiff_t> positionToIndex(const std::vector<Point>& positions,
std::ptrdiff_t stride)
{
    const auto size = positions.size();
    std::vector<std::ptrdiff_t> indices(size);

    for(std::size_t i = 0; i < size; ++i)
    {
        const auto xi = roundIndex(positions[i].x);
        const auto yi = roundIndex(positions[i].y);
        indices[i] = xi + stride * yi;
    }

    return indices;
}

void normalizeGradient(const int16_t* srcGx, const int16_t* srcGy, float*
dstGx, float* dstGy, int16_t* dstG, uint8_t* dstGMask, int16_t gMaskThreshold,
std::size_t height, std::size_t width, std::ptrdiff_t srcStride,
std::ptrdiff_t dstStride)
{
    for (std::size_t j = 0; j < height; ++j, srcGx += srcStride, srcGy +=
srcStride, dstGx += dstStride, dstGy += dstStride, dstG += dstStride, dstGMask
+= dstStride)
    {
        for (std::size_t i = 0; i < width; ++i)
        {
            //euclidean distance approximation
            dstG[i] = approximateDistance(srcGx[i], srcGy[i]);
            auto g = static_cast<float>(dstG[i]);
            dstGx[i] = srcGx[i] / g;
            dstGy[i] = srcGy[i] / g;
            dstGMask[i] = dstG[i] > gMaskThreshold;
        }
    }
}

int16_t approximateDistance(int16_t dx, int16_t dy)
{
   uint16_t min, max;

   dx = abs(dx);
   dy = abs(dy);

   if (dx < dy)
   {
      min = dx;
      max = dy;
   }
   else
   {
      min = dy;
      max = dx;
   }

   // equivalent to (15 * max + 8 * min) / 16
   return ((max << 4) - max + (min << 3)) >> 4;
}

void matchShape(const ShapeModel& model, const float* gx, const float* gy,
const uint8_t* gMask, std::ptrdiff_t stride, int startOffsetX, int startOffsetY,
float* dst, std::size_t dstHeight, std::size_t dstWidth)
{
    const auto& positions = model.position();
    const auto indices = positionToIndex(positions, stride);
    const auto indicesCount = indices.size();
    const auto& directions = model.direction();

    const auto startIndex = startOffsetX + stride * startOffsetY;
    gx += startIndex;
    gy += startIndex;
    gMask += startIndex;

    const auto padding = stride - dstWidth;

    for (std::size_t j = 0; j < dstHeight;
         ++j, dst += stride, gx += padding, gy += padding, gMask += padding)
    {
        for (std::size_t i = 0; i < dstWidth; ++i, ++gx, ++gy, ++gMask)
        {
            float score = 0;
            for (std::size_t k = 0; k < indicesCount; ++k)
            {
                const auto index = indices[k];
                const auto gxk = gx[index];
                const auto gyk = gy[index];
                const auto gMaskk = gMask[index];
                const auto dx = directions[k].x;
                const auto dy = directions[k].y;
                if (gMaskk)
                {
                    score += (dx * gxk + dy * gyk);
                }
            }
            score /= indicesCount;
            dst[i] = score;
        }
    }
}

std::vector<MatchCandidate> matchShapeAll(const ShapeModel& model, const float*
gx, const float* gy, const uint8_t* gMask, std::size_t height, std::size_t
width, std::ptrdiff_t stride, float minScore, double angleStart, double
angleStep, double angleExtent)
{
    if (angleStep <= 0)
    {
        angleStep = 1;
    }

    float *score = new float[height * stride];

    std::vector<MatchCandidate> candidates;

    for (auto angle = angleStart; angle <= angleStart + angleExtent;
         angle += angleStep)
    {
        const ShapeModel rotatedModel(model, angle);

        const auto topLeft = rotatedModel.tl();
        const auto bottomRight = rotatedModel.br();
        const auto start = Point(-topLeft.x, -topLeft.y);
        const auto stop = Point(width - bottomRight.x - 1,
                                height - bottomRight.y - 1);
        const auto range = Point(stop.x - start.x + 1,
                                 stop.y - start.y + 1);

        if (range.x <= 0 || range.y <= 0)
        {
            //cannot perform match if model is larger than image!
            std::cout << "ERROR: range.x < 0 || range.y < 0" << std::endl;
            continue;
        }

        matchShape(rotatedModel, gx, gy, gMask, stride, start.x, start.y, score,
range.y, range.x);

        std::vector<Point> peakPosition;
        std::vector<float> peakValue;

        findPeak(score, range.y, range.x, stride, peakPosition, peakValue,
minScore);

        for (std::size_t i = 0; i < peakPosition.size(); ++i)
        {
            candidates.push_back
            (
                MatchCandidate
                (
                    peakPosition[i].x + start.x,
                    peakPosition[i].y + start.y,
                    peakValue[i],
                    angle
                )
            );
        }
    }

    delete[] score;

    return candidates;
}

MatchCandidate matchShapeOne(const ShapeModel& model, const MatchCandidate&
candidate, const float* gx, const float* gy, const uint8_t* gMask, std::size_t
height, std::size_t width, std::ptrdiff_t stride, double angleStart, double
angleStep, double angleExtent)
{
    if (angleStep <= 0)
    {
        angleStep = 1;
    }

    auto *score = new float[height * stride];

    auto candidateOut = candidate;
    candidateOut.score = -1.0f;

    const auto angleEnd = angleStart + angleExtent;

    for (auto angle = angleStart; angle <= angleEnd; angle += angleStep)
    {
        const ShapeModel rotatedModel(model, angle);

        const auto topLeft = rotatedModel.tl();
        const auto bottomRight = rotatedModel.br();
        const auto margin = 3;

        Point start(candidate.x - margin, candidate.y - margin);
        if (start.x + topLeft.x < 0)
        {
            start.x = -topLeft.x;
        }
        if (start.y + topLeft.y < 0)
        {
            start.y = -topLeft.y;
        }

        Point stop(start.x + 2 * margin, start.y + 2 * margin);
        if (stop.x + bottomRight.x > width - 1)
        {
            stop.x = width - bottomRight.x - 1;
            start.x = stop.x - 2 * margin;
        }
        if (stop.y + bottomRight.y > height - 1)
        {
            stop.y = height - bottomRight.y - 1;
            start.y = stop.y - 2 * margin;
        }

        const auto range = Point(stop.x - start.x + 1,
                                 stop.y - start.y + 1);

        if (start.x + topLeft.x < 0 || start.x + topLeft.x >= width
            || start.y + topLeft.y < 0 || start.y + topLeft.y >= height
            || stop.x + bottomRight.x < 0 || stop.x + bottomRight.x >= width
            || stop.y + bottomRight.y < 0 || stop.y + bottomRight.y >= height
            || range.x > width || range.y > height)
            {
                std::cout  << "matchShapeOne(): invalid search range"
                           << std::endl;
                continue;
            }

        matchShape(rotatedModel, gx, gy, gMask, stride, start.x, start.y, score,
range.y, range.x);

        Point peakPosition;
        float peakScore;

        findPeak(score, range.y, range.x, stride, peakPosition, peakScore);

        if (peakScore > candidateOut.score)
        {
            candidateOut.x = peakPosition.x + start.x;
            candidateOut.y = peakPosition.y + start.y;
            candidateOut.angle = angle;
            candidateOut.score = peakScore;
        }
    }

    delete[] score;

    return candidateOut;
}

void findPeak(const float* image, std::size_t height, std::size_t width,
std::ptrdiff_t stride, std::vector<Point>& peakPosition, std::vector<float>&
peakValue, float threshold)
{
    cv::Mat imageCv(height, width, CV_32FC1, const_cast<float*>(image),
stride * sizeof(*image));
    cv::Mat imageDilate(height, width, CV_32FC1);
    cv::dilate(imageCv, imageDilate, cv::Mat(), cv::Point(-1, -1), 2, 1, 1);
    cv::Mat peaks;
    peaks = (imageCv == imageDilate) & (imageCv > threshold);
    peakPosition = findNonZero(peaks.data, peaks.rows, peaks.cols, peaks.step);

    peakValue.clear();
    for (const auto& peak : peakPosition)
    {
        peakValue.push_back(imageCv.at<float>(peak.y, peak.x));
    }
}

void findPeak(const float* image, std::size_t height, std::size_t width,
std::ptrdiff_t stride, Point& peakPosition, float& peakValue)
{
    peakValue = image[0];
    peakPosition = Point(0, 0);

    for (std::size_t j = 0; j < height; ++j, image += stride)
    {
        for (std::size_t i = 0; i < width; ++i)
        {
            if (image[i] > peakValue)
            {
                peakValue = image[i];
                peakPosition = Point(i, j);
            }
        }
    }
}

void filterCandidate(std::vector<MatchCandidate>& candidates, const ShapeModel&
model, double maxOverlap, unsigned int maxCandidates)
{
    std::sort(candidates.begin(), candidates.end(),
              [](const auto& lhs, const auto& rhs)
              {
                  return lhs.score > rhs.score;
              });

    const auto topLeft = model.tl();
    const auto bottomRight = model.br();

    const float rect[4][2] =
    {
        {topLeft.x, topLeft.y},
        {bottomRight.x, topLeft.y},
        {bottomRight.x, bottomRight.y},
        {topLeft.x, bottomRight.y}
    };

    const auto modelArea = abs((topLeft.x - bottomRight.x) *
                           (topLeft.y - bottomRight.y));

    for (auto i = candidates.begin(); i != candidates.end(); ++i)
    {
        for (auto j = i + 1; j != candidates.end();)
        {
            const auto area = overlapArea(*i, *j, rect);
            const auto areaRatio = area / modelArea;
            if (areaRatio > maxOverlap)
            {
                candidates.erase(j);
            }
            else
            {
                ++j;
            }
        }
    }

    if (maxCandidates > 0)
    {
        if (candidates.size() > maxCandidates)
        {
            candidates.resize(maxCandidates);
        }
    }
}

float overlapArea(MatchCandidate a, MatchCandidate b, const float rect[4][2])
{
    std::vector<Point> intersection;
    float rectA[4][2];
    float rectB[4][2];

    const auto angleA = a.angle * M_PI / 180;
    const auto cosA = cos(angleA);
    const auto sinA = sin(angleA);

    for (int i = 0; i < 4; i++)
    {
        rectA[i][0] = cosA * rect[i][0] - sinA * rect[i][1] + a.x;
        rectA[i][1] = sinA * rect[i][0] + cosA * rect[i][1] + a.y;
    }

    const auto angleB = b.angle * M_PI / 180;
    const auto cosB = cos(angleB);
    const auto sinB = sin(angleB);

    for (auto i = 0; i < 4; ++i)
    {
        rectB[i][0] = cosB * rect[i][0] - sinB * rect[i][1] + b.x;
        rectB[i][1] = sinB * rect[i][0] + cosB * rect[i][1] + b.y;
    }

    float vecA[4][2];
    float vecB[4][2];

    for (auto i = 0; i < 4; ++i)
    {
        vecA[i][0] = rectA[(i + 1) % 4][0] - rectA[i][0];
        vecA[i][1] = rectA[(i + 1) % 4][1] - rectA[i][1];

        vecB[i][0] = rectB[(i + 1) % 4][0] - rectB[i][0];
        vecB[i][1] = rectB[(i + 1) % 4][1] - rectB[i][1];
    }

    for (auto i = 0; i < 4; ++i)
    {
        for ( auto j = 0; j < 4; ++j)
        {
            const auto xba = rectB[j][0] - rectA[i][0];
            const auto yba = rectB[j][1] - rectA[i][1];

            const auto vxa = vecA[i][0];
            const auto vya = vecA[i][1];

            const auto vxb = vecB[j][0];
            const auto vyb = vecB[j][1];

            const auto det = vxb * vya - vxa * vyb;

            const auto t1 = (vxb * yba - vyb * xba) / det;
            const auto t2 = (vxa * yba - vya * xba) / det;

            if (t1 >= 0 && t1 <= 1 && t2 >= 0 && t2 <= 1)
            {
                intersection.push_back(
                    Point
                    (
                        rectA[i][0] + vecA[i][0] * t1,
                        rectA[i][1] + vecA[i][1] * t1
                    )
                );
            }
        }
    }

    for (auto i = 0; i < 4; ++i)
    {
        int positive = 0;
        int negative = 0;
        auto x = rectA[i][0];
        auto y = rectA[i][1];

        for (auto j = 0; j < 4; ++j)
        {
            const auto A = -vecB[j][1];
            const auto B = vecB[j][0];
            const auto C = -(A * rectB[j][0] + B * rectB[j][1]);

            const auto s = A * x + B * y + C;
            if (s >= 0)
            {
                positive++;
            }
            else
            {
                negative++;
            }
        }

        if (positive == 4 || negative == 4)
        {
            intersection.push_back(
                Point
                (
                    rectA[i][0],
                    rectA[i][1]
                )
            );
        }
    }

    for (auto i = 0; i < 4; ++i)
    {
        int positive = 0;
        int negative = 0;
        auto x = rectB[i][0];
        auto y = rectB[i][1];

        for (auto j = 0; j < 4; ++j)
        {
            const auto A = -vecA[j][1];
            const auto B = vecA[j][0];
            const auto C = -(A * rectA[j][0] + B * rectA[j][1]);

            const auto s = A * x + B * y + C;

            if (s >= 0)
            {
                positive++;
            }
            else
            {
                negative++;
            }
        }

        if (positive == 4 || negative == 4)
        {
            intersection.push_back(
                Point
                (
                    rectB[i][0],
                    rectB[i][1]
                )
            );
        }
    }

    for (auto i = intersection.begin(); i != intersection.end(); ++i)
    {
        for (auto j = i + 1; j != intersection.end();)
        {
            const auto dij = abs(j->x - i->x) + abs(j->y - i->y);
            const auto epsilon = 1e-16f;
            if (dij < epsilon)
            {
                intersection.erase(j);
            }
            else
            {
                ++j;
            }
        }
    }

    if (intersection.empty())
    {
        return 0;
    }

    for (auto i = intersection.begin(); i != std::prev(intersection.end()); ++i)
    {
        auto di = Point((i + 1)->x - i->x, (i + 1)->y - i->y);
        for (auto j = i + 2; j != intersection.end(); ++j)
        {
            const auto dj = Point(j->x - i->x, j->y - i->y);
            if (di.x * dj.y - di.y * dj.x < 0)
            {
                std::iter_swap(i + 1, j);
                di = dj;
            }
        }
    }

    const auto n = intersection.size();
    auto overlapArea = 0.0f;
    auto previous = intersection[n - 1];

    for (std::size_t i = 0; i < n; ++i)
    {
        const auto& point = intersection[i];
        overlapArea += previous.x * point.y - previous.y * point.x;
        previous = point;
    }
    overlapArea = abs(overlapArea) * 0.5;

    return overlapArea;
}


} //namespace filter
} //namespace precitec
