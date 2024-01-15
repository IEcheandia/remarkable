#include "speedCompensation.h"
#include <module/moduleLogger.h>
#include <fliplib/TypeToDataTypeImpl.h>
#include <overlay/overlayCanvas.h>
#include <filter/armStates.h>
#include <filter/productData.h>
#include <overlay/font.h>
#include <geo/rect.h>
#include <geo/size.h>
#include <coordinates/fieldDistortionMapping.h>
#include <cmath>

#define FILTER_ID                 "5eeb27fa-749e-11ed-a1eb-0242ac120002"
#define VARIANT_ID                "74ad22be-749e-11ed-a1eb-0242ac120002"

#define PIPE_ID_CONTOURIN         "c6b65f48-7535-11ed-a1eb-0242ac120002"
#define PIPE_ID_CONTOUROUT        "a4795912-7535-11ed-a1eb-0242ac120002"
#define PIPE_ID_XSPEED            "64851d92-749e-11ed-a1eb-0242ac120002"
#define PIPE_ID_YSPEED            "6d589e4e-749e-11ed-a1eb-0242ac120002"
#define PIPE_ID_ABSOLUTE_POSITION_X "a5cc1730-9ce2-11ed-a8fc-0242ac120002"
#define PIPE_ID_ABSOLUTE_POSITION_Y "afe74bfe-9ce2-11ed-a8fc-0242ac120002"

namespace {
static const auto attributeLaserVelocity = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity;
}

