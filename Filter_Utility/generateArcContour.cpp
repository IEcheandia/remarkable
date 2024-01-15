/**
 * 	@file
 * 	@copyright	Precitec Vision GmbH & Co. KG
 * 	@author		LB
 * 	@date		2020
 * 	@brief		This filter generates a contour in the shape of an arc, from the intersection between a rectangle
 * (input ROI) and a circle (input x,y,r) .
 */


#include "generateArcContour.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "filter/algoArray.h"

#include "math/mathCommon.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include "fliplib/TypeToDataTypeImpl.h"

namespace precitec {
namespace filter {

using fliplib::Parameter;

GenerateArcContour::GenerateArcContour():
    TransformFilter("GenerateArcContour", Poco::UUID("2771F862-4DEA-4C9F-BCD4-5FD6B0F47D1A")),
    m_pPipeInROI(nullptr),
    m_pPipeInCenterX(nullptr),
    m_pPipeInCenterY(nullptr),
    m_pPipeInRadius(nullptr),
    m_oPipeOutData(this,"Contour"),
    m_numberOutputPoints(20),
    m_oMultipleArcsStrategy(MultipleArcsStrategy::ChooseLongestArc)
    {
        parameters_.add("NumPoints", Parameter::TYPE_int, m_numberOutputPoints);
        parameters_.add("MultipleArcsStrategy", Parameter::TYPE_int, (int)(m_oMultipleArcsStrategy));

        setInPipeConnectors({{Poco::UUID("9A673CC2-BFD8-4309-830E-1799CBC204A7"), m_pPipeInROI, "roi", 1, "roi"},{Poco::UUID("FBD27D39-236E-415E-A6C9-F1DE001878CA"), m_pPipeInCenterX, "centerX", 1, "centerX"},{Poco::UUID("53B62D63-3BE8-4F26-A3E0-2D5EBDB23A75"), m_pPipeInCenterY, "centerY", 1, "centerY"},{Poco::UUID("205641BF-4DD6-4958-AA2D-5FC57462E612"), m_pPipeInRadius, "radius", 1, "radius"}});
        setOutPipeConnectors({{Poco::UUID("4E9E640D-ED17-42C4-9B5F-D7B9A7BABC5A"), &m_oPipeOutData, "Contour", 0, "Contour"}});
        setVariantID(Poco::UUID("A5C21C23-1AFD-415D-B7BB-2499B00570ED"));
    }

void GenerateArcContour::setParameter()
{
    TransformFilter::setParameter();
    m_numberOutputPoints = parameters_.getParameter("NumPoints").convert<int>();
    int oMultipleArcsStrategy = parameters_.getParameter("MultipleArcsStrategy").convert<int>();
    m_oMultipleArcsStrategy = (oMultipleArcsStrategy >= 0 && m_oMultipleArcsStrategy < MultipleArcsStrategy::COUNT) ?
                                   MultipleArcsStrategy(oMultipleArcsStrategy) :  MultipleArcsStrategy::InvalidIfMultipleArcs ;
}

bool GenerateArcContour::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "roi" )
    {
        m_pPipeInROI  = dynamic_cast<pipe_image_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "centerX" )
    {
        m_pPipeInCenterX  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "centerY" )
    {
        m_pPipeInCenterY  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "radius" )
    {
        m_pPipeInRadius  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void GenerateArcContour::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
    using interface::GeoDoublearray;

    poco_assert_dbg(m_pPipeInCenterX != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInCenterY != nullptr); // to be asserted by graph editor

    const auto & rFrameIn = m_pPipeInROI->read(m_oCounter);
    const auto & rGeoDoubleArrayInX = m_pPipeInCenterX->read(m_oCounter);
    const auto & rGeoDoubleArrayInY = m_pPipeInCenterY->read(m_oCounter);
    const auto & rGeoDoubleArrayInRadius = m_pPipeInRadius->read(m_oCounter);

    const auto & rOutputContext = rFrameIn.context();
    m_oSpTrafo = rOutputContext.trafo();

    bool allValidInput = true;
    int minimumInputRank = eRankMax;


    enum class inputType {x,y,radius};
    auto fParseInputToROIContext = [this, & allValidInput, &minimumInputRank](const GeoDoublearray & rGeoArray, const std::string & debugName, inputType t)
    {
        auto size = rGeoArray.ref().size();
        if (size == 0)
        {
            allValidInput = false;
            minimumInputRank = eRankMin;
            return 0.0;
        }
        if (size > 1)
        {
            wmLog(eDebug, "Filter '%s': Received %u %s values. Can only process first element, rest will be discarded.\n", name().c_str(), size, debugName.c_str());
        };
        auto rankIn = rGeoArray.ref().getRank().front();
        minimumInputRank = (rankIn < minimumInputRank) ?  rankIn : minimumInputRank;

        double valueIn = rGeoArray.ref().getData().front();
        switch(t)
        {
            case inputType::x: return valueIn + rGeoArray.context().trafo()->dx() - m_oSpTrafo->dx(); break;
            case inputType::y: return valueIn + rGeoArray.context().trafo()->dy() - m_oSpTrafo->dy(); break;
            case inputType::radius: return valueIn ; break;
        }
        assert(false && "case not handled"); return 0.0;
    };

    const auto & rImageIn = rFrameIn.data();
    m_oInputROISize = rImageIn.size();

    interface::ResultType oGeoAnalysisResult = std::max ({rFrameIn.analysisResult(), rGeoDoubleArrayInX.analysisResult(), rGeoDoubleArrayInY.analysisResult(), rGeoDoubleArrayInRadius.analysisResult()});
    if (!rImageIn.isValid() || !allValidInput )
    {
        clearOutPoints();
        const interface::GeoVecAnnotatedDPointarray oGeoOut( rOutputContext, m_oOutPoints, oGeoAnalysisResult, 1.0 );

        preSignalAction();
        m_oPipeOutData.signal(oGeoOut);
    }

    //perform computation in the context of ROI

    m_oInputCircle.m_center.x = fParseInputToROIContext (rGeoDoubleArrayInX, m_pPipeInCenterX->tag(), inputType::x);
    m_oInputCircle.m_center.y = fParseInputToROIContext (rGeoDoubleArrayInY, m_pPipeInCenterY->tag(), inputType::y);
    m_oInputCircle.m_radius = fParseInputToROIContext (rGeoDoubleArrayInRadius, m_pPipeInRadius->tag(), inputType::radius);

    auto oArcBounds = intersectCircleWithRectangle(m_oInputCircle, {0,0}, m_oInputROISize);

    if (oArcBounds.size() == 0)
    {
        clearOutPoints();
    }
    else if (oArcBounds.size() == 1)
    {
        updateOutPoints(oArcBounds.front(), minimumInputRank);
    }
    else
    {
        switch(m_oMultipleArcsStrategy)
        {
            case MultipleArcsStrategy::InvalidIfMultipleArcs:
            case MultipleArcsStrategy::COUNT:
                clearOutPoints();
                break;
            case MultipleArcsStrategy::ChooseLongestArc:
                {
                    auto compareArc = [] (ArcBounds a, ArcBounds b){
                            return (std::abs(a.m_end - a.m_start) < std::abs(b.m_end - b.m_start));
                        };
                    auto itArc = std::max_element(oArcBounds.begin(), oArcBounds.end(), compareArc);
                    updateOutPoints(*itArc, minimumInputRank);
                }
                break;
        }
    }

    const interface::GeoVecAnnotatedDPointarray oGeoOut( rOutputContext, m_oOutPoints, oGeoAnalysisResult, 1.0 );

    preSignalAction();
    m_oPipeOutData.signal(oGeoOut);

}

void GenerateArcContour::paint()
{
    using namespace precitec::image;

    if (m_oVerbosity < VerbosityType::eLow)
    {
        return;
    }

    if (m_oSpTrafo.isNull())
    {
        return;
    }

    OverlayCanvas &rCanvas ( canvas<OverlayCanvas>(m_oCounter) );

    OverlayLayer & rLayerPosition (rCanvas.getLayerPosition());
    OverlayLayer & rLayerContour (rCanvas.getLayerLine() );

    auto oPointColor = Color::Magenta();

    for (auto & rOutDPointarray : m_oOutPoints)
    {
        auto & rContourData = rOutDPointarray.getData();

        int numPoints = rOutDPointarray.size();
        if (numPoints == 0)
        {
            continue;
        }
        geo2d::Point previousCanvasPoint;
        for (int i = 0 ; i < numPoints; i++)
        {
            auto & rPoint = rContourData[i];
            auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));
            rLayerPosition.add<OverlayPoint>(oCanvasPoint.x, oCanvasPoint.y, oPointColor);
            if (i > 0 && m_oVerbosity >= VerbosityType::eMedium)
            {
                rLayerContour.add<OverlayLine>(previousCanvasPoint.x, previousCanvasPoint.y, oCanvasPoint.x, oCanvasPoint.y, oPointColor);
            }
            previousCanvasPoint = oCanvasPoint;
        }
    }

    if (m_oVerbosity < VerbosityType::eHigh)
    {
        return;
    }

    rLayerContour.add<OverlayCircle>(std::round(m_oInputCircle.m_center.x + m_oSpTrafo->dx()), std::round(m_oInputCircle.m_center.y + m_oSpTrafo->dy()), m_oInputCircle.m_radius, Color::Yellow());
    geo2d::Point upperLeftCorner = m_oSpTrafo->apply(geo2d::Point(0,0));
    rLayerContour.add<OverlayRectangle>(upperLeftCorner.x, upperLeftCorner.y, m_oInputROISize.width, m_oInputROISize.height, Color::Cyan());

}



