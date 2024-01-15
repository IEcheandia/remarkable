#include "intersect2Lines.h"
#include "math/mathCommon.h"
#include <fliplib/TypeToDataTypeImpl.h>

#include <module/moduleLogger.h>
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "util/calibDataSingleton.h"

namespace precitec
{
namespace filter
{
    using fliplib::Parameter;
    using math::angleUnit;
    using math::radiansToDegrees;
    using math::LineEquation;
    using filter::LaserLine;
    using image::Color;
    using image::Font;
    using image::OverlayCanvas;
    using image::OverlayLayer;
    using image::OverlayLine;
    using image::OverlayCross;
    using image::OverlayText;
    using image::OverlayCircle;

    const std::string Intersect2Lines::m_oFilterName = "Intersect2Lines";
    const std::string Intersect2Lines::m_oPipeOutIntersectionResultName = "IntersectionPresent";
    const std::string Intersect2Lines::m_oPipeOutXName = "IntersectionPositionX";
    const std::string Intersect2Lines::m_oPipeOutYName = "IntersectionPositionY";
    const std::string Intersect2Lines::m_oPipeOutAngleName = "AngleBetweenLines";

    Intersect2Lines::Intersect2Lines() :
        TransformFilter(Intersect2Lines::m_oFilterName, Poco::UUID{"dfbafa58-a67c-43a0-bd56-461f1ad178d2"}),
        m_pPipeInImage(nullptr),
        m_pPipeInFirstLine(nullptr),
        m_pPipeInSecondLine(nullptr),
        m_oPipeOutIntersectionResult(this, Intersect2Lines::m_oPipeOutIntersectionResultName),
        m_oPipeOutX(this, Intersect2Lines::m_oPipeOutXName),
        m_oPipeOutY(this, Intersect2Lines::m_oPipeOutYName),
        m_oPipeOutAngle(this, Intersect2Lines::m_oPipeOutAngleName),
        m_oValidOnlyIfInRoi(false),
        m_oAngleOnScreenCoordinates(true),
        m_oTypeOfLaserLine(filter::LaserLine::FrontLaserLine)
    {
        parameters_.add("ValidOnlyIfInRoi", Parameter::TYPE_bool, m_oValidOnlyIfInRoi);
        int oPlaneForAngleComputation = static_cast<int> (PlaneForAngleComputation::Screen);
        if ( !m_oAngleOnScreenCoordinates )
        {
            static_assert((int) PlaneForAngleComputation::LineLaser1 == (int)LaserLine::FrontLaserLine, "");
            static_assert((int) PlaneForAngleComputation::LineLaser2 == (int) LaserLine::BehindLaserLine, "");
            static_assert((int) PlaneForAngleComputation::LineLaser3 == (int) LaserLine::CenterLaserLine, "");
            oPlaneForAngleComputation = (int)(m_oTypeOfLaserLine); //they are equivalent by construction
        }
        parameters_.add("PlaneForAngleComputation", Parameter::TYPE_int, oPlaneForAngleComputation);

        setInPipeConnectors({{Poco::UUID("792170d0-1b17-4d43-ad65-4ab2e88cd05a"), m_pPipeInImage, "ImageFrame", 1, "ImageFrame"}, {Poco::UUID("4cbdfbc3-9c4c-4a5f-8c54-2deece152293"), m_pPipeInFirstLine, "FirstLine", 1, "FirstLine"},
        {Poco::UUID("dde0dc19-404f-4a4c-801b-d83ab1681080"), m_pPipeInSecondLine, "SecondLine", 1, "SecondLine"}});
        setOutPipeConnectors({{Poco::UUID("2823e2fe-db03-43a0-bb8f-76a50c702bc1"), &m_oPipeOutIntersectionResult, m_oPipeOutIntersectionResultName, 0, ""},
        {Poco::UUID("bfb07af0-8498-4ef2-acbd-e7c8f78fa3ac"), &m_oPipeOutX, m_oPipeOutXName, 0, ""},
        {Poco::UUID("e1404b69-3f1d-47ef-b3c2-4b7c2492c881"), &m_oPipeOutY, m_oPipeOutYName, 0, ""},
        {Poco::UUID("2721fc75-360f-43c0-b97e-bae4ac9a2b37"), &m_oPipeOutAngle, m_oPipeOutAngleName, 0, ""}});
        setVariantID(Poco::UUID("ab86943d-0768-4dc5-9f3c-dd330302aff3"));
    }

