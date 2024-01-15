/*
 * calibrationResult.cpp
 *
 *  Created on: Jun 19, 2013
 *      Author: abeschorner/LB
 */

#include <algorithm>

#include "calibrationResult.h"
#include "overlay/overlayCanvas.h"
#include "overlay/overlayPrimitive.h"
#include "module/moduleLogger.h"
#include "Poco/Thread.h"
#include "util/calibDataParameters.h"
#include <iostream>
#include <fstream>

#include "util/calibDataSingleton.h"
#include "math/mathCommon.h"
#include "math/2D/avgAndRegression.h"
#include "math/2D/LineEquation.h"
#if defined __QNX__
#include "system/toString.h"
#endif

//Pipe connectors
#include <fliplib/TypeToDataTypeImpl.h>
//

namespace
{
	const double evaluationTolerance = 1e-5;
}

namespace precitec {

using fliplib::SynchronePipe;
using fliplib::PipeEventArgs;
using fliplib::PipeGroupEventArgs;
using fliplib::Parameter;
using math::angleUnit;

using namespace precitec::interface;
using namespace precitec::geo2d;

using namespace image;

namespace filter {

const std::string CalibrationResult::m_oFilterName = std::string("CalibrationResult");
const std::string CalibrationResult::m_oResultName = std::string("CalibResults");

CalibrationResult::CalibrationResult() : ResultFilter( CalibrationResult::m_oFilterName, Poco::UUID{"FB33CD6F-BD9D-4F64-9A4F-B86F37131994"} ),
		m_pPipeInTopLayer( nullptr ), m_pPipeInBotLayer(nullptr), m_oPipeOutResults(this, CalibrationResult::m_oResultName ),
			m_oLayerSize( 50 ),
		m_ShowCoordinates(false),
		m_oIsTopLayerHigher(true),
		m_oTypeOfLaserLine(filter::LaserLine::FrontLaserLine)
{
	parameters_.add("LayerSize", Parameter::TYPE_int, m_oLayerSize);

    //gapwidth and gapheight should be parameters
	//default values
	m_oGapWidth = 3;
	m_oGapHeight = 2;
    setInPipeConnectors({{Poco::UUID("B332017C-E3F6-4FA2-BB57-0B0DB23180C5"), m_pPipeInTopLayer, "LineTop", 1, "TopLayer"},
    {Poco::UUID("01D7942F-C978-4A20-8F89-088E2DDFB389"), m_pPipeInBotLayer, "LineBot", 1, "BottomLayer"}});
    setVariantID(Poco::UUID{"B042DB3F-61A1-4948-B3A4-1C927DC49C5E"});
}

CalibrationResult::~CalibrationResult() {
	// TODO Auto-generated destructor stub
}

Point CalibrationResult::getImageCoords(const int & i, const int & j, const ELayer pLayer) const
{
	const interface::Trafo &rTopTrafo(*m_oSpTopTrafo);
	const interface::Trafo &rBotTrafo(*m_oSpBotTrafo);

	auto & rHigherTrafo = m_oIsTopLayerHigher ? rTopTrafo : rBotTrafo;
	auto & rLowerTrafo = m_oIsTopLayerHigher ? rBotTrafo : rTopTrafo;

	//	Offset ROI Koordinaten -> Bildkoordinaten
	geo2d::Point oPoint;

	switch ( pLayer )
	{
		case eHigherLayer:
			oPoint = Point(i + rHigherTrafo.dx(), j + rHigherTrafo.dy());
			break;
		case eLowerLayer:
		default:
			oPoint = Point(i + rLowerTrafo.dx(), j + rLowerTrafo.dy());
			break;
	}
	return oPoint;
};

Point CalibrationResult::getSensorCoords(const int & i, const int & j, const ELayer pLayer) const
{
	//	Offset Bildkoordinaten -> Sensorkoordinaten
	return getImageCoords(i, j, pLayer) + m_oHwRoi;
}


void CalibrationResult::evaluateGapWidth(std::ostringstream & oInfoStream, double & gapWidth3D) const
{
    auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
	gapWidth3D = rCalib.distFrom2D(m_oHighLeft_SensorCoords.x, m_oHighLeft_SensorCoords.y, m_oHighRight_SensorCoords.x, m_oHighRight_SensorCoords.y);


	double oMagnificationError = m_oGapWidth / gapWidth3D;

	oInfoStream << "Nominal Width:Unit.Mm:" << m_oGapWidth << "\n";
	oInfoStream << "Measured Width:Unit.Mm:" << gapWidth3D << "\n";

	if ( std::abs(oMagnificationError - 1) > 0.001  && rCalib.isScheimpflugCase())
    {
        oInfoStream << "Suggested correction x :Unit.None:" << oMagnificationError << "\n"; // ":" is key value delimiter
    }



	auto oLeft_3DCoords = rCalib.to3D(m_oHighLeft_SensorCoords.x, m_oHighLeft_SensorCoords.y, m_oTypeOfLaserLine);
	auto oRight_3DCoords = rCalib.to3D(m_oHighRight_SensorCoords.x, m_oHighRight_SensorCoords.y, m_oTypeOfLaserLine);

	auto oGapHorizontal3D = oRight_3DCoords - oLeft_3DCoords;
    oInfoStream << "Left Point X:Unit.Mm:" << oLeft_3DCoords[0] << "\n";
    oInfoStream << "Left Point Y:Unit.Mm:" << oLeft_3DCoords[1] << "\n";
    oInfoStream << "Left Point Z:Unit.Mm:" << oLeft_3DCoords[2] << "\n";
    oInfoStream << "Right Point X:Unit.Mm:" << oRight_3DCoords[0] << "\n";
    oInfoStream << "Right Point Y:Unit.Mm:" << oRight_3DCoords[1] << "\n";
    oInfoStream << "Right Point Z:Unit.Mm:" << oRight_3DCoords[2] << "\n";

	//in the scheimpflug case, the internal plane is the laser plane
// 	//in the coax case, the internal plane is the horizontal plane, I can't compare the these distances unless the line line is perfectly horizontal
    if ( rCalib.isScheimpflugCase())
	{
        double gapWidth2D = rCalib.distanceOnInternalPlane( m_oHighLeft_SensorCoords.x, m_oHighLeft_SensorCoords.y, m_oHighRight_SensorCoords.x, m_oHighRight_SensorCoords.y);   //with internal plane coordinates
        if ( std::abs(gapWidth2D - gapWidth3D) > evaluationTolerance )
        {
            //it should never be here, maybe there is a division by zero somewhere
            wmLog(eWarning, "Inconsistent Calibration GapWidth length %f %f", gapWidth2D, gapWidth3D);
        }

        if ( std::abs(gapWidth2D - std::sqrt(oGapHorizontal3D.norm2())) > evaluationTolerance )
        {
            //it should never be here, maybe there is a division by zero somewhere
            wmLog(eWarning, "Inconsistent Calibration GapWidth 3D %f %f", gapWidth2D, std::sqrt(oGapHorizontal3D.norm2()));
        }
    }
	oInfoStream << "Gap Width H:Unit.Pixels:" << m_oHighRight_SensorCoords.x - m_oHighLeft_SensorCoords.x << "\n";
	oInfoStream << "Gap Width V:Unit.Pixels:" << m_oHighRight_SensorCoords.y - m_oHighLeft_SensorCoords.y << "\n";
	oInfoStream << "Gap Width x:Unit.Mm:" << oGapHorizontal3D[0] << "\n";
	oInfoStream << "Gap Width y:Unit.Mm:" << oGapHorizontal3D[1] << "\n";
	oInfoStream << "Gap Width z:Unit.Mm:" << oGapHorizontal3D[2] << "\n";

}


void CalibrationResult::evaluateSlopes(std::ostringstream & oInfoStream, std::array<math::LineEquation, 2> & oSlopes) const
{
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
	std::array<math::LineEquation, 2>  oSlopesInImage;

	for ( auto layer : {eHigherLayer, eLowerLayer} )
	{
		//compute p_rSlope, p_rIntercept, oRegCoeff
		auto & rLayerPoints = (layer == eHigherLayer ? m_oHigherLayerPoints : m_oLowerLayerPoints);
		assert(rLayerPoints.size() % 2 == 0);
		auto numPoints = rLayerPoints.size() / 2;

		//the elements of rLayerPoints are: x0,y0,x1,y1,x2, y2 ..
		std::vector<double> oXCoordInternPlane(numPoints);
		std::vector<double> oYCoordInternPlane(numPoints);
		std::vector<double> oXCoordImage(numPoints);
		std::vector<double> oYCoordImage(numPoints);

		size_t indexPoints = 0;
		for ( size_t indexLayer = 0;
			indexLayer < rLayerPoints.size();
			indexLayer += 2 )
		{
			assert(indexPoints == 0 || indexPoints + 1 == indexLayer / 2);
			indexPoints = indexLayer / 2;
			auto oSensorPoint = getSensorCoords(int(rLayerPoints[indexLayer]), int(rLayerPoints[indexLayer + 1]), layer);
			float oX, oY;
            rCalib.getCoordinates(oX, oY, oSensorPoint.x, oSensorPoint.y); //coordinated on internal plane, no triangulation
			//should check if valid, see getCoordinates
			oXCoordInternPlane[indexPoints] = double(oX);
			oYCoordInternPlane[indexPoints] = double(oY);
			oXCoordImage[indexPoints] = oSensorPoint.x;
			oYCoordImage[indexPoints] = oSensorPoint.y;
		}
		assert(indexPoints + 1 == numPoints); //indexPoints 0 based

		double oSlope, oIntercept, oRegCoeff;
		math::linearRegression2D(oSlope, oIntercept, oRegCoeff, indexPoints, oXCoordInternPlane.data(), oYCoordInternPlane.data());
		double a, b, c;
		math::orthogonalLinearRegression2D(a, b, c, indexPoints, oXCoordInternPlane.data(), oYCoordInternPlane.data());
		assert(std::abs(-a / b - oSlope) < 1e-3);
		oSlopes[layer] = math::LineEquation(a, b, c);
		oSlopesInImage[layer] = math::LineEquation(oXCoordImage, oYCoordImage);
	} //end compute slope for each layer

	wmLog(eInfo, "Fit Line on pixels: higher layer rotation %f deg, intercept %f\n", oSlopesInImage[eHigherLayer].getInclinationDegrees(), oSlopesInImage[eHigherLayer].getY(0));
	wmLog(eInfo, "Fit Line on pixels: lower layer rotation %f deg, intercept %f\n", oSlopesInImage[eLowerLayer].getInclinationDegrees(), oSlopesInImage[eLowerLayer].getY(0));

	//evaluate the slopes, compare with the one computed with the endpoint
	bool slopeOk = true;
	{
		float oX0, oY0, oX1, oY1;
		rCalib.getCoordinates(oX0, oY0, m_oHighLeft_SensorCoords.x, m_oHighLeft_SensorCoords.y);
		rCalib.getCoordinates(oX1, oY1, m_oHighRight_SensorCoords.x, m_oHighRight_SensorCoords.y);
		float oHighSlope = (oY1 - oY0) / (oX1 - oX0);
		double oAngleHigherDeg = std::atan(oHighSlope) * 180 / math::pi;
		if ( std::abs(oAngleHigherDeg - oSlopes[eHigherLayer].getInclinationDegrees()) > 0.01 )
		{
			wmLog(eInfo, "Error in regression %f %f\n", oAngleHigherDeg, oSlopes[eHigherLayer].getInclinationDegrees());
			slopeOk = false;
		}
	}
	if ( slopeOk )
	{
		auto oAngleHigherLayer = oSlopes[eHigherLayer].getInclinationDegrees();
		auto oAngleLowerLayer = oSlopes[eLowerLayer].getInclinationDegrees();
		if ( std::abs(oAngleHigherLayer - oAngleLowerLayer) > 0.5 )
		{
			wmLog(eInfo, "Laser lines not parallel (%f, %f, delta: %f) \n", oAngleHigherLayer , oAngleLowerLayer, oAngleHigherLayer - oAngleLowerLayer);
		}

		if ( std::abs(oAngleHigherLayer) > 5 )
		{
			wmLog(eInfo, "Horizontal axis rotated by %f degrees", oAngleHigherLayer);
		}

	}
}


void CalibrationResult::evaluateGapHeight(std::ostringstream & oInfoStream, int & gapHeightPixel,  double & gapHeightZ, double & gapHeight3D,
	const std::array<math::LineEquation, 2> & oSlopes) const
{
	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));

