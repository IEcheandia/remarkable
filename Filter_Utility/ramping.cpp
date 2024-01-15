#include "ramping.h"

#include <utility>
#include <cmath>
#include <vector>
#include <algorithm>

using precitec::filter::eRankMax;

static const double MIN_RESOLUTION_RTC6 {0.001};                 //[mm] -> =1µm

namespace {
    const auto attributeLaserPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower;
    const auto attributeLaserPowerRing = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing;
    const auto attributeLaserVelocity = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity;

    bool fuzzyCompare(double d1, double d2)
    {
        return (std::abs(d1 - d2) * 100000.f <= std::min(std::abs(d1), std::abs(d2)));
    }
}

Ramping::Ramping()
{ }

Ramping::~Ramping() = default;

void Ramping::setLength(double length)
{
    m_length = length;
}

void Ramping::setStartPower(double power)
{
    m_startPower = power;
}

void Ramping::setEndPower(double power)
{
    m_endPower = power;
}

void Ramping::setStartPowerRing(double power)
{
    m_startPowerRing = power;
}

void Ramping::setEndPowerRing(double power)
{
    m_endPowerRing = power;
}

void Ramping::setRampStep(double newRampStep)
{
    m_rampStep = newRampStep;
}

precitec::geo2d::AnnotatedDPointarray Ramping::createRamp(const precitec::geo2d::AnnotatedDPointarray& contour, std::size_t startContourPoint)
{
    if (contour.getData().empty() || m_rampStep <= 0.0 || m_length <= 0.0)
    {
        return contour;
    }

    precitec::geo2d::AnnotatedDPointarray ramp;
    ramp.reserve(m_length / m_rampStep);

    auto& rampContour = ramp.getData();
    auto& laserPower = ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower);
    laserPower.emplace_back(m_startPower);
    auto& laserPowerRing = ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing);
    laserPowerRing.emplace_back(m_startPowerRing);
    auto& rampVelocity = ramp.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity);
    rampVelocity.emplace_back(contour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).at(0));

    rampContour.emplace_back(contour.getData().at(startContourPoint));
    ramp.getRank().push_back(eRankMax);

    double length = 0.0;
    std::size_t currentPoint = startContourPoint + 1;

    while (length < m_length && currentPoint < contour.getData().size())
    {
        const auto& startPoint = rampContour.back();
        const auto& endPoint = contour.getData()[currentPoint];
        const auto& vector = vectorFromPoints(startPoint, endPoint);
        const auto distance = lengthFromVector(vector);

        if (distance < m_rampStep)
        {
            if (precitec::geo2d::distance(rampContour.back(), contour.getData()[currentPoint]) > MIN_RESOLUTION_RTC6)        //Avoid getting two points which are too close.
            {
                rampContour.emplace_back(contour.getData()[currentPoint]);
                ramp.getRank().push_back(eRankMax);
                length += distance;
                laserPower.emplace_back(linearInterpolationCorePower(length));
                laserPowerRing.emplace_back(linearInterpolationRingPower(length));
                rampVelocity.emplace_back(contour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity)[currentPoint]);
            }
            else
            {
                if (currentPoint < contour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity).size())
                {
                    rampVelocity.back() = contour.getScalarData(precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity)[currentPoint];
                }
            }
            currentPoint++;
        }
        else
        {
            if ((length + m_rampStep) > m_length)
            {
                auto factor = (m_length - length) / m_rampStep;
                rampContour.emplace_back(startPoint + (vector / distance * m_rampStep * factor));
                ramp.getRank().push_back(eRankMax);
                length += (m_rampStep * factor);
                laserPower.emplace_back(linearInterpolationCorePower(length));
                laserPowerRing.emplace_back(linearInterpolationRingPower(length));
                rampVelocity.emplace_back(-1.0);
                break;
            }
            else
            {
                rampContour.emplace_back(startPoint + (vector / distance * m_rampStep));
                ramp.getRank().push_back(eRankMax);
                length += m_rampStep;
                laserPower.emplace_back(linearInterpolationCorePower(length));
                laserPowerRing.emplace_back(linearInterpolationRingPower(length));
                rampVelocity.emplace_back(-1.0);
            }
        }
    }

    return ramp;
}


double Ramping::lengthFromVector(const precitec::geo2d::TPoint< double >& vector)
{
    return std::sqrt(std::pow(vector.x, 2) + std::pow(vector.y, 2));
}

precitec::geo2d::TPoint< double > Ramping::vectorFromPoints(const precitec::geo2d::TPoint< double >& startPoint, const precitec::geo2d::TPoint< double >& endPoint)
{
    return endPoint - startPoint;
}

double Ramping::linearInterpolationCorePower(double length)
{
    return ((m_endPower - m_startPower) / m_length * length) + m_startPower;
}

double Ramping::linearInterpolationRingPower(double length)
{
    return ((m_endPowerRing - m_startPowerRing) / m_length * length) + m_startPowerRing;
}

precitec::geo2d::AnnotatedDPointarray Ramping::reversePoints(const precitec::geo2d::AnnotatedDPointarray& contour)
{
    precitec::geo2d::AnnotatedDPointarray reversedContour {contour};
    std::reverse(reversedContour.getData().begin(), reversedContour.getData().end());

    return reversedContour;
}

std::vector<double> Ramping::changeOrderRampOutVelocity(const precitec::geo2d::AnnotatedDPointarray& contour, const precitec::geo2d::AnnotatedDPointarray& contourRampOut)
{
    std::vector<double> contourWithRightVelocities {contourRampOut.getScalarData(attributeLaserVelocity)};

    std::size_t pointBeforeRampOutID = contour.size();
    double length = 0.0;
    for (std::size_t currentPoint = contour.size() - 1; currentPoint > 0; currentPoint--)
    {
        const auto& startPoint = contour.getData()[currentPoint];
        const auto& endPoint = contour.getData()[currentPoint - 1];
        length += lengthFromVector(vectorFromPoints(startPoint, endPoint));

        if (length >= m_length || fuzzyCompare(length, m_length))
        {
            pointBeforeRampOutID = currentPoint - 1;
            break;
        }
    }

    if (pointBeforeRampOutID < contour.getScalarData(attributeLaserVelocity).size())
    {
        contourWithRightVelocities.front() = contour.getScalarData(attributeLaserVelocity)[pointBeforeRampOutID];
    }

    std::size_t pointIDRampOut = 0;
    for (const auto& pointInRampOutContour : contourRampOut.getData())
    {
        auto foundPointWhichExistInContour = std::find_if(contour.getData().begin(), contour.getData().end(), [pointInRampOutContour](const auto& currentPointInContour){ return currentPointInContour == pointInRampOutContour;});
        if (foundPointWhichExistInContour == contour.getData().end())
        {
            pointIDRampOut++;
            continue;
        }
        auto pointPosition = std::distance(contour.getData().begin(), foundPointWhichExistInContour);

        if (static_cast<unsigned int> (pointPosition) < contour.getScalarData(attributeLaserVelocity).size())
        {
            contourWithRightVelocities[pointIDRampOut] = contour.getScalarData(attributeLaserVelocity)[pointPosition];
        }
        pointIDRampOut++;
    }

    return contourWithRightVelocities;
}