    Intersect2Lines::~Intersect2Lines()
    {

    }

    void Intersect2Lines::setParameter()
    {
        TransformFilter::setParameter();
        m_oValidOnlyIfInRoi = parameters_.getParameter("ValidOnlyIfInRoi");
        int oPlaneForAngleComputation = parameters_.getParameter("PlaneForAngleComputation");

        if ( oPlaneForAngleComputation < 0 || oPlaneForAngleComputation >= static_cast<int> (PlaneForAngleComputation::Screen) )
        {
            m_oAngleOnScreenCoordinates = true;
        }
        else
        {
            m_oAngleOnScreenCoordinates = false;
            m_oTypeOfLaserLine = static_cast<filter::LaserLine>(oPlaneForAngleComputation);
        }

    }

    bool Intersect2Lines::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
    {
        if ( p_rPipe.tag() == "ImageFrame" )
        {
            m_pPipeInImage = dynamic_cast<fliplib::SynchronePipe < interface::ImageFrame > *>(&p_rPipe);
        }
        if ( p_rPipe.tag() == "FirstLine" )
        {
            m_pPipeInFirstLine = dynamic_cast<fliplib::SynchronePipe < interface::GeoLineModelarray > *>(&p_rPipe);
        }
        if ( p_rPipe.tag() == "SecondLine" )
        {
            m_pPipeInSecondLine = dynamic_cast<fliplib::SynchronePipe < interface::GeoLineModelarray > *>(&p_rPipe);
        }
        return BaseFilter::subscribe(p_rPipe, p_oGroup);
    }

    Intersect2Lines::LineSegment Intersect2Lines::parseLineInput(const interface::GeoLineModelarray & rGeoLineModel, int lineNumber) const
    {
        auto oLineModel = rGeoLineModel.ref().getData()[lineNumber];
        auto oInputTrafo = rGeoLineModel.context().trafo();

        double a, b, c;
        oLineModel.getCoefficients(a, b, c);

        LineSegment result;
        result.m_oLine = math::LineEquation(a,b,c);
        result.m_oPoint = oLineModel.getCenter();
        assert(math::isClose(result.m_oLine.distance(result.m_oPoint.x, result.m_oPoint.y), 0.0));

        //convert to the output context
        double dx = oInputTrafo->dx() - this->m_oSpTrafo->dx();
        double dy = oInputTrafo->dy() - this->m_oSpTrafo->dy();
        result.m_oLine.applyTranslation(dx, dy);
        result.m_oPoint.x += dx;
        result.m_oPoint.y += dy;

#ifndef NDEBUG
        assert(math::isClose(result.m_oLine.distance(result.m_oPoint.x, result.m_oPoint.y), 0.0) && "the corrected center of the segment does not belong to the line anymore");
        geo2d::DPoint oInputPoint(oLineModel.getCenter());
        geo2d::Point oOutputPoint(std::round(result.m_oPoint.x), std::round(result.m_oPoint.y));
        auto oOutputPointInReferenceTrafo = m_oSpTrafo->apply(oOutputPoint);
        auto oOutputPointToInputTrafo = oInputTrafo->applyReverse(m_oSpTrafo->apply(oOutputPoint));
		double tol = 0.5 + 1e-6;
		if ( oInputPoint.x <0.5 || oInputPoint.y <0.5 )
		{
			//for negative numbers, cast to int and round have different behaviours
			tol = 1 + 1e-6;
		}

		assert(math::isClose<double>(oOutputPointToInputTrafo.x, oInputPoint.x, tol)
			&& math::isClose<double>(oOutputPointToInputTrafo.y, oInputPoint.y, tol));

#endif
        return result;

    }