namespace precitec
{
namespace filter
{
using fliplib::Parameter;

const std::string SpeedCompensation::m_filterName("SpeedCompensation");
const std::string SpeedCompensation::m_contourInName("ContourIn");
const std::string SpeedCompensation::m_contourOutName("ContourOut");
const std::string SpeedCompensation::m_speedInXDirectionName("SpeedInXDirection");
const std::string SpeedCompensation::m_speedInYDirectionName("SpeedInYDirection");
const std::string SpeedCompensation::m_absolutePositionXName("AbsolutePositionX");
const std::string SpeedCompensation::m_absolutePositionYName("AbsolutePositionY");

SpeedCompensation::SpeedCompensation():
    TransformFilter(SpeedCompensation::m_filterName, Poco::UUID{FILTER_ID})
    , m_pipeContourIn(nullptr)
    , m_pipeSpeedInXDirection(nullptr)
    , m_pipeSpeedInYDirection(nullptr)
    , m_pPipeInAbsolutePositionX(nullptr)
    , m_pPipeInAbsolutePositionY(nullptr)
    , m_pipeContourOut(this, m_contourOutName)
    , m_productSpeed(0)
    , m_targetVelocity(0)
    , m_laserVelocity(0)
    , m_speedX(0)
    , m_speedY(0)
    , m_paint(false)

{
    parameters_.add("TargetVelocity", Parameter::TYPE_int, static_cast<int>(m_targetVelocity));
    unsigned int group = 1;
    setInPipeConnectors({
        {Poco::UUID(PIPE_ID_CONTOURIN), m_pipeContourIn, m_contourInName, group, m_contourInName},
        {Poco::UUID(PIPE_ID_XSPEED), m_pipeSpeedInXDirection, m_speedInXDirectionName, group, m_speedInXDirectionName},
        {Poco::UUID(PIPE_ID_YSPEED), m_pipeSpeedInYDirection, m_speedInYDirectionName, group, m_speedInYDirectionName},
        {Poco::UUID(PIPE_ID_ABSOLUTE_POSITION_X), m_pPipeInAbsolutePositionX, m_absolutePositionXName, group, m_absolutePositionXName},
        {Poco::UUID(PIPE_ID_ABSOLUTE_POSITION_Y), m_pPipeInAbsolutePositionY, m_absolutePositionYName, group, m_absolutePositionYName}
    });
    group = 0;
    setOutPipeConnectors({
         {Poco::UUID(PIPE_ID_CONTOUROUT), &m_pipeContourOut, std::string{m_contourOutName}, group, std::string{m_contourOutName}}
    });
    setVariantID(Poco::UUID(VARIANT_ID));
}


void SpeedCompensation::setParameter()
{
    TransformFilter::setParameter();
    m_targetVelocity = parameters_.getParameter("TargetVelocity").convert<int>();
    wmLog(eDebug, "laser velocity from filter parameter [mm/s]: %d\n", m_targetVelocity);
}

void SpeedCompensation::arm(const fliplib::ArmStateBase& state)
{
    if (state.getStateID() == eSeamStart)
    {
        // get product information
        const analyzer::ProductData* productData = externalData<analyzer::ProductData>();
        const auto inspectionVelocity = static_cast<double>(productData->m_oInspectionVelocity); // use information about speed [um/ms] from the product
        m_productSpeed = inspectionVelocity / 1000.0;
        wmLog(eDebug, "laser velocity from product [mm/s]: %f\n", m_productSpeed);
    }
}

void SpeedCompensation::paint()
{
    if (!m_paint)
    {
        return;
    }

    image::OverlayCanvas& overlayCanvas(canvas<image::OverlayCanvas>(m_oCounter));
    image::OverlayLayer& layerContour(overlayCanvas.getLayerContour());
    image::OverlayLayer& layerPosition(overlayCanvas.getLayerPosition());
    image::OverlayLayer& layerText(overlayCanvas.getLayerText());

    const auto inputPositionDifferentFromActual = (m_scannerPositionActual.m_hasPosition
            && !(math::isClose(m_scannerInputPosition_at_TCP.x, m_scannerPositionActual.m_x, 1e-3)
            && math::isClose(m_scannerInputPosition_at_TCP.y, m_scannerPositionActual.m_y, 1e-3)));

    if (inputPositionDifferentFromActual)
    {
        layerPosition.add<image::OverlayCross>(m_tcpInputPositionToPaint.x, m_tcpInputPositionToPaint.y, image::Color::Red());
    }

    //draw in any case the TCP as of the current camera position, yellow if it's different from the input, green if is equal
    {
        const auto color = inputPositionDifferentFromActual ? image::Color::Yellow() : image::Color::Green();
        std::ostringstream msg;
        msg << std::setprecision(2) << std::fixed << " TCP ";
        if (m_scannerPositionActual.m_hasPosition)
        {
            msg << "at " << m_scannerPositionActual.m_x << " " << m_scannerPositionActual.m_y ;
        }
        msg << " mm ";
        layerText.add<image::OverlayText>(msg.str(), image::Font(14), geo2d::Rect( 0, 10 , 200, 20), color);
        layerPosition.add<image::OverlayCross>( m_tcpActualPositionToPaint.x, m_tcpActualPositionToPaint.y, color);
    }

    if (m_inPointsToPaint.size() == 0 || m_outPointsToPaint.size() == 0 )
    {
        return;
    }

    auto pointInColor = image::Color::Red();
    auto lineInColor = pointInColor;
    lineInColor.alpha = 125;

    auto pointOutColor = image::Color::Green();
    auto lineOutColor = pointOutColor;
    lineOutColor.alpha = 125;

    drawPoints(layerPosition, layerContour, m_inPointsToPaint, lineInColor);
    drawPoints(layerPosition, layerContour, m_outPointsToPaint, lineOutColor);
}

void SpeedCompensation::prepareDataForPainting(
    const interface::ImageContext& context,
    const geo2d::TAnnotatedArray<geo2d::TPoint<double>>& contour,
    geo2d::AnnotatedDPointarray& pointsToPaint,
    const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionX,
    const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionY)
{
    m_scannerPositionActual = context.m_ScannerInfo;
    if (!m_scannerPositionActual.m_hasPosition)
    {
        //should not happen, set some reasonable value for safety
        m_scannerPositionActual.m_x = 0;
        m_scannerPositionActual.m_y = 0;
    }

    bool isInputPositionGood;
    std::tie(m_scannerInputPosition_at_TCP, isInputPositionGood) = parseInputPosition(geoAbsolutePositionX, geoAbsolutePositionY);

    // do not add any trafo offset to the coordinates in the paint routine
    m_spTrafo = new interface::LinearTrafo(0,0);
    //let's compute everything on the canvas, not the roi
    interface::ImageContext dummyContext(context, m_spTrafo);
    const auto coordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, dummyContext, MeasurementType::Image);

    //Compute the TCP in the image currently shown
    const auto sensorCoordinatesActualPositionTCP = getTCPfromCameraPosition_pix (0,0); // on the sensor, before HWROI
    const auto canvasCoordinatesActualPositionTCP = geo2d::DPoint{sensorCoordinatesActualPositionTCP.x - context.HW_ROI_x0,
                                                            sensorCoordinatesActualPositionTCP.y - context.HW_ROI_y0 };

    m_tcpActualPositionToPaint.x = std::floor(canvasCoordinatesActualPositionTCP.x);
    m_tcpActualPositionToPaint.y = std::floor(canvasCoordinatesActualPositionTCP.y);