	geo2d::Point  oMidPointHigh = (m_oHighLeft_SensorCoords + m_oHighRight_SensorCoords) / 2;
	geo2d::Point  oMidPointLow = (m_oLowLeft_SensorCoords + m_oLowRight_SensorCoords) / 2;

	auto oMidPointLow3D = rCalib.to3D(oMidPointLow.x, oMidPointLow.y, filter::LaserLine::FrontLaserLine );
	auto oMidPointHigh3D = rCalib.to3D(oMidPointHigh.x, oMidPointHigh.y, filter::LaserLine::FrontLaserLine);

    gapHeightPixel = std::abs(oMidPointLow.y - oMidPointHigh.y);

    oInfoStream << "Low Point X:Unit.Mm:" << oMidPointLow3D[0] << "\n";
    oInfoStream << "Low Point Y:Unit.Mm:" << oMidPointLow3D[1] << "\n";
    oInfoStream << "Low Point Z:Unit.Mm:" << oMidPointLow3D[2] << "\n";
    oInfoStream << "High Point X:Unit.Mm:" << oMidPointHigh3D[0] << "\n";
    oInfoStream << "High Point Y:Unit.Mm:" << oMidPointHigh3D[1] << "\n";
    oInfoStream << "High Point Z:Unit.Mm:" << oMidPointHigh3D[2] << "\n";