    Intersect2Lines::LineSegment Intersect2Lines::convertScreenLineToLaserPlaneLine(math::LineEquation p_oLine, geo2d::DPoint p_oCenter,
        geo2d::Point oSensorVertexUpperLeft, geo2d::Point oSensorVertexLowerRight, const interface::ImageContext & rContext) const
    {
        assert(p_oLine.isValid() && "computeIntersection didn't return with invalid output");
        assert(math::isClose(p_oLine.distance(p_oCenter.x, p_oCenter.y), 0.0) && "invalid input - point does not belong to line");

        math::SensorId oSensorId(math::SensorId::eSensorId0);
        auto &rCalibCoords = system::CalibDataSingleton::getCalibrationCoords(oSensorId);
        auto pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(oSensorId,rContext, m_oTypeOfLaserLine);

        //convert input from screen coordinates to sensor coordinate
        geo2d::Point offset = pCoordTransformer->getSensorPoint(0, 0);

        //input is passed by value, we modify it directly
        p_oLine.applyTranslation(offset.x, offset.y);
        p_oCenter.x += offset.x;
        p_oCenter.y += offset.y;
        assert(math::isClose(p_oLine.distance(p_oCenter.x, p_oCenter.y), 0.0) && "invalid conversion to sensor coordinates");

        //choose 2 points A,B that define the line [screen coordinates]
        auto oPoints = p_oLine.intersectRectangle(oSensorVertexUpperLeft.x, oSensorVertexLowerRight.x, oSensorVertexUpperLeft.y, oSensorVertexLowerRight.y);
        if ( oPoints.size() != 2 )
        {
            assert(false);
            return {math::LineEquation(), geo2d::DPoint(0, 0)};
        }

        //add trafo and HWROI to the points A,B [sensor coordintes]
        const auto & rSensorPointA = oPoints[0];
        const auto & rSensorPointB = oPoints[1];

        //build a list of coordinates to fit, in order to get a line
        std::vector<double> oX;
        std::vector<double> oY;

    #ifndef NDEBUG
        std::vector<double> oSensorCoordinateX, oSensorCoordinateY;
    #endif

        if (rCalibCoords.isScheimpflugCase())
        {

        //compute coordinates of the points A,B on the laser plane [mm, 2D]
            float xa, ya, xb, yb;
            bool valid = rCalibCoords.convertScreenToLaserPlane(xa, ya, rSensorPointA[0], rSensorPointA[1], m_oTypeOfLaserLine);
            valid &= rCalibCoords.convertScreenToLaserPlane(xb, yb, rSensorPointB[0], rSensorPointB[1], m_oTypeOfLaserLine);
            if ( !valid )
            {
                assert(false);
                return {math::LineEquation(), geo2d::DPoint(0, 0)};
            }

            double n = std::abs(rSensorPointB[0] - rSensorPointA[0]);
            for ( int i = 0; i < n; ++i )
            {
                auto xSensor = rSensorPointA[0] + i * (rSensorPointB[0] - rSensorPointA[0]) / n;
                auto ySensor = p_oLine.getY(xSensor);
                if ( std::isnan(ySensor) )
                {
                    continue;
                }
                float x,y;
                bool valid = rCalibCoords.convertScreenToLaserPlane(x, y, xSensor, std::round(ySensor), m_oTypeOfLaserLine);
                if ( valid )
                {
    #ifndef NDEBUG
                    oSensorCoordinateX.push_back(std::round(xSensor));
                    oSensorCoordinateY.push_back(std::round(ySensor));
    #endif
                    oX.push_back(x);
                    oY.push_back(y);
                }
            }


            n = std::abs(rSensorPointB[1] - rSensorPointA[1]);
            for ( int i = 0; i < n; ++i )
            {
                auto ySensor = rSensorPointA[1] + i * (rSensorPointB[1] - rSensorPointA[1]) / n;
                auto xSensor = p_oLine.getX(ySensor);
                if ( std::isnan(xSensor) )
                {
                    continue;
                }
                float x, y;
                bool valid = rCalibCoords.convertScreenToLaserPlane(x, y, std::round(xSensor), ySensor, m_oTypeOfLaserLine);
                if ( valid )
                {
    #ifndef NDEBUG
                    oSensorCoordinateX.push_back(std::round(xSensor));
                    oSensorCoordinateY.push_back(std::round(ySensor));
    #endif
                    oX.push_back(x);
                    oY.push_back(y);
                }
            }
        } //end Scheimpflug
        else
        {
            //coax case, we can have a better precision if we use derive the coordinates from the formula
            const auto & rCalib = system::CalibDataSingleton::getCalibrationData(oSensorId);
            auto oTriangAngle = rCalibCoords.getTriangulationAngle(math::angleUnit::eRadians, m_oTypeOfLaserLine);

            auto oCoaxData = rCalib.getCoaxCalibrationData();

            auto fConvertSensorCoordinateToLaserPlaneCoax = [&oCoaxData, &oTriangAngle, &rCalibCoords] (double xSensor, double ySensor)
            {
                //similar to Calibration3DCoords::convertScreenToLaserPlane, but supporting subpixels

                //compute coordinate in the horizontal plane
                const double oRatioHorizontal_mm_pix = oCoaxData.m_oDpixX / oCoaxData.m_oBeta0;
                const double oRatioVertical_mm_pix = oCoaxData.m_oDpixY / oCoaxData.m_oBeta0;
                double xHorizontalPlane = (xSensor - oCoaxData.m_oOrigX)*oRatioHorizontal_mm_pix;
                double yHorizontalPlane = -(ySensor - oCoaxData.m_oOrigY)*oRatioVertical_mm_pix;

                //compute corresponding coordinates on the laser plane (NOT 3D space)
                double xLaserPlane = xHorizontalPlane;
                double yLaserPlane = yHorizontalPlane / std::sin(oTriangAngle);
#ifndef NDEBUG

                double oPixelDiagonal_mm = std::sqrt((oRatioHorizontal_mm_pix*oRatioHorizontal_mm_pix + oRatioVertical_mm_pix*oRatioVertical_mm_pix));
                double d = rCalibCoords.distFrom2D(std::round(xSensor), std::round(ySensor), oCoaxData.m_oOrigX, oCoaxData.m_oOrigY);
                if (!math::isClose(d, std::sqrt(yLaserPlane*yLaserPlane + xLaserPlane*xLaserPlane), 0.5* oPixelDiagonal_mm))
                {
                    wmLog(eError, "Error converting coax coordinates y laser plane %f dist 2d pixel diag %f %f\n", yLaserPlane, d, oPixelDiagonal_mm );
                }
#endif
                return std::make_pair(xLaserPlane, yLaserPlane);
            };

        double xa, ya, xb, yb;
        std::tie(xa, ya) = fConvertSensorCoordinateToLaserPlaneCoax(rSensorPointA[0], rSensorPointA[1]);
        std::tie(xb, yb) = fConvertSensorCoordinateToLaserPlaneCoax(rSensorPointB[0], rSensorPointB[1]);

        //build a list of coordinates to fit, in order to get a line
            oX = {xa, xb};
            oY = {ya, yb};

        } //end coax

        //convert the segment center
        float xc, yc;
        bool valid = rCalibCoords.convertScreenToLaserPlane(xc, yc, p_oCenter.x, p_oCenter.y, m_oTypeOfLaserLine);
        if ( !valid )
        {
            wmLog(eWarning, "Could not convert segment center to laser plane %d %d \n", p_oCenter.x, p_oCenter.y);
            xc = oX.front();
            yc = oY.front();
        }

#ifndef NDEBUG

        math::LineEquation oOutLine(oX, oY);
        assert(oOutLine.isValid());

        double oRatioHorizontal_mm_pix = 1 / rCalibCoords.factorHorizontal(5, xc, yc);
        double oRatioVertical_mm_pix = 1 / rCalibCoords.factorVertical(5, xc, yc, m_oTypeOfLaserLine);
        double oPixelDiagonal_mm = std::sqrt((oRatioHorizontal_mm_pix*oRatioHorizontal_mm_pix + oRatioVertical_mm_pix*oRatioVertical_mm_pix));
        double oPixelDiagonal_pix = std::sqrt(2);

        auto d = oOutLine.distance(xc, yc);
        if ( !math::isClose(d, 0.0) )
        {
            std::cout << p_oLine << " -> " << oOutLine << std::endl;
            std::cout << p_oCenter << " -> " << xc << " " << yc << " distance " << d << std::endl;
            assert(d <= oPixelDiagonal_mm);
        }

        if (oSensorCoordinateX.size() > 0)
        {

            math::LineEquation oCorrespondentSensorLine(oSensorCoordinateX, oSensorCoordinateY);

            double m1 = p_oLine.getInclinationDegrees();
            double m2 = oCorrespondentSensorLine.getInclinationDegrees();
            assert(math::isClose(m1,m2,0.02));
            bool verticalLine = math::isClose(std::abs(m1), 90.0, 5.0);
            auto lineIntercept = verticalLine ? p_oLine.getX(0) : p_oLine.getY(0);
            auto correspondentSensorLineIntercept = verticalLine ? oCorrespondentSensorLine.getX(0) : oCorrespondentSensorLine.getY(0);
            if (!math::isClose(lineIntercept, correspondentSensorLineIntercept, oPixelDiagonal_pix / 2))
            {
                assert(false && "error in converting sensor line");
            }
        }
#endif

        return {math::LineEquation(oX, oY), geo2d::DPoint(xc, yc)};

    }


