#include "testCalibrationGrid.h"
#include "module/moduleLogger.h"
#include "util/calibDataSingleton.h"
#include "math/calibration3DCoords.h"
#include "system/tools.h" //pwd
#include "common/bitmap.h"
#include "common/calibrationConfiguration.h" //toWMCalibFile
#include <overlay/overlayPrimitive.h>
#include <overlay/layerType.h>
#include <overlay/overlayCanvas.h>
#if defined __QNX__
#include "system/toString.h"
#endif

#include <filter/armStates.h>
#include <common/systemConfiguration.h>

#include <fliplib/TypeToDataTypeImpl.h>

namespace precitec {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;

using namespace precitec::interface;
using namespace precitec::geo2d;
using  precitec::image::BImage;
using  precitec::image::Color;

namespace filter {

using math::SensorId;
using precitec::coordinates::CalibrationConfiguration;

const std::string TestCalibrationGrid::m_oFilterName = std::string("TestCalibrationGrid");
const std::string TestCalibrationGrid::m_oPipeXName = std::string("x");


TestCalibrationGrid::TestCalibrationGrid() :
	TransformFilter( TestCalibrationGrid::m_oFilterName, Poco::UUID{"9a7f6a00-6d6b-48aa-b44b-efc7114b09f0"} ),
	m_pPipeInImageFrame( nullptr ),
	m_oPipeOutCoordX(this, TestCalibrationGrid::m_oPipeXName),
	m_oSensorId(SensorId::eSensorId0),
	m_oLaserLine(LaserLine::FrontLaserLine),
	mCalibrationChecked(false),
	m_oCheckerboardSide(2)
{
	parameters_.add("Side", Parameter::TYPE_double, m_oCheckerboardSide);
    m_oCheckerboard.clear();

    const math::CalibrationData & rCalibData(system::CalibDataSingleton::getCalibrationData(m_oSensorId));
    if (!rCalibData.isInitialized())
    {
        wmLog(eInfo, "Filter TestCalibrationGrid constructor: calibration data not fully initialized \n");
    }

    setInPipeConnectors({{Poco::UUID("e3ece676-b00a-441e-bab7-626381ade40c"), m_pPipeInImageFrame, "Image", 0, ""}});
    setOutPipeConnectors({{Poco::UUID("1b997e99-584b-4024-8fbb-899d6e0f82ca"), &m_oPipeOutCoordX, "x", 0, ""}});
    setVariantID(Poco::UUID{"8DF1026B-C8AF-471B-99FD-9694F927B2CC"});
}

TestCalibrationGrid::~TestCalibrationGrid()
{
}


/// Set filter parameters as defined in database / xml file.
void TestCalibrationGrid::setParameter()
{
    TransformFilter::setParameter();
    double prevSide = m_oCheckerboardSide;
    m_oCheckerboardSide = parameters_.getParameter("Side").convert<double>();
    if (m_oCheckerboardSide <= 0.001)
    {
        m_oCheckerboardSide = 0.001;
        wmLog(eInfo, "Checkerboard side set to the minimum value (%d)\n", m_oCheckerboardSide);
    }

    if (m_oCheckerboardSide != prevSide)
    {
        m_oCheckerboard.clear();
    }
    assert((m_oCheckerboardSide == prevSide || !m_oCheckerboard.isValid()) && "checkerboard changed, but next iteration will not trigger recomputation");

    std::ostringstream oMsg;
    system::CalibDataSingleton::getCalibrationData(m_oSensorId).showData(oMsg);
    wmLog(eInfo, "TestCalibrationGrid[setParameter] " + oMsg.str());


}

bool TestCalibrationGrid::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	m_pPipeInImageFrame = dynamic_cast<fliplib::SynchronePipe<ImageFrame> *>(&p_rPipe);
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
}


void TestCalibrationGrid::arm(const fliplib::ArmStateBase& p_rArmstate)
{
    if (p_rArmstate.getStateID() == eSeamStart)
    {
        m_oCheckerboard.clear();
        std::ostringstream oMsg;
        system::CalibDataSingleton::getCalibrationData(m_oSensorId).showData(oMsg);
        wmLog(eInfo, "TestCalibrationGrid[SeamStart] " + oMsg.str());
    }
}

void TestCalibrationGrid::proceed(const void* p_pSender, fliplib::PipeEventArgs& p_rEventArg)
{

	poco_assert_dbg(m_pPipeInImageFrame != nullptr); // to be asserted by graph editor

	auto & r3DCoords = system::CalibDataSingleton::getCalibrationCoords(m_oSensorId);
	std::ostringstream oMsg;
	system::CalibDataSingleton::getCalibrationData(m_oSensorId).showData(oMsg);
	wmLog(eInfo, "TestCalibrationGrid[img %d] " + oMsg.str(), m_oCounter);

	// Empfangenes Frame auslesen
	const ImageFrame & rFrame = m_pPipeInImageFrame->read(m_oCounter);
	const BImage & rImage = rFrame.data();
	const ImageContext & rContext(rFrame.context());

	m_oSpTrafo = rContext.trafo();

	const Trafo & rTrafo(*m_oSpTrafo);
	const Point rTrafoOffset = rTrafo.apply(Point(0,0));
	const int imgWidth = rImage.width();
	const int imgHeight = rImage.height();


	m_oHwRoi.x = rContext.HW_ROI_x0;
	m_oHwRoi.y = rContext.HW_ROI_y0;


    if (!mCalibrationChecked)
    {
        wmLog(eInfo, "Filter TestCalibrationGrid: check validity 3D coordinates \n");
        // expensive operation, to be done only once
        // this can't be done in the constructor, because the filter could be instantiated wheb upgrading the product
        // when the calibration is not fully initialized yet
        m_oDiscontinuityPoints = r3DCoords.checkDiscontinuities();
        const math::CalibrationData & rCalibData(system::CalibDataSingleton::getCalibrationData(m_oSensorId));
        bool ok = rCalibData.checkCalibrationValuesConsistency(rCalibData.CALIB_VALUES_TOL_MIN, eWarning, true);
        if ( !ok )
        {
            wmLog(eWarning, "Inconsistency in calibration values \n");
        }
        mCalibrationChecked = true;
    }

    if (!m_oCheckerboard.isValid())
    {
        auto oSensorSize = r3DCoords.getSensorSize();

        int oCameraInterfaceType = SystemConfiguration::instance().getInt("CameraInterfaceType", 0);

        if ( (oCameraInterfaceType == 0 && (oSensorSize.width != 1024 || oSensorSize.height != 1024) )
            || (oCameraInterfaceType == 1 && (oSensorSize.width != 1280 || oSensorSize.height != 1024))
        )
        {
            wmLog(eWarning, "Unexpected sensor Size: %d, %d\n", oSensorSize.width, oSensorSize.height);
        }


        if ( m_oVerbosity >= eHigh )
        {
            wmLog(eInfo, "Computing equivalent checkerboard with side %f mm\n", m_oCheckerboardSide);
            r3DCoords.coordsToCheckerBoardImage(m_oCheckerboard, Point(0, 0), oSensorSize, m_oCheckerboardSide,
                math::Calibration3DCoords::CoordinatePlaneMode::InternalPlane);

            std::ostringstream oFilename;
            oFilename << "/tmp/" << "CoordinateInternalPlane_" << std::setprecision(2) << m_oCheckerboardSide << ".bmp";
            fileio::Bitmap oBitmap(oFilename.str().c_str(), m_oCheckerboard.width(), m_oCheckerboard.height(), false); //default value of topdown gives mirrored images
            assert(oBitmap.isValid());
            bool ok = oBitmap.isValid() && oBitmap.save(m_oCheckerboard.data());
            if ( ok )
            {
                wmLog(eInfo, "Saved internal plane to %s\n", oFilename.str().c_str());
            }
            else
            {
                wmLog(eInfo, "Error saving %s\n", oFilename.str().c_str());
            }
        }
    }

	//corner points (of the input roi) in current roi coordinates

    auto pCoordsTransformer = system::CalibDataSingleton::getImageCoordsto3DCoordTransformer(math::SensorId::eSensorId0, rContext, m_oLaserLine);
	std::array<Point, eNUMPOINTS> oRoiPoints{};
	oRoiPoints[eTopLeft] = Point(0, 0);
	oRoiPoints[eTopRight] = Point(imgWidth - 1, 0);
	oRoiPoints[eBottomLeft] = Point(0, imgHeight - 1);
	oRoiPoints[eBottomRight] = Point(imgWidth - 1, imgHeight - 1);
	oRoiPoints[eCenter] = Point(imgWidth/2, imgHeight/2);

	for (unsigned int i = 0; i < eNUMPOINTS; i++ )
	{
		auto pointImage = rTrafo.apply(oRoiPoints[i]);  //inside HWROI
		poco_assert_dbg(pointImage.x == (rTrafo.dx() + oRoiPoints[i].x));
		poco_assert_dbg(pointImage.y == (rTrafo.dy() + oRoiPoints[i].y));
		Point pointSensor = Point(pointImage + m_oHwRoi);  //absolute coordinates

		tPhysicalCoord coordTr; //coord Triangulation Point
		tPhysicalCoord coordInt; //coord InternalPlane Point
		bool ok = true;

		ok &= r3DCoords.to3D(coordTr[eX], coordTr[eY], coordTr[eZ], pointSensor.x, pointSensor.y, m_oLaserLine);

		{
			//test alternative computation method

			auto point = pCoordsTransformer->imageCoordTo3D(oRoiPoints[i].x, oRoiPoints[i].y);
			if ( ok )
			{
				assert(point[eX] == coordTr[eX]);
				assert(point[eY] == coordTr[eY]);
				assert(point[eZ] == coordTr[eZ]);
			}
		}

		r3DCoords.getCoordinates(coordInt[eX], coordInt[eY], pointSensor.x, pointSensor.y);
		coordInt[eZ] = 0; //it's a plane


		if ( ok )
		{
			//for every model, x doesn't change between laser plane and internal plane
			poco_assert_dbg(coordInt[eX] == coordTr[eX]);
		}
		else
		{
			coordTr.fill(-1);
		}

		m_oImagePoints[i] = pointImage;
		m_oSensorPoints[i] = pointSensor;
		m_oPointsTriangulation[i] = coordTr;
		m_oPointsInternalPlane[i] = coordInt;
	}

	if ( m_oCounter == 0 )
	{
        //rCalibData.print3DCoords(m_oSensorPoints[eTopLeft], rImage.size());
	}


	geo2d::Doublearray oXOut;
	oXOut.assign(1, m_oPointsInternalPlane[eTopLeft][eX], eRankMax);

	const GeoDoublearray oGeoOutX(rContext, oXOut, rFrame.analysisResult(), eRankMax);

	//debugImage: it's a deep copy of m_oCheckerboard
	if ( m_oVerbosity >= eHigh )
	{
		unsigned int oWidth = rImage.width();
		unsigned int oHeight = rImage.height();

		auto oXStart = m_oSensorPoints[eTopLeft].x;
		auto oXEnd = oXStart + oWidth;
		auto oYStart = m_oSensorPoints[eTopLeft].y;

		m_oImageOut.resize(Size2D(oWidth, oHeight));
		for ( unsigned int j = 0; j < oHeight; ++j )
		{
			std::copy(m_oCheckerboard.rowBegin(j + oYStart)+ oXStart, m_oCheckerboard.rowBegin(j + oYStart)+oXEnd, m_oImageOut.rowBegin(j));
		}

	}
	else
	{
		m_oImageOut.clear();
	}

	preSignalAction();
	m_oPipeOutCoordX.signal(oGeoOutX);


}


void TestCalibrationGrid::paint()
{
	using namespace precitec::image;
	if ( m_oVerbosity < eLow || m_oSpTrafo.isNull() )
	{
		return;
	}

	OverlayCanvas & rCanvas(canvas<OverlayCanvas>(m_oCounter));
	OverlayLayer & rLayerText(rCanvas.getLayerText());
	OverlayLayer & rLayerImage(rCanvas.getLayerImage());
	OverlayLayer & rLayerPosition(rCanvas.getLayerPosition());
	OverlayLayer & rLayerContour(rCanvas.getLayerContour());

	const Trafo &rTrafo(*m_oSpTrafo);

	if ( m_oImageOut.isValid() )
	{
		//image overlay of result
		const Point	oPosition = rTrafo(Point(0, 0));
		const auto	oTitle = OverlayText("Equivalent Checkerboard " + std::to_string(m_oCheckerboardSide) + " mm ",
			Font(), Rect(150, 18), Color::Black());
		rLayerImage.add<OverlayImage>(oPosition, m_oImageOut, oTitle);

		//draw grid (point by point, looking at the gradient line by line
		auto xStart = m_oSensorPoints[PointsElement::eTopLeft].x;
		auto xEnd = m_oSensorPoints[PointsElement::eTopRight].x + 1;
		for ( int j = m_oSensorPoints[PointsElement::eTopLeft].y; j < m_oSensorPoints[PointsElement::eBottomLeft].y; ++j )
		{
			auto previousRowPixel = m_oCheckerboard.rowBegin(j> 0 ? j - 1 : j) + xStart;
			auto pixel = m_oCheckerboard.rowBegin(j) + xStart;
			int lastPixelValue = (*pixel); //signed

			for ( int i = xStart; i < xEnd; ++i, ++pixel, ++previousRowPixel )
			{
				assert(i == xStart || lastPixelValue == m_oCheckerboard.getValue(i - 1, j));
				assert((*pixel) == m_oCheckerboard.getValue(i, j));
				//check gradient horizontally
				if ( lastPixelValue - (*pixel) != 0 )
				{
					rLayerContour.add<OverlayPoint>(i - m_oHwRoi.x, j - m_oHwRoi.y, Color::Yellow());
				}
				else
				{
					//check gradient vertically
					if ( int(*previousRowPixel) - (*pixel) != 0 )
					{
						rLayerContour.add<OverlayPoint>(i - m_oHwRoi.x, j - m_oHwRoi.y, Color::Yellow());
					}
				}
				lastPixelValue = (*pixel);
			}

		}
	}

	for ( auto & point : m_oDiscontinuityPoints )
	{
		rLayerPosition.add<OverlayPoint>(point.ScreenX, point.ScreenY, Color::Red());
	}

	//for ( auto i = 0; i < eNUMPOINTS; i++ )
	for ( auto i : {PointsElement::eTopRight, PointsElement::eBottomLeft} )
	{
		const auto & imagePoint = m_oImagePoints[i];
		const auto & sensorPoint = m_oSensorPoints[i];
		const auto & coordTr = m_oPointsTriangulation[i];

		//text box params
		int offset = 5;
		int p=4;
		const int h = 50;
		const int w = 500;
		const auto font = Font(14);
		const auto color = Color::Green();

		{
			std::ostringstream	oMsg;
			oMsg << std::setprecision(p) << i <<  " Sensor Coord  X: " << sensorPoint.x << "  Y: " << sensorPoint.y << "\n";
			rLayerText.add<OverlayText>(oMsg.str(), font,
				Rect(imagePoint.x, imagePoint.y + offset, w, h), color);
			if ( m_oCounter % 100 == 0 )
			{
				wmLog(eInfo, oMsg.str().c_str());
			}
			offset += h;
		}

		{
			std::ostringstream	oMsg;
			oMsg << std::setprecision(p)  << "World Coords from Laser Line " << laserLineName(m_oLaserLine) <<
				" x: " << coordTr[eX] << "; y: " << coordTr[eY] << "; z: " << coordTr[eZ] << "\n";
			rLayerText.add<OverlayText>(oMsg.str(), font,
				Rect(imagePoint.x, imagePoint.y + offset, w, h), color);
			offset += h;
			if ( m_oCounter % 100 == 0 )
			{
				wmLog(eInfo, oMsg.str().c_str());
			}
		}
	}






}


} // namespaces
}

