/**
* 	@file
* 	@copyright	Precitec Vision GmbH & Co. KG
* 	@author		KIH, Simon Hilsenbeck (HS)
* 	@date		2012
* 	@brief
*/

#include "TCPDistance.h"

#include "filter/algoArray.h"
#include "module/moduleLogger.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "util/calibDataSingleton.h"

#include "math/3D/projectiveMathStructures.h"

#include "coordinates/fieldDistortionMapping.h"

#include <fliplib/TypeToDataTypeImpl.h>

using namespace fliplib;
namespace precitec {
	using namespace interface;
	using namespace image;
	using namespace geo2d;
namespace filter {


const std::string TCPDistance::m_oFilterName 		= std::string( "TCPDistance" );
const std::string TCPDistance::m_oPipeXName			= std::string( "xko" );
const std::string TCPDistance::m_oPipeYName			= std::string( "yko" );


TCPDistance::TCPDistance() :
	TransformFilter		( TCPDistance::m_oFilterName, Poco::UUID{"b7562aa9-415e-4446-a889-9811b6a93352"} ),
	m_pPipeInPosX		( nullptr ),
	m_pPipeInPosY		( nullptr ),
	m_oPipeOutCoordX	( this, TCPDistance::m_oPipeXName ),
	m_oPipeOutCoordY	( this, TCPDistance::m_oPipeYName ),
	m_oSensorSize(1024, 1024),
	m_oTypeOfMeasurement (MeasurementType::LineLaser1),
	m_oComputeDistanceFromScannerCenter(false)
{
    assert(!m_oComputeDistanceFromScannerCenter || m_oTypeOfMeasurement == MeasurementType::Image);

	parameters_.add("TypeOfLaserLine", fliplib::Parameter::TYPE_int, int( m_oTypeOfMeasurement ));

    setInPipeConnectors({{Poco::UUID("ec9c2d24-a976-4ff6-8c0e-f05ae8f4a2cb"), m_pPipeInPosX, "PositionX", 1, "position_x"},
    {Poco::UUID("423F88A4-11BD-405a-A377-6F81B47251BD"), m_pPipeInPosY, "PositionY", 1, "position_y"}});
    setOutPipeConnectors({{Poco::UUID("baedb27e-73f4-42b4-87a9-caa648844fc8"), &m_oPipeOutCoordX, "xko", 0, ""},
    {Poco::UUID("5ff7712c-dd28-4419-8527-137de29b423b"), &m_oPipeOutCoordY, "yko", 0, ""}});
    setVariantID(Poco::UUID("82aea97a-618f-4fcc-8f63-3555d51d366c"));
} // TCPDistance()



void TCPDistance::setParameter() {
	TransformFilter::setParameter();

	m_oSensorSize = system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0).getSensorSize();

	static const int MeasurementTypeDistanceFromScannerCenter = 4;
    static_assert(MeasurementTypeDistanceFromScannerCenter != static_cast<int>(MeasurementType::Image), "conflict with enum class MeasurementType " );