	//compare plausibility of the simple computation of midpoints with fitted lines
	{
        float oHigh_InternalPlane_x, oHigh_InternalPlane_y;
        rCalib.convertScreenToLaserPlane(oHigh_InternalPlane_x, oHigh_InternalPlane_y, oMidPointHigh.x, oMidPointHigh.y, m_oTypeOfLaserLine);
		//here slopes and projections are on the laser plane
		double oMidPointHighY, oMidPointHighX;
		oSlopes[eHigherLayer].project(oMidPointHighX, oMidPointHighY, oHigh_InternalPlane_x, oHigh_InternalPlane_y);

		double oMidPointProjectedX, oMidPointProjectedY;
		oSlopes[eLowerLayer].project(oMidPointProjectedX, oMidPointProjectedY, oMidPointHighX, oMidPointHighY);

		float xH, yH, xL, yL;
		rCalib.getCoordinates(xH, yH, oMidPointHigh.x, oMidPointHigh.y);
		rCalib.getCoordinates(xL, yL, oMidPointLow.x, oMidPointLow.y);

		auto dH = oSlopes[eHigherLayer].distance(xH, yH);
		auto dL = oSlopes[eLowerLayer].distance(xL, yL);
		if ( std::abs(dH) > 1 )
		{
			wmLog(eInfo, "Noisy laser line higher: avg point %f %f is not on the fitted line (distance %f) \n", oMidPointHigh.x, oMidPointHigh.y, oMidPointHighX, oMidPointHighY, dH);
			assert(false);
		}

		if ( std::abs(dL) > 1 )
		{
			wmLog(eInfo, "Noisy laser line lower: avg point %f %f is not on the fitted line (distance %f)  \n", oMidPointLow.x, oMidPointLow.y, oMidPointProjectedX, oMidPointProjectedY, dL);
			assert(false);
		}


	}

	gapHeightZ = oMidPointHigh3D[2] - oMidPointLow3D[2];

	double oMagnificationErrorZ = m_oGapHeight / gapHeightZ;
	auto oGapVertical3D = rCalib.to3D(oMidPointLow.x, oMidPointLow.y,filter::LaserLine::FrontLaserLine) - rCalib.to3D(oMidPointHigh.x, oMidPointHigh.y, filter::LaserLine::FrontLaserLine);
    gapHeight3D = rCalib.distFrom2D(oMidPointHigh.x, oMidPointHigh.y, oMidPointLow.x, oMidPointLow.y);

	oInfoStream << "Gap Height H:Unit.Pixels:" << oMidPointLow.x - oMidPointHigh.x << "\n";
	oInfoStream << "Gap Height V:Unit.Pixels:" << oMidPointLow.y - oMidPointHigh.y << "\n";


	//in the scheimpflug case, the internal plane is the laser plane
	//in the coax case, the internal plane is the horizontal plane, I can't compare the these distances

	if ( rCalib.isScheimpflugCase())
	{
        double gapHeight2D = rCalib.distanceOnInternalPlane(oMidPointHigh.x, oMidPointHigh.y, oMidPointLow.x, oMidPointLow.y);   //with internal plane coordinates
		if ( std::abs(gapHeight2D - gapHeight3D) > evaluationTolerance )
		{
			//it should never be here, maybe there is a division by zero somewhere
			wmLog(eWarning, "Inconsistent Calibration GapHeight %f %f", gapHeight2D, gapHeight3D);
			assert(false);
		}
		oInfoStream << "Gap Laser Dist:Unit.Mm: " << gapHeight2D << "\n";

	}

	oInfoStream << "Triangulation Angle[" <<  (int)(m_oTypeOfLaserLine) << "] Degrees :Unit.None:" << rCalib.getTriangulationAngle(angleUnit::eDegrees, m_oTypeOfLaserLine) << "\n";
	oInfoStream << "Gap Height x:Unit.Mm: " << oGapVertical3D[0] << "\n";
	oInfoStream << "Gap Height y:Unit.Mm: " << oGapVertical3D[1] << "\n";
	oInfoStream << "Gap Height z:Unit.Mm: " << oGapVertical3D[2] << "\n";
	oInfoStream << "Nominal Height:Unit.Mm:" << m_oGapHeight << "\n";
	oInfoStream << "Measured Height:Unit.Mm:" << gapHeightZ << "\n";
	if ( std::abs(oMagnificationErrorZ - 1) > 0.001 && rCalib.isScheimpflugCase() )
	{
		oInfoStream << "Suggested correction z:Unit.None:" << oMagnificationErrorZ << "\n";
	}
}