/*static */ std::vector<GenerateArcContour::ArcBounds> GenerateArcContour::intersectCircleWithRectangle(const Circle & rCircle, geo2d::DPoint roiTopLeftCorner, geo2d::Size roiSize)
{
    assert(rCircle.m_tolerance >= 0);
    if (rCircle.m_radius < rCircle.m_tolerance || roiSize.area() == 0)
    {
        return {};
    }

    std::set<theta_rad> allSortedIntersections;
    std::vector<theta_rad> currentIntersections;


    //use semi open segment to avoid including end points twice should they lie on the circle (they could be slightly different due to double value precision)

    //corners in clockwise order, starting from bottom left
    geo2d::DPoint A (roiTopLeftCorner.x , roiTopLeftCorner.y + roiSize.height);
    geo2d::DPoint B (roiTopLeftCorner.x , roiTopLeftCorner.y);
    geo2d::DPoint C (roiTopLeftCorner.x + roiSize.width, roiTopLeftCorner.y);
    geo2d::DPoint D (roiTopLeftCorner.x + roiSize.width, roiTopLeftCorner.y + roiSize.height);

    //left rectangle side
    currentIntersections = rCircle.intersectWithVerticalSemiOpenSegment(A, B.y);
    for (auto & theta : currentIntersections)
    {
        allSortedIntersections.insert(theta);
    }

    //top rectangle side
    currentIntersections = rCircle.intersectWithHorizontalSemiOpenSegment(B, C.x);
    for (auto & theta : currentIntersections)
    {
        allSortedIntersections.insert(theta);
    }

    //right rectangle side
    currentIntersections = rCircle.intersectWithVerticalSemiOpenSegment(C, D.y);
    for (auto & theta : currentIntersections)
    {
        allSortedIntersections.insert(theta);
    }

    //bottom rectangle side
    currentIntersections = rCircle.intersectWithHorizontalSemiOpenSegment(D, A.x);
    for (auto & theta : currentIntersections)
    {
        allSortedIntersections.insert(theta);
    }

    if (allSortedIntersections.size() < 2 )
    {
        //no intersections or the circle is tangent to the roi: the circle is either completely inside or completely outside

        bool circleCompletelyInside = true;
        for (double i = 0; i <=2 ; i+= 0.5)
        {
            double theta = i*M_PI;
            circleCompletelyInside &= (isPointInROI(rCircle.getPoint(theta), roiTopLeftCorner, roiSize));
        }
        if (circleCompletelyInside)
        {
            return {{0.0, M_PI * 2}};
        }
        else
        {
            return {};
        }
    }

    std::vector<ArcBounds> results;

    // analyze the arcs between the intersections and decide which are inside the roi

    assert(allSortedIntersections.size() >= 2);

    //start with the arc between the last intersection and the first
    auto previousTheta = *(allSortedIntersections.rbegin()) - M_PI * 2;
    for (auto & currentTheta : allSortedIntersections)
    {
        theta_rad midAngle = (previousTheta + currentTheta) / 2.0;
        auto midPoint = rCircle.getPoint(midAngle);
        if (isPointInROI(midPoint, roiTopLeftCorner, roiSize))
        {
            results.push_back({previousTheta, currentTheta});
        }
        previousTheta = currentTheta;
    }
    return results;

}