    std::optional<Intersect2Lines::LinesIntersection> Intersect2Lines::updateIntersection(const LineSegment & rLineSegment1, const LineSegment & rLineSegment2, const geo2d::Size & rImageSize, const interface::ImageContext & rContext) const
    {
        LinesIntersection result;

        result.m_oX = 0;
        result.m_oY = 0;
        if ( !rLineSegment1.m_oLine.isValid() || !rLineSegment1.m_oLine.isValid() )
        {
            result.m_oLine1ForAngleComputation = {};
            result.m_oLine2ForAngleComputation = {};
            result.m_ohasIntersection = false;
            result.m_oAngle = 0;
            return {};
        }

        result.m_ohasIntersection = rLineSegment1.m_oLine.intersect(result.m_oX, result.m_oY, rLineSegment2.m_oLine);
        if ( !result.m_ohasIntersection )
        {
            wmLog(eDebug, "Intersection not found\n");
            //the lines are parallel, if does not make sense to continue computing the angle
            result.m_oAngle = 0.0;
            // return true because we had valid data and the computation has succeded (we are not returning m_ohasIntersection)
            return {result};
        }

            if ( m_oValidOnlyIfInRoi )
            {
                if ( result.m_oX < 0
                    || result.m_oX >= rImageSize.width
                    || result.m_oY < 0
                    || result.m_oY >= rImageSize.height )
                {
                    result.m_ohasIntersection = false;
                }
            }

        wmLog(eDebug, "Intersection at %f %f\n", result.m_oX, result.m_oY);

        //compute angle between lines. Decide which one based on the center point of the input segments

        if ( m_oAngleOnScreenCoordinates )
        {
            result.m_oAngle = computeAngleAtIntersection(rLineSegment1, rLineSegment2, result.m_oX);
            //needed for paint
            result.m_oLine1ForAngleComputation = rLineSegment1;
            result.m_oLine2ForAngleComputation = rLineSegment2;
            return {result};
        }

        assert(!m_oAngleOnScreenCoordinates);
        //recompute the lines on the laser plane

        //we assume no radial distortion, we just transform 2 points on the laser plane and recompute the line equations

        auto &rCalibCoords(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));