//used for evaluation of the result
void CalibrationResult::evaluateCurrentCalibration(double & gapWidth2D, double & gapHeightZ)
{

	assert(m_oGapWidth > 1e-3);
	assert(m_oGapHeight > 1e-3);

	auto &rCalib(system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0));
    std::string oSensorType = rCalib.isScheimpflugCase() ? "Scheimpflug": "Coax";

    wmLog(eInfo, "Evaluating current calibration of type %s, assuming target with gap  %f,%f (mm) \n", oSensorType.c_str(), m_oGapWidth,  m_oGapHeight  );


	std::ostringstream oInfoStream;

	std::array<math::LineEquation, 2> oSlopes;
	evaluateSlopes(oInfoStream, oSlopes);

	/*
	* Evaluate Magnification
	*/
	evaluateGapWidth(oInfoStream, gapWidth2D);

	/*
	* Compute slope in internal plane coordinates (shows if the plane coordinate has a rotation, when the x axis is not parallel to pixels)
	*/
	double gapHeight2D;
    int gapHeightPixel(0);
	evaluateGapHeight(oInfoStream, gapHeightPixel, gapHeightZ, gapHeight2D, oSlopes);


    if (!rCalib.isScheimpflugCase())
    {
        //simplified version of CalibrateIbOpticalSystem::performMagnificationComputation
        auto  oCoaxParams(system::CalibDataSingleton::getCalibrationData(math::SensorId::eSensorId0).getCoaxCalibrationData());
        double oSuggestedBeta0 = (m_oHighRight_SensorCoords.x - m_oHighLeft_SensorCoords.x) * oCoaxParams.m_oDpixX / m_oGapWidth;
        double oSuggestedBetaz = gapHeightPixel * oCoaxParams.m_oDpixY/m_oGapHeight;
        double oSuggestedTriangulationAngle = std::atan2(oSuggestedBetaz, oSuggestedBeta0);
        wmLog(eInfo , "Suggested Coax Parameters: beta0 = %f betaZ = %f (angle = %f deg)\n",
              oSuggestedBeta0, oSuggestedBetaz, math::radiansToDegrees(oSuggestedTriangulationAngle));
    }
	wmLog(eInfo, "Internal Plane: W=%f H=%f\n", gapWidth2D, gapHeight2D);
	wmLog(eInfo, "3D: angle=%f\n", rCalib.getTriangulationAngle(angleUnit::eDegrees, m_oTypeOfLaserLine));

	m_oInfo += oInfoStream.str();
	std::array<int, 2 > key = {m_oHighLeft_SensorCoords.x, m_oHighLeft_SensorCoords.y};
	std::array<double, 2>  currentEvaluation = { gapWidth2D, gapHeightZ  };
	m_oEvaluationStats[key].push_back(currentEvaluation);

	double oSumWidth = 0;
	double oSumHeight = 0;
	int numMeasurements = 0;
	int numSeparateMeasurements = 0;
	for ( auto & evaluationSet : m_oEvaluationStats )
	{
		auto & rPoint = evaluationSet.first;
		auto & rMeasurements = evaluationSet.second;

		numSeparateMeasurements++;
		double oCurrSumWidth = 0;
		double oCurrSumHeight = 0;
		for ( auto & evaluationValues : rMeasurements )
		{
			numMeasurements++;
			oCurrSumWidth += evaluationValues[0];
			oCurrSumHeight += evaluationValues[1];
		}
		auto numCurrentMeas = rMeasurements.size();
		double oCurrWidth = oCurrSumWidth / numCurrentMeas;
		double oCurrHeight = oCurrSumHeight / numCurrentMeas;

		oSumWidth += oCurrWidth;
		oSumHeight += oCurrHeight;
		wmLog(eInfo, "Avg on %d measurements at %d %d: Gap Width %f  Gap Height %f\n",
			 numCurrentMeas, rPoint[0], rPoint[1], oCurrWidth, oCurrHeight);
	}
	auto oAvgGapWidth = oSumWidth / numSeparateMeasurements;
	auto oAvgGapHeight = oSumHeight / numSeparateMeasurements;
	wmLog(eInfo, "Avg on %d measurements on different points: Gap Width %f  Gap Height %f\n",
		numSeparateMeasurements, oAvgGapWidth, oAvgGapHeight);
	oInfoStream << "Avg Gap Width:Unit.Mm:" << oAvgGapWidth << "\n";
	oInfoStream << "Avg Gap Height:Unit.Mm:" << oAvgGapHeight << "\n";

}

