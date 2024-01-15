/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		This filter generate a contour in the shape of an arc, from the intersection between a rectangle
 * (input ROI) and a circle (input x,y,r) .
 */

#ifndef GENERATEARCCONTOUR_H
#define GENERATEARCCONTOUR_H

#include "fliplib/Fliplib.h"
#include "fliplib/TransformFilter.h"
#include "fliplib/SynchronePipe.h"
#include <geo/geo.h>
#include <common/frame.h>


namespace precitec {
namespace filter {
    
class FILTER_API GenerateArcContour : public fliplib::TransformFilter
{
public:
    GenerateArcContour();
    void setParameter();
    
private:
    typedef fliplib::SynchronePipe<interface::ImageFrame> pipe_image_t;
    typedef fliplib::SynchronePipe<interface::GeoDoublearray> pipe_scalar_t;
    typedef fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray> pipe_contour_t;
    typedef double theta_rad;
    
    struct Circle
    {
        enum class SegmentPosition {vertical, horizontal};
        
        template<SegmentPosition t>
        std::vector<theta_rad> intersectWithSemiOpenSegmentRelative(const double fixedCoordinate, const double variableCoordinateStart, const double variableCoordinateEnd) const;
        
        std::vector<theta_rad> intersectWithHorizontalSemiOpenSegment(geo2d::DPoint startPoint, double xEnd) const;
        std::vector<theta_rad> intersectWithVerticalSemiOpenSegment(geo2d::DPoint startPoint, double yEnd) const;

        geo2d::DPoint getPoint (theta_rad theta) const;
        bool isPointOnCircle(geo2d::DPoint point) const;
        
        geo2d::DPoint m_center;
        double m_radius;
        double m_tolerance = 1e-12;
    };
    
    struct ArcBounds
    {
        theta_rad m_start;
        theta_rad m_end;
    };
    enum class MultipleArcsStrategy
    {
        InvalidIfMultipleArcs = 0, 
        ChooseLongestArc,
        COUNT
    };
    
    static std::vector<ArcBounds> intersectCircleWithRectangle(const Circle & rCircle, geo2d::DPoint topLeftCorner, geo2d::Size rectSize);
    static bool isPointInROI(geo2d::DPoint point, geo2d::DPoint roiTopLeftCorner, geo2d::Size roiSize);
    
    void clearOutPoints();
    void updateOutPoints(const ArcBounds & rArcBounds, int rank);
    
    bool subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup);
    void proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEvent );
    void paint();

    const pipe_image_t * m_pPipeInROI; 
    const pipe_scalar_t * m_pPipeInCenterX; 
    const pipe_scalar_t * m_pPipeInCenterY; 
    const pipe_scalar_t * m_pPipeInRadius; 
    pipe_contour_t m_oPipeOutData; 
    
    int m_numberOutputPoints;
    MultipleArcsStrategy m_oMultipleArcsStrategy;
    interface::SmpTrafo m_oSpTrafo;
    Circle m_oInputCircle;
    geo2d::Size m_oInputROISize;
    std::vector<geo2d::AnnotatedDPointarray> m_oOutPoints;
};


}
}

#endif