        //choose 2 points that define the line (set a border to be sure that calibration is well defined)
        geo2d::Point oScreenVertexUpperLeft, oScreenVertexLowerRight;
        {
            const int border = rCalibCoords.isScheimpflugCase() ? 100 : 0;
            auto oSensorSize = rCalibCoords.getSensorSize();
            //convert from sensorCoords to the same reference system as m_oLine (defined in  parseLineInput)
            oScreenVertexUpperLeft = geo2d::Point(border, border);
            oScreenVertexLowerRight = geo2d::Point(oSensorSize.width - 1 - border, oSensorSize.height - 1 - border);
        }


        //change reference system  of LineEquation to sensor [0-1024, 0-1024]
        result.m_oLine1ForAngleComputation = convertScreenLineToLaserPlaneLine(rLineSegment1.m_oLine, rLineSegment1.m_oPoint, oScreenVertexUpperLeft, oScreenVertexLowerRight, rContext);

        geo2d::DPoint oPoint2ForAngleComputation;
        result.m_oLine2ForAngleComputation = convertScreenLineToLaserPlaneLine(rLineSegment2.m_oLine, rLineSegment2.m_oPoint, oScreenVertexUpperLeft, oScreenVertexLowerRight, rContext);

        geo2d::DPoint oIntersectionOnPlane;
        bool oHasIntersection = result.m_oLine1ForAngleComputation.m_oLine.intersect(oIntersectionOnPlane.x, oIntersectionOnPlane.y, result.m_oLine2ForAngleComputation.m_oLine);

#ifndef NDEBUG
        if ( oHasIntersection )
        {
            assert(math::isClose(result.m_oLine1ForAngleComputation.m_oLine.distance(oIntersectionOnPlane.x, oIntersectionOnPlane.y), 0.0));
            assert(math::isClose(result.m_oLine2ForAngleComputation.m_oLine.distance(oIntersectionOnPlane.x, oIntersectionOnPlane.y), 0.0));

            double offsetX = m_oSpTrafo->dx() + rContext.HW_ROI_x0;
            double offsetY = m_oSpTrafo->dy() + rContext.HW_ROI_y0;

            float xc2d, yc2d;

            int sensorX = std::round (result.m_oX + offsetX);
            int sensorY = std::round (result.m_oY + offsetY);
            bool calibDefined = rCalibCoords.convertScreenToLaserPlane(xc2d, yc2d, sensorX, sensorY, m_oTypeOfLaserLine);
            if (calibDefined)
            {
                //tolerance: average pixel dimension
                double oTriangAngle =  rCalibCoords.getTriangulationAngle(angleUnit::eRadians, m_oTypeOfLaserLine);
                double tol_x = 1 / rCalibCoords.factorHorizontal(5, sensorX, sensorY);
                double tol_y = (1 / rCalibCoords.factorVertical(5, sensorX, sensorY, m_oTypeOfLaserLine))/std::sin(oTriangAngle);
                auto diff_x = xc2d - oIntersectionOnPlane.x;
                auto diff_y = yc2d - oIntersectionOnPlane.y;
                if ( std::abs(diff_x) > tol_x || std::abs(diff_y) > tol_y )
                {
                    wmLog(eError, "Intersection point does not agree!\n");
                    assert(false);
                }


            }
        }
        else
        {
            //when the lines are almost parallel, it could happen that an intersection is found in screen coordinates but not in laser plane coordinates
            // (see threshold for determinant in LineEquation.intersect)
            assert(math::isClose(result.m_oLine1ForAngleComputation.m_oLine.getInclinationDegrees(), result.m_oLine2ForAngleComputation.m_oLine.getInclinationDegrees(), 1e-3));
            assert(!std::isnan(oIntersectionOnPlane.x));
            assert(!std::isnan(oIntersectionOnPlane.y));
        }

#endif
        if ( oHasIntersection )
        {
            result.m_oAngle = computeAngleAtIntersection(result.m_oLine1ForAngleComputation, result.m_oLine2ForAngleComputation, oIntersectionOnPlane.x);
        }
        else
        {
            result.m_oAngle = 0;
        }