void CalibrationResult::paint()
{
	if ( (m_oEndpointsHigher.size() != 2) || (m_oEndpointsLower.size() != 2) )
	{
		return;
	}

	if(m_oVerbosity >= eLow)
	{
		OverlayCanvas & rCanvas( canvas<OverlayCanvas>(m_oCounter) );
		OverlayLayer & rLayer( rCanvas.getLayerPosition() ); // layer 0
		OverlayLayer & rLayerInfoBox( rCanvas.getLayerInfoBox() );
		OverlayLayer & rLayerText(rCanvas.getLayerText());


		interface::Trafo &rTopTrafo ( *m_oSpTopTrafo );

		//nb: here the points are in the image ref system, in evaluateCalibration in the sensor ref system
		geo2d::Point oHighLeft(getImageCoords(m_oEndpointsHigher[0].m_oXPos, m_oEndpointsHigher[0].m_oYPos, eHigherLayer));
		geo2d::Point oHighRight(getImageCoords(m_oEndpointsHigher[1].m_oXPos, m_oEndpointsHigher[1].m_oYPos, eHigherLayer));
		geo2d::Point oLowLeft(getImageCoords(m_oEndpointsLower[0].m_oXPos, m_oEndpointsLower[0].m_oYPos, eLowerLayer));
		geo2d::Point oLowRight(getImageCoords(m_oEndpointsLower[1].m_oXPos, m_oEndpointsLower[1].m_oYPos, eLowerLayer));

		rLayer.add<OverlayCross>(oHighLeft , Color::Red() );
		rLayer.add<OverlayCross>(oHighRight, Color::Red());
		rLayer.add<OverlayCross>(oLowLeft, Color::Yellow());
		rLayer.add<OverlayCross>(oLowRight, Color::Yellow());

		if ( m_ShowCoordinates )
		{

			std::string oLaserLine = laserLineName(m_oTypeOfLaserLine);

			auto & rCalib = system::CalibDataSingleton::getCalibrationCoords(math::SensorId::eSensorId0);

			//warning copy-paste, here hte points are in the image ref system
			geo2d::Point  oMidPointLow = (oLowLeft + oLowRight) / 2;
			geo2d::Point  oMidPointHigh = (oHighLeft + oHighRight) / 2;

			const auto	oTextFont = Font(16);
			const auto	oTextSize = Size(500, 20);

			double gapWidth2D, gapHeightZ;
			evaluateCurrentCalibration(gapWidth2D, gapHeightZ);


			OverlayLayer &rLayerContour(rCanvas.getLayerContour());
			rLayerContour.add<OverlayLine>(oHighLeft, oHighRight, Color::Red());
			rLayerContour.add<OverlayLine>(oMidPointLow, oMidPointHigh, Color::Magenta());
			rLayerContour.add<OverlayText>(
				"W=" + std::to_string(gapWidth2D),
				oTextFont, Rect(oHighRight + Point(-20, -20), oTextSize), Color::Red());
			rLayerContour.add<OverlayText>(
				" DZ= " + std::to_string(gapHeightZ),
				oTextFont, Rect(oMidPointLow + Point(0, 20), oTextSize), Color::Magenta());

			std::vector<OverlayText> oFeatureLines;
			{
				std::stringstream lineStream(m_oInfo);
				std::string tok;
				int i = 0;
				while ( std::getline(lineStream, tok, '\n') )
				{
					wmLog(eInfo, "%s\n",  tok.c_str());
					oFeatureLines.push_back(OverlayText( tok, Font(12), Rect(oHighRight, Size(5000, 20)), Color::Red(), i ));
					i++;
				}

			}

			rLayerInfoBox.add<OverlayInfoBox>(image::eSurface, m_oCounter, std::move(oFeatureLines), Rect(oHighRight, Size(5000,500)));

			if ( m_oVerbosity >= eMax )
			{
				rLayerText.add<OverlayText>(
                    "Coordinate on the internal plane " + std::string(rCalib.isScheimpflugCase() ? "laser" : "horizontal"),
					oTextFont, rTopTrafo(Rect(Point(0, 0), oTextSize)), Color::Orange());

				rLayer.add<OverlayText>(
					"Coordinate 3D on " + oLaserLine + " laser line",
					oTextFont, rTopTrafo(Rect(Point(0, 30), oTextSize)), Color::Green());

				for ( auto oPoint : {oHighLeft, oHighRight, oLowLeft, oLowRight} )
				{
					auto oSensorPoint = oPoint + m_oHwRoi;
					float oX, oY;
					rCalib.getCoordinates(oX, oY, oSensorPoint.x, oSensorPoint.y);
					auto o3DCoord = rCalib.to3D(oSensorPoint.x, oSensorPoint.y, filter::LaserLine::FrontLaserLine);

					std::ostringstream oMsg, oMsg1, oMsg2;

					oMsg1 << std::setprecision(5) << oX << " " << oY;
					oMsg2 << std::setprecision(5) << o3DCoord[0] << " " << o3DCoord[1] << " " << o3DCoord[2];
					int oTextLength = std::max(oMsg1.str().size(), oMsg2.str().size());

					const auto oBoxCenter = Point(oPoint.x - oTextLength / 2, oPoint.y);
					const auto	oTextBox = Rect(oBoxCenter, Size(20 * oTextLength, 20));

					rLayerText.add<OverlayText>(
						oMsg1.str().c_str(), oTextFont, oTextBox, Color::Orange());

					const auto	oTextBox2 = Rect(Point(oBoxCenter.x, oBoxCenter.y + 20), oTextBox.size());

					rLayer.add<OverlayText>(
						oMsg2.str().c_str(),
						oTextFont, oTextBox2, Color::Green());


					{
						oMsg << "POINT " << oSensorPoint.x << " " << oSensorPoint.y << "(sensor) ;"
							<< std::string(rCalib.isScheimpflugCase() ? "laser": "horizontal" ) << " plane: "
							<< oMsg1.str() << ";"
							<< "3D laser" << oLaserLine << " : " << oMsg2.str() << std::endl;
						wmLog(eInfo, oMsg.str().c_str());
					}
				}
			}

		} //end show coordinates
	}


}

bool CalibrationResult::subscribe(fliplib::BasePipe& p_rPipe, int p_oGroup)
{
	if (p_rPipe.tag() == "TopLayer")
	{
		m_pPipeInTopLayer = dynamic_cast< fliplib::SynchronePipe < interface::GeoVecDoublearray >* >(&p_rPipe);
	} else if (p_rPipe.tag() == "BottomLayer")
	{
		m_pPipeInBotLayer = dynamic_cast< fliplib::SynchronePipe< interface::GeoVecDoublearray >* >(&p_rPipe);
	}
	return BaseFilter::subscribe( p_rPipe, p_oGroup );
} // subscribe

/// Set filter parameters as defined in database / xml file.
void CalibrationResult::setParameter()
{
	m_oEvaluationStats.clear();

	ResultFilter::setParameter();
	m_oLayerSize = parameters_.getParameter("LayerSize").convert<int>();


	//temporary workaround, instead of m_ShowCoordinates = parameters_.getParameter("ShowCoordinates").convert<bool>();
	if (m_oVerbosity >= eHigh )
	{
		m_ShowCoordinates = true;
	}

}


// ------------------------------------------------------------------------------------------

int CalibrationResult::binarizeRank(const int oRank)
{
	return oRank < (eRankMax/8) ? eRankMin : eRankMax;
}

auto CalibrationResult::testForEndpoint(const std::vector<int> &p_rRank, const unsigned int p_oPos) -> tEndpoint
{
	// checks for constant rank at positions p_oPos-m_oLayerSize+1...p_opos and for constant (opposite) rank at positions p_oPos+1...p_oPos+m_oLayerSize
	const int oRank = binarizeRank(p_rRank[p_oPos]);

	bool oLeftOK = true; bool oRightOK = true;
	for (unsigned int i=p_oPos; i > p_oPos-m_oLayerSize; --i)
	{
		oLeftOK = oLeftOK && (binarizeRank(p_rRank[i]) == oRank);
	}
	for (unsigned int i=p_oPos+1; i <= p_oPos+m_oLayerSize; ++i)
	{
		oRightOK = oRightOK && (binarizeRank(p_rRank[i]) != oRank);
	}

	if (!oLeftOK || !oRightOK)
	{
		return eEndpointNone;
	}
	if (oRank > eRankMin)
	{
		return eEndpointRight; // _______+-----------  the plus marks the spot...
	}
	return eEndpointLeft; // -----------+_______
}


