#include "offsetContour.h"

#include "module/moduleLogger.h"
#include "common/bitmap.h"

#include "fliplib/TypeToDataTypeImpl.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <vector>
#include <utility>
#include <cmath>
#include <algorithm>
#include <iterator>

#include <opencv2/core/types.hpp>

#define FILTER_ID                 "7c9c2368-4994-4b64-b043-f5f9f009f300"
#define VARIANT_ID                "fe915ebf-c628-49a2-8472-e02471a732c1"

#define PIPE_ID_CONTOURIN         "c29d9e74-7cd5-4bf8-8c73-630a8d30a21b"
#define PIPE_ID_OFFSET            "aa648979-9553-4658-90a8-406a55ab7fd2"
#define PIPE_ID_CONTOUROUT        "16207fb6-4c50-4a4a-b099-4b720529ca82"

namespace {

constexpr double numericTol = 1e-10;

bool IsCloseTo(cv::Point2d const &first, cv::Point2d const &second) {
  return cv::norm(first - second) < numericTol;
}

/**
 * @brief Calculates the left normal vector of the line segment given by first
 * and second and scales it to the length given by amount.
 *
 * Precondition: first and second are not almost identical.
 */
cv::Point2d ShiftedNormal(cv::Point2d const &first, cv::Point2d const &second,
                          double amount) {
  auto diff = second - first;
  double scale = amount / cv::norm(diff);
  // The normal vector of a vector (a, b) is either (-b, a) or
  // (b, -a) since then the scalar product is zero. We
  // arbirarily choose the first, pointing to the left.
  return cv::Point2d{-diff.y, diff.x} * scale;
}

struct LineInNormalForm {
  cv::Point2d point;
  cv::Point2d normal;
};

LineInNormalForm ToShiftedLine(cv::Point2d point, cv::Point2d normal) {
  point += normal;
  return {std::move(point), std::move(normal)};
}

/**
 * @brief Calculates the point of intersection between two lines.
 *
 * Basic formula taken from
 * https://de.wikipedia.org/wiki/Gerade#Schnittpunkt_zweier_Geraden which is
 * just the solution of the system of two line equations.
 *
 * In case the two lines are parallel, we return the point of the second line.
 * This is the correct choice here, because we apply this function to lines
 * based on consecutive line segments, which have one point in common by
 * construction. Hence, the two shifted lines are not only parallel but
 * identical. The resulting line segments we are interested in touch at the base
 * point of the second line, which is just the shifted intermediate point of the
 * original line segments.
 */
cv::Point2d Intersect(LineInNormalForm l1, LineInNormalForm l2) {
  auto const [a1, b1] = l1.normal;
  auto const [a2, b2] = l2.normal;
  double const det = a1 * b2 - a2 * b1;
  if (std::abs(det) < numericTol) {
    // The lines are parallel.
    return l2.point;
  }

  double const c1 = l1.normal.ddot(l1.point);
  double const c2 = l2.normal.ddot(l2.point);
  return {(c1 * b2 - c2 * b1) / det, (a1 * c2 - a2 * c1) / det};
}

std::vector<cv::Point2d>
ShiftedFirstAndLast(std::vector<cv::Point2d> const &original,
                    std::vector<LineInNormalForm> const &shiftedLines) {
  return {shiftedLines.front().point,
          original.back() + shiftedLines.back().normal};
}

} // namespace

std::vector<cv::Point2d> ScaleContour(std::vector<cv::Point2d> linePoints,
                                      double amount) {
  auto last = std::unique(linePoints.begin(), linePoints.end(), IsCloseTo);
  linePoints.erase(last, linePoints.end());

  if (linePoints.size() < 2) {
    return {};
  }

  // Calculate vectors perpendicular to each segment, pointing to the left when
  // amount is positive.
  std::vector<LineInNormalForm> shiftedLines{};
  shiftedLines.reserve(linePoints.size() - 1);
  std::transform(
      linePoints.begin(), std::prev(linePoints.end()),
      std::next(linePoints.begin()), std::back_inserter(shiftedLines),
      [&](cv::Point2d const &first, cv::Point2d const &second) {
        return ToShiftedLine(first, ShiftedNormal(first, second, amount));
      });

  bool const isClosedContour = IsCloseTo(linePoints.front(), linePoints.back());

  if (shiftedLines.size() == 1) {
    return ShiftedFirstAndLast(linePoints, shiftedLines);
  }

  std::vector<cv::Point2d> shiftedPoints{};
  if (isClosedContour) {
    auto firstCorner = Intersect(shiftedLines.back(), shiftedLines.front());
    shiftedPoints.emplace_back(firstCorner);
    std::transform(shiftedLines.begin(), std::prev(shiftedLines.end()),
                   std::next(shiftedLines.begin()),
                   std::back_inserter(shiftedPoints), Intersect);
    shiftedPoints.emplace_back(std::move(firstCorner));
  } else {
    auto firstAndLast = ShiftedFirstAndLast(linePoints, shiftedLines);
    shiftedPoints.reserve(linePoints.size());
    shiftedPoints.emplace_back(std::move(firstAndLast[0]));
    std::transform(shiftedLines.begin(), std::prev(shiftedLines.end()),
                   std::next(shiftedLines.begin()),
                   std::back_inserter(shiftedPoints), Intersect);
    shiftedPoints.emplace_back(std::move(firstAndLast[1]));
  }

  return shiftedPoints;
}

namespace precitec
{
namespace filter
{

OffsetContour::OffsetContour()
    : TransformFilter("OffsetContour", Poco::UUID(FILTER_ID))
    , m_contourIn(nullptr)
    , m_offset(nullptr)
    , m_contourOut(this, "ContourOut")
{
    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_contourIn, "ContourIn", 1, "ContourIn"},
        {Poco::UUID(PIPE_ID_OFFSET), m_offset, "Offset", 1, "Offset"},
    });

    setOutPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOUROUT), &m_contourOut, "ContourOut", 1, "ContourOut"},
    });

    setVariantID(Poco::UUID(VARIANT_ID));
}

void OffsetContour::setParameter()
{
    TransformFilter::setParameter();
}

void OffsetContour::paint()
{
    if (m_oVerbosity == eNone)
    {
        return;
    }
}

bool OffsetContour::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == "ContourIn")
    {
        m_contourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }
    else if (pipe.tag() == "Offset")
    {
        m_offset = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }

    return BaseFilter::subscribe(pipe, group);
}

void OffsetContour::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto &contourIn = m_contourIn->read(m_oCounter);
    const auto scaleXArray = m_offset->read(m_oCounter);
    const auto offset = scaleXArray.ref().getData().at(0);

    auto contourOut = contourIn;

    auto& contours = contourOut.ref();

    for (auto &contour : contours)
    {
        std::vector<geo2d::DPoint>& contourPoints = contour.getData();
        std::vector<cv::Point2d> cvContourIn;
        for (const auto& p : contourPoints)
        {
            cvContourIn.emplace_back(p.x, p.y);
        }
        const auto cvContourOut = ScaleContour(cvContourIn, offset);

        if (cvContourOut.size() != contourPoints.size())
        {
            throw std::invalid_argument("Offset transform for the given input contour not supported");
        }

        for (std::size_t i = 0; i < contourPoints.size(); ++i)
        {
            contourPoints[i] = {cvContourOut[i].x, cvContourOut[i].y};
        }
    }

    preSignalAction();
    m_contourOut.signal(contourOut);
}

} //namespace filter
} //namespace precitec