        return {result};
    }

    double Intersect2Lines::computeAngleAtIntersection(const LineSegment & rLineSegment1, const LineSegment & rLineSegment2, const double & rXIntersection)
    {
        //unit vectors u1,v1 (line 1) and u2,v2 (line2)
        double u1, v1, u2, v2;
        rLineSegment1.m_oLine.getVector(u1, v1, rLineSegment1.m_oPoint.x > rXIntersection);
        rLineSegment2.m_oLine.getVector(u2, v2, rLineSegment2.m_oPoint.x > rXIntersection);

        //if we want to compute a signed angle (from first vector to second), in the case of laser plane coordinates
        //we still need to change sign to v1 and v2,because y[mm] has the opposite direction of y[screen]

        double oAngleRadians = std::abs(LineEquation::computeRadiansBetweenUnitVectors(u1, v1, u2, v2));
        double oAngleDeg = math::radiansToDegrees(oAngleRadians);
        return oAngleDeg;
    }


    geo2d::Point Intersect2Lines::convertToScreenPoint(const double & r_X, const double & r_Y) const
    {
        //cast to int and apply trafo
        return m_oSpTrafo->apply(geo2d::Point(int(std::round(r_X)), int(std::round(r_Y))));
    }

    void Intersect2Lines::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rE)
    {
        //// to be asserted by graph editor
        poco_assert_dbg(m_pPipeInImage != nullptr);
        poco_assert_dbg(m_pPipeInFirstLine != nullptr);
        poco_assert_dbg(m_pPipeInSecondLine != nullptr);


        // output context
        const interface::ImageFrame & rImage(m_pPipeInImage->read(m_oCounter));
        m_oSpTrafo = rImage.context().trafo();

        assert(!m_oSpTrafo.isNull());

        auto & rGeoLineModel1 = m_pPipeInFirstLine->read(m_oCounter);
        auto & rGeoLineModel2 = m_pPipeInSecondLine->read(m_oCounter);
        auto oNbLineModels1 = rGeoLineModel1.ref().size();
        auto oNbLineModels2 = rGeoLineModel2.ref().size();

        auto oNbLineModels = (oNbLineModels1 == 1 || oNbLineModels2 == 1) ? std::max(oNbLineModels1, oNbLineModels2) : std::min(oNbLineModels1, oNbLineModels2);
        bool alwaysUseFirstInput1 = oNbLineModels1 != oNbLineModels;
        if (alwaysUseFirstInput1 && oNbLineModels1 != 1)
        {
            wmLog(eDebug, "Filter '%s': Received %u values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), oNbLineModels1);
        }
        bool alwaysUseFirstInput2 = oNbLineModels2 != oNbLineModels;
        if (alwaysUseFirstInput2 && oNbLineModels2 != 1)
        {
            wmLog(eDebug, "Filter '%s': Received %u values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), oNbLineModels2);
        }
        if ( oNbLineModels == 0 )
        {
            wmLog(eWarning, "Filter '%s': Received empty input \n", m_oFilterName.c_str());
        }

        m_oPaintSegment1 = {};
        m_oPaintSegment2 = {};
        m_oPaintIntersection = {};
        assert(!m_oPaintSegment1.m_oLine.isValid() && !m_oPaintSegment2.m_oLine.isValid());

        double outRank = oNbLineModels > 0 ? interface::Limit : interface::NotPresent;
        interface::GeoDoublearray oGeoOutIntersectionResult(rImage.context(), geo2d::Doublearray(oNbLineModels), rImage.analysisResult(), outRank);
        interface::GeoDoublearray oGeoOutX(rImage.context(), geo2d::Doublearray(oNbLineModels), rImage.analysisResult(), outRank);
        interface::GeoDoublearray oGeoOutY(rImage.context(), geo2d::Doublearray(oNbLineModels), rImage.analysisResult(), outRank);
        interface::GeoDoublearray oGeoOutAngle(rImage.context(), geo2d::Doublearray(oNbLineModels), rImage.analysisResult(), outRank);

        for (std::size_t lineN = 0; lineN < oNbLineModels; lineN ++)
        {
            //input lines (already according to the output context)
            auto oLineSegment1 = parseLineInput(rGeoLineModel1, alwaysUseFirstInput1 ? 0 : lineN);
            auto oLineSegment2 = parseLineInput(rGeoLineModel2, alwaysUseFirstInput2 ? 0 : lineN);
            //compute results
            auto optLineIntersection = updateIntersection(oLineSegment1, oLineSegment2, rImage.data().size(), rImage.context());
            ValueRankType oOutputRank = optLineIntersection ? ValueRankType::eRankMax : ValueRankType::eRankMin;

            auto oLineIntersection = optLineIntersection.value_or(LinesIntersection{});

            oGeoOutIntersectionResult.ref()[lineN] = std::make_pair(double(oLineIntersection.m_ohasIntersection), oOutputRank );
            oGeoOutX.ref()[lineN] = std::make_pair(double(oLineIntersection.m_oX), oOutputRank );
            oGeoOutY.ref()[lineN] = std::make_pair(double(oLineIntersection.m_oY), oOutputRank );
            oGeoOutAngle.ref()[lineN] = std::make_pair(double(oLineIntersection.m_oAngle), oOutputRank );

            if (lineN == 0)
            {
                m_oPaintSegment1 = oLineSegment1;
                m_oPaintSegment2 = oLineSegment2;
                m_oPaintIntersection = oLineIntersection;
            }
        }

        preSignalAction();
        m_oPipeOutIntersectionResult.signal(oGeoOutIntersectionResult);
        m_oPipeOutX.signal(oGeoOutX);
        m_oPipeOutY.signal(oGeoOutY);
        m_oPipeOutAngle.signal(oGeoOutAngle);
    }

    void Intersect2Lines::paint()
    {
        if ( m_oVerbosity < eLow || m_oSpTrafo.isNull() )
        {
            return;
        } // if

        const double oLineLength = 200; //evtl should be taken from input context

        OverlayCanvas & rCanvas(canvas<OverlayCanvas>(m_oCounter));
        OverlayLayer & rLayerPosition(rCanvas.getLayerPosition());
        OverlayLayer & rLayerContour(rCanvas.getLayerContour());
        OverlayLayer & rLayerText(rCanvas.getLayerText());

        const auto oColor1 = Color::Magenta();
        const auto oColor2 = Color::Cyan();
        const auto oColorX = Color::Red();
        const geo2d::Size oTxtSize(300, 20);

        // Display intersection point position and angle between lines
        if ( m_oPaintIntersection.m_ohasIntersection )
        {
            rLayerPosition.add<OverlayCross>(convertToScreenPoint(m_oPaintIntersection.m_oX, m_oPaintIntersection.m_oY), oColorX);
            std::string oMsg = std::to_string(int(m_oPaintIntersection.m_oAngle)) + " deg";
            rLayerText.add<OverlayText>(oMsg.c_str(),
                Font(14), geo2d::Rect(convertToScreenPoint(m_oPaintIntersection.m_oX + 20, m_oPaintIntersection.m_oY), oTxtSize), oColorX);
        }

        if ( m_oVerbosity < VerbosityType::eHigh )
        {
            return;
        }
        // from here, display advanced information


        //display first line
        if ( m_oPaintSegment1.m_oLine.isValid())
        {
            geo2d::DPoint oStart, oEnd;
            m_oPaintSegment1.m_oLine.getPointOnLine(oStart.x, oStart.y, m_oPaintSegment1.m_oPoint.x, m_oPaintSegment1.m_oPoint.y, -oLineLength / 2);
            m_oPaintSegment1.m_oLine.getPointOnLine(oEnd.x, oEnd.y, oStart.x, oStart.y, oLineLength);
            rLayerContour.add<OverlayLine>(convertToScreenPoint(oStart.x, oStart.y), convertToScreenPoint(oEnd.x, oEnd.y), oColor1);
            if ( m_oPaintIntersection.m_ohasIntersection )
            {
                rLayerContour.add<OverlayLine>(convertToScreenPoint(m_oPaintSegment1.m_oPoint.x, m_oPaintSegment1.m_oPoint.y), convertToScreenPoint(m_oPaintIntersection.m_oX, m_oPaintIntersection.m_oY), oColor1);
            }
        }


        //display second line
        if ( m_oPaintSegment2.m_oLine.isValid() )
        {
            geo2d::DPoint oStart, oEnd;
            m_oPaintSegment2.m_oLine.getPointOnLine(oStart.x, oStart.y, m_oPaintSegment2.m_oPoint.x, m_oPaintSegment2.m_oPoint.y, -oLineLength / 2);
            m_oPaintSegment2.m_oLine.getPointOnLine(oEnd.x, oEnd.y, oStart.x, oStart.y, oLineLength);
            rLayerContour.add<OverlayLine>(convertToScreenPoint(oStart.x, oStart.y), convertToScreenPoint(oEnd.x, oEnd.y), oColor2);

            if ( m_oPaintIntersection.m_ohasIntersection )
            {
                rLayerContour.add<OverlayLine>(convertToScreenPoint(m_oPaintSegment2.m_oPoint.x, m_oPaintSegment2.m_oPoint.y), convertToScreenPoint(m_oPaintIntersection.m_oX, m_oPaintIntersection.m_oY), oColor2);
            }
        }


        //display inclination (but in the overlay the y axis is upside down, so it will look like the value of the angle in III and IV quadrans)
        // and "mid points" of the input lines
        if ( m_oVerbosity >= VerbosityType::eMax )
        {
            std::string oMsg = "  Angle1 = " + std::to_string((int) (m_oPaintIntersection.m_oLine1ForAngleComputation.m_oLine.getInclinationDegrees()));
            auto oPoint1Screen = convertToScreenPoint(m_oPaintSegment1.m_oPoint.x, m_oPaintSegment1.m_oPoint.y);
            rLayerText.add<OverlayText>(oMsg.c_str(), Font(14),
                geo2d::Rect(oPoint1Screen, oTxtSize), oColor1);
            rLayerPosition.add<OverlayCircle>(oPoint1Screen.x, oPoint1Screen.y, 2, oColor1);

            oMsg = "   Angle2 = " + std::to_string((int) (m_oPaintIntersection.m_oLine2ForAngleComputation.m_oLine.getInclinationDegrees()));
            auto oPoint2Screen = convertToScreenPoint(m_oPaintSegment2.m_oPoint.x, m_oPaintSegment2.m_oPoint.y);
            rLayerText.add<OverlayText>(oMsg.c_str(), Font(14),
                geo2d::Rect(oPoint2Screen, oTxtSize), oColor2);
            rLayerPosition.add<OverlayCircle>(oPoint2Screen.x, oPoint2Screen.y, 2, oColor2);
        }
    }

} //end namespaces
}