void GenerateArcContour::clearOutPoints()
{
    m_oOutPoints.resize(1); //process only first element (contour)
    auto & rOutDPointarray = m_oOutPoints[0];
    rOutDPointarray.assign(0);
}

void GenerateArcContour::updateOutPoints(const ArcBounds & rArcBounds, int outRank)
{
    double rangeTheta = (rArcBounds.m_end - rArcBounds.m_start);

    m_oOutPoints.resize(1); //process only first element (contour)
    auto & rOutDPointarray = m_oOutPoints[0];
    auto & rContourData = rOutDPointarray.getData();
    auto & rContourRank = rOutDPointarray.getRank();

    rContourData.resize(m_numberOutputPoints);
    rContourRank.assign( m_numberOutputPoints, outRank);

    if (m_numberOutputPoints == 1)
    {
        theta_rad theta = rArcBounds.m_start + 0.5*rangeTheta;
        rContourData[0] = m_oInputCircle.getPoint(theta);
        return;
    }

    for (int i = 0; i < m_numberOutputPoints; i++)
    {
        theta_rad theta = rArcBounds.m_start + i * rangeTheta / (m_numberOutputPoints-1);
        rContourData[i] = m_oInputCircle.getPoint(theta);
    }
}


bool GenerateArcContour::isPointInROI ( geo2d::DPoint point, geo2d::DPoint roiTopLeftCorner, geo2d::Size roiSize )
{
    return ( roiTopLeftCorner.x <=  point.x && point.x <= roiTopLeftCorner.x + roiSize.width )
           && ( roiTopLeftCorner.y <=  point.y && point.y <= roiTopLeftCorner.y + roiSize.height );
}


