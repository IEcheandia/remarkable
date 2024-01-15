/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		LB
* 	@date		2016
* 	@brief		This filter computes the intersection between 2 straight lines
*/


#ifndef INTERSECT2LINES
#define INTERSECT2LINES

// WM includes
#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
#include <common/frame.h>
#include "math/2D/LineEquation.h"

#include <optional>

namespace precitec
{
namespace filter
{

class FILTER_API Intersect2Lines : public fliplib::TransformFilter
{
public:

    /**
    * CTor.
    */
    Intersect2Lines();
    /**
    * @brief DTor.
    */
    virtual ~Intersect2Lines();

    // Declare constants
    static const std::string m_oFilterName; ///< Filter name.
    static const std::string m_oPipeOutIntersectionResultName;///< Pipe: Data out-pipe.
    static const std::string m_oPipeOutXName;///< Pipe: Data out-pipe.
    static const std::string m_oPipeOutYName;///< Pipe: Data out-pipe.
    static const std::string m_oPipeOutAngleName;///< Pipe: Data out-pipe.

    /**
    * @brief Set filter parameters.
    */
    void setParameter();

    void paint();



protected:

    /**
    * @brief In-pipe registration.
    * @param p_rPipe Reference to pipe that is getting connected to the filter.
    * @param p_oGroup Group number (0 - proceed is called whenever a pipe has a data element, 1 - proceed is only called when all pipes have a data element).
    */
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);

    /**
    * @brief Processing routine.
    * @param p_pSender pointer to
    * @param p_rEvent
    */
    void proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE);

private:
    enum class PlaneForAngleComputation
    {
        LineLaser1 = 0,
        LineLaser2,
        LineLaser3,
        Screen
    };

    struct LineSegment
    {
        math::LineEquation m_oLine;
        geo2d::DPoint m_oPoint;
    };

    struct LinesIntersection
    {
        bool m_ohasIntersection = false;
        double m_oX = 0.0;
        double m_oY = 0.0;
        LineSegment m_oLine1ForAngleComputation; //line 1 [ref system according to PlaneForAngleComputation ]
        LineSegment m_oLine2ForAngleComputation; //line 2 [ref system according to PlaneForAngleComputation ]
        double m_oAngle = 0.0;
    };

    static double computeAngleAtIntersection(const LineSegment & rLineSegment1, const LineSegment & rLineSegment2, const double & rXIntersection);

    LineSegment parseLineInput(const interface::GeoLineModelarray & rGeoLineModel, int lineNumber) const;

    geo2d::Point convertToScreenPoint(const double & r_X, const double & r_Y) const;

    LineSegment convertScreenLineToLaserPlaneLine(
        math::LineEquation p_oLine, geo2d::DPoint p_oCenter, 
        geo2d::Point oSensorVertexUpperLeft, geo2d::Point oSensorVertexLowerRight, const interface::ImageContext & rContext) const;

    std::optional<LinesIntersection> updateIntersection (const LineSegment & rLineSegment1, const LineSegment & rLineSegment2, const geo2d::Size & rImageSize, const interface::ImageContext & rContext) const;

    const fliplib::SynchronePipe< interface::ImageFrame >* m_pPipeInImage;
    const fliplib::SynchronePipe< interface::GeoLineModelarray >* m_pPipeInFirstLine;
    const fliplib::SynchronePipe< interface::GeoLineModelarray >* m_pPipeInSecondLine;

    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutIntersectionResult;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutX;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutY;
    fliplib::SynchronePipe< interface::GeoDoublearray > m_oPipeOutAngle;

    bool m_oValidOnlyIfInRoi; //parameter
    bool m_oAngleOnScreenCoordinates; //internal variable derived from parameter PlaneForAngleComputation
    filter::LaserLine m_oTypeOfLaserLine; //internal variable derived from parameter PlaneForAngleComputation

    interface::SmpTrafo m_oSpTrafo; ///< roi translation
    LineSegment m_oPaintSegment1; // coordinates according to m_oSpTrafo
    LineSegment m_oPaintSegment2; // coordinates according to m_oSpTrafo
    LinesIntersection m_oPaintIntersection;

};



}
}

#endif