bool CalibrationResult::findSegment(std::vector<Endpoint> &p_rEndpoints, const unsigned int p_oLeft, const unsigned int p_oRight,
	const VecDoublearray &p_rLine)
{
	// oLeft, oRight: endpoint[0] and endpoint[1] of top layer line
	auto oRank = p_rLine[0].getRank();
	unsigned int oPos = p_oLeft;
	int oFound = 0; int oEnd = 0;
	unsigned int oCnt = 0;

	p_rEndpoints.resize(2); p_rEndpoints.assign(2, Endpoint(0, 0, eEndpointNone));

	while ( (oPos < p_oRight) && (oFound != 2) )
	{
		oCnt = (oRank[oPos] > eRankMin) ? oCnt+1 : 0;
		if (oCnt > 0)
		{
			oEnd = oCnt;
		}
		if (oCnt == 1)
		{
			p_rEndpoints[0].m_oXPos = oPos;
			oEnd = 1;
		}
		if (oFound > 0)
		{
			if ( (oPos >= (p_oRight-1)) ||(oCnt == 0) )
			{
				oFound = 2;
			}
		} else
		{
			oFound = (int)(oCnt >= m_oLayerSize);
		}
		++oPos;
	}
	if (oFound)
	{
		// [0].Xpos already set above
		p_rEndpoints[0].m_oYPos = p_rLine[0].getData()[p_rEndpoints[0].m_oXPos];
		p_rEndpoints[0].m_oType = eEndpointLeft;

		p_rEndpoints[1].m_oXPos = (p_rEndpoints[0].m_oXPos + oEnd- 1);
		p_rEndpoints[1].m_oYPos = p_rLine[0].getData()[p_rEndpoints[1].m_oXPos];
		p_rEndpoints[1].m_oType = eEndpointRight;
	}

	return oFound != 0;
}

void CalibrationResult::findEndPointsHigherLayer(std::vector<Endpoint> &p_rEndpoints, const VecDoublearray &p_rLine)
{
	auto oData = p_rLine[0].getData(); auto oRank = p_rLine[0].getRank();
	tEndpoint oEndpointType;

	unsigned int oMaxTemp = 0;
	if( oData.size() > m_oLayerSize)
	{
		oMaxTemp = oData.size() - m_oLayerSize;
	}

	for (unsigned int i=m_oLayerSize; i < oMaxTemp; ++i) // ignore left- and rightmost pixels
	{
		oEndpointType = testForEndpoint(oRank, i);
		if (oEndpointType != eEndpointNone)
		{
			int oLeftOffset = (int)(oEndpointType == eEndpointLeft);
			if (oEndpointType)
			p_rEndpoints.push_back(Endpoint(i, oData[i+oLeftOffset], oEndpointType));  // left endpoint is the one left to the line start, hence the offset
		}
	}
}

bool CalibrationResult::collectLayerPoints(const VecDoublearray &p_rLineHigher, const VecDoublearray &p_rLineLower)
{
	auto oRankHigher = p_rLineHigher[0].getRank(); auto oDataHigher = p_rLineHigher[0].getData();
	auto oRankLower = p_rLineLower[0].getRank(); auto oDataLower = p_rLineLower[0].getData();

	// For higher layer: First end point is a left one and we should be able to get a few adjacent points from the laser line to its left
	unsigned int oX = m_oEndpointsHigher[0].m_oXPos;
	//while ( (oX > m_oLayerSize) && (oRankHigher[oX] > eRankMin) )
	while ((oX > 0) && (oRankHigher[oX] > eRankMin))
	{
		m_oHigherLayerPoints.push_back((double)oX); m_oHigherLayerPoints.push_back(oDataHigher[oX]);
		--oX;
	}
	oX = m_oEndpointsHigher[1].m_oXPos+1;
	while ((oX < oRankHigher.size()-1) && (oRankHigher[oX] > eRankMin))
	{
		m_oHigherLayerPoints.push_back((double)oX); m_oHigherLayerPoints.push_back(oDataHigher[oX]);
		++oX;
	}

	for (unsigned int i=m_oEndpointsLower[0].m_oXPos; i <= m_oEndpointsLower[1].m_oXPos; ++i)
	{
		m_oLowerLayerPoints.push_back( 1.0*i ); m_oLowerLayerPoints.push_back(oDataLower[i]);
	}

	 // at least 3 points for each layer should be there... Should be at least m_oLayerSize...
	return ((m_oHigherLayerPoints.size() > 2) && (m_oLowerLayerPoints.size() > 2));
}

void CalibrationResult::invalidateResult( std::vector<double> &p_rResHeader, std::vector<double> &p_rResElements )
{
	p_rResHeader.clear();
	p_rResHeader.resize( precitec::calibration::eNumberOfComponentsOfHeader );
	p_rResHeader[precitec::calibration::eResultOK] = 0;
	p_rResElements.clear();
	p_rResElements.resize(0 );
}