geo2d::DPoint GenerateArcContour::Circle::getPoint(theta_rad theta) const
{
    //try to get a better precisions for known angles
    if (theta == 0.0 || theta == 2 * M_PI)
    {
        return {m_center.x + m_radius, m_center.y};
    }
    if (theta == M_PI_2)
    {
        return {m_center.x, m_center.y + m_radius};
    }
    if (theta == M_PI)
    {
        return {m_center.x - m_radius, m_center.y};
    }
    if (theta == - M_PI_2)
    {
        return {m_center.x, m_center.y - m_radius};
    }
    //for other angles, use trigonometric functions
    return {m_center.x + m_radius * std::cos(theta), m_center.y + m_radius * std::sin(theta) };
}


bool GenerateArcContour::Circle::isPointOnCircle(geo2d::DPoint point) const
{
    return math::isClose(geo2d::distance(point, m_center), m_radius, m_tolerance);
}


template<GenerateArcContour::Circle::SegmentPosition t>
std::vector< precitec::filter::GenerateArcContour::theta_rad > GenerateArcContour::Circle::intersectWithSemiOpenSegmentRelative(const double fixedCoordinate, const double variableCoordinateStart, const double variableCoordinateEnd ) const
{

    const double variableCoordinateMin = std::min( variableCoordinateStart,variableCoordinateEnd );
    const double variableCoordinateMax = std::max( variableCoordinateStart,variableCoordinateEnd );

    auto variableCoordinateBelongsToSemiOpenSegment = [&variableCoordinateMin, &variableCoordinateMax, &variableCoordinateStart, this] (double value)
    {
        return  math::isClose(value, variableCoordinateStart, m_tolerance)
                    || ( (variableCoordinateMin < value ) && (value < variableCoordinateMax ));
    };

    if (m_radius < m_tolerance)
    {
        // negative radius, or circle is just a point
        return {};
    }

    std::vector< precitec::filter::GenerateArcContour::theta_rad > result;

    double distFromCenter = std::abs(fixedCoordinate);

    if (math::isClose(distFromCenter, m_radius,m_tolerance))
    {
        // the line is tangent to the circle. since here we are working with a circle centered at 0,0 and axis-aligned segments, the intersection point lies on one of the axis
        if ( variableCoordinateBelongsToSemiOpenSegment (0.0) )
        {
            switch(t)
            {
                case SegmentPosition::horizontal:
                    {
                        //tangential horizontal segment: 90 deg or -90 deg
                        auto & y = fixedCoordinate;
                        theta_rad theta = (y > 0) ? M_PI_2 : - M_PI_2;
                        return {theta};
                    }
                    break;
                case SegmentPosition::vertical:
                    {
                        //tangential vertical segment: 0 deg or 180 deg
                        auto & x = fixedCoordinate;
                        theta_rad theta = (x > 0) ? 0.0 : M_PI;
                        return {theta};
                    }
                    break;
            }
            assert (false && "case not handled");
            return {};
        }
        else
        {
            // the tangent point does not belong to the segment
            return {};
        }
    }

    if ( distFromCenter > m_radius)
    {
        // completely outside, no intersections
        return {};
    }


    // if we got here the line intersects the circle in 2 points, check if they belong to the segment

    double intersectionAbsValue = std::sqrt( m_radius* m_radius - distFromCenter * distFromCenter);

    for ( double value : { intersectionAbsValue, -intersectionAbsValue})
    {
        if ( variableCoordinateBelongsToSemiOpenSegment(value))
        {
            double x,y;
            switch(t)
            {
                case SegmentPosition::horizontal:
                    x = value;
                    y = fixedCoordinate;
                    break;
                case SegmentPosition::vertical:
                    x = fixedCoordinate;
                    y = value;
                    break;
            }
            result.push_back(std::atan2(y,x));
        }
    }
    return result;
}