	int intTypeOfLaserLine = parameters_.getParameter("TypeOfLaserLine").convert<int>();
    if (intTypeOfLaserLine == MeasurementTypeDistanceFromScannerCenter)
    {
        m_oTypeOfMeasurement = MeasurementType::Image;
        m_oComputeDistanceFromScannerCenter = true;
    }
    else
    {
        m_oTypeOfMeasurement = static_cast<MeasurementType>(intTypeOfLaserLine);
        if ( (int) m_oTypeOfMeasurement != intTypeOfLaserLine)
        {
            wmLog(eWarning, "Unknown TypeOfLaserLine %d, using %d \n", intTypeOfLaserLine, (int) m_oTypeOfMeasurement );
        }
        m_oComputeDistanceFromScannerCenter = false;
    }
    assert(!m_oComputeDistanceFromScannerCenter || m_oTypeOfMeasurement == MeasurementType::Image);

} // setParameter



void TCPDistance::paint() {
	if(m_oVerbosity < eLow ||  m_oSpTrafo.isNull() || m_oXOut.size() == 0 || m_oYOut.size() == 0 )
    {
		return;
	} // if

    bool resultIsValid = !inputIsInvalid(m_oXOut) && !inputIsInvalid(m_oYOut);
    auto color =  resultIsValid ? Color::Green() : Color { byte(255),byte(0),byte(0), 125 }; // green or semi-transparent red

	const Trafo			&rTrafo			( *m_oSpTrafo );
	OverlayCanvas		&rCanvas		( canvas<OverlayCanvas>(m_oCounter) );
	OverlayLayer		&rLayerLine		( rCanvas.getLayerLine());
	OverlayLayer		&rLayerPosition	( rCanvas.getLayerPosition());

	const Point			oPositionIn		( roundToT<int>(m_oPositionIn.x), roundToT<int>(m_oPositionIn.y) );
    if (resultIsValid)
    {
        rLayerPosition.add<OverlayCross>(rTrafo(oPositionIn), color); // draw cross at in-position
    }

	if(m_oVerbosity < eMedium){
		return;
	} // if

	const Point			oTcpPosition	( roundToT<int>(m_oTcpPosition.x - m_oHwRoi.x), roundToT<int>(m_oTcpPosition.y - m_oHwRoi.y) );

	rLayerLine.add<OverlayLine>(rTrafo(oPositionIn), oTcpPosition, color); // draw line from in-position to tcp-position

    if ( m_oVerbosity < eMax)
    {
        return;
    }

    if (!resultIsValid)
    {
        rLayerPosition.add<OverlayCross>(rTrafo(oPositionIn), color); // draw cross at in-position
    }

    OverlayLayer	&rLayerText( rCanvas.getLayerText() );
    std::ostringstream	oMsg;
    if (m_oComputeDistanceFromScannerCenter)
    {
        oMsg << "Scanner Position: " ;
    }
    else
    {
        oMsg << "Dist P-TCP: " ;
    }
    oMsg << m_oXOut.getData()[0] << " " << m_oYOut.getData()[0];
    int offset = 5;
    int h = 20;
    int w = 400;
    rLayerText.add<OverlayText>( oMsg.str(), Font( 14 ),
        rTrafo( Rect( int(m_oPositionIn.x) + offset, int(m_oPositionIn.y) + offset, w, h ) ), color) ;





} // paint



bool TCPDistance::subscribe(BasePipe& p_rPipe, int p_oGroup) {
	if (p_rPipe.tag() == "position_x") {
		m_pPipeInPosX  = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // if
	else if (p_rPipe.tag() == "position_y") {
		m_pPipeInPosY  = dynamic_cast<scalar_pipe_t*>(&p_rPipe);
	} // else if

	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe



void TCPDistance::proceedGroup(const void* p_pSender, PipeGroupEventArgs& p_rEvent) {
	poco_check_ptr(m_pPipeInPosX != nullptr); // to be asserted by graph editor
	poco_check_ptr(m_pPipeInPosY != nullptr); // to be asserted by graph editor

	const GeoDoublearray&		rGeoPosXIn			( m_pPipeInPosX->read(m_oCounter) );
	const GeoDoublearray&		rGeoPosYIn			( m_pPipeInPosY->read(m_oCounter) );
	const Doublearray&			rPosXIn				( rGeoPosXIn.ref() );
	const Doublearray&			rPosYIn				( rGeoPosYIn.ref() );
	const ImageContext&			rContextX			( rGeoPosXIn.context() );
	const ImageContext&			rContextY			( rGeoPosYIn.context() );

	m_oHwRoi.x	= rContextX.HW_ROI_x0;
	m_oHwRoi.y	= rContextX.HW_ROI_y0;

    const bool measureOnGrayscaleImage = ( m_oTypeOfMeasurement == MeasurementType::Image);
    filter::LaserLine laserLineForTCP = measureOnGrayscaleImage ? filter::LaserLine::FrontLaserLine : static_cast<filter::LaserLine>( m_oTypeOfMeasurement );

    auto pCoordTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, rGeoPosXIn.context(), m_oTypeOfMeasurement );



	m_oSpTrafo	= rGeoPosXIn.context().trafo();

	auto &rCalib( system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0) );

	if(inputIsInvalid(rGeoPosXIn) ||  inputIsInvalid(rGeoPosYIn)) {
		if(m_oVerbosity >= eMedium) {
			wmLog(eInfo, "TCPDistance: Input position invalid.\n");
		} // if

		m_oXOut.assign(1, 0, eRankMin);
		m_oYOut.assign(1, 0, eRankMin);

		const GeoDoublearray		oGeoOutX		( rContextX, m_oXOut, rGeoPosXIn.analysisResult(), NotPresent ); // bad geo rank
		const GeoDoublearray		oGeoOutY		( rContextY, m_oYOut, rGeoPosYIn.analysisResult(), NotPresent ); // bad geo rank

		preSignalAction();
		m_oPipeOutCoordX.signal(oGeoOutX);
		m_oPipeOutCoordY.signal(oGeoOutY);

		return; // return
	} // if

	if (rPosXIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosXIn.size());
	}
	if (rPosYIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosYIn.size());
	}

	if (rGeoPosXIn.context() != rGeoPosYIn.context()) { // contexts expected to be equal
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Different contexts for x and y value: '" << rGeoPosXIn.context() << "', '" << rGeoPosYIn.context() << "'\n";
		wmLog(eWarning, oMsg.str());
	}
	auto hasSampling = [] (const ImageContext & rContext)
    {
        return rContext.SamplingX_ != 1 || rContext.SamplingY_ != 1;
    };

	if (hasSampling(rGeoPosXIn.context()) || hasSampling(rGeoPosYIn.context()))
    {
		std::ostringstream oMsg;
		oMsg << m_oFilterName << ": Sampling in input coordinates not supported \n";
		wmLog(eWarning, oMsg.str());
	}
	m_oPositionIn.x	= rPosXIn.getData().front();
	m_oPositionIn.y	= rPosYIn.getData().front();

	const TPoint<double>	oGlobalPos			( m_oSpTrafo->dx(), m_oSpTrafo->dy() );	//	Offset ROI Koordinaten -> Bildkoordinaten
	const TPoint<double>	oSensorPos			( m_oPositionIn + oGlobalPos + m_oHwRoi );	//	Offset Bildkoordinaten -> Sensorkoordinaten

	math::Vec3D oSensorCoord = pCoordTransformer->imageCoordTo3D((int)(m_oPositionIn.x + 0.5) , (int)(m_oPositionIn.y + 0.5));

	// tcp position
	m_oTcpPosition = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0).getTCPCoordinate(m_oCounter % g_oNbPar, laserLineForTCP);
	math::Vec3D oTcpPosCoord = rCalib.to3D(std::round(m_oTcpPosition.x), std::round(m_oTcpPosition.y), laserLineForTCP);

	double oDeltaMmOutX		( oSensorCoord[0] - oTcpPosCoord[0] );
    m_oXOut.assign(1, oDeltaMmOutX, rPosXIn.getRank().front());

    if (measureOnGrayscaleImage)
    {
        const auto& calibrationData = system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0);

        std::vector<double> K;
        const auto distortionCorrectionEnable = calibrationData.scanfieldDistortionCorrectionFactor(K);

        //for backward compatibility reasons, only perform distortion correction if the key value 'scanfieldDistortionCorrectionEnable' is activated
        if (distortionCorrectionEnable)
        {
            const auto scannerX = rContextX.m_ScannerInfo.m_x;
            const auto scannerY = rContextY.m_ScannerInfo.m_y;
            const auto distortionCoefficient = scannerPositionToDistortionCoefficient(scannerX, scannerY, K);
            const auto measuredPositionWorld = pixelToWorld(m_oPositionIn.x - m_oTcpPosition.x, m_oPositionIn.y - m_oTcpPosition.y, distortionCoefficient);

            oSensorCoord = math::Vec3D(measuredPositionWorld.first, measuredPositionWorld.second, 0);
            oTcpPosCoord = math::Vec3D(0, 0, 0);

            oDeltaMmOutX = oSensorCoord[0] - oTcpPosCoord[0];
            m_oXOut.assign(1, oDeltaMmOutX, rPosXIn.getRank().front());
        }

        double oDeltaMmOutY		( oSensorCoord[1] - oTcpPosCoord[1] ); //Achtung z-Abstand muss hier ausgegeben werden
        if(m_oVerbosity >= eMedium)
        {
            wmLog(eDebug, "Output: Delta x,y to TCP in mm: (%f, %f). distortionCorrectionEnable: %f\n", oDeltaMmOutX, oDeltaMmOutY, distortionCorrectionEnable);
        } // if

        m_oYOut.assign(1, oDeltaMmOutY, rPosYIn.getRank().front());
        if (m_oComputeDistanceFromScannerCenter)
        {
            const auto & rScannerInfo = rContextX.m_ScannerInfo;
            if (rScannerInfo.m_hasPosition)
            {
                m_oXOut.getData().back() += rScannerInfo.m_x;
                m_oYOut.getData().back() += rScannerInfo.m_y;
                if(m_oVerbosity >= eMedium)
                {
                    wmLog(eDebug, "Output: context scanner position: (%f, %f).\n", rScannerInfo.m_x, rScannerInfo.m_y);
                }
            }
            else
            {
               // wmLog(eWarning, "Distance from scanner center requested, but scanner info is not present \n");
                m_oXOut.getRank().back() = eRankMin;
                m_oYOut.getRank().back() = eRankMin;
            }
        }
    }
    else
    {
        assert(!m_oComputeDistanceFromScannerCenter);
        if (rCalib.usesOrientedLineCalibration())
        {
            auto & rScannerInfo = rGeoPosXIn.context().m_ScannerInfo;
            if (! (rScannerInfo.m_hasPosition && math::isClose(rScannerInfo.m_x ,0.0) && math::isClose(rScannerInfo.m_y ,0.0)))
            {
                wmLog(eError, "Filter '%s': Implemented only for scanner position 0,0\n", m_oFilterName.c_str());
            }

        }
        //in case of the oriented line, TCP y can't be used, we use the absolute Z value instead
        const double oDeltaMmOutZ = rCalib.usesOrientedLineCalibration() ? oSensorCoord[2]  : ( oSensorCoord[2] - oTcpPosCoord[2] ); //Achtung z-Abstand muss hier ausgegeben werden

        if(m_oVerbosity >= eMedium){
            wmLog(eDebug, "Filter '%s': Output: Delta x,z to TCP in mm: (%f, %f).\n", m_oFilterName.c_str(), oDeltaMmOutX, oDeltaMmOutZ);
        } // if

        m_oYOut.assign(1, oDeltaMmOutZ, rPosYIn.getRank().front());
    }

    if(m_oVerbosity >= eMedium)
    {
        wmLog(eDebug, "Filter '%s': Hardware ROI: (%f, %f).\n", m_oFilterName.c_str(), m_oHwRoi.x, m_oHwRoi.y);
        wmLog(eDebug, "Filter '%s': Input: Position ROI coordinates: (%f, %f).\n", m_oFilterName.c_str(), m_oPositionIn.x, m_oPositionIn.y);
        wmLog(eDebug, "Filter '%s': Input: Position in mm: (%f, %f,  %f).\n", m_oFilterName.c_str(), oSensorCoord[0], oSensorCoord[1], oSensorCoord[2] );
        wmLog(eDebug, "Filter '%s': Sensor Height: %f, Sensor Width: %f", m_oFilterName.c_str(), m_oSensorSize.height, m_oSensorSize.width);
    }

	const GeoDoublearray	oGeoOutX	( rContextX, m_oXOut, rGeoPosXIn.analysisResult(), rGeoPosXIn.rank() );
	const GeoDoublearray	oGeoOutY	( rContextY, m_oYOut, rGeoPosYIn.analysisResult(), rGeoPosYIn.rank() );

	if (rPosXIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u X values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosXIn.size());
	}
	if (rPosYIn.size() != 1) { // result is always one point
		wmLog(eDebug, "Filter '%s': Received %u Y values. Can only process first element, rest will be discarded.\n", m_oFilterName.c_str(), rPosYIn.size());
	}

	preSignalAction();
	m_oPipeOutCoordX.signal(oGeoOutX);
	m_oPipeOutCoordY.signal(oGeoOutY);

} // proceedGroup


} // namespace filter
} // namespace precitec