    //Compute distance in the scanner reference system [mm] between the input (0,0 mm if the contour is in absolute coordinates) and the current scanner position
    const auto scannerPositionDifference_mm = m_scannerInputPosition_at_TCP - geo2d::DPoint{m_scannerPositionActual.m_x, m_scannerPositionActual.m_y};

    //Compute where would the input be in the current image?
    const auto canvasCoordinatesInputPositionTCP = coordTransformer->distanceTCPmmToImageCoordCoax(std::vector<geo2d::DPoint>{scannerPositionDifference_mm},
                                                                                            sensorCoordinatesActualPositionTCP.x, sensorCoordinatesActualPositionTCP.y)[0];


    //Compute distance in the image [pixels] between the input  and the current scanner position
    const auto scannerPositionDifference_pix = canvasCoordinatesInputPositionTCP - canvasCoordinatesActualPositionTCP;
    m_tcpInputPositionToPaint.x = std::round(canvasCoordinatesInputPositionTCP.x);
    m_tcpInputPositionToPaint.y = std::round(canvasCoordinatesInputPositionTCP.y);

    // Convert distance to scanner input [mm] to distance to current TCP [pixel]
    // First pretend that there is no input difference (so that the calibration computes valid coordinate),
    // then apply the pixel offset
    preparePointsToPaint(contour, pointsToPaint, sensorCoordinatesActualPositionTCP, coordTransformer, scannerPositionDifference_pix);
}

void SpeedCompensation::preparePointsToPaint(
    const geo2d::TAnnotatedArray<geo2d::TPoint<double>>& contour,
    geo2d::AnnotatedDPointarray& pointsToPaint,
    const geo2d::DPoint& sensorCoordinatesActualPositionTCP,
    const std::unique_ptr<math::ImageCoordsTo3DCoordsTransformer, std::default_delete<math::ImageCoordsTo3DCoordsTransformer>>& coordTransformer,
    const geo2d::TPoint<double>& scannerPositionDifference_pix)
{
    const auto& calibrationData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);
    std::vector<double> K;
    const auto distortionCorrectionEnable = calibrationData.scanfieldDistortionCorrectionFactor(K);

    if (distortionCorrectionEnable)
    {
        const auto distortionCoefficient = scannerPositionToDistortionCoefficient(m_scannerPositionActual.m_x, m_scannerPositionActual.m_y, K);
        const auto& contourWorld = contour.getData();
        const auto contourSize = contourWorld.size();
        auto& contourPixel = pointsToPaint.getData();
        contourPixel.resize(contourSize);
        for (std::size_t i = 0; i < contourSize; ++i)
        {
            const auto shiftedContourX = contourWorld[i].x + m_scannerInputPosition_at_TCP.x - m_scannerPositionActual.m_x;
            const auto shiftedContourY = contourWorld[i].y + m_scannerInputPosition_at_TCP.y - m_scannerPositionActual.m_y;
            const auto point = worldToPixel(shiftedContourX, shiftedContourY, distortionCoefficient);
            contourPixel[i] = {
                point.first + sensorCoordinatesActualPositionTCP.x,
                point.second + sensorCoordinatesActualPositionTCP.y
            };
        }
    }
    else
    {
        pointsToPaint.getData() =  coordTransformer->distanceTCPmmToImageCoordCoax(contour.getData(),
        sensorCoordinatesActualPositionTCP.x, sensorCoordinatesActualPositionTCP.y);
        for (auto& rPoint : pointsToPaint.getData())
        {
            rPoint = rPoint + scannerPositionDifference_pix;
        }
    }

    //Now that the coordinates are computed, fill the other fields of the TArray
    if (pointsToPaint.getData().size() > 0)
    {
        pointsToPaint.getRank() = contour.getRank();
    }
    else
    {
        pointsToPaint.getRank().clear();
    }
}

void SpeedCompensation::drawPoints(
    image::OverlayLayer& layerPosition,
    image::OverlayLayer& layerContour,
    const geo2d::AnnotatedDPointarray& pointsToPaint,
    const image::Color& color) const
{
    const auto numPoints = pointsToPaint.size();
    auto prevPoint = drawPoint(layerPosition, 0, pointsToPaint, color);
    for (std::size_t point_index = 1 ; point_index < numPoints; point_index++)
    {
        const auto curPoint = drawPoint(layerPosition, point_index, pointsToPaint, color);
        layerContour.add<image::OverlayLine>(prevPoint.x, prevPoint.y, curPoint.x, curPoint.y, color);
        prevPoint = curPoint;
    }
}

