#pragma once

#include <vector>
#include <memory>

#include "contourMetaLanguage.h"

namespace precitec
{
namespace hardware
{

/**
* Contour generator is a class which generates an array filled with the contour meta language from a input array that has the positions, laser powers and speeds of the contour.
* The contour meta language is constructed with different data structures which represent the position and settings.
**/
class ContourGenerator
{
public:
    ContourGenerator();
    ~ContourGenerator();
    const std::vector<std::shared_ptr<contour::Command>>& generatedContour() const
    {
        return m_generatedContour;
    }
    bool empty() const
    {
        return m_generatedContour.empty();
    }
    void resetContour();
    void addInitialize();
    void addMark(double x, double y);
    void addJump(double x, double y);
    void addLaserPower(double power);
    void addMarkSpeed(double speed);

private:
    std::vector<std::shared_ptr<contour::Command>> m_generatedContour;
};

}
}
