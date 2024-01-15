#pragma once

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "geo/geo.h"
#include "common/frame.h"

#include <cstddef>
#include <cstdint>
#include <cmath>
#include <vector>

#include <opencv2/opencv.hpp>

namespace precitec
{
namespace filter
{

struct Point // TODO: use WM Point class instead
{
    float x;
    float y;

    Point();
    Point(float x, float y);
};

std::ostream& operator<<(std::ostream& os, const Point& point);
std::ostream& operator<<(std::ostream& os, const std::vector<Point>& points);

struct MatchCandidate
{
    float x;
    float y;
    float score;
    float angle;
    float scale;

    MatchCandidate();
    MatchCandidate(float x, float y, float score, float angle);
};

class ShapeModel
{
private:
    std::vector<Point> m_position;
    std::vector<Point> m_direction;
    Point m_topLeft;
    Point m_bottomRight;

public:
    ShapeModel(const cv::Mat& tmpl, double contrast);
    ShapeModel(const ShapeModel& model, double angle, double scale = 1);

    ShapeModel& rotate(double angle);

    const std::vector<Point>& position() const;
    const std::vector<Point>& direction() const;
    Point tl() const;
    Point br() const;

    double optimalAngleStep() const;

private:
    void updateBoundingBox();
};

inline std::ptrdiff_t roundIndex(float index)
{
    return static_cast<std::ptrdiff_t>(std::lround(index));
}

inline std::ptrdiff_t roundIndex(double index)
{
    return static_cast<std::ptrdiff_t>(std::lround(index));
}

std::vector<Point> findNonZero(const uint8_t* src, std::size_t height,
std::size_t width, std::ptrdiff_t stride);

std::vector<std::ptrdiff_t> positionToIndex(const std::vector<Point>& positions,
std::ptrdiff_t stride);

/*
Given gradient components 'Gx' and 'Gy', calculate:
1) the magnitude of gradient 'G'
2) the normalized gradient
3) the thresholding of gradient magnitude 'GMask'
 */
void normalizeGradient(const int16_t* srcGx, const int16_t* srcGy, float*
dstGx, float* dstGy, int16_t* dstG, uint8_t* dstGMask, int16_t gMaskThreshold,
std::size_t height, std::size_t width, std::ptrdiff_t srcStride,
std::ptrdiff_t dstStride);

/*
Approximate sqrt(dx^2 + dy^2). Provides better accuracy than abs(dx) + abs(dy),
which has an error factor of 1.414 when 'dx' and 'dy' have similar values. The
error causes distortion when normalizing matching score and is too large for
score comparison. This function supports 'dx' and 'dy' values from -1023 to 1023
 */
int16_t approximateDistance(int16_t dx, int16_t dy);

/*
Given a shape model and gradient of image to be matched, calculate the matching
score for each pixel position within the region defined by 'startOffset' and
destination height/width parameters.

'gx' and 'gy' should be normalized gradient vectors. 'gMask' specifies which
gradient pixels should be considered in the calculation of score. Normally,
when the gradient magnitude is below a certain threshold, the pixel should be
ignored because it is most likely not an edge pixel which would cause noise.
 */
void matchShape(const ShapeModel& model, const float* gx, const float* gy,
const uint8_t* gMask, std::ptrdiff_t stride, int startOffsetX, int startOffsetY,
float* dst, std::size_t dstHeight, std::size_t dstWidth);

/*
Calls matchShape function for each angle pose, find all peaks for score maps
of each angle pose and return peaks as a vector of MatchCandidate
 */
std::vector<MatchCandidate> matchShapeAll(const ShapeModel& model, const
float* gx, const float* gy, const uint8_t* gMask, std::size_t height,
std::size_t width, std::ptrdiff_t stride, float minScore, double angleStart,
double angleStep, double angleExtent);

/*
Performs matchShape around a given approximate position and returns a
MatchCandidate with a better accuracy
 */
MatchCandidate matchShapeOne(const ShapeModel& model, const MatchCandidate&
candidate, const float* gx, const float* gy, const uint8_t* gMask, std::size_t
height, std::size_t width, std::ptrdiff_t stride, double angleStart, double
angleStep, double angleExtent);

/*
Finds all local peaks in a float image whose value is greater than the threshold
 */
void findPeak(const float* image, std::size_t height, std::size_t width,
std::ptrdiff_t stride, std::vector<Point>& peakPosition, std::vector<float>&
peakValue, float threshold);

/*
Finds the global peak in a float image
 */
void findPeak(const float* image, std::size_t height, std::size_t width,
std::ptrdiff_t stride, Point& peakPosition, float& peakValue);

/*
Given a list of MatchCandidates, sort the list based on candidate score,
remove those candidates that have lower score than the other overlapping
candidates, when the overlap ratio exceeds 'maxOverlap'. Lastly, cap the list
size to 'maxCandidates' if 'maxCandidates' is not zero.
 */
void filterCandidate(std::vector<MatchCandidate>& candidates, const ShapeModel&
model, double maxOverlap, unsigned int maxCandidates = 0);

/*
Given two match candidates and their respective shape model bounding box, find
the overlap area between the two candidates
 */
float overlapArea(MatchCandidate a, MatchCandidate b, const float rect[4][2]);

} //namespace filter
} //namespace precitec
