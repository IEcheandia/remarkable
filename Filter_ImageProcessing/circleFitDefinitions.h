#pragma once

// local includes
#include "fliplib/Fliplib.h"			// export macro
#include "fliplib/TransformFilter.h"	// base class
#include "fliplib/SynchronePipe.h"		// in- / output

#include "system/types.h"				// byte
#include "common/frame.h"				// ImageFrame
#include "filter/parameterEnums.h"
#include "image/image.h"				///< BImage

// std lib
#include <string>


namespace precitec {
	namespace filter {

        
class Circle
{
public:
    Circle() = default;

    Circle(double middleX, double middleY, double radius)
    : m_middleX(middleX), m_middleY(middleY), m_radius(radius)
    {}

	double m_middleX = 0;
    double m_middleY = 0;
    double m_radius = 0;
};

class Ellipse
{
public:
	Ellipse() = default;
	Ellipse(double middleX, double middleY, double a, double b, double phi)
        : m_middleX(middleX), m_middleY(middleY), m_a(a), m_b(b), m_phi(phi)
    {
        
    }

	double m_middleX = 0;
    double m_middleY = 0;
    double m_a = 0;
    double m_b = 0; 
    double m_phi = 0;
};


typedef std::pair<Circle, double> hough_circle_t;
struct CircleHoughParameters
{
    enum class ScoreType
    {
        Accumulator, LongestConnectedArc
    };
    double m_oRadiusStart;
    double m_oRadiusEnd;
    double m_oRadiusStep;
    int m_oNumberMax;
    bool m_oCoarse;
    ScoreType m_oScoreType;
    double m_oConnectedArcToleranceDegrees;
};

enum class SearchType
{
    OnlyCenterInsideROI,
    AllowCenterOutsideROI,
    OnlyCompleteCircleInsideROI
};


} // namespace filter
} // namespace precitec