void CalibrationResult::createResultVector(std::vector<double> &p_rResHeader, std::vector<double> &p_rResElements)
{
	using namespace precitec::calibration;
	// Due to rewritten ArrayResults, the result vector creation is rewritten, too, as of 2014/10/01

	// Result vectors. ResHeader now holds header & init data ONLY, resLower all coordinates of higher AND lower layer line segment
	invalidateResult( p_rResHeader, p_rResElements );


	interface::Trafo &rHighTrafo(m_oIsTopLayerHigher?  *m_oSpTopTrafo : *m_oSpBotTrafo );
	interface::Trafo &rLowTrafo(m_oIsTopLayerHigher ? *m_oSpBotTrafo : *m_oSpTopTrafo );

	// Pre: put 1 into vec to signal everything is fine...
	p_rResHeader[eResultOK] = 1.0;

	p_rResHeader[eTopLayerHigher] = m_oIsTopLayerHigher ; // nr. [3] m_oIsTopLayerHigher

	// Now for the calibration TOP results: First two elements (four values) are the endpoints of the higher layer
	geo2d::Point oET = rHighTrafo( geo2d::Point( (int) m_oEndpointsHigher[0].m_oXPos, (int) m_oEndpointsHigher[0].m_oYPos )) ;
	p_rResHeader[eHighLeftX] = 1.0*oET.x;
	p_rResHeader[eHighLeftY] = 1.0*oET.y;

	oET = rHighTrafo( geo2d::Point( (int) m_oEndpointsHigher[1].m_oXPos, (int) m_oEndpointsHigher[1].m_oYPos ) );
	p_rResHeader[eHighRightX] = 1.0*oET.x;
	p_rResHeader[eHighRightY] = 1.0*oET.y;

	// Now for the calibration BOT results: First two elements (four values) are the start- and endpoint of the lower layer
	oET = rLowTrafo( geo2d::Point( (int) m_oEndpointsLower[0].m_oXPos, (int) m_oEndpointsLower[0].m_oYPos ) );
	p_rResHeader[eLowLeftX] = 1.0*oET.x;
	p_rResHeader[eLowLeftY] = 1.0*oET.y;
	oET = rLowTrafo( geo2d::Point( (int) m_oEndpointsLower[1].m_oXPos, (int) m_oEndpointsLower[1].m_oYPos ) );
	p_rResHeader[eLowRightX] = 1.0*oET.x;
	p_rResHeader[eLowRightY] = 1.0*oET.y;

	// now the number of elements for both layers...
	p_rResHeader[eNumElementsHigherLayer] = 1.0*m_oHigherLayerPoints.size();
	p_rResHeader[eNumElementsLowerLayer] = 1.0*m_oLowerLayerPoints.size(); // [12] and [13]

	// ...and the points itself, to resElements this time
	p_rResElements.resize( m_oHigherLayerPoints.size() + m_oLowerLayerPoints.size() );
	for (unsigned int i=0,n=0; i < (m_oHigherLayerPoints.size()/2); ++i, n+=2)
	{
		int i_x = m_oHigherLayerPoints.size() - 2 - (2 * i);
		int i_y = m_oHigherLayerPoints.size() - 1 - (2 * i);
		oET = rHighTrafo( geo2d::Point( (int) m_oHigherLayerPoints[i_x], (int) m_oHigherLayerPoints[i_y] ) );
		p_rResElements[n] = 1.0*oET.x;
		p_rResElements [n+1] = 1.0*oET.y;
	}

	for (unsigned int i=0, n =m_oHigherLayerPoints.size() ; i < (m_oLowerLayerPoints.size()/2); ++i, n+=2)
	{
		int i_x = 2 * i;
		int i_y = 2 * i + 1;
		oET = rLowTrafo( geo2d::Point( (int) m_oLowerLayerPoints[i_x], (int) m_oLowerLayerPoints[i_y] ) );
		p_rResElements[n] = 1.0*oET.x;
		p_rResElements[n+1] =  1.0*oET.y;

	}

}

void CalibrationResult::signalSend(const std::vector<double> &p_rResHeader, const std::vector<double> &p_rResElements, const ResultType p_oType,
		const ImageContext &p_rImgContextHigher, const ImageContext &p_rImgContextLower)
{

	double oRank = 1.0*(p_oType == interface::CalibrationOK); //Was soll den das sein ??
	auto oAnalysisState = AnalysisOK;
	if (oRank < 0.0001)
	{
		std::cout<<"Calib filter: signal values, bad Rank "<<std::endl;
		/*
		const GeoDoublearray oVal(p_rImgContextHigher, Doublearray(1), AnalysisOK, NotPresent);
		const ResultDoubleArray oRes( id(), p_oType, AnalysisErrBadCalibration, p_rImgContextHigher, oVal, TRange<double>(-9999999.0, 9999999.0));
		oAnalysisState = AnalysisErrBadCalibration;
        preSignalAction();
        m_oPipeOutResults.signal(oRes);
		*/
	}


	Doublearray	oValHeader	{ p_rResHeader.size(), 0, eRankMax };
	oValHeader.getData()	=	p_rResHeader;

	const GeoDoublearray oGeoValHeader(p_rImgContextHigher, oValHeader, AnalysisOK, oRank);
	const ResultDoubleArray oResHeader( id(), p_oType, oAnalysisState, p_rImgContextHigher, oGeoValHeader, TRange<double>(-9999999.0, 9999999.0));

	Doublearray	oValElements	{ p_rResElements.size(), 0, eRankMax };
	oValElements.getData()	=	p_rResElements;

	const GeoDoublearray oGeoValElements(p_rImgContextLower, oValElements, AnalysisOK, oRank);
	const ResultDoubleArray oResElements( id(), p_oType, oAnalysisState, p_rImgContextLower, oGeoValElements, TRange<double>( -9999999.0, 9999999.0 ) );
	//std::cout << "calibrationResult: signalsend \n" << precitec::calibration::describeResultHeader( p_rResHeader ) << std::endl;

	preSignalAction();

	m_oPipeOutResults.signal(oResHeader);
	m_oPipeOutResults.signal(oResElements);


}


//reads m_oIsTopLayerHigher
//writes m_oEndpointsHigher, m_oEndpointsLower, m_oHigherLayerPoints, m_oLowerLayerPoints
bool CalibrationResult::findPointsBothLayers( const geo2d::VecDoublearray &rTopLayer, const geo2d::VecDoublearray &rBotLayer )
{
	// Step 1: find higher layer calibration workpiece endpoints for computing magnification beta0
	auto rHigherLayer = m_oIsTopLayerHigher ? rTopLayer : rBotLayer;
	auto rLowerLayer = m_oIsTopLayerHigher ? rBotLayer : rTopLayer;

	m_oEndpointsHigher.clear();
	m_oEndpointsLower.clear();
	m_oHigherLayerPoints.clear();
	m_oLowerLayerPoints.clear();

	findEndPointsHigherLayer( m_oEndpointsHigher, rHigherLayer );
	if ( m_oEndpointsHigher.size() < 2 )
	{
		return false;
	}

	// Step 2: collect pixel for computing laser plane
	//     2a: find endpoints of lower line and choose (left, right)-pair with largest distance
	int oIdx = 0, oFound = -1;
	while ( oIdx < ( (int) (m_oEndpointsHigher.size()) - 1) && (oFound == -1) )
	{
		auto& oCurEndPoint = m_oEndpointsHigher[oIdx];
		auto& oNextEndPoint = m_oEndpointsHigher[oIdx + 1];
		if ( (oNextEndPoint.m_oType == eEndpointLeft) || (oCurEndPoint.m_oType == eEndpointRight) )
		{
			// Sieht vielversprechend aus, untersuche dieses Stueck:
			if ( findSegment( m_oEndpointsLower, oCurEndPoint.m_oXPos, oNextEndPoint.m_oXPos, rLowerLayer ) )
			{
				oFound = oIdx;
			}
		}
		// Falls dieses Stueck nicht vielversprechend war, dann suche weiter:
		++oIdx;
	}
	if ( oFound < 0 )
	{
		m_oEndpointsHigher.clear(); //just to be sure to recognize the error condition later
		return false;
	}
	// Der folgende Code speichert die relevanten Endpunkte in der oberen Linie ab.
	// Eine fruehere Version dieses Codes hat nur dann funktioniert, wenn oFound = 0, denn
	// in dem Fall macht der Code nichts. Die von mir korrigierte Version funktioniert auch
	// dann, wenn oFound ungleich 0. Achtung: Dabei ist die eigentliche Korrektur subtil,
	// denn sie besteht nur im Vertauschen der Zeilen !!
	m_oEndpointsHigher[0] = m_oEndpointsHigher[oFound];
	m_oEndpointsHigher[1] = m_oEndpointsHigher[oFound + 1];
	m_oEndpointsHigher.resize( 2 );

	//    2b: collect adjacent points from both layers
	//std::cout<<"Calib filter: Collect Layer Points  "<<std::endl;
	if ( !collectLayerPoints( rHigherLayer, rLowerLayer ) )
	{
		return false;
	}
	//no errors so far , endpoints and layer points are succesfully computed
	m_oHighLeft_SensorCoords = getSensorCoords(m_oEndpointsHigher[0].m_oXPos, m_oEndpointsHigher[0].m_oYPos, eHigherLayer);
	m_oHighRight_SensorCoords = getSensorCoords(m_oEndpointsHigher[1].m_oXPos, m_oEndpointsHigher[1].m_oYPos, eHigherLayer);
	m_oLowLeft_SensorCoords = getSensorCoords(m_oEndpointsLower[0].m_oXPos, m_oEndpointsLower[0].m_oYPos, eLowerLayer);
	m_oLowRight_SensorCoords = getSensorCoords(m_oEndpointsLower[1].m_oXPos, m_oEndpointsLower[1].m_oYPos, eLowerLayer);
	return true;
}