geo2d::DPoint SpeedCompensation::drawPoint(
    image::OverlayLayer& layerPosition,
    const int point_index,
    const geo2d::AnnotatedDPointarray& pointsToPaint,
    const image::Color& pointColor) const
{
    auto point = pointsToPaint.getData()[point_index];
    point.x += m_spTrafo->dx();
    point.y += m_spTrafo->dy();

    if (pointsToPaint.getRank()[point_index] == 255)
    {
        layerPosition.add<image::OverlayPoint>(point.x, point.y, pointColor);
    }
    else
    {
        layerPosition.add<image::OverlayCircle>(point.x, point.y, 3, pointColor);
    }
    return point;
};

bool SpeedCompensation::subscribe(fliplib::BasePipe& pipe, int group)
{
    if (pipe.tag() == m_contourInName)
    {
        m_pipeContourIn = dynamic_cast<fliplib::SynchronePipe<interface::GeoVecAnnotatedDPointarray>*>(&pipe);
    }
    else if (pipe.tag() == m_speedInXDirectionName)
    {
        m_pipeSpeedInXDirection = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == m_speedInYDirectionName)
    {
        m_pipeSpeedInYDirection = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == m_absolutePositionXName) //output position at tcp
    {
        m_pPipeInAbsolutePositionX = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    else if (pipe.tag() == m_absolutePositionYName)
    {
        m_pPipeInAbsolutePositionY = dynamic_cast<fliplib::SynchronePipe<interface::GeoDoublearray>*>(&pipe);
    }
    return BaseFilter::subscribe(pipe, group);
}

void SpeedCompensation::proceedGroup(const void* sender, fliplib::PipeGroupEventArgs& event)
{
    const auto& contourInArray = m_pipeContourIn->read(m_oCounter);
    const auto& geoDoubleArrayXIn = m_pipeSpeedInXDirection->read(m_oCounter);
    const auto& geoDoubleArrayYIn = m_pipeSpeedInYDirection->read(m_oCounter);
    const auto& geoAbsolutePositionX = m_pPipeInAbsolutePositionX->read(m_oCounter);
    const auto& geoAbsolutePositionY = m_pPipeInAbsolutePositionY->read(m_oCounter);
    auto rank = contourInArray.rank();
    const auto context = geoDoubleArrayYIn.context();
    const auto geoAnalysisResult = contourInArray.analysisResult();

    auto isValid = true;
    if (geoAnalysisResult == interface::AnalysisOK
        && contourInArray.ref().size() != 0
        && geoDoubleArrayXIn.ref().size() != 0
        && geoDoubleArrayYIn.ref().size() != 0
        && geoAbsolutePositionX.ref().size() != 0
        && geoAbsolutePositionY.ref().size() != 0)
    {
        setLaserVelocity(contourInArray.ref().at(0));
        m_speedX = std::get<0>(geoDoubleArrayXIn.ref()[0]);
        m_speedY = std::get<0>(geoDoubleArrayYIn.ref()[0]);

        if (m_laserVelocity == 0)
        {
            wmLog(eError, "SpeedCompensation: laser velocity should not be 0 (image %d)\n", m_oCounter);
            isValid = false;
        }
        if ((m_laserVelocity - std::abs(m_speedY)) == 0 || (m_laserVelocity - std::abs(m_speedX)) == 0)
        {
            wmLog(eError, "SpeedCompensation: speedX/ speedY must not have the same value as the laser velocity (image %d)\n", m_oCounter);
            isValid = false;
        }
    }
    else
    {
        wmLog(eError, "SpeedCompensation: input not valid (image %d)\n", m_oCounter);
        isValid = false;
    }

    if (isValid
    && (geoDoubleArrayYIn.ref().getRank().at(0) != 255
    || contourInArray.ref().at(0).getRank().at(0) != 255
    || geoAbsolutePositionX.ref().getRank().at(0) != 255
    || geoAbsolutePositionY.ref().getRank().at(0) != 255))
    {
        isValid = false;
    }

    if (!isValid)
    {
        const auto rank(interface::NotPresent);
        const auto geoOut = interface::GeoVecAnnotatedDPointarray(
            context,
            std::vector<geo2d::AnnotatedDPointarray>{},
            geoAnalysisResult,
            rank);
        preSignalAction();
        m_pipeContourOut.signal(geoOut);
        return;
    }
    m_paint = (m_oVerbosity >= VerbosityType::eMedium);
    compensateContour(contourInArray);
    auto outContour = contourInArray.ref().at(0);
    outContour.getScalarData(attributeLaserVelocity) = m_compensatedLaserVelocity;
    outContour.getData() = m_compensatedContour;
    const auto contourRankIn = contourInArray.ref().at(0).getRank();
    const auto arrayYRankIn = geoDoubleArrayYIn.ref().getRank();

    outContour.getRank() = arrayYRankIn;
    m_inPointsToPaint.clear();
    m_outPointsToPaint.clear();

    if (m_paint)
    {
        prepareDataForPainting(context, contourInArray.ref().at(0), m_inPointsToPaint, geoAbsolutePositionX, geoAbsolutePositionY);
        prepareDataForPainting(context, outContour, m_outPointsToPaint, geoAbsolutePositionX, geoAbsolutePositionY);
    }

    interface::GeoVecAnnotatedDPointarray geoOut(
        context,
        {outContour},
        geoAnalysisResult,
        rank);
    preSignalAction();
    m_pipeContourOut.signal(geoOut);
}

std::pair<geo2d::DPoint, bool> SpeedCompensation::parseInputPosition(const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionX, const interface::TGeo<geo2d::TArray<double>>& geoAbsolutePositionY)
{
    const auto x = geoAbsolutePositionX.ref().getData()[0];
    const auto y = geoAbsolutePositionY.ref().getData()[0];
    bool goodRank = geoAbsolutePositionX.ref().getRank()[0] != eRankMin && geoAbsolutePositionY.ref().getRank()[0] != eRankMin;

    if (m_oVerbosity >= VerbosityType::eMedium)
    {
        if (!goodRank)
        {
            wmLog(eDebug, "Input scanner position %f %f has bad rank %d %d\n", x,y, geoAbsolutePositionX.ref().getRank()[0], geoAbsolutePositionY.ref().getRank()[0]);
        }
        const auto& calibrationData = system::CalibDataSingleton::getCalibrationDataReference(math::SensorId::eSensorId0);

        if (calibrationData.hasCameraCorrectionGrid())
        {
            auto contextScannerInfo = calibrationData.getCurrentScannerInfo(m_oCounter % g_oNbPar);
            if (goodRank)
            {
                if (!contextScannerInfo.m_hasPosition || contextScannerInfo.m_x != x || contextScannerInfo.m_y != y)
                {
                    //usually when the input is provided in mm, they are absolute coordinates (input position = 0, 0)
                    if (x != 0.0 && y != 0.0)
                    {
                        wmLog(eDebug, "CalibrationData uses scanner position %f %f, but scanner position from input pipes is %f %f \n",
                                contextScannerInfo.m_x, contextScannerInfo.m_y, x,y);
                    }
                }
            }
        }
    }
    return std::make_pair(geo2d::DPoint{x,y}, goodRank);
};

geo2d::DPoint SpeedCompensation::getTCPfromCameraPosition_pix (int HW_ROI_x0, int HW_ROI_y0) const
{
    // we want to use ytcp , so let's set the laser to 1
    geo2d::DPoint sensorCoordinatesTCP = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0 ).getTCPCoordinate(m_oCounter % g_oNbPar, LaserLine::FrontLaserLine);
    return {sensorCoordinatesTCP.x - HW_ROI_x0, sensorCoordinatesTCP.y - HW_ROI_y0};
}