std::vector< precitec::filter::GenerateArcContour::theta_rad > GenerateArcContour::Circle::intersectWithHorizontalSemiOpenSegment(geo2d::DPoint startPoint, double xEnd) const
{
    auto result = intersectWithSemiOpenSegmentRelative<SegmentPosition::horizontal>(startPoint.y - m_center.y, startPoint.x - m_center.x, xEnd - m_center.x);

#ifndef NDEBUG
    {
        double xStart = startPoint.x;
        double xMin = std::min(xStart, xEnd) - m_tolerance;
        double xMax = std::max(xStart, xEnd) + m_tolerance;
        for (auto & theta : result)
        {
            auto point = getPoint(theta);
            assert(math::isClose(point.y, startPoint.y));
            assert(xMin < point.x && point.x < xMax);
            assert(isPointOnCircle(point));
        }
    }
#endif
    return result;
}


std::vector< precitec::filter::GenerateArcContour::theta_rad > GenerateArcContour::Circle::intersectWithVerticalSemiOpenSegment(geo2d::DPoint startPoint, double yEnd) const
{
    auto result = intersectWithSemiOpenSegmentRelative<SegmentPosition::vertical>(startPoint.x - m_center.x, startPoint.y - m_center.y, yEnd - m_center.y);

#ifndef NDEBUG
    {
        double yStart = startPoint.y;
        double yMin = std::min(yStart, yEnd) - m_tolerance;
        double yMax = std::max(yStart, yEnd) + m_tolerance;
        for (auto & theta : result)
        {
            auto point = getPoint(theta);
            assert(math::isClose(point.x, startPoint.x));
            assert (yMin < point.y && point.y < yMax);
            assert(isPointOnCircle(point));
        }
    }
#endif
    return result;
}


}
}

