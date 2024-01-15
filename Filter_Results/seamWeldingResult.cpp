#include "seamWeldingResult.h"

#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"
#include "util/calibDataSingleton.h"
#include <numeric>
#include <fliplib/TypeToDataTypeImpl.h>
#include "common/definesScanlab.h"

#include "coordinates/fieldDistortionMapping.h"

namespace {
static const auto attributeLaserPower = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPower;
static const auto attributeLaserPowerRing = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserPowerRing;
static const auto attributeLaserVelocity = precitec::geo2d::AnnotatedDPointarray::Scalar::LaserVelocity;
}

namespace precitec {
namespace filter {

using fliplib::Parameter;
using interface::ResultType;
using interface::GeoDoublearray;
using interface::ResultDoubleArray;


SeamWeldingResult::SeamWeldingResult():
    ResultFilter("SeamWeldingResult", Poco::UUID{"7e1ac196-1d32-4d7d-a957-b1a42f59a888"}),
    	m_pPipeInContour_pix	( nullptr),
        m_pPipeInAbsolutePositionX (nullptr),
        m_pPipeInAbsolutePositionY (nullptr),
		m_oPipeResultPath_mm( this, "SeamWeldingResultOutput" ),
		m_inputContourType(InputContourType::pixel),
        m_inputAbsolutePositionUnit (InputAbsolutePositionUnit::um),
        m_outputType(ScanmasterResultType::SeamWelding),
        m_maxXCoordinate_mm(130.0),
        m_maxYCoordinate_mm(100.0)
{
    parameters_.add("InputContourType", Parameter::TYPE_int, static_cast<int>(m_inputContourType));
    parameters_.add("InputPositionUnit", Parameter::TYPE_int, static_cast<int>(m_inputAbsolutePositionUnit));
    parameters_.add("Result", Parameter::TYPE_int, static_cast<int>(m_outputType));
    parameters_.add("MaxXCoordinate", Parameter::TYPE_double, static_cast<double>(m_maxXCoordinate_mm));
    parameters_.add("MaxYCoordinate", Parameter::TYPE_double, static_cast<double>(m_maxYCoordinate_mm));

    setInPipeConnectors({{Poco::UUID("25a9a85b-76d6-43a4-9d36-e03ff337aafc"), m_pPipeInContour_pix, "contour", 1, "contour"},
    {Poco::UUID("e6c27fb7-630c-4cf2-a340-cdc2be1ae3b1"), m_pPipeInAbsolutePositionX, "absolute_position_x", 1, "absolute_position_x"},
    {Poco::UUID("62bddd9d-bb36-43ae-81d0-001083329902"), m_pPipeInAbsolutePositionY, "absolute_position_y", 1, "absolute_position_y"}});
    setVariantID(Poco::UUID("b5c8596f-2419-4908-8c82-86f4ff1e98f4"));
} //ctor



void SeamWeldingResult::setParameter()
{
    ResultFilter::setParameter();

    m_inputContourType = static_cast<InputContourType>(parameters_.getParameter("InputContourType").convert<int>());
    m_inputAbsolutePositionUnit = static_cast<InputAbsolutePositionUnit>(parameters_.getParameter("InputPositionUnit").convert<int>());

    switch (parameters_.getParameter("Result").convert<int>())
    {
        case int(ScanmasterResultType::SeamWelding):
            m_outputType = ScanmasterResultType::SeamWelding;
            break;
        case int(ScanmasterResultType::ScannerMovingToFirstPoint ):
            m_outputType = ScanmasterResultType::ScannerMovingToFirstPoint;
            break;
        case int(ScanmasterResultType::SeamWeldingAndSendEndOfSeam ):
            m_outputType = ScanmasterResultType::SeamWeldingAndSendEndOfSeam;
            break;
        case int(ScanmasterResultType::PrepareContour ):
            m_outputType = ScanmasterResultType::PrepareContour;
            break;
        default:
            wmLog(eInfo, "Invalid value %d for parameter result, using Preview instead \n", parameters_.getParameter("Result").convert<int>());
            //FALLTHROUGH
        case int(ScanmasterResultType::Dummy):
            m_outputType = ScanmasterResultType::Dummy;
            break;

    }
    m_maxXCoordinate_mm = parameters_.getParameter("MaxXCoordinate").convert<double>();
    m_maxYCoordinate_mm = parameters_.getParameter("MaxYCoordinate").convert<double>();


} // setParameter



bool SeamWeldingResult::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if ( p_rPipe.tag() == "contour" )
    {
		m_pPipeInContour_pix  = dynamic_cast<pipe_contour_t*>(&p_rPipe);
    }
	else if ( p_rPipe.tag() == "absolute_position_x" ) //output position at tcp
    {
		m_pPipeInAbsolutePositionX  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }

	else if ( p_rPipe.tag() == "absolute_position_y" )
    {
		m_pPipeInAbsolutePositionY  = dynamic_cast<pipe_scalar_t*>(&p_rPipe);
    }
    else
    {
        poco_assert_dbg(false); // to be asserted by graph editor
    }

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

void SeamWeldingResult::proceedGroup( const void* p_pSender, fliplib::PipeGroupEventArgs& e )
{
    poco_assert_dbg(m_pPipeInContour_pix != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInAbsolutePositionX != nullptr); // to be asserted by graph editor
    poco_assert_dbg(m_pPipeInAbsolutePositionY != nullptr); // to be asserted by graph editor

    const auto & rGeoArrayIn = m_pPipeInContour_pix->read(m_oCounter);
    const auto & rGeoAbsolutePositionX = m_pPipeInAbsolutePositionX->read(m_oCounter);
    const auto & rGeoAbsolutePositionY = m_pPipeInAbsolutePositionY->read(m_oCounter);

    const ResultType oAnalysisResult = rGeoArrayIn.analysisResult();

    auto oMaxCoordinate = std::max(m_maxXCoordinate_mm, m_maxYCoordinate_mm);
    auto oMinMaxRange = geo2d::TRange<double>( -oMaxCoordinate, oMaxCoordinate );

    m_oPaint = (m_oVerbosity >= VerbosityType::eMedium);
    m_pointsToPaint.clear();

    ResultType oResultType = [this] ()
    {
        switch(m_outputType)
        {
            case ScanmasterResultType::SeamWelding:
                return ResultType::ScanmasterSeamWelding;
                break;
            case ScanmasterResultType::ScannerMovingToFirstPoint:
                return ResultType::ScanmasterScannerMoving;
                break;
            case ScanmasterResultType::SeamWeldingAndSendEndOfSeam:
                return ResultType::ScanmasterSeamWeldingAndEndOfSeamMarker;
                break;
            case ScanmasterResultType::PrepareContour:
                return ResultType::PrepareContour;
                break;
            case ScanmasterResultType::Dummy:
                return ResultType::ScanmasterScannerMoving; //but the actual result will always have rank 0
                break;
        }
        assert(false && "not all cases handled in switch");
        return ResultType::ScanmasterScannerMoving;
    }();


    // return nio if input data is not good
    if ( oAnalysisResult != interface::AnalysisOK
        || rGeoArrayIn.ref().size() == 0
        || rGeoAbsolutePositionX.ref().size() == 0
        || rGeoAbsolutePositionY.ref().size() == 0
        )
    {
        wmLog(eDebug, "SeamWeldingResult: input not valid (image %d)\n", m_oCounter);
        m_oResultArray.resize(0);
        m_oPaint = false;
        const auto oGeoValueOut = GeoDoublearray{ rGeoArrayIn.context(),
                                                  m_oResultArray,
                                                  oAnalysisResult,
                                                interface::NotPresent }; // bad rank

        auto oResultNIO = ResultDoubleArray{
            id(),
            oResultType,
            oAnalysisResult,
            rGeoArrayIn.context(),
            oGeoValueOut,
            oMinMaxRange,
            true }; // set nio

        preSignalAction();
        m_oPipeResultPath_mm.signal(oResultNIO);

        return;
    }

    auto parseInputPosition = [this, &rGeoAbsolutePositionX, &rGeoAbsolutePositionY, &oResultType] ()
    {
        auto x = rGeoAbsolutePositionX.ref().getData()[0];
        auto y = rGeoAbsolutePositionY.ref().getData()[0];
        bool goodRank = rGeoAbsolutePositionX.ref().getRank()[0] != eRankMin && rGeoAbsolutePositionY.ref().getRank()[0] != eRankMin;
        if (m_inputAbsolutePositionUnit == InputAbsolutePositionUnit::um)
        {
            x /= 1000.0;
            y /= 1000.0;
        }
        if (m_oVerbosity >= VerbosityType::eHigh)

        {
            if (!goodRank && m_outputType != ScanmasterResultType::Dummy)
            {
                wmLog(eDebug, "Input scanner position %f %f has bad rank %d %d, result %d will not be sent to HW \n", x,y, rGeoAbsolutePositionX.ref().getRank()[0], rGeoAbsolutePositionY.ref().getRank()[0], oResultType);
            }
            auto &rCalibrationData = system::CalibDataSingleton::getCalibrationDataReference(math::SensorId::eSensorId0);

            if (rCalibrationData.hasCameraCorrectionGrid())
            {
                auto contextScannerInfo = rCalibrationData.getCurrentScannerInfo(m_oCounter % g_oNbPar);
                if (goodRank)
                {
                    if (!contextScannerInfo.m_hasPosition || contextScannerInfo.m_x != x || contextScannerInfo.m_y != y)
                    {
                        if (m_inputContourType == InputContourType::pixel)
                        {
                            wmLog(eWarning, "SeamWeldingResult will compute TCP distance with scanner position %f %f, but scanner position from input pipes is %f %f \n",
                                    contextScannerInfo.m_x, contextScannerInfo.m_y, x,y);
                        }
                        else
                        {
                            //usually when the input is provided in mm, they are absolute coordinates (input position = 0, 0)
                            if (x != 0.0 &&  y != 0.0)
                            {
                                wmLog(eDebug, "CalibrationData uses scanner position %f %f, but scanner position from input pipes is %f %f \n",
                                        contextScannerInfo.m_x, contextScannerInfo.m_y, x,y);
                            }
                        }
                    }
                }

            }
            else
            {
                wmLog(eWarning, "SeamWeldingResult: No camera correction grid present \n");
            }
        }
        return std::make_pair(geo2d::DPoint{x,y}, goodRank);
    };

    bool isInputPositionGood;
    std::tie(m_scannerInputPosition_at_TCP, isInputPositionGood) = parseInputPosition();
    bool forceBadRank = !isInputPositionGood || m_outputType == ScanmasterResultType::Dummy;
    bool validResult = !forceBadRank;

    m_oSpTrafo = rGeoArrayIn.context().trafo();


    const geo2d::AnnotatedDPointarray & rArrayToProcess = [&]()
        {
            const auto & rArrayIn = rGeoArrayIn.ref()[0];
            switch(m_outputType)
            {
                case ScanmasterResultType::SeamWelding:
                case ScanmasterResultType::SeamWeldingAndSendEndOfSeam:
                case ScanmasterResultType::PrepareContour:
                case ScanmasterResultType::Dummy:
                    return rArrayIn;
                case ScanmasterResultType::ScannerMovingToFirstPoint:
                    if (rArrayIn.size() > 0)
                    {
                        return geo2d::AnnotatedDPointarray (1, rArrayIn.getData()[0], rArrayIn.getRank()[0]);
                    }
            }
            return geo2d::AnnotatedDPointarray (0);
        }();


    switch(m_inputContourType)
    {
        case(InputContourType::mm_relative):
            {
                auto addAbsoluteScannerPosition =  [this](geo2d::DPoint point)
                {
                    return point + m_scannerInputPosition_at_TCP;
                };
                validResult = transformPointsToScanmasterResultArray(m_oResultArray, rArrayToProcess, addAbsoluteScannerPosition, forceBadRank,
                                                       -m_maxXCoordinate_mm, m_maxXCoordinate_mm, -m_maxYCoordinate_mm, m_maxYCoordinate_mm );

                if (m_oPaint)
                {
                    auto & rContext = rGeoArrayIn.context();
                    m_scannerPositionActual = rContext.m_ScannerInfo;
                    if (!m_scannerPositionActual.m_hasPosition)
                    {
                        //should not happen, set some reasonable value for safety
                        m_scannerPositionActual.m_x = 0;
                        m_scannerPositionActual.m_y = 0;
                    }

                    // do not add any trafo offset to the coordinates in the paint routine
                    m_oSpTrafo = new interface::LinearTrafo(0,0);
                    //let's compute everything on the canvas, not the roi
                    interface::ImageContext dummyContext(rContext, m_oSpTrafo);
                    auto pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, dummyContext, MeasurementType::Image);

                    //Compute the TCP in the image currently shown
                    auto oSensorCoordinatesActualPositionTCP = getTCPfromCameraPosition_pix (0,0); // on the sensor, before HWROI
                    auto oCanvasCoordinatesActualPositionTCP = geo2d::DPoint{oSensorCoordinatesActualPositionTCP.x - rContext.HW_ROI_x0,
                                                                            oSensorCoordinatesActualPositionTCP.y - rContext.HW_ROI_y0 };

                    m_tcpActualPositionToPaint.x = std::floor( oCanvasCoordinatesActualPositionTCP.x);
                    m_tcpActualPositionToPaint.y = std::floor( oCanvasCoordinatesActualPositionTCP.y);

                    //Compute distance in the scanner reference system [mm] between the input (0,0 mm if the contour is in absolute coordinates) and the current scanner position
                    geo2d::DPoint oScannerPositionDifference_mm = m_scannerInputPosition_at_TCP - geo2d::DPoint{m_scannerPositionActual.m_x, m_scannerPositionActual.m_y};

                    //Compute where would the input be in the current image?
                    auto oCanvasCoordinatesInputPositionTCP = pCoordTransformer->distanceTCPmmToImageCoordCoax( std::vector<geo2d::DPoint>{oScannerPositionDifference_mm},
                                                                                                                oSensorCoordinatesActualPositionTCP.x, oSensorCoordinatesActualPositionTCP.y
                                                                                                              )[0];


                    //Compute distance in the image [pixels] between the input  and the current scanner position
                    geo2d::DPoint oScannerPositionDifference_pix = oCanvasCoordinatesInputPositionTCP - oCanvasCoordinatesActualPositionTCP;

                    m_tcpInputPositionToPaint.x = std::round(oCanvasCoordinatesInputPositionTCP.x);
                    m_tcpInputPositionToPaint.y = std::round(oCanvasCoordinatesInputPositionTCP.y);


                    // Convert distance to scanner input [mm] to distance to current TCP [pixel]
                    // First pretend that there is no input difference (so that the calibration computes valid coordinate),
                    // then apply the pixel offset

                    const auto& calibrationData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);
                    std::vector<double> K;
                    const auto distortionCorrectionEnable = calibrationData.scanfieldDistortionCorrectionFactor(K);
                    const auto &contourWorld = rArrayToProcess.getData();
                    if (distortionCorrectionEnable)
                    {
                        const auto distortionCoefficient = scannerPositionToDistortionCoefficient(m_scannerPositionActual.m_x, m_scannerPositionActual.m_y, K);
                        const auto contourSize = contourWorld.size();
                        auto &contourPixel = m_pointsToPaint.getData();
                        contourPixel.resize(contourSize);
                        for (std::size_t i = 0; i < contourSize; ++i)
                        {
                            //shift input contour based on the two input pipes
                            const auto shiftedContourX = contourWorld[i].x + m_scannerInputPosition_at_TCP.x - m_scannerPositionActual.m_x;
                            const auto shiftedContourY = contourWorld[i].y + m_scannerInputPosition_at_TCP.y - m_scannerPositionActual.m_y;
                            const auto point = worldToPixel(shiftedContourX, shiftedContourY, distortionCoefficient);
                            contourPixel[i] = {
                                point.first + oSensorCoordinatesActualPositionTCP.x,
                                point.second + oSensorCoordinatesActualPositionTCP.y
                            };
                        }
                    }
                    else
                    {
                        m_pointsToPaint.getData() =  pCoordTransformer->distanceTCPmmToImageCoordCoax( rArrayToProcess.getData(),
                            oSensorCoordinatesActualPositionTCP.x, oSensorCoordinatesActualPositionTCP.y);
                        for (auto && rPoint : m_pointsToPaint.getData())
                        {
                            rPoint = rPoint + oScannerPositionDifference_pix;
                        }
                    }

                    //Now that the coordinates are computed, fill the other fields of the TArray
                    if (m_pointsToPaint.getData().size() > 0)
                    {
                        m_pointsToPaint.getRank() = rArrayToProcess.getRank();
                        if (rArrayToProcess.hasScalarData(attributeLaserPower))
                        {
                            m_pointsToPaint.insertScalar(attributeLaserPower);
                            m_pointsToPaint.getScalarData(attributeLaserPower) = rArrayToProcess.getScalarData(attributeLaserPower);
                        }
                    }
                    else
                    {
                        m_pointsToPaint.getRank().clear();
                    }

                }
            }
        break;
        case (InputContourType::pixel):
            {
                auto & rContext = rGeoArrayIn.context();
                auto pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, rContext, filter::MeasurementType::Image);

