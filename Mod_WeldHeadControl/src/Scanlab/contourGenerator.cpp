#include "viWeldHead/Scanlab/contourGenerator.h"

namespace precitec
{
namespace hardware
{

ContourGenerator::ContourGenerator() = default;

ContourGenerator::~ContourGenerator() = default;

void ContourGenerator::resetContour()
{
    m_generatedContour.clear();
}

void ContourGenerator::addInitialize()
{
    std::shared_ptr<contour::Initialize> initialize(new contour::Initialize);
    m_generatedContour.push_back(std::move(initialize));
}

void ContourGenerator::addMark(double x, double y)
{
    std::shared_ptr<contour::Mark> mark(new contour::Mark);
    mark->x = x;
    mark->y = y;
    m_generatedContour.push_back(std::move(mark));
}

void ContourGenerator::addJump(double x, double y)
{
    std::shared_ptr<contour::Jump> jump(new contour::Jump);
    jump->x = x;
    jump->y = y;
    m_generatedContour.push_back(std::move(jump));
}

void ContourGenerator::addLaserPower(double power)
{
    std::shared_ptr<contour::LaserPower> laserPower(new contour::LaserPower);
    laserPower->power = power;
    m_generatedContour.push_back(std::move(laserPower));
}

void ContourGenerator::addMarkSpeed(double speed)
{
    std::shared_ptr<contour::MarkSpeed> markSpeed(new contour::MarkSpeed);
    markSpeed->speed = speed;
    m_generatedContour.push_back(std::move(markSpeed));
}

}
}
