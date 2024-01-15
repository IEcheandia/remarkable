#define _USE_MATH_DEFINES

#include "generateContour.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"

#include <fliplib/TypeToDataTypeImpl.h>

#include "math/mathCommon.h"


namespace precitec {
namespace filter {

using fliplib::Parameter;

GenerateContour::GenerateContour():
    TransformFilter("GenerateContour", Poco::UUID{"1105250c-b312-4c40-bd9d-8d230ff352ee"}),
    m_pPipeInDataA1(nullptr),
    m_pPipeInDataA2(nullptr),
    m_pPipeInDataB1(nullptr),               //Added!
    m_pPipeInDataB2(nullptr),               //Added!
    m_oPipeOutData(this,"Contour"),
    m_numberOutputPoints(3),
    m_minimumDistanceBetweenPoints(1e-6),
    m_InputType(InputType::SegmentExtremes ),
    m_radius(0)
    {
        parameters_.add("NumPoints", Parameter::TYPE_int, m_numberOutputPoints);

        setInPipeConnectors({{Poco::UUID("b7b63356-1cb5-4def-b21f-f20dd259762f"), m_pPipeInDataA1, "a1", 1, "a1"},
        {Poco::UUID("7eef8beb-4427-4e9e-9a73-d531e5abd4cf"), m_pPipeInDataA2, "a2", 1, "a2"},
        {Poco::UUID("58955c89-f814-4a77-90db-01dd4eca70bd"), m_pPipeInDataB1, "b1", 1, "b1"},
        {Poco::UUID("611de710-1b30-4f7f-b19f-834c34d276ea"), m_pPipeInDataB2, "b2", 1, "b2"}});
        setOutPipeConnectors({{Poco::UUID("ecaf2aed-2980-497e-92cb-5e3f782fce53"), &m_oPipeOutData, "Contour", 0, ""}});
        setVariantID(Poco::UUID("efcc499d-7d40-4075-846e-e0cbce8a2abf"));

        parameters_.add("InputType", Parameter::TYPE_int, static_cast<int>(m_InputType));
        parameters_.add("Radius", Parameter::TYPE_double, m_radius);
    }

void GenerateContour::setParameter()
{
    TransformFilter::setParameter();
    m_numberOutputPoints = parameters_.getParameter("NumPoints").convert<int>();
    m_InputType = static_cast<InputType>(parameters_.getParameter("InputType").convert<int>());
    m_radius = parameters_.getParameter("Radius").convert<double>();
}

bool GenerateContour::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
    if ( p_rPipe.tag() == "a1" )
    {
        m_pPipeInDataA1  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "a2" )
    {
        m_pPipeInDataA2  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "b1" )
    {
        m_pPipeInDataB1  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else if ( p_rPipe.tag() == "b2" )
    {
        m_pPipeInDataB2  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe


void GenerateContour::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
    using interface::GeoDoublearray;

    poco_assert_dbg(m_pPipeInDataA1 != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInDataA2 != nullptr); // to be asserted by graph editor

    const auto & rGeoDoubleArrayInA1 = m_pPipeInDataA1->read(m_oCounter);
    const auto & rGeoDoubleArrayInA2 = m_pPipeInDataA2->read(m_oCounter);
    const auto & rGeoDoubleArrayInB1 = m_pPipeInDataB1->read(m_oCounter);
    const auto & rGeoDoubleArrayInB2 = m_pPipeInDataB2->read(m_oCounter);

    const auto & rOutputContext = rGeoDoubleArrayInA1.context();
    m_oSpTrafo = rOutputContext.trafo();

    bool allValidInput = true;
    interface::ResultType oGeoAnalysisResult = rGeoDoubleArrayInA1.analysisResult();

    enum inputType {x,y};
    auto parseInputAsValue = [this, &allValidInput, &oGeoAnalysisResult](double &rOutValue, int &rOutRank, const GeoDoublearray & rGeoArray, const std::string & debugName)
    {
        oGeoAnalysisResult = std::max(oGeoAnalysisResult, rGeoArray.analysisResult());

        auto size = rGeoArray.ref().size();
        if ( size == 0)
        {
            allValidInput = false;
            rOutRank = 0;
            return ;
        }
        if (size > 1)
        {
            wmLog(eDebug, "Filter '%s': Received %u %s values. Can only process first element, rest will be discarded.\n", name().c_str(), size, debugName.c_str());
        };
        rOutRank = rGeoArray.ref().getRank().front();
        if (rOutRank != eRankMax)
        {
            wmLog(eDebug, "Filter '%s': Input %s has bad rank.\n", name().c_str(), debugName.c_str());
        }

        rOutValue = rGeoArray.ref().getData().front();
    };

    auto parseInputAsPoint = [this, &allValidInput, &oGeoAnalysisResult](geo2d::DPoint &rOutPoint, int &rOutRank,  const GeoDoublearray & rGeoArray, const std::string & debugName, inputType t)
    {
        oGeoAnalysisResult = std::max(oGeoAnalysisResult, rGeoArray.analysisResult());

        auto size = rGeoArray.ref().size();
        if ( size == 0)
        {
            allValidInput = false;
            rOutRank = 0;
            switch(t)
            {
                case inputType::x: rOutPoint.x = 0; break;
                case inputType::y: rOutPoint.y = 0; break;
            }
            return ;
        }
        if (size > 1)
        {
            wmLog(eDebug, "Filter '%s': Received %u %s values. Can only process first element, rest will be discarded.\n", name().c_str(), size, debugName.c_str());
        };
        rOutRank = rGeoArray.ref().getRank().front();
        if (rOutRank != eRankMax)
        {
            wmLog(eDebug, "Filter '%s': Input %s has bad rank.\n", name().c_str(), debugName.c_str());
        }

        double valueIn = rGeoArray.ref().getData().front();
        switch(t)
        {
            case inputType::x: rOutPoint.x = valueIn + rGeoArray.context().trafo()->dx() - m_oSpTrafo->dx(); break;
            case inputType::y: rOutPoint.y = valueIn + rGeoArray.context().trafo()->dy() - m_oSpTrafo->dy(); break;
        }
    };

    auto getPoint = [&parseInputAsPoint]
        (const GeoDoublearray & rGeoArrayX, const std::string & debugNameX,
        const GeoDoublearray & rGeoArrayY, const std::string & debugNameY)
    {
        //fCheckInput use the minimum between the input and the existing rank
        geo2d::DPoint point;
        int rankX,rankY;
        parseInputAsPoint (point, rankX, rGeoArrayX, debugNameX, inputType::x);
        parseInputAsPoint (point, rankY, rGeoArrayY, debugNameY, inputType::y);
        return std::tuple<geo2d::DPoint,int>{point,std::min(rankX, rankY)};
    };


    m_oOutPoints.resize(1); //process only first element
    auto & rOutDPointarray = m_oOutPoints[0];

    switch (m_InputType)
    {
        case InputType::SegmentExtremes:
        {
            auto start = getPoint (rGeoDoubleArrayInA1, "A1", rGeoDoubleArrayInA2, "A2");
            auto end = getPoint (rGeoDoubleArrayInB1, "B1", rGeoDoubleArrayInB2, "B2");
            generateSegment(rOutDPointarray, start, end);
            break;
        }
        case InputType::ArcPolarCoordinates:
        {
            auto center = getPoint (rGeoDoubleArrayInA1, "A1", rGeoDoubleArrayInA2, "A2");
            ArcWithFixedRadius arc {std::get<0>(center).x, std::get<0>(center).y};
            int rankStart(0), rankEnd(0);
            parseInputAsValue (arc.theta0, rankStart, rGeoDoubleArrayInB1, "thetaStart");
            parseInputAsValue (arc.theta1, rankEnd, rGeoDoubleArrayInB2, "thetaEnd");
            generateArc(rOutDPointarray, arc, std::get<1>(center), rankStart, rankEnd);
            break;
        }
        case InputType::ArcTangentHorizontal:
        {
            geo2d::DPoint pointTangent, approximatedEnd;
            int rankTangent, rankEnd;
            std::tie(pointTangent, rankTangent) = getPoint (rGeoDoubleArrayInA1, "A1", rGeoDoubleArrayInA2, "A2");
            std::tie(approximatedEnd, rankEnd) = getPoint (rGeoDoubleArrayInB1, "B1", rGeoDoubleArrayInB2, "B2");
            auto arc = getArcFromHorizontalTangent(pointTangent, approximatedEnd);
            generateArc(rOutDPointarray,arc, rankTangent, rankTangent, rankEnd);
            break;
        }
        case InputType::ArcTangentVertical:
        {
            geo2d::DPoint pointTangent, approximatedEnd;
            int rankTangent, rankEnd;
            std::tie(pointTangent, rankTangent) = getPoint (rGeoDoubleArrayInA1, "A1", rGeoDoubleArrayInA2, "A2");
            std::tie(approximatedEnd, rankEnd) = getPoint (rGeoDoubleArrayInB1, "B1", rGeoDoubleArrayInB2, "B2");
            auto arc = getArcFromVerticalTangent(pointTangent, approximatedEnd);
            generateArc(rOutDPointarray,arc, rankTangent, rankTangent, rankEnd);
            break;
        }
    }
    const interface::GeoVecAnnotatedDPointarray oGeoOut( rOutputContext, m_oOutPoints, oGeoAnalysisResult, 1.0 );

    preSignalAction();
    m_oPipeOutData.signal(oGeoOut);

}

void GenerateContour::paint()
{
    using namespace precitec::image;

    if (m_oVerbosity < VerbosityType::eMax)
    {
        return;
    }

    if (m_oSpTrafo.isNull() || m_oOutPoints.size() == 0)
    {
        return;
    }

    OverlayCanvas	&rCanvas ( canvas<OverlayCanvas>(m_oCounter) );
    OverlayLayer	&rLayerPosition ( rCanvas.getLayerPosition());

    auto oColor = Color::Magenta();

    for (auto & rOutDPointarray : m_oOutPoints)
    {
        auto & rData = rOutDPointarray.getData();
        auto & rRank = rOutDPointarray.getRank();
        for (int i = 0, n = rOutDPointarray.size(); i < n; i++)
        {
            auto & rPoint = rData[i];
            auto oCanvasPoint = m_oSpTrafo->apply(geo2d::Point( (int)std::round(rPoint.x), (int)std::round(rPoint.y)));

            if (rRank[i] == eRankMax)
            {
                rLayerPosition.add<OverlayCross>(oCanvasPoint.x, oCanvasPoint.y, 3, oColor);
            }
            else
            {
                rLayerPosition.add<OverlayCircle>(oCanvasPoint.x, oCanvasPoint.y, 3, oColor);
            }

        }
    }
}

void GenerateContour::generateSegment(geo2d::AnnotatedDPointarray & rOutDPointarray, std::tuple<geo2d::DPoint,int> start, std::tuple<geo2d::DPoint,int> end)
{
    auto & rData = rOutDPointarray.getData();
    auto & rRank = rOutDPointarray.getRank();

    auto & pointA = std::get<0>(start);
    auto & rankA = std::get<1>(start);
    auto & pointB = std::get<0>(end);
    auto & rankB = std::get<1>(end);

    bool interpolate = (m_numberOutputPoints > 2) &&
                (rankA == eRankMax && rankB == eRankMax);
    if (interpolate)
    {
        assert(rankA == eRankMax && rankB == eRankMax);
        rOutDPointarray.assign(m_numberOutputPoints);
        std::fill(rRank.begin(),rRank.end(),eRankMax);


        double rangeX = pointB.x - pointA.x;
        double rangeY = pointB.y - pointA.y;
        int n = m_numberOutputPoints -2; //number interpolated points

        int index = 0;
        rData[index] = pointA;
        index =1;
        double deltaX = rangeX /double(n+1);
        double deltaY = rangeY /double(n+1);
        if (std::abs(rangeX) > m_minimumDistanceBetweenPoints || std::abs(rangeY) > m_minimumDistanceBetweenPoints )
        {
            for (; index <=n ; ++index)
            {
                rData[index].x = pointA.x + index * deltaX;
                rData[index].y = pointA.y + index * deltaY;
            }
        }
        else
        {
            //points almost overlapping, do not interpolate
            rOutDPointarray.assign(2);
            rData[0] = pointA;
            rRank[0] = rankA;
        }
        assert(index == (int)(rData.size())-1);
        rData[index] = pointB;
    }
    else
    {
        rOutDPointarray.assign(2);
        rData[0] = pointA;
        rData[1] = pointB;
        rRank[0] = rankA;
        rRank[1] = rankB;
    }
}

void GenerateContour::generateArc(geo2d::AnnotatedDPointarray & rOutDPointarray, GenerateContour::ArcWithFixedRadius arc, int rankCenter, int rankStart, int rankEnd )
{

    auto & rData = rOutDPointarray.getData();
    auto & rRank = rOutDPointarray.getRank();

    auto & thetaStart = arc.theta0;
    auto & thetaEnd = arc.theta1;

    bool interpolate = (m_numberOutputPoints > 2) &&
                (rankCenter == eRankMax && rankStart == eRankMax && rankEnd == eRankMax);

    auto polarToDPoint = [this, & arc](double theta)
    {
        return geo2d::DPoint{ arc.centerX + std::cos(theta) * m_radius,
                              arc.centerY + std::sin(theta) * m_radius };
    };

    if (interpolate)
    {
        rOutDPointarray.assign(m_numberOutputPoints);
        std::fill(rRank.begin(),rRank.end(),eRankMax);


        double rangeTheta = thetaEnd - thetaStart;

        int n = m_numberOutputPoints -2; //number interpolated points

        int index = 0;
        rData[index] = polarToDPoint (thetaStart);
        index =1;
        double deltaTheta = rangeTheta /double(n+1);

        if ( std::abs(m_radius * rangeTheta) > m_minimumDistanceBetweenPoints)
        {
            for (; index <=n ; ++index)
            {
                rData[index] = polarToDPoint (thetaStart + index * deltaTheta);
            }
        }
        else
        {
            //points almost overlapping, do not interpolate
            rOutDPointarray.assign(2);
            rData[0] = polarToDPoint (thetaStart);
            rRank[0] = rankStart;
        }
        assert(index == (int)(rData.size())-1);
        rData[index] = polarToDPoint (thetaEnd);

    }
    else
    {
        rOutDPointarray.assign(2);
        rData[0] = polarToDPoint (thetaStart);
        rData[1] = polarToDPoint (thetaEnd);
        rRank[0] = rankStart;
        rRank[1] = rankEnd;
    }

}


GenerateContour::ArcWithFixedRadius GenerateContour::getArcFromHorizontalTangent ( geo2d::DPoint start, geo2d::DPoint approximateEnd) const
{

    //the end point it's used to find the arc direction and bounding y, it does not need to lie on the circle defined by m_radius

    geo2d::DPoint pointCenter{ start.x,  approximateEnd.y > start.y ? start.y + m_radius : start.y - m_radius};

    if (std::abs(approximateEnd.y - start.y) > m_radius || m_radius < 1e-16)
    {
        wmLog(eDebug,"GenerateContour::getArcFromHorizontalTangent: end point(%f,%f) outside of circle (%f,%f, r=%f) or arc > 90 deg \n",
            approximateEnd.x, approximateEnd.y, pointCenter.x, pointCenter.y, m_radius);
        double fallbackTheta = (approximateEnd.y > start.y) ? M_PI_2 : -M_PI_2;
        return ArcWithFixedRadius{pointCenter.x, pointCenter.y, fallbackTheta, fallbackTheta};
    }

    auto delta = approximateEnd - pointCenter;
    double candidateTheta = std::asin(delta.y / m_radius);

    //check the quadrant to choose the shortest arc
    double thetaStart(0), thetaEnd(0);
    switch (2*(delta.y >= 0) + (delta.x >= 0))
    {
        case 2*1+1: //quadrant I
            assert(delta.y >= 0 && delta.x >= 0);
            thetaStart = M_PI_2;
            thetaEnd = candidateTheta;
            break;
        case 2*1+0: //quadrant II
            assert(delta.y >= 0 && delta.x < 0);
            thetaStart = M_PI_2;
            thetaEnd = M_PI - candidateTheta;
            break;
        case 2*0+0: //quadrant III
            assert(delta.y < 0 && delta.x < 0 && candidateTheta < 0);
            thetaStart = - M_PI_2;
            thetaEnd = - M_PI - candidateTheta ;
            break;
        case 2*0+1: //quadrant IV
            assert(delta.y < 0 && delta.x >= 0 && candidateTheta < 0);
            thetaStart = - M_PI_2;
            thetaEnd =  candidateTheta;
            break;
        default:
            assert(false);
            break;
    }
    return {pointCenter.x, pointCenter.y, thetaStart, thetaEnd};
}

GenerateContour::ArcWithFixedRadius GenerateContour::getArcFromVerticalTangent( geo2d::DPoint start, geo2d::DPoint approximateEnd) const
{

    //the end point it's used to find the arc direction and bounding x, it does not need to lie on the circle defined by m_radius
    geo2d::DPoint pointCenter{ approximateEnd.x > start.x ? start.x + m_radius : start.x - m_radius, start.y};

    if (std::abs(approximateEnd.x - start.x) > m_radius || m_radius < 1e-16)
    {
        wmLog(eDebug,"GenerateContour::getArcFromVerticalTangent: end point(%f,%f) outside of circle (%f,%f, r=%f) or arc > 90 deg \n",
            approximateEnd.x, approximateEnd.y, pointCenter.x, pointCenter.y, m_radius);
        double fallbackTheta = (approximateEnd.x > start.x) ? 0 : M_PI;
        return ArcWithFixedRadius{pointCenter.x, pointCenter.y, fallbackTheta, fallbackTheta};
    }


    auto delta = approximateEnd - pointCenter;

    double candidateTheta = std::acos(delta.x/m_radius);

    //check the quadrant to choose the shortest arc
    double thetaStart(0), thetaEnd(0);
    switch (2*(delta.y > 0) + (delta.x > 0))
    {
        case 2*1+1:  //quadrant I
            assert(delta.y > 0 && delta.x > 0);
            thetaStart = 0;
            thetaEnd = candidateTheta;
            break;
        case 2*1+0: //quadrant II
            assert(delta.y  > 0 && delta.x <= 0 && candidateTheta >= M_PI_2);
            thetaStart = M_PI;
            thetaEnd = candidateTheta;
            break;
        case 2*0+0: //quadrant III
            assert(delta.y  < 0 && delta.x <= 0 && candidateTheta >= M_PI_2);
            thetaStart = M_PI;
            thetaEnd =  2* M_PI - candidateTheta;
            break;
        case 2*0+1: //quadrant IV
            assert(delta.y  <= 0 && delta.x > 0);
            thetaStart = 0;
            thetaEnd =  - candidateTheta;
            break;
        default:
            assert(false);
            break;
    }

    return {pointCenter.x, pointCenter.y, thetaStart, thetaEnd};
}
}
}