/**
    Conversion of the original contour into a compensated contour,
    which eliminates the vertical and horizontal displacements caused
    by the constant movement during "on-the-fly processing".
**/
void SpeedCompensation::compensateContour(
    const interface::GeoVecAnnotatedDPointarray& contourInArray)
{
    wmLog(eDebug, "SpeedCompensation::compensateContour\n");
    const auto& contourIn = contourInArray.ref().at(0);
    const auto& contourPoints = contourIn.getData();

    wmLog(eDebug, "horizontal speed [mm/s]: %f, vertical speed [mm/s]: %f, laserVelocity [mm/s]: %f\n", m_speedX, m_speedY, m_laserVelocity);
    auto numPoints = contourPoints.size();
    auto prevPoint = contourPoints.at(0);
    m_compensatedContour = std::vector<geo2d::DPoint>(numPoints);
    m_compensatedLaserVelocity = std::vector<double>(numPoints);
    m_compensatedContour.front() = prevPoint;

    for (std::size_t i = 1; i < numPoints; ++i)
    {
        const auto& curPoint = contourPoints.at(i); // points in mm
        auto& preCompensatedPoint = m_compensatedContour.at(i-1);
        auto& curCompensatedPoint = m_compensatedContour.at(i);
        curCompensatedPoint = transformToCompensatedPoint(prevPoint, curPoint, preCompensatedPoint);
        m_compensatedLaserVelocity.at(i-1) = recalculateLaserVelocity(prevPoint, curPoint, preCompensatedPoint, curCompensatedPoint);
        prevPoint = contourPoints.at(i);
        if (m_paint)
        {
            wmLog(eDebug, "originalPoint(%f, %f), transformedPoint(%f, %f)\n", curPoint.x, curPoint.y, curCompensatedPoint.x, curCompensatedPoint.y);
        }
    }
    --numPoints;
    m_compensatedLaserVelocity.at(numPoints) = 0;
}