                //TCP in current ROI [pixel] and in mm according to the camera reference system
                auto oROI_TCP = getTCPfromCameraPosition_pix (rContext.HW_ROI_x0  + m_oSpTrafo->dx() , rContext.HW_ROI_y0 + rContext.HW_ROI_y0 );
                auto oTCPCamera_mm = pCoordTransformer->imageCoordTo3D(oROI_TCP.x, oROI_TCP.y);

                auto computeTCPDistance =  [this, & pCoordTransformer, &oTCPCamera_mm](geo2d::DPoint point)
                {
                    //first get the TCP distance in mm in the camera reference system, then add the scanner position at TCP
                    auto oPointCamera_mm = pCoordTransformer->imageCoordTo3D(point.x, point.y);
                    return geo2d::DPoint{oPointCamera_mm[0] - oTCPCamera_mm[0] + m_scannerInputPosition_at_TCP.x,
                                 oPointCamera_mm[1] - oTCPCamera_mm[1]  + m_scannerInputPosition_at_TCP.y};
                };

                validResult = transformPointsToScanmasterResultArray(m_oResultArray,rArrayToProcess, computeTCPDistance, forceBadRank,
                    -m_maxXCoordinate_mm, m_maxXCoordinate_mm, -m_maxYCoordinate_mm, m_maxYCoordinate_mm);
                if (m_oPaint)
                {
                    m_pointsToPaint =  rArrayToProcess;
                    if (rArrayToProcess.hasScalarData(attributeLaserPower))
                    {
                        m_pointsToPaint.insertScalar(attributeLaserPower);
                        m_pointsToPaint.getScalarData(attributeLaserPower) = rArrayToProcess.getScalarData(attributeLaserPower);
                    }

                    auto oCanvas_TCP = getTCPfromCameraPosition_pix (rContext.HW_ROI_x0, rContext.HW_ROI_y0);
                    m_tcpInputPositionToPaint.x = std::floor( oCanvas_TCP.x);
                    m_tcpInputPositionToPaint.y = std::floor( oCanvas_TCP.y);
                    m_tcpActualPositionToPaint = m_tcpInputPositionToPaint;
                    m_scannerPositionActual = rContext.m_ScannerInfo;
                }
            }
        break;
    }

    assert(m_pointsToPaint.getData().size() == m_pointsToPaint.getRank().size());
    bool emptyInput = rGeoArrayIn.ref()[0].size() == 0;
    if (emptyInput)
    {
        //explicitly set the first rank to 0, to avoid problems in the ResultsServer
        m_oResultArray.assign(1, 0, eRankMin);
        assert(m_oResultArray.getRank().front() == eRankMin);
        assert(m_pointsToPaint.size() == 0);
    }

    if (m_oVerbosity > eHigh)
    {
        std::string resultDescription = "?";

        switch(m_outputType)
        {
            case ScanmasterResultType::SeamWelding:
            case ScanmasterResultType::SeamWeldingAndSendEndOfSeam:
            case ScanmasterResultType::PrepareContour:
                assert(emptyInput || m_oResultArray.size() == SEAMWELDING_RESULT_FIELDS_PER_POINT* rGeoArrayIn.ref()[0].size());
                resultDescription = "Welding";
                break;

            case ScanmasterResultType::ScannerMovingToFirstPoint:
                assert(emptyInput ||  m_oResultArray.size() == SEAMWELDING_RESULT_FIELDS_PER_POINT);
                resultDescription = "Moving";
                break;
            case ScanmasterResultType::Dummy:
                assert(emptyInput || m_oResultArray.size() == SEAMWELDING_RESULT_FIELDS_PER_POINT* rGeoArrayIn.ref()[0].size());
                resultDescription = "Welding Preview";
                break;
        }
        wmLog(eDebug, "Scanmaster Result %s: %f points, first rank %d\n", resultDescription.c_str(), m_oResultArray.size()/double(SEAMWELDING_RESULT_FIELDS_PER_POINT), m_oResultArray.getRank()[0]);

        for (unsigned int i = 0, numPoints = m_oResultArray.size()/SEAMWELDING_RESULT_FIELDS_PER_POINT; i < numPoints; i ++)
        {
            int index = SEAMWELDING_RESULT_FIELDS_PER_POINT * i;
            std::ostringstream oMsg;
            oMsg << "Scanmaster Result " << i <<": "
            << m_oResultArray.getData()[index] <<", " << m_oResultArray.getData()[index+1]
                << " Laser Power "<<  m_oResultArray.getData()[index+2]
                << " Ring Laser Power "<< m_oResultArray.getData()[index+3]
                << " Velocity "<< m_oResultArray.getData()[index+4]<<"\n";
            wmLog(eDebug, oMsg.str());
        }

    }


    const auto	oIsNio = false;
    auto oRankOut = rGeoArrayIn.rank();
    if (oRankOut != 0 && rGeoArrayIn.context().m_transposed)
    {
        wmLog(eWarning, "Result %d contains data from a transposed image, setting rank to 0 \n", oResultType);
        oRankOut = 0.0;
    }
    // Do not include the contour in the result in case of Preview or a bad rank in the array
    // We won't trigger the weld with a bad rank, no point in storing the contour
    if (m_outputType == ScanmasterResultType::Dummy || m_oResultArray.getRank()[0] != eRankMax)
    {
        m_oResultArray.assign(1, 0, eRankMin);
    }
    const auto	oGeoValueOut = GeoDoublearray{ rGeoArrayIn.context(),
                                               m_oResultArray,
                                                oAnalysisResult,
                                                oRankOut };

    auto oResultDoubleOut = ResultDoubleArray{ id(),
        oResultType,
        ResultType::ValueOutOfLimits ,
        rGeoArrayIn.context(),
        oGeoValueOut,
        oMinMaxRange,
        oIsNio };


    oResultDoubleOut.setValid(validResult);

    // signal result
    preSignalAction();
    m_oPipeResultPath_mm.signal(oResultDoubleOut);
} // proceed