void CalibrationResult::proceedGroup(const void* p_pSender, fliplib::PipeGroupEventArgs& p_rEventArg)
{
	m_oEndpointsHigher.clear(); // needs to be at the very top to make sure paint will not be called when errors occur!
	m_oInfo.clear();
	poco_assert_dbg(m_pPipeInTopLayer != nullptr); // to be asserted by graph editor
	poco_assert_dbg(m_pPipeInBotLayer != nullptr);

	//Top, Bot from the point of view of the image, not necessarily the calibration piece (see m_oIsTopLayerHigher)
	const GeoVecDoublearray &rTopLayerVec( m_pPipeInTopLayer->read(m_oCounter) );
	const VecDoublearray &rTopLayer = rTopLayerVec.ref();
	const GeoVecDoublearray &rBotLayerVec( m_pPipeInBotLayer->read(m_oCounter) );
	const VecDoublearray &rBotLayer = rBotLayerVec.ref();

	// as we work on two different ROIs, we have different contexts here, so no use checking them for being equal!
	const ImageContext &rTopContext(rTopLayerVec.context());
	m_oSpTopTrafo = rTopLayerVec.context().trafo();
	const ImageContext &rBotContext(rBotLayerVec.context());
	m_oSpBotTrafo = rBotLayerVec.context().trafo();

	m_oHwRoi.x = rTopContext.HW_ROI_x0;
	m_oHwRoi.y = rTopContext.HW_ROI_y0;

	assert(m_oHwRoi.x == rBotContext.HW_ROI_x0 && "Contexts have different HW roi");
	assert(m_oHwRoi.y == rBotContext.HW_ROI_y0 && "Contexts have different HW roi");

	for ( auto pt : {&m_oHighLeft_SensorCoords, &m_oHighRight_SensorCoords,
		&m_oLowLeft_SensorCoords, &m_oLowLeftProjected_SensorCoords,
		&m_oLowRight_SensorCoords} )
	{
		pt->x = 0;
		pt->y = 0;
	}

	invalidateResult( m_oResultHeader, m_oResultElements );

	// forward potential error from preceding calibration filter
	if ( (rTopLayerVec.analysisResult() != interface::AnalysisOK) || (rBotLayerVec.analysisResult() != interface::AnalysisOK) )
	{
		std::cout<<"Higher or Lower Layer Result is invalid - threshold upper and lower layer"<<std::endl;
		wmLog(eInfo,"Higher or Lower Layer Result is invalid\n");
		signalSend(m_oResultHeader, m_oResultElements, rTopLayerVec.analysisResult(), rTopContext, rBotContext);
		return;
	}

	bool foundEndPoints = findPointsBothLayers( rTopLayer, rBotLayer );

	if ( !foundEndPoints )
	{
		//maybe it's just the laser line that comes from the opposite direction
		m_oIsTopLayerHigher = !m_oIsTopLayerHigher;
		foundEndPoints = findPointsBothLayers( rTopLayer, rBotLayer );
	}
	if ( !foundEndPoints )
	{	//parse error conditions
		if ( m_oEndpointsHigher.size() < 2 )
		{
			std::cout << "Calib filter: No endpoints on higher layer" << std::endl;
			signalSend( m_oResultHeader, m_oResultElements, interface::AnalysisErrBadCalibration, rTopContext, rBotContext );
			return ;
		}
		if ( m_oEndpointsLower.size() < 2 )
		{
			std::cout << "Calib filter: No endpoints on lower layer" << std::endl;
			signalSend( m_oResultHeader, m_oResultElements, interface::AnalysisErrBadCalibration, rTopContext, rBotContext );
			return;
		}

		// at least 3 points for each layer should be there... Should be at least m_oLayerSize...
		if (! ((m_oHigherLayerPoints.size() > 2) && (m_oLowerLayerPoints.size() > 2)))
		{
			std::cout << "Calib filter: Error CollectLayerPoints  " << std::endl;
			signalSend( m_oResultHeader, m_oResultElements, interface::AnalysisErrBadCalibration, rTopContext, rBotContext );
			return;
		}
	}

	// Step 3: create Result vector
	createResultVector(m_oResultHeader, m_oResultElements);

	auto & rHighContext = m_oIsTopLayerHigher ? rTopContext : rBotContext;
	auto & rLowContext = m_oIsTopLayerHigher ? rBotContext : rTopContext;
	signalSend(m_oResultHeader, m_oResultElements, interface::CalibrationOK, rHighContext, rLowContext );
}

} /* namespace filter */
} /* namespace precitec */