/**
    Transformation of the input contour into the compensated contour.
**/

geo2d::TPoint<double> SpeedCompensation::transformToCompensatedPoint(
    const geo2d::TPoint<double>& prevPoint,
    const geo2d::TPoint<double>& currPoint,
    const geo2d::TPoint<double>& prevCompensatedPoint) const
{
    if (m_speedX == 0 && m_speedY == 0)
    {
        return currPoint;
    }
    auto x = currPoint.x;
    auto y = currPoint.y;
    const auto distance = euclideanDistance(currPoint.x, prevPoint.x, currPoint.y, prevPoint.y);
    if (m_speedY != 0)
    {
        const auto yc = (distance / m_laserVelocity) * m_speedY;
        y = currPoint.y - ((prevPoint.y - prevCompensatedPoint.y) + yc);
    };
    if (m_speedX != 0)
    {
        const auto xc = (distance / m_laserVelocity) * m_speedX;
        x = currPoint.x - ((prevPoint.x - prevCompensatedPoint.x) + xc);
    }
    return {x, y};
}

/**
    Recalculates the laser velocity so that the time of lasering
    remains unchanged despite the changed contour length
**/
double SpeedCompensation::recalculateLaserVelocity(
    const geo2d::TPoint<double>& prevPoint,
    const geo2d::TPoint<double>& curPoint,
    const geo2d::TPoint<double>& prevCompensatedPoint,
    const geo2d::TPoint<double>& curCompensatedPoint) const
{
    double result = m_laserVelocity;
    if (m_laserVelocity != 0)
    {
        const auto distanceOriginalPoint = euclideanDistance(curPoint.x, prevPoint.x, curPoint.y, prevPoint.y);
        const auto distanceCompensatedPoint = euclideanDistance(prevCompensatedPoint.x, curCompensatedPoint.x, prevCompensatedPoint.y, curCompensatedPoint.y);
        const double weldingTime = distanceOriginalPoint / m_laserVelocity;
        result = m_laserVelocity * (distanceCompensatedPoint / distanceOriginalPoint);
        if (m_paint)
        {
            wmLog(eDebug, "compensation speed [mm/s]: %f, welding time [s]: %f, distance before compensation [mm]: %f, distance after compensation [mm]: %f\n", result, weldingTime, distanceOriginalPoint, distanceCompensatedPoint);
        }
    }
    return result;
}

void SpeedCompensation::setLaserVelocity(const geo2d::TAnnotatedArray<geo2d::TPoint< double>>& contour)
{
    auto laserVelocityOfFirstPoint = 0.;
    bool hasLaserVelocity = contour.hasScalarData(attributeLaserVelocity);
    if (hasLaserVelocity)
    {
        laserVelocityOfFirstPoint = contour.getScalarData(attributeLaserVelocity).front();
    }
    m_laserVelocity = updateLaserVelocity(laserVelocityOfFirstPoint);
}

double SpeedCompensation::updateLaserVelocity(const double& laserVelocityOfFirstPoint)
{
    auto laserVelocity = m_productSpeed;
    if (laserVelocityOfFirstPoint != -1)
    {
        laserVelocity = laserVelocityOfFirstPoint;
    }
    else
    {
        if (m_targetVelocity != 0)
        {
            laserVelocity = static_cast<double>(m_targetVelocity);
        }
    }
    return laserVelocity;
}

double SpeedCompensation::euclideanDistance(const double& x1, const double& x2, const double& y1, const double& y2) const
{
    const auto x = x1 - x2;
    const auto y = y1 - y2;
    return sqrt(pow(x, 2) + pow(y, 2));
}


}
}