void SeamWeldingResult::paint()
{
    using namespace precitec::image;
    if (!m_oPaint)
    {
        return;
    }

    OverlayCanvas	&rCanvas			( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer	&rLayerContour		( rCanvas.getLayerContour());
    OverlayLayer	&rLayerPosition		( rCanvas.getLayerPosition());
	OverlayLayer	&rLayerText			( rCanvas.getLayerText());


    bool inputPositionDifferentFromActual = (m_scannerPositionActual.m_hasPosition
            && !(math::isClose(m_scannerInputPosition_at_TCP.x, m_scannerPositionActual.m_x, 1e-3)
                && math::isClose(m_scannerInputPosition_at_TCP.y, m_scannerPositionActual.m_y, 1e-3)));

    if (inputPositionDifferentFromActual)
    {
        std::ostringstream	oMsg;
        oMsg << std::setprecision(2) << std::fixed << " Input " << m_scannerInputPosition_at_TCP.x << " " << m_scannerInputPosition_at_TCP.y << " mm ";
        rLayerText.add<OverlayText>(oMsg.str(), Font(14), geo2d::Rect( 0, 40, 200, 20), Color::Red());
        rLayerPosition.add<OverlayCross>( m_tcpInputPositionToPaint.x, m_tcpInputPositionToPaint.y, Color::Red());
    }

    //draw in any case the TCP as of the current camera position, yellow if it's different from the input, green if is equal
    {
        std::ostringstream	oMsg;
        oMsg << std::setprecision(2) << std::fixed << " TCP ";
        if (m_scannerPositionActual.m_hasPosition)
        {
            oMsg << "at " << m_scannerPositionActual.m_x << " " << m_scannerPositionActual.m_y ;
        }
        oMsg << " mm ";
        Color oColor = inputPositionDifferentFromActual ? Color::Yellow() : Color::Green();
        rLayerText.add<OverlayText>(oMsg.str(), Font(14), geo2d::Rect( 0, 10 , 200, 20), oColor);
        rLayerPosition.add<OverlayCross>( m_tcpActualPositionToPaint.x, m_tcpActualPositionToPaint.y, oColor);
    }

    if (m_pointsToPaint.size() == 0)
    {
        return;
    }

    auto pointColor = Color::Red();

    Color pointDescriptionColor = pointColor;
    auto lineColor = pointColor;
    lineColor.alpha = 125;

    auto drawPoint = [this, &rLayerPosition, &pointColor] (int point_index)
    {
        auto point = m_pointsToPaint.getData()[point_index];
        point.x += m_oSpTrafo->dx();
        point.y += m_oSpTrafo->dy();

        if (m_pointsToPaint.getRank()[point_index] == 255)
        {
            rLayerPosition.add<OverlayPoint>(point.x, point.y, pointColor);
        }
        else
        {
            rLayerPosition.add<OverlayCircle>(point.x, point.y, 3, pointColor);
        }
        return point;
    };

    auto describePoint = [this, &rLayerText, &pointDescriptionColor] (int point_index)
    {
        //write last point position
        auto point = m_pointsToPaint.getData()[point_index];
        point.x += m_oSpTrafo->dx();
        point.y += m_oSpTrafo->dy();
        std::ostringstream oMsg;
        if (m_outputType == ScanmasterResultType::ScannerMovingToFirstPoint && point_index == 0)
        {
            oMsg << "MOVE TO ";
        }
        else
        {
            oMsg << point_index <<": ";
        }
        oMsg << std::setprecision(4) << m_oResultArray.getData()[point_index*SEAMWELDING_RESULT_FIELDS_PER_POINT] << " " << m_oResultArray.getData()[point_index*SEAMWELDING_RESULT_FIELDS_PER_POINT+1] << " mm";
        rLayerText.add<OverlayText>(oMsg.str(), Font(14), geo2d::Rect(point.x, point.y, 200, 20), pointDescriptionColor);
    };

    int numPoints = m_pointsToPaint.size();        //write last point position

    auto prevPoint = drawPoint(0);
    for (int point_index = 1 ; point_index < numPoints; point_index++)
    {
        auto curPoint = drawPoint(point_index);
        rLayerContour.add<OverlayLine>(prevPoint.x, prevPoint.y, curPoint.x, curPoint.y, lineColor);
        prevPoint = curPoint;
    }
    describePoint (0);
    describePoint (numPoints-1);

}

template<class UnaryFunction>
/*static*/ bool SeamWeldingResult::transformPointsToScanmasterResultArray( geo2d::Doublearray & rResultArray,const geo2d::AnnotatedDPointarray & points, UnaryFunction f, bool forceBadRank,
     double xMin, double xMax, double yMin, double yMax)
{

    const auto & rValIn = points.getData();
    const auto & rRankIn = points.getRank();


    assert(rValIn.size() == rRankIn.size());
    rResultArray.resize(SEAMWELDING_RESULT_FIELDS_PER_POINT*rValIn.size());
    bool coordinatesInRange = true;

    {
        auto itData = rResultArray.getData().begin();
        bool hasLaserPower = points.hasScalarData(attributeLaserPower);
        if (hasLaserPower && points.getScalarData(attributeLaserPower).size() != rValIn.size())
        {
            wmLog(eWarning, "Contour has inconsistent power data %d points %d power values \n",rValIn.size(),points.getScalarData(attributeLaserPower).size());
            hasLaserPower = false;
        }
        const double * pLaserPowerData = hasLaserPower ? points.getScalarData(attributeLaserPower).data() : nullptr;
#ifndef NDEBUG
        const double * pLaserPowerDataEnd = hasLaserPower ? points.getScalarData(attributeLaserPower).data() + points.size() : nullptr;
#endif
        bool hasRingLaserPower = points.hasScalarData(attributeLaserPowerRing);
        if (hasRingLaserPower && points.getScalarData(attributeLaserPowerRing).size() != rValIn.size())
        {
            wmLog(eWarning, "Contour has inconsistent ring power data %d points %d ring power values\n", rValIn.size(), points.getScalarData(attributeLaserPowerRing).size());
            hasRingLaserPower = false;
        }
        const double * pRingLaserPowerData = hasRingLaserPower ? points.getScalarData(attributeLaserPowerRing).data() : nullptr;
#ifndef NDEBUG
        const double * pRingLaserPowerDataEnd = hasRingLaserPower ? points.getScalarData(attributeLaserPowerRing).data() + points.size() : nullptr;
#endif
        bool hasLaserVelocity = points.hasScalarData(attributeLaserVelocity);
        if (hasLaserVelocity && points.getScalarData(attributeLaserVelocity).size() != rValIn.size())
        {
            wmLog(eWarning, "Contour has inconsistent velocity data %d points %d velocity values \n",rValIn.size(),points.getScalarData(attributeLaserVelocity).size());
            hasLaserVelocity = false;
        }
        const double * pLaserVelocityData = hasLaserVelocity ? points.getScalarData(attributeLaserVelocity).data() : nullptr;
#ifndef NDEBUG
        const double * pLaserVelocityDataEnd = hasLaserVelocity ? points.getScalarData(attributeLaserVelocity).data() + points.size() : nullptr;
#endif
        for (auto & rPoint : rValIn)
        {
            auto outPoint = f(rPoint);
            if (outPoint.x <= xMin || outPoint.x >= xMax || outPoint.y <= yMin || outPoint.y >= yMax )
            {
                coordinatesInRange = false;
            }
            *itData = outPoint.x;
            ++itData;
            *itData = outPoint.y;
            ++itData;
            if (pLaserPowerData == nullptr)
            {
                *itData = SCANMASTERWELDINGDATA_UNDEFINEDVALUE; //default value, will be handled by the Scanlab class
            }
            else
            {
                assert(pLaserPowerData != pLaserPowerDataEnd);
                *itData = *pLaserPowerData;
                ++pLaserPowerData;
            }
            ++itData;
            if (pRingLaserPowerData == nullptr)
            {
                *itData = SCANMASTERWELDINGDATA_UNDEFINEDVALUE; //default value, will be handled by the Scanlab class
            }
            else
            {
                assert(pRingLaserPowerData != pRingLaserPowerDataEnd);
                *itData = *pRingLaserPowerData;
                ++pRingLaserPowerData;
            }
            ++itData;
            if (pLaserVelocityData == nullptr)
            {
                *itData = SCANMASTERWELDINGDATA_UNDEFINEDVALUE; //default value, will be handled by the Scanlab class
            }
            else
            {
                assert(pLaserVelocityData != pLaserVelocityDataEnd);
                *itData = *pLaserVelocityData;
                ++pLaserVelocityData;
            }
            ++itData;

        }
        assert(itData == rResultArray.getData().end());
    }

    bool validResult = true;
    if (forceBadRank || !coordinatesInRange || std::find(rRankIn.begin(), rRankIn.end(), eRankMin) !=  rRankIn.end())
    {
        // if any element is not valid, set the whole array as not valid (see also viComunicator::ResultsServer, which checks the first element rank)
        std::fill(rResultArray.getRank().begin(), rResultArray.getRank().end(), eRankMin);
        validResult = false;
    }
    else
    {
        auto itRank = rResultArray.getRank().begin();
        for (auto & rRank : rRankIn)
        {
            for (int i = 0; i < SEAMWELDING_RESULT_FIELDS_PER_POINT; i++)
            {
                *itRank = rRank;
                ++itRank;
            }
        }
        assert(itRank == rResultArray.getRank().end());
    }

    return validResult;
}


geo2d::DPoint SeamWeldingResult::getTCPfromCameraPosition_pix (int HW_ROI_x0, int HW_ROI_y0) const
{
    // we want to use ytcp , so let's set the laser to 1
    geo2d::DPoint oSensorCoordinatesTCP = system::CalibDataSingleton::getCalibrationData ( math::SensorId::eSensorId0 ).getTCPCoordinate ( m_oCounter % g_oNbPar, LaserLine::FrontLaserLine );

    return {oSensorCoordinatesTCP.x - HW_ROI_x0, oSensorCoordinatesTCP.y - HW_ROI_y0};
}


}
}
