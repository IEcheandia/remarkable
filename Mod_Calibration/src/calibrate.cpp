/*
 * Triangulation.cpp
 *
 *  Created on: Jun 10, 2013
 *      Author: abeschorner
 */

#include "Poco/Thread.h" // for sleep
#include "Poco/File.h" //for file rename

#include "math/2D/avgAndRegression.h"
#include "math/mathUtils.h"
#include "fliplib/BaseFilter.h"
#include <limits>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <memory>
#include <map>
#include <cmath>

#include "calibration/calibrate.h"
#include "common/systemConfiguration.h"
#include "util/calibDataParameters.h"
#include "math/mathUtils.h"
#include "math/calibrationStructures.h"
#include "calibration/calibrateLaserLineOriented.h"
#include <common/bitmap.h>

#include <fstream>


namespace precitec {

using namespace precitec::math;

using filter::LaserLine;


/**
 * @brief In this namespace all the calibration related classes are organized - the CalibrationManager is the main class that controls and executes the calibration procedures.
 */
namespace calibration {


CalibrateIb::CalibrateIb(CalibrationManager &p_rCalibMgr) :
		CalibrationProcedure(p_rCalibMgr),
		  m_toleranceParameterUpdate(0.0000001),
		m_oisTopLayerHigher(true)
{
}

CalibrateIb::~CalibrateIb()
{
}

bool CalibrateIb::printParameterUpdate(const double p_newValue, const double p_oldValue, const std::string p_name, const double p_tol, const LogType p_LogLevel)
{
	bool changed = false;
	if ( std::abs(p_newValue - p_oldValue) > p_tol )
	{
		wmLog(p_LogLevel, "%s changed value from %f to %f\n", p_name.c_str(),  p_oldValue, p_newValue);
		changed = true;
	}
	return changed;
}

// Set parameters for calibration filter graph. Parameters have fixed GUIDs and can be changed on the respective tab on wimMAIN.
bool CalibrateIb::setFilterParametersLine(CalibrationGraph &p_rGraph, const LaserLine p_oWhichLine, bool useTransposedImage, const precitec::math::CalibrationParamMap& rCalibrationParameters)
{
	std::shared_ptr<fliplib::FilterGraph> pGraph = p_rGraph.getGraph();

	// set parameters from calibration parameter page for detect calibration layers filter
	fliplib::BaseFilter* pFilter = pGraph->find(Poco::UUID("429fa5ef-5bb1-4118-8c6f-985669db6f5f")); // instance filter id of detect calibration layer filter
	if(pFilter==NULL){
		wmLog(eWarning,  "instance filter id of detect calibration layer filter missing\n");
		return false;
	}
	pFilter->getParameters().update("Threshold", "int", rCalibrationParameters.getInt("intensityAvgThreshold"));
	pFilter->getParameters().update("Extent", "int", rCalibrationParameters.getInt("layerExtend"));

	// set parameters from calibration parameter page for top layer parallel maximum
	pFilter = pGraph->find(Poco::UUID("6e1a8374-89b6-40d0-9182-1964700b6668")); // instance filter id of detect parallel max filter for top layer
	if(pFilter==NULL) {
		wmLog(eWarning,  "instance filter id of detect parallel max filter for top layer missing\n");
		return false;
	}
	pFilter->getParameters().update("Threshold", "UInt32", static_cast<std::uint32_t>(rCalibrationParameters.getInt("thresholdTop")));

	// set parameters from calibration parameter page for bottom layer parallel maximum
	pFilter = pGraph->find(Poco::UUID("04fa2b2e-58e3-4750-9d10-526f2cae737f")); // instance filter id of detect parallel max filter for bottom layer
	if(pFilter==NULL){
		wmLog(eWarning,  "instance filter id of detect parallel max filter for bottom layer missing\n");
		return false;
	}
	pFilter->getParameters().update("Threshold", "UInt32", static_cast<std::uint32_t>(rCalibrationParameters.getInt("thresholdBot")));

	// set parameters from calibration parameter page for calibration result filter
	pFilter = pGraph->find(Poco::UUID("1b8f12a0-48ba-4b14-9b1a-4da4a76c01cb")); //instance filter id of calibration result filter
	if(pFilter==NULL){
		wmLog(eWarning, "instance filter id of calibration result filter missing \n");
		return false;
	}
	pFilter->getParameters().update("LayerSize", "int", rCalibrationParameters.getInt("pixPerLayer"));

	// set parameters from calibration parameter page for parameter filter
	// for the roi parameters, the suffix is not the same as CoaxCalibrationData::ParameterKeySuffix
	std::string oLineNameInParameter;
	switch (p_oWhichLine)
	{
	case LaserLine::FrontLaserLine:
		oLineNameInParameter = "0";
		break;
	case LaserLine::BehindLaserLine:
		oLineNameInParameter = "2";
		break;
	default:
	case LaserLine::CenterLaserLine:
		oLineNameInParameter = "TCP";
		break;
	}

	std::string oXLeftName = "XLeftEdge";
	std::string oXRightName = "XRightEdge";
	std::string oYTopName = "YTopEdge";
	std::string oYBottomName = "YBottomEdge";

	int oOffsetRoiX = rCalibrationParameters.getInt(oXLeftName + oLineNameInParameter);

	int oXRightValue = rCalibrationParameters.getInt(oXRightName + oLineNameInParameter);
	int oXWidth = oXRightValue - oOffsetRoiX;
	if (oXWidth < 0)
	{
		// waehle einen sinnvollen Standardwert
		oXWidth = 10;
	}

	int oOffsetRoiY = rCalibrationParameters.getInt(oYTopName + oLineNameInParameter);

	int oYBottomValue = rCalibrationParameters.getInt(oYBottomName + oLineNameInParameter);
	int oYHeight = oYBottomValue - oOffsetRoiY;
	if (oYHeight < 0)
	{
		oYHeight = 10;
	}

	if (useTransposedImage)
	{
		std::swap(oOffsetRoiX, oOffsetRoiY);
		std::swap(oXWidth,  oYHeight);
	}

	pFilter = pGraph->find(Poco::UUID("4737ca9e-f070-48fd-a652-f20972c5f57b")); //instance filter id of parameter filter RoiX
	if(pFilter==NULL){
		wmLog(eWarning,  "instance filter id of parameter filter RoiX missing \n");
		return false;
	}


	pFilter->getParameters().update("scalar", "double", static_cast<double>(oOffsetRoiX));


	pFilter = pGraph->find(Poco::UUID("4b69b5cc-a5d1-4a30-b695-1e6d4e4958c0")); //instance filter id of parameter filter RoiDx
	if(pFilter==NULL){
		wmLog(eWarning,  "instance filter id of parameter filter RoiDx missing \n");
		return false;
	}


	pFilter->getParameters().update("scalar", "double", oXWidth);

	pFilter = pGraph->find(Poco::UUID("2ee17778-61a8-4b43-bc26-d24822c0e3e4")); //instance filter id of parameter filter RoiY
	if(pFilter==NULL){
		wmLog(eWarning,  "instance filter id of parameter filter RoiY missing \n");
		return false;
	}

	pFilter->getParameters().update("scalar", "double", oOffsetRoiY);

	pFilter = pGraph->find(Poco::UUID("40c901f4-ba0e-4e15-a476-2dbb6f1d64ce")); //instance filter id of parameter filter RoiDy
	if(pFilter==NULL){
			wmLog(eWarning, "instance filter id of parameter filter RoiDy missing \n");
			return false;
	}
	pFilter->getParameters().update("scalar", "double", oYHeight);


    if (useTransposedImage)
    {
        //use an higher resolution with an oriented laser line
        for (auto parMaxID : {Poco::UUID("6e1a8374-89b6-40d0-9182-1964700b6668"), Poco::UUID("04fa2b2e-58e3-4750-9d10-526f2cae737f") })
        {
            pFilter = pGraph->find(parMaxID);
            if(pFilter==NULL)
            {
                wmLog(eWarning, "instance filter id of parameter filter ParallelMaximum missing \n");
                return false;
            }
            pFilter->getParameters().update("ResX", "uint", 1);
        }

        for (auto lowPassID : {Poco::UUID("5ae10718-8521-404b-b929-2eaf96b38dcb"), Poco::UUID("d8f39f32-43d8-4862-808c-c8f51881ff88") })
        {
            pFilter = pGraph->find(lowPassID);
            if(pFilter==NULL)
            {
                wmLog(eWarning, "instance filter id of parameter filter LowPassAndScale missing \n");
                return false;
            }
            pFilter->getParameters().update("FIRWeight", "int", 15);
        }

    }

	std::cout << "activate new parameters and save them to filters and graph" << std::endl;
	analyzer::ParameterSetter oParamSetter;
	pGraph->control(oParamSetter);

	return true;
}




// Compute line regression parameters slope and intercept given two vectors and x and y coordinates, respectively.
bool CalibrateIb::computeLineRegression(double &p_rSlope, double &p_rIntercept, tVecDouble &p_rPlaneX, tVecDouble &p_rPlaneY)
{
	// Scheimpflug
	double oRegCoeff;

	//compute p_rSlope, p_rIntercept, oRegCoeff
	return math::linearRegression2D( p_rSlope, p_rIntercept, oRegCoeff, p_rPlaneX.size(), p_rPlaneX.data(), p_rPlaneY.data()); // call main routine for fast computation
}

// Make several single shot images and compute averaged results via filter graph. Averaging is done for stability reasons to reduce potential effects from jitter.
// updates m_oXHigher, m_oYHigher, m_oXLower, m_oYLower
bool CalibrateIb::getAvgResultOfImages(CalibrationGraph &p_rGraph,  const unsigned int p_oNumImages, const int p_oSensorID, const math::CameraRelatedParameters & rCameraRelatedParameters,
    const std::string p_oTitle)
{
	std::vector< interface::ResultArgs* > oResults; // results of individual images
	std::vector< double > oAvgResult;               // cumulative averaged result
	bool oResultOK = false;
	int oCntBadCalibs=0;  // number of failed calibrations
	bool oModified;       // true if an error occurs during calibration filter graph execution
	double oHigherXLeft=0, oHigherYLeft=0, oHigherXRight=0, oHigherYRight=0; // Averaged higher layer boundary coordinates of calibration workpiece.
	double oLowerXLeft=0, oLowerYLeft=0, oLowerXRight=0, oLowerYRight=0; // Averaged lower layer boundary coordinates of calibration workpiece.
	unsigned int oNumHigherEls=std::numeric_limits<unsigned int>::max(), oNumLowerEls=std::numeric_limits<unsigned int>::max(); //number of elements (pixel coordinates) of the layers
	
	std::stringstream oTitle("");


	for (unsigned int iImage=0; iImage < p_oNumImages; ++iImage)
	{
		oTitle.str("");
		oTitle << p_oTitle << "#" << iImage+1 << "/" << p_oNumImages;
		wmLog( eInfo, "Calibration graph: loop %s\n" , oTitle.str().c_str());

 		oModified = false;  // oCntBadCalibs not yet raised
 		oResults.clear();   // New individual result for each run
		oResults = p_rGraph.execute(true, oTitle.str(), p_oSensorID); // Execute calibration filter graph
 		m_oLastImage = m_rCalibrationManager.getCurrentImage();

		TriggerContext oContext(m_rCalibrationManager.getTriggerContext());
		m_oHWRoiX = oContext.HW_ROI_x0;
		m_oHWRoiY = oContext.HW_ROI_y0;


		if ( (oResults.size() != 2) ||
			(oResults[0]->value<double>().size() != int( eNumberOfComponentsOfHeader ))) // header and data need to have been sent by filter calibrationResult
		{

			wmLog(eWarning, "Calib Graph Results wrong size: [%d, %d, %d] - should be [2,%d,n]\n",
				oResults.size(),
				oResults.size()>0 ? oResults[0]->value<double>().size() : -1,
				oResults.size()>1 ? oResults[1]->value<double>().size() : -1,
				eNumberOfComponentsOfHeader		);

			wmLogTr(eWarning, "QnxMsg.Calib.CorruptSize", "Calibration failed-Error in result vector size .\n");
			oModified = true;
		}
		else
		{
			// 1 | 0, hoehe, breite
			double oIsCalibrationOK = oResults[0]->value<double>()[eResultOK];
            std::cout<<"Calib oIsCalibrationOK: "<<oIsCalibrationOK<<std::endl;

		   if ( ((int)oIsCalibrationOK != 1) || (oResults[0]->resultType() != interface::CalibrationOK) )
           {
				std::cout<<"Calib Graph Execution failed -- "<<std::endl;

				wmLog(eInfo,"Calib Graph Execution failed, bad rank\n");
				wmLogTr(eWarning, "QnxMsg.Calib.BadCalib", "Calibration failed. Bad line or illumination of calibration workpiece or improper parameters.\n");
				oModified = true;
			}

		}

		if (oModified) // only proceed if no errors have occured so far.
		{
			++oCntBadCalibs;
			oResultOK = false;

		}
		else
		{
			oResultOK = true;

			//results sent by CalibrationResult
			auto& oResultHeader = oResults[0]->value<double>();
			auto& oResultElements = oResults[1]->value<double>();

			// Update averaged result variables
			oHigherXLeft += oResultHeader[eHighLeftX];
			oHigherYLeft += oResultHeader[eHighLeftY];


			oHigherXRight += oResultHeader[eHighRightX];
			oHigherYRight += oResultHeader[eHighRightY];


			oLowerXLeft += oResultHeader[eLowLeftX];
			oLowerYLeft += oResultHeader[eLowLeftY];

			oLowerXRight += oResultHeader[eLowRightX];
			oLowerYRight += oResultHeader[eLowRightY];

			bool isTopLayerHigher = oResultHeader[eTopLayerHigher] > 0;

			//check consistency of relative coordinate position
			if ( m_oisTopLayerHigher == isTopLayerHigher )
			{
				if (! checkLayerPosition(m_oisTopLayerHigher, oHigherYLeft,  oLowerYLeft))
				{
					std::ostringstream oMsg;
					oMsg << oTitle.str() << " Inconsistency in coordinate position . isTopLayerHigher ? " << isTopLayerHigher << "\n"
						<< "HigherLayer Left (" << oHigherXLeft << "," << oHigherYLeft << ") \n"
						<< "LowerLayer Left (" << oLowerXLeft << "," << oLowerYLeft << ") \n";
					wmLog( eWarning, oMsg.str().c_str() );
					oResultOK = false;
				}
				if (!checkLayerPosition(m_oisTopLayerHigher, oHigherYRight,  oLowerYRight))
				{
					std::ostringstream oMsg;
					oMsg << oTitle.str() << " Inconsistency in coordinate position . isTopLayerHigher ? " << isTopLayerHigher << "\n"
						<< "HigherLayer Right(" << oHigherXRight << "," << oHigherYRight << " )" << "\n"
						<< "LowerLayer Right (" <<  oLowerXRight << "," << oLowerYRight << " )" << "\n";
					wmLog( eWarning, oMsg.str().c_str() );
					oResultOK = false;
				}
			}
			else
			{ //isTopLayerHigher is changed respect previous iteration
				if ( iImage == 0 )
				{
					//at the first iteration m_oisTopLayerHigher is not correctly set
					m_oisTopLayerHigher = isTopLayerHigher;
				}
				else
				{
					std::ostringstream oMsg;
					oMsg << oTitle.str() << " Inconsistency in coordinate position respect previous iteration . isTopLayerHigher ? " << isTopLayerHigher << "\n";
					wmLog( eWarning, oMsg.str().c_str() );
					oResultOK = false;
				}
			}


			//check number of elements higher and lower
			oNumHigherEls = std::min(oNumHigherEls, (unsigned int)(oResultHeader[eNumElementsHigherLayer]));
			oNumLowerEls = std::min(oNumLowerEls, (unsigned int)(oResultHeader[eNumElementsLowerLayer]));

			if (oAvgResult.size() < (oNumHigherEls+oNumLowerEls)) // can not grow larger once created due to preceeding min() calls
			{
				oAvgResult.resize(oNumHigherEls+oNumLowerEls);
				oAvgResult.assign(oNumHigherEls+oNumLowerEls, 0.0);
			}

			if ( (oNumHigherEls+oNumLowerEls) > oResultElements.size() )
			{
				++oCntBadCalibs;
				// internal Error
				wmLogTr(eError, "QnxMsg.Calib.CorruptSize", "[Int.] BAD SIZE %d + %d instead of %d!\n", oNumHigherEls, oNumLowerEls, oResults.size());
				oResultOK = false;
			}
			//end  size check of HigherEls and LowerEls

			if (oResultOK)
			{
				for (unsigned int j=0; j < (oNumHigherEls+oNumLowerEls); ++j)
				{
					oAvgResult[j] += oResultElements[j];
				}
			}
		} //end for image

		Poco::Thread::sleep(100); // we do not wanna have a buffer overrun, and as we are in no hurry, we allow ourselves a relaxed and calm calibration...
	}//end for loop p_oNumImages

	if (!oResultOK)  //it would be only the last image?
	{
		return false;
	}

	// at least half of the calibrations must be ok
	if   ( oCntBadCalibs >= (int) (0.5*p_oNumImages + (0.5*(p_oNumImages % 2))) )
	{
		wmLog(eWarning, "Calib Graph Execution failed; bad calibs: %d/%d\n", oCntBadCalibs, p_oNumImages);
		return false;
	}

	if ( (oNumHigherEls == std::numeric_limits<unsigned int>::max()) || (oNumLowerEls == std::numeric_limits<unsigned int>::max()))
	{
		wmLogTr(eError, "QnxMsg.Calib.CorruptData", "[Int.] Calibration impossible due to corrupt data from calibration graph.\n");
		return false; // should not happen
	}

	if ( (rCameraRelatedParameters.m_oDpixX < math::eps) || (rCameraRelatedParameters.m_oDpixY < math::eps) )
	{
		wmLogTr(eError, "QnxMsg.Calib.BadDpixParams", "Invalid calibration parameters for camera pixel:length ratio (DPixX / DPixY).\n");
		return false;
	}

	// Compute average over number of valid calibrations
	const double oFactor=1.0/(p_oNumImages-oCntBadCalibs);
	oHigherXLeft *= oFactor;
	oHigherYLeft *= oFactor;
	oHigherXRight *= oFactor;
	oHigherYRight *= oFactor;
	oLowerXLeft *= oFactor;
	oLowerYLeft *= oFactor;
	oLowerXRight *= oFactor;
	oLowerYRight *= oFactor;

	m_oXPlaneHigher.clear(); m_oYPlaneHigher.clear();
	m_oXPlaneLower.clear(); m_oYPlaneLower.clear();

	// Copy vectors to plane arrays (we store pixel coordinates) and continue computing averaged results
	for (unsigned int j=0; j < (oNumHigherEls+oNumLowerEls); ++j)
	{
		oAvgResult[j] *= oFactor;
		if (j < oNumHigherEls) // higher layer average over all valid calibration image evaluations
		{
			if ( j % 2 )
			{
				m_oYPlaneHigher.push_back( oAvgResult[j] );
			} else
			{
				m_oXPlaneHigher.push_back( oAvgResult[j] );
			}
		} else // lower layer average over all valid calibration image evaluations
		{
			if ( j % 2 )
			{
				m_oYPlaneLower.push_back( oAvgResult[j] );
			} else
			{
				m_oXPlaneLower.push_back( oAvgResult[j] );
			}
		}

	} // all results from all (valid) calibration image analysis' done?

	m_oXHigher.clear(); m_oYHigher.clear();
	m_oXLower.clear(); m_oYLower.clear();


	// Done averaging. Basically, oG0 is unimportant at this point, as it is added to all y-Values and due to translation invariance does not have any impact.
	// higher layer line end markers
	m_oXHigher.push_back( oHigherXLeft );
	m_oYHigher.push_back( oHigherYLeft );

	m_oXHigher.push_back( oHigherXRight );
	m_oYHigher.push_back( oHigherYRight );

	m_oXLower.push_back( oLowerXLeft );
	m_oYLower.push_back( oLowerYLeft );

	m_oXLower.push_back( oLowerXRight );
	m_oYLower.push_back( oLowerYRight );

	return true;
}


bool CalibrateIb::checkLayerPosition(const bool isTopLayerHigher, const double Yhigher, const double Ylower){
	//At the top of the image y is smaller
	return isTopLayerHigher ? Yhigher < Ylower : Yhigher > Ylower;
}


// =============================================================================
// ======================= optical system specific stuff =======================
// =============================================================================


CalibrateIbOpticalSystem::CalibrateIbOpticalSystem(CalibrationManager &p_rCalibMgr) :
		CalibrateIb(p_rCalibMgr), m_oGraph(p_rCalibMgr, "GraphCalibrationNoAxis.xml")
{
    char* oEnvStrg = getenv((char *)"WM_STATION_NAME");
    if (oEnvStrg != nullptr)
    {
        if (strcmp("WM-QNX-PC", oEnvStrg) == 0)
        {
            m_oHasCamera = SystemConfiguration::instance().getBool("HasCamera", true);
        }
        else
        {
            m_oHasCamera = false;
        }
    }
    else
    {
        m_oHasCamera = false;
    }
}

CalibrateIbOpticalSystem::~CalibrateIbOpticalSystem()
{
}

void CalibrateIbOpticalSystem::loadCalibrationTargetSize(int pSensorId)
{
    const auto & rCalibrationData = m_rCalibrationManager.getCalibrationData( pSensorId );
    const auto & rCalibrationDataParams = rCalibrationData.getParameters();
    m_oLayerDelta = rCalibrationDataParams.getDouble("GrooveDepth"); //Nut Tiefe
    m_oGapWidth = rCalibrationDataParams.getDouble("GrooveWidth");   //Nut Breite
}


//start der Kalibration (called  by calibrateLine) only coax case
bool CalibrateIbOpticalSystem::startCalibration(CalibrationGraph &p_rGraph, const math::SensorId pSensorID, const filter::LaserLine p_oWhichLine, bool useOrientedLine) // todo: sensorID
{
    //read CalibrationDataParams

    loadCalibrationTargetSize(pSensorID);

	std::string suffix = CoaxCalibrationData::ParameterKeySuffix(p_oWhichLine);
	std::string strBeta0 = "beta0";
	std::string strBetaZ = "betaZ" + suffix;
	std::string strTriangulationAngle = "triangulationAngle" + suffix;
	std::string strLineDirection = "HighPlaneOnImageTop" + suffix;
	std::string strLineEquationA = "laserLine_a" + suffix;
	std::string strLineEquationB = "laserLine_b" + suffix;
	std::string strLineEquationC = "laserLine_c" + suffix;
	
	double betaZ_old = -1.0;
	bool highPlaneOnImageTop_old = false;
	double beta0_old = -1.0;
	math::LineEquation laserLineOnXYPlane_old;

	{
		const auto & rCalibrationData = m_rCalibrationManager.getCalibrationData(pSensorID);
		if ( !rCalibrationData.isInitialized() )
		{
		    wmLog(eWarning, "Sensor %d not initialized", pSensorID);
		    assert(false);
		}
		assert(rCalibrationData.getSensorId() == pSensorID);

		wmLogTr(eWarning, "QnxMsg.Calib.WarnReset", "Warning! New calibration invalidates potential former grid computations and data!\n");
		wmLogTr(eInfo, "QnxMsg.Calib.PleaseWait", "Calibration in progress, please wait...\n");
		
		//get actual / old values
		const auto oCalibrationParam_old = rCalibrationData.getCoaxCalibrationData();

		std::cout << "startCalibration: old calib values: " <<
		    oCalibrationParam_old.m_oBeta0 << " " << oCalibrationParam_old.computeTriangulationAngle(p_oWhichLine) << std::endl;
		oCalibrationParam_old.getLineDependentParameters(betaZ_old, highPlaneOnImageTop_old, laserLineOnXYPlane_old, p_oWhichLine);
		beta0_old = oCalibrationParam_old.m_oBeta0;
	
	}

	bool retPerf = false;

    if (useOrientedLine)
    {

        CalibrateLaserLineOriented oCalibrateLaserLineOriented(m_oGapWidth, m_oLayerDelta);
        CalibrateLaserLineOriented::PaintInfo oPaintInfo;

        std::tie(retPerf, m_oComputedParams, oPaintInfo) =  oCalibrateLaserLineOriented.determineTriangulationAngle(p_rGraph, m_rCalibrationManager, p_oWhichLine);
        m_oLastImage = oCalibrateLaserLineOriented.getLastImage();

        if (retPerf)
        {
            //export the computed data so that it can be used by drawAngleInfo
            geo2d::Point drawOffset{0,0};
            if (std::abs(m_oComputedParams.m_laserLineOnXYPlane.getInclinationDegrees()) > 45)
            {
                //the laser line is vertical, it must be recentered in order to be shown in the canvas
                drawOffset.x = 100 - (oPaintInfo.m_pointProjectedHigherLayer.x+oPaintInfo.m_pointLowerLayer.x)/2;
                drawOffset.y = 512 - (oPaintInfo.m_pointProjectedHigherLayer.y+oPaintInfo.m_pointLowerLayer.y)/2;
            }
            m_oBetaZPaintInfo.m_oProjX = oPaintInfo.m_pointProjectedHigherLayer.x + drawOffset.x;
            m_oBetaZPaintInfo.m_oProjY = oPaintInfo.m_pointProjectedHigherLayer.y + drawOffset.y;
            m_oBetaZPaintInfo.m_oValX = oPaintInfo.m_pointLowerLayer.x + drawOffset.x;
            m_oBetaZPaintInfo.m_oValY = oPaintInfo.m_pointLowerLayer.y + drawOffset.y;

            m_oLayerLines.m_oSlopeHigher = oPaintInfo.m_oSlopeHigher;
            m_oLayerLines.m_oSlopeLower = oPaintInfo.m_oSlopeLower;
            m_oLayerLines.m_oInterceptHigher = oPaintInfo.m_oInterceptHigher + drawOffset.y;
            m_oLayerLines.m_oInterceptLower = oPaintInfo.m_oInterceptLower  + drawOffset.y;
            m_oXLower = {oPaintInfo.m_oPointLowLeft.x + drawOffset.x,
                oPaintInfo.m_oPointLowRight.x + drawOffset.x};
            m_oXHigher = {oPaintInfo.m_oPointHighLeft.x + drawOffset.x,
                oPaintInfo.m_oPointHighRight.x + drawOffset.x};
            m_oYLower = {oPaintInfo.m_oPointLowLeft.y + drawOffset.y,
                oPaintInfo.m_oPointLowRight.y + drawOffset.y};
            m_oYHigher = {oPaintInfo.m_oPointHighLeft.y + drawOffset.y,
                oPaintInfo.m_oPointHighRight.y + drawOffset.y};

            drawAngleInfo(oPaintInfo.m_LengthBZ, m_oComputedParams.getAngle(angleUnit::eDegrees));
        }
    }
    else
    {
        std::cout<<"CAL performMagnificationComputation "<<std::endl;
        retPerf = performMagnificationComputation(p_rGraph, pSensorID, p_oWhichLine);
        std::cout<<"CAL performMagnificationComputation ended with "<<retPerf<<std::endl;
    }
    if (!retPerf)
    {
        //calibration was not successful
        const auto & rCalibrationData = m_rCalibrationManager.getCalibrationData(pSensorID);
        const auto & rCurrentCalibrationDataParams = rCalibrationData.getParameters();
        if (! math::isClose( rCurrentCalibrationDataParams.getDouble(strBetaZ), betaZ_old)
            || !math::isClose( rCurrentCalibrationDataParams.getDouble(strBeta0), beta0_old)
            || !math::isClose( bool(rCurrentCalibrationDataParams.getInt(strLineDirection)), highPlaneOnImageTop_old )
        )
        {
            wmLog(eError, "Calibration failed - internal error, old values not restored\n");
        }
        rCalibrationData.checkCalibrationValuesConsistency();

        return false;
    }

	auto & rCalibrationData = m_rCalibrationManager.getCalibrationData(pSensorID);

	//modify rCalibrationDataParams

	printParameterUpdate(m_oComputedParams.beta0, beta0_old, "beta0 from line " + suffix,
			                      m_toleranceParameterUpdate,  eWarning);
	printParameterUpdate(m_oComputedParams.betaz, betaZ_old, strBetaZ, m_toleranceParameterUpdate, eWarning);
	//TriangulationAngle and lineDirection are already updated (and logged) in determineTriangulationAngle
    double a,b,c;
    {
        double a_old, b_old, c_old;
        m_oComputedParams.m_laserLineOnXYPlane.getCoefficients(a,b,c);
        laserLineOnXYPlane_old.getCoefficients(a_old, b_old, c_old);

        printParameterUpdate(a, a_old, strLineEquationA, m_toleranceParameterUpdate, eWarning);
        printParameterUpdate(b, b_old, strLineEquationB, m_toleranceParameterUpdate, eWarning);
        printParameterUpdate(c, c_old, strLineEquationC, m_toleranceParameterUpdate, eWarning);
    }

        rCalibrationData.setKeyValue("sensorWidth", m_oCameraRelatedParameters.m_oWidth);
        rCalibrationData.setKeyValue("sensorHeight", m_oCameraRelatedParameters.m_oHeight);

        rCalibrationData.setKeyValue("DpixX", m_oCameraRelatedParameters.m_oDpixX);
        rCalibrationData.setKeyValue("DpixX", m_oCameraRelatedParameters.m_oDpixY);

	rCalibrationData.setKeyValue("beta0", m_oComputedParams.beta0);  // zu beta0 gibt es nur einen Wert, d.h. den koennen wir immer setzen, unabhaengig davon, welche Linie wir kalibriert haben.

	rCalibrationData.setKeyValue(strBetaZ, m_oComputedParams.betaz);

	rCalibrationData.setKeyValue(strLineDirection, m_oComputedParams.highPlaneOnImageTop);

    rCalibrationData.setKeyValue(strLineEquationA, a);
    rCalibrationData.setKeyValue(strLineEquationB, b);
    rCalibrationData.setKeyValue(strLineEquationC, c);
        rCalibrationData.setKeyValue("SensorParametersChanged",false);
	wmLogTr(eInfo, "QnxMsg.Calib.CalibOK", "Calibration successfully completed!\n");

	//gehe zu calibartionData schreibe die Parameterliste
	m_rCalibrationManager.sendCalibDataChangedSignal(pSensorID, true); // transfer xml file & 3D coords
	wmLog(eDebug,"calibrateLine: called signalCalibDataChanged(%d) \n", pSensorID );

	if ( !m_rCalibrationManager.getCalibrationData(pSensorID).checkCalibrationValuesConsistency() )
	{
		wmLog(eDebug, "performMagnificationComputation: parameters updated, but inconsistencies found\n" );
	}


    //debug: draw the laser line at z = 0 and z = -GrooveDepth
    if (useOrientedLine)
    {
        interface::ScannerContextInfo scannerInfo{true, 0.0, 0.0}; //FIXME get actual values
        int yText = 100;
        for (double z_mm : {0.0, -m_oLayerDelta})
        {
            auto line = rCalibrationData.getLaserLineAtZCollimatorHeight(z_mm, scannerInfo, p_oWhichLine);
            m_rCalibrationManager.drawLine(int(std::round(line.getX(0))), 0,
                                           int(std::round(line.getX(1024))), 1024,
                                           Color::Red());
            char titleBuffer[50];
            sprintf(titleBuffer,"Z=%0.2f", z_mm );
            m_rCalibrationManager.drawText(int(std::round(line.getX(yText))), yText, std::string(titleBuffer), Color::Red());
            yText += 150;
        }
        m_rCalibrationManager.renderImage(m_rCalibrationManager.getCurrentImage());
    }

	return true;
}

bool CalibrateIbOpticalSystem::calibrateLine(filter::LaserLine p_oWhichLine, bool useOrientedLine, const math::CameraRelatedParameters & rCameraRelatedParameters) // todo: sensorID
{
    m_oCameraRelatedParameters = rCameraRelatedParameters;
	bool retCal = false;

	std::cout<<"CAL calibrate Line "<< CoaxCalibrationData::ParameterKeySuffix(p_oWhichLine) <<std::endl;

 	m_oLastImage.clear();
	//start calibration and update values
	retCal = startCalibration(m_oGraph, math::SensorId::eSensorId0 , p_oWhichLine, useOrientedLine);
	std::cout<<"CAL calibrate Line finished with: "<<retCal<<std::endl;

	if(!retCal)
	{
		// todo: log error
		wmLog( eWarning, "Error in calibrate line %d\n", int(p_oWhichLine) );

 		if (m_oLastImage.isValid())
 		{
 			int oSensorID = 0;
 			const auto& rCalibData = m_rCalibrationManager.getCalibrationData(oSensorID);

 			std::string oFilename = rCalibData.getFilenamesConfiguration().getLastFailedLineCalibrationFilename();
 			fileio::Bitmap oBitmap(oFilename, m_oLastImage.width(), m_oLastImage.height(), false); //default value of topdown gives mirrored images
 			bool ok = oBitmap.isValid() && oBitmap.save(m_oLastImage.data());
 			if ( ok )
 			{
				wmLog(eDebug, "Saved last image of failed calibration " + oFilename + "\n");
 			}
 			else
 			{
				wmLog(eDebug, "Could not save last image of failed calibration to " + oFilename + "\n");
 			}
		}
		else
		{
 			wmLog(eDebug, "No valid image acquired during last calibration\n");
		}

		return false;
	}

	return true;
}

// Get magnification values for both base and grid calibration methods.
// update m_oXHigher... , m_oBeta0
bool CalibrateIbOpticalSystem::performMagnificationComputation( CalibrationGraph &p_rGraph,
	const int p_oSensorID, LaserLine p_oWhichLine)
{
	const auto& rCalibrationDataParams = m_rCalibrationManager.getCalibrationData(p_oSensorID).getParameters();
    bool useTransposedImage = false;
	bool ret = setFilterParametersLine(p_rGraph, p_oWhichLine, useTransposedImage, rCalibrationDataParams);
    if(!ret)
    {
    	std::cout<<"UUIDs not found in Calibration graph "<<std::endl;
    	wmLog(eError, "UUIDs not found in calibration graph \n");
    	return false;
    }

	m_rCalibrationManager.clearCanvas();
	//rechne die Bildpunkte ueber mehrere Bilder
	//calls getAvgResultOfImages, which updates m_oXHigher, m_oYHigher, m_oXLower, m_oYLower
	if ( !evaluateCalibrationLayers(p_oSensorID) )
	{
		wmLog( eWarning, "Could not performMagnificationComputation, error in Layers evaluation\n" );
		return false;
	}

	// Start computation
	m_oComputedParams.beta0 = 0.5;
	m_oComputedParams.betaz = 0.5;
	m_oComputedParams.highPlaneOnImageTop = m_oisTopLayerHigher;
	double oPixDeltaB0(0.0), oPixDeltaBZ(0.0);
	double oLengthBZ = 0.0;  //strecke zwischen oberer und unterer Linie in pixel (signed)

	//read rCalibrationDataParams
    m_oAdditionalDebug = rCalibrationDataParams.getInt("Debug") > 0;
	m_oLayerDelta = rCalibrationDataParams.getDouble("GrooveDepth"); //Nut Tiefe
	m_oGapWidth = rCalibrationDataParams.getDouble("GrooveWidth");   //Nut Breite

    m_oLayerLines.reset(m_oXPlaneHigher, m_oYPlaneHigher, m_oXPlaneLower, m_oYPlaneLower, m_oHWRoiX, m_oHWRoiY, m_oisTopLayerHigher);

	m_oComputedParams.beta0 = computeBeta0(oPixDeltaB0, m_oCameraRelatedParameters.m_oDpixX );
	m_oComputedParams.betaz = computeBetaZ(oPixDeltaBZ, oLengthBZ, m_oBetaZPaintInfo, m_oCameraRelatedParameters.m_oDpixY);


    // jetzt noch den Winkel berechnen
    // checken im koax fall - alle daten sind bekannt ....
    // Vereinfachung waere hier atan(betaz /beta0)
	
	double oTriangAngle = m_oComputedParams.getAngle(angleUnit::eRadians); //determineTriangulationAngle(p_oSensorID, true /*isCoax*/, p_oWhichLine);
	const std::string  suffix = CoaxCalibrationData::ParameterKeySuffix(p_oWhichLine);
	std::ostringstream oMsg;
	oMsg << "Perform Magnification Computation" 
		<< " beta0:" << m_oComputedParams.beta0 << "\n"
		<< "betaZ" << suffix << ": " << m_oComputedParams.betaz << "\n"
		<< "triangulationAngle" << suffix << ": " << oTriangAngle << " " << m_oComputedParams.getAngle(angleUnit::eDegrees) << "\n";
	wmLog(eDebug, oMsg.str().c_str());

    drawAngleInfo(oLengthBZ, m_oComputedParams.getAngle(angleUnit::eDegrees));
  
	// Validity checks. All OK? Only in this case we modify the CalibrationData parameters...
	if ( (oPixDeltaB0 > 0) && (oPixDeltaBZ > 0) )
	{
		wmLogTr(eInfo, "QnxMsg.Calib.NewBeta0", "New X-magnification value: %f, %f pix horiz. = %f mm.\n", m_oComputedParams.beta0, oPixDeltaB0, m_oGapWidth);
		wmLogTr(eInfo, "QnxMsg.Calib.NewBetaZ", "New Z-magnification value: %f, %f pix vert. = %f mm.\n", m_oComputedParams.betaz, oPixDeltaBZ, m_oLayerDelta);

		return true;

	} else
	{
		wmLogTr(eError, "QnxMsg.Calib.EitherBad", "Cannot determine beta0 and/or betaZ. Calibration aborted!\n");
		return false;
	}


	}


// compute beta0
double CalibrateIbOpticalSystem::computeBeta0(double &p_rPixDeltaBeta0, const double p_oDpixX) const
{
    assert(m_oLayerLines.isInitialized());
	double oBeta0(0.5);

	//start computeMagnificationBeta0
	p_rPixDeltaBeta0 = std::abs(m_oXHigher[1] - m_oXHigher[0]);
	const double oDpixX = p_oDpixX > math::eps ? p_oDpixX : 1;
	tVecDouble oCoordX(2);
	oCoordX[0] = m_oXHigher[0] * oDpixX;
	oCoordX[1] = m_oXHigher[1] * oDpixX;
	bool ok = math::lengthRatio(oBeta0, m_oLayerLines.m_oSlopeHigher, 0, m_oGapWidth, oCoordX);
	if (!ok)
	{
		p_rPixDeltaBeta0 = -1.0;
	}
	//end computeMagnificationBeta0

	// use higher layer for computation of beta0, should be better illuminated and more stable
	if (  p_rPixDeltaBeta0 < 0 )
	{
		wmLogTr(eInfo, "QnxMsg.Calib.BadBeta0", "Calibration failed. Cannot determine X-magnification.\n");
		return -1.0;
	}

	return oBeta0;
}


// Compute triangulation angle and updates calibration parameters (for the coax case, see computeBetaZ)
double CalibrateIbOpticalSystem::computeTriangAngle(double &p_rLength, const int p_oOffsetX, const int p_oOffsetY, 
	const math::SensorId p_oSensorID, const bool p_oCoax)
{
	/*
	 * Using the grid (Coax or Scheimpflug), the triangulation angle is determined as follows:
	 * Via linear regression, higher and lower layer regression line of best fits are computed for each.
	 * The lower line data is then projected onto the higher line to get orthogonal, smallest distances.
	 * As this is done in the grid, the 3D coord distances are summed up and averaged.
	 * The final step computes the arctan, where the first parameter (y) is the known calibration workpiece height (for instance 2mm)
	 * and the second one (x) the measured average height of the projections.
	 */
    m_oLayerLines.reset(m_oXPlaneHigher, m_oYPlaneHigher, m_oXPlaneLower, m_oYPlaneLower, m_oHWRoiX, m_oHWRoiY, m_oisTopLayerHigher);
	std::cout << "-----------------distance abs.unit: " << p_rLength << std::endl;

	assert (p_oCoax == ( m_rCalibrationManager.getCalibrationData(p_oSensorID).getSensorModel() == SensorModel::eLinearMagnification));
	if(p_oCoax)
	{
        wmLog(eWarning, "Method compute triangulation angle available only for Scheimpflug sensors\n");
        return m_oComputedParams.getAngle(angleUnit::eRadians) ;
    }

    assert(m_rCalibrationManager.getCalibrationData(p_oSensorID).getSensorId() == p_oSensorID);
    //keep params in sync
    const auto& rCalibrationDataParams = m_rCalibrationManager.getCalibrationData(p_oSensorID).getParameters();
    m_oLayerDelta = rCalibrationDataParams.getDouble("GrooveDepth"); //Nut Tiefe
    m_oGapWidth = rCalibrationDataParams.getDouble("GrooveWidth");   //Nut Breite    
    


    const auto &rCalibCoords( m_rCalibrationManager.getCalibrationData(p_oSensorID).getCalibrationCoords());
    int oDiv = 0;
    //p_rLength = 0.0; -- mit der Laenge von aussen in die Funktion, diese wurde bestimmt in betaZ Berechnung
    double oPixDelta(0);
    int oProjX(0), oProjY(0);
    float oHigherProjX(0.0), oHigherProjY(0.0);
    float oLowerX(0.0), oLowerY(0.0);


    m_oBetaZPaintInfo.m_oValX = 0;
    m_oBetaZPaintInfo.m_oValY = 0;
    m_oBetaZPaintInfo.m_oProjX = 0;
    m_oBetaZPaintInfo.m_oProjY = 0; // for drawing beta z line



    // Angle is computed by averaging over many points.

    Vec3D oLower;


    // This functions projects ALL points from the lower layer linear regression onto the higher layer linear regression...
    for ( size_t i = 0; i < m_oXPlaneLower.size(); ++i )
    {
        ++oDiv;
        //oLower = rCalib.to3D(static_cast<int>(m_oXPlaneLower[i]+p_oOffsetX),
        //	static_cast<int>(m_oInterceptLower+m_oSlopeLower*(m_oXPlaneLower[i]+ p_oOffsetX) + p_oOffsetY), p_oSensorID);
        //since we are in the Scheimpflug case, the internal plane is the laser plane
        auto xLowerWithOffset = m_oXPlaneLower[i] + p_oOffsetX;
        rCalibCoords.getCoordinates(oLowerX, oLowerY, 
            static_cast<int>(xLowerWithOffset), static_cast<int>(m_oLayerLines.getYLower(xLowerWithOffset) + p_oOffsetY));
        projectOntoHigherOffset(xLowerWithOffset, m_oLayerLines.getYLower(xLowerWithOffset) + p_oOffsetY, p_oOffsetX, p_oOffsetY,
            oProjX, oProjY, oPixDelta);

        rCalibCoords.getCoordinates(oHigherProjX, oHigherProjY,  oProjX, oProjY);
        if ( i == 10 ) // for painting
        {
            m_oBetaZPaintInfo.m_oValX = xLowerWithOffset; m_oBetaZPaintInfo.m_oValY = m_oLayerLines.getYLower(xLowerWithOffset) + p_oOffsetY;
            m_oBetaZPaintInfo.m_oProjX = oProjX; m_oBetaZPaintInfo.m_oProjY = oProjY;
        }
        //compute the 2D distance, instead of assuming that the 3Dcoordinate are aligned with our reference system
        float distX = (oHigherProjX - oLowerX);
        float distY = (oHigherProjY - oLowerY);
        float sign = (oHigherProjY - oLowerY) > 0 ? 1:-1;
        p_rLength += (sign * sqrt (distX * distX + distY * distY)); // keep orientation information (signature). use y coords and an angle of zero to compute new angle!
    }

    // ... and returns the average projection length
    if ( (oDiv > 0) && (std::abs(p_rLength) > math::eps) )
    {
        
        
        p_rLength /= oDiv;
        std::cout << "********************laenge y: " << p_rLength << " mm  in the laser plane reference system; h= "<< m_oLayerDelta << std::endl;
        if (m_oLayerDelta/p_rLength > 1)
        {
            wmLog(eWarning, "Laser line length on the plane (%f) must be longer than groove heigth (%f)\n", p_rLength, m_oLayerDelta);
            
            //p_rLength = 0; // error condition in the calling functions
            return 0.01;
        }
        return (std::acos(m_oLayerDelta/ p_rLength )); // Compute angle between laser plane and calibration piece
    }
    p_rLength = 0; // error condition in the calling functions
    return 0.0;
}

// Linear regression for computing betaZ, the vertical magnification necessary for pixel y coord delta <-> mm delta computations.
double CalibrateIbOpticalSystem::computeBetaZ(double &p_rPixDelta, double &p_rLength, BetaZPaintInfo & p_rPaintInfo, const double p_oDpixY) const
{
    assert(m_oLayerLines.isInitialized());

	p_rPixDelta = 0.0;
	int oDiv=0;
	double oLen = 0.0;

	int oXDraw=0, oYDraw=0;
	p_rPaintInfo.m_oValX = 0; p_rPaintInfo.m_oValY = 0; p_rPaintInfo.m_oProjX = 0; p_rPaintInfo.m_oProjY = 0; // for drawing beta z line

	// This functions projects ALL points from the lower layer linear regression onto the higher laser linear regression...
	for (size_t i=0; i < m_oXPlaneLower.size(); ++i)
	{
		++oDiv;
		oLen += projectOntoHigherRegression(m_oXPlaneLower[i], m_oLayerLines.getYLower(m_oXPlaneLower[i]), oXDraw, oYDraw, p_rPixDelta);
		if (i == 10) // for painting
		{
			p_rPaintInfo.m_oValX = m_oXPlaneLower[i];
			p_rPaintInfo.m_oValY =  m_oLayerLines.getYLower(m_oXPlaneLower[i]);
			p_rPaintInfo.m_oProjX = oXDraw;
			p_rPaintInfo.m_oProjY = oYDraw;
		}
	}

	// ... and returns the average projection length
	if (oDiv > 0)
	{
		oLen /= oDiv; 
        auto oMeanLen = oLen; //copy for debug message
		oLen *= p_oDpixY;
		p_rPixDelta /= oDiv;

		std::cout<<"*************** oLen: "<<oLen<<std::endl;
		std::cout<<"*************** p_rPixDelta: "<<p_rPixDelta <<std::endl;
		std::cout<<"**************** m_oLayerDelta: "<<m_oLayerDelta<<std::endl;

		if(m_oComputedParams.beta0 > 0)
		{
			p_rLength = oLen / m_oComputedParams.beta0;   // oLen ist die laenge auf dem chip in mm--> getilt durch beta0 gibt die distanz auf Werstueck
			
            wmLog(eDebug, "computeBetaZ: mean distance layers  %f pix[mm] -> projection y %f [mm], Target DZ %f [mm] : betaZ %f \n",
                  oMeanLen, p_rLength, m_oLayerDelta, oLen/m_oLayerDelta );
                  
		}
		else                               // der entsprechende Abstand auf dem Werkstueck ist oLen / beta0
		{
            wmLog(eError, "Beta0 is 0, BetaZ can't be computed \n");
			return(0.0);
		}
		return (oLen/m_oLayerDelta);      // Compute ratio to known distance
	} else //oDiv == 0
	{
        wmLog(eError, "No lower plane detected \n");
		p_rPixDelta = 0.0;
		p_rLength = 0.0;
	}

	return -1.0;
}

double CalibrateIbOpticalSystem::projectOntoHigherRegression(const double p_oXPosLower, const double p_oYPosLower,
		int &p_rXHigher, int &p_rYHigher, double &p_rPixDelta) const
{
	return projectOntoHigherOffset( p_oXPosLower, p_oYPosLower, 0, 0, p_rXHigher, p_rYHigher, p_rPixDelta );

}

// p_rPixDelta is INCREMENTED by the computed length ( oYPosProj - p_oYSrc)
double CalibrateIbOpticalSystem::projectOntoHigherOffset(const double p_oXSrc, const double p_oYSrc, const int p_oOffsetX, const int p_oOffsetY,
		int &p_rXHigher, int &p_rYHigher, double &p_rPixDelta) const
{
	double oAngle = -std::atan(m_oLayerLines.m_oSlopeHigher); // - as screen has increasing y values from higher to lower! Hence left end higher than right end -> negative slope originally, now positive

	// some basic geometry computations to project point from lower calibration workpiece plane to upper regression
	double oPiHalf = std::atan2(0.0, -1.0) / 2.0;
	double oAlpha = oPiHalf - oAngle;
	// m_oInterceptLower+m_oSlopeLower*(m_oXPlaneLower[i]+ p_oOffsetX) + p_oOffsetY)
	double oYPosProj = m_oLayerLines.getYHigher(p_oXSrc) + p_oOffsetY; //TODO check if p_oOffsetX is needed
	double oLength = std::abs(oYPosProj - p_oYSrc);
	p_rPixDelta += oLength;
	double oProj = oLength*std::sin(oAlpha);
	p_rXHigher = (int) (p_oXSrc - std::cos(oAlpha)*oProj+0.5);
	p_rYHigher = (int) (m_oLayerLines.getYHigher(p_rXHigher) +0.5 + p_oOffsetY);
	return oProj;
}
//m_oInterceptLower+m_oSlopeLower*(m_oXPlaneLower[i]+ p_oOffsetX) + p_oOffsetY


void CalibrateIb::showLivePic(const std::string p_oTitle)
{
	BImage oImage = m_rCalibrationManager.getCurrentImage();
	if ( (oImage.width() <= 0) || (oImage.height() <= 0) )
	{
		return;
	}
	m_rCalibrationManager.clearCanvas();
	m_rCalibrationManager.drawText(0, 0, p_oTitle, Color::Yellow());
	m_rCalibrationManager.renderImage(oImage);
}


// Show results of successful triangulation angle determination
void CalibrateIbOpticalSystem::drawAngleInfo( const double p_oLength, const double p_oAngle )
{
	m_rCalibrationManager.clearCanvas();

	BImage oImage = m_rCalibrationManager.getCurrentImage();
	if ( (oImage.width() < 0) || (oImage.height() <= 0) )
	{
		return;
	}

	std::setprecision(3); 
    std::stringstream oTmp; 
    if (m_oAdditionalDebug)
    {
        oTmp << "Ratio y/z: " << p_oLength << " / " << m_oLayerDelta << " mm  => ";
    }
    oTmp << "Angle = " << p_oAngle << " deg";

	double oWidth = oImage.width(); //m_rCalibrationManager.getCanvas()->width();
	for (int i=1; i < (int)oWidth-1; ++i)
	{
		m_rCalibrationManager.drawPixel(i, (int)(m_oLayerLines.getYHigher(i)+0.5), Color::Cyan());
	}
	for (int i=m_oXLower[0]; i < m_oXLower[1]; ++i)
	{
		m_rCalibrationManager.drawPixel(i, (int)(m_oLayerLines.getYLower(i) +0.5), Color::Cyan());
	}

	m_rCalibrationManager.drawLine((int)(m_oXHigher[0]+0.5), (int)(m_oYHigher[0]+0.5), (int)(m_oXHigher[1]+0.5), (int)(m_oYHigher[1]+0.5), Color::Green());

	if (m_oBetaZPaintInfo.m_oProjX > 0)
	{
		m_rCalibrationManager.drawLine(m_oBetaZPaintInfo.m_oValX, m_oBetaZPaintInfo.m_oValY, m_oBetaZPaintInfo.m_oProjX, m_oBetaZPaintInfo.m_oProjY, Color::Yellow());
		m_rCalibrationManager.drawText(m_oBetaZPaintInfo.m_oValX-static_cast<int>( (m_oXHigher[0]+5) ), m_oBetaZPaintInfo.m_oValY-25, oTmp.str(), Color::Yellow());
	}

	m_rCalibrationManager.renderImage(oImage);
}

/* Determines slopes and intercepts for higher and lower layer lines determined by the calibration filter graph.
in the implementation it reads m_oXPlaneHigher, m_oYPlaneHigher, m_oHWRoiX, m_oHWRoiY
writes m_oSlopeHighLayer, m_oInterceptHighLayer, m_oSlopeLowLayer, m_oInterceptLowerLayer
*/
void CalibrateIbOpticalSystem::LayerLines::reset(tVecDouble m_oXPlaneHigher, tVecDouble m_oYPlaneHigher,
            tVecDouble m_oXPlaneLower, tVecDouble m_oYPlaneLower,
            double m_oHWRoiX, double m_oHWRoiY, bool m_oisTopLayerHigher)
{
	// Add HWRois for correct intercept
	tVecDouble oCoordX, oCoordY;

	oCoordX.resize(m_oXPlaneHigher.size(), 0.0); oCoordY.resize(m_oYPlaneHigher.size(), 0.0);
	for(size_t i=0; i < m_oXPlaneHigher.size(); ++i)
	{
		oCoordX[i] = m_oXPlaneHigher[i] + m_oHWRoiX; oCoordY[i] = m_oYPlaneHigher[i] + m_oHWRoiY;
	}
	computeLineRegression(m_oSlopeHigher, m_oInterceptHigher, oCoordX, oCoordY);

	oCoordX.resize(m_oXPlaneLower.size(), 0.0); oCoordY.resize(m_oYPlaneLower.size(), 0.0);
	for(size_t i=0; i < m_oXPlaneLower.size(); ++i)
	{
		oCoordX[i] = m_oXPlaneLower[i] + m_oHWRoiX; oCoordY[i] = m_oYPlaneLower[i] + m_oHWRoiY;
	}
	computeLineRegression(m_oSlopeLower, m_oInterceptLower, oCoordX, oCoordY);


	std::cout << "-----------------m_oSlopeHigher: " << m_oSlopeHigher << std::endl;
	std::cout << "-----------------m_oSlopeLower: " << m_oSlopeLower << std::endl;
	std::cout << "-----------------m_oInterceptHigher: " << m_oInterceptHigher << std::endl;
	std::cout << "-----------------m_oInterceptLower: " << m_oInterceptLower << std::endl;
	//sanity check
	if ( std::abs(m_oSlopeLower - m_oSlopeHigher) < 0.00001 )
	{

		if ( !CalibrateIb::checkLayerPosition(m_oisTopLayerHigher, m_oInterceptHigher, m_oInterceptLower) )
		{
			wmLog(eError, "Error in layer positions! Is  top layer the higher one: %d  y higher: %f  ylower: %f", int(m_oisTopLayerHigher), m_oInterceptHigher, m_oInterceptLower);
		}

		assert(CalibrateIb::checkLayerPosition(m_oisTopLayerHigher, m_oInterceptHigher, m_oInterceptLower) && "getSlopes: inconsistency in layer position");
	}
	else
	{
		wmLog(eInfo, "Layers are not exactly parallel %f %f", m_oSlopeLower, m_oSlopeHigher);
	}



}


system::CamGridData CalibrateIbOpticalSystem::getCamGridDataFromCamera(const int p_oSensorID)
{    
    /* 
     * Normal initialization with camera:
     * 1) read calibration from binary file, if available
     * 2) read calibration header from camera, if available
     * 3) if the calibration from file corresponds to the calibration header, calibration loading was successful (loaded from cache),
     *    if not, use the calibration from the camera (and rewrite the binary file)
     * 4) if the calibration could not be read from the camera, use the calibration from the csv file in config folder as a FALLBACK
     * 4-bis) for backcompatibility: use the calibration from the csv file in calib folder as FALLBACK
     * 5) if neither calibration is available, return false 
     * 
     * Initialization in simulation or test system (hasCamera = False):
     * 1) read calibration from binary file, if available
     * 2) SKIP [read calibration header from camera, if available]
     * 3) SKIP [if the calibration from file corresponds to the calibration header, calibration loading was successful (loaded from cache),
     *    if not, use the calibration from the camera (and rewrite the binary file)]
     * 4) if the calibration could not be read from the BINARY FILE , use the calibration from the csv file in config folder as a FALLBACK
     * 4-bis) for backcompatibility: use the calibration from the csv file in calib folder as FALLBACK
     * 5) if neither calibration is available, return false 
     */
    
    system::CamGridData oCamGridData;

    //initialization order
    const bool useFallbackCSVFile = true;
    const bool useFallbackCSVFileInCalibFolder = true;
        
    //get the instance of calibrationdata just to get the correct filenames    
    const auto & rCalibData = m_rCalibrationManager.getCalibrationData(p_oSensorID);
    const auto & rFilenames = rCalibData.getFilenamesConfiguration();
    const std::string oCalibBinCacheFile = rFilenames.getCamGridDataBinaryFilename();
    const std::string oCalibBinCacheFileFailed = oCalibBinCacheFile+"FAILED.dat";
    const std::string oCalibFallbackFile = useFallbackCSVFile? rFilenames.getCSVFallbackFilename(): "";
    const std::string oCalibFallbackFileInCalibFolder = useFallbackCSVFileInCalibFolder? rFilenames.getCSVFallbackFilenameInCalibFolder(): oCalibFallbackFile;
    const std::string oCalibCopyFile = rFilenames.getCopyCSVFilename(); //human readable copy of the calibration data, if empty no copy is saved
    
    //1) read calibration from binary file
    std::uint32_t oCamGridDataCacheChecksum;
    wmLog(eDebug, "Try reading from cache file " + oCalibBinCacheFile + "\n"); 
    bool validCache = oCamGridData.loadFromBytes(oCalibBinCacheFile, oCamGridDataCacheChecksum, true);
    assert(oCamGridData.hasData() == validCache);
    wmLog(eDebug, "Calibration data cache is%svalid\n", std::string(oCamGridData.hasData() ? " " : " not ").c_str()); 
        
    //2) read calibration header from camera
    if (m_oHasCamera)
    {   
        std::uint32_t oCamGridDataChecksum;
        std::uint16_t oEndAddress;
        wmLog(eDebug,"Read CamGridData header from camera\n");
        std::vector<std::uint8_t> oCamGridDataHeader = m_rCalibrationManager.readUserFlashData(0, system::CamGridData::mBytesHeaderSize, p_oSensorID);
        bool validHeader = system::CamGridData::checkBytesHeader(oCamGridDataHeader, oEndAddress, oCamGridDataChecksum);        
        wmLog(eDebug, "Header shows %d bytes \n", oEndAddress);

        //3) if the calibration from file corresponds to the calibration header
        if (validCache)
        {
            wmLog(eDebug, "Check header from camera with cached calibration data \n");
            if (validHeader && (oCamGridDataCacheChecksum == oCamGridDataChecksum) )
            {
                assert(oCamGridData.isCompatibleWithHeader(oCamGridDataHeader));
                wmLog(eDebug,"Cached camgridata correspond to data in camera\n");
                assert(oEndAddress > 0 && "camgridata cached considered valid, but camera memory empty");
                assert(oCamGridData.hasData());
            }
            else
            {
                wmLog(eDebug,"Discard cached camgridata\n");
                oCamGridData.setDefaults();
                assert(!oCamGridData.hasData());
                try
                {
                    //Poco::File(oCalibBinCacheFile).remove()
                    Poco::File(oCalibBinCacheFile).renameTo(oCalibBinCacheFileFailed);
                }
                catch(const Poco::Exception & p_rException)
                {
                    wmLog(eDebug, "Could not rename cache file \n");
                }
            }
        }
        
        //3)no calibration from file, use the calibration from the camera (and rewrite the binary file)
        if (!oCamGridData.hasData() && validHeader)
        {                        
            wmLog(eWarning, "Reading CamGridData from flash memory, initialization will be longer \n");
            std::vector<std::uint8_t> oCamGridDataBytes = m_rCalibrationManager.readUserFlashData(0, oEndAddress);
            wmLog(eDebug, "%d bytes have been read \n", oCamGridDataBytes.size());
            
            bool validCamGridData = oCamGridData.loadFromBytes(oCamGridDataBytes, oCamGridDataChecksum, true); 
            wmLog(eDebug, "CamGridData loaded from camera flash memory: %s\n", validCamGridData ? "OK": "NO");

            //re-create cache
            const std::string newCacheFilename = validCamGridData? oCalibBinCacheFile: oCalibBinCacheFileFailed;
            wmLog(eDebug, "Recreate cache %s \n", newCacheFilename.c_str());
            std::ofstream output(oCalibBinCacheFile  , std::ios::binary);
            std::copy(oCamGridDataBytes.begin(), oCamGridDataBytes.end(), std::ostreambuf_iterator<char>(output));

            if (validCamGridData && oCalibCopyFile!= "")
            {
                //save an human readable copy of the calibration data that has just been read
                oCamGridData.saveToCSV(oCalibCopyFile);                
            }                
        } 
    } //end readFromCamera

    //4) if the calibration could not be read from the camera, use the calibration from the csv file in config folder as a FALLBACK
    if (!oCamGridData.hasData() && useFallbackCSVFile)
    {    
        std::string errString = oCamGridData.loadFromCSV(oCalibFallbackFile);
        bool validCamGridData = errString.empty();        
        wmLog(eInfo, "CamGridData loaded from fallback file " + oCalibFallbackFile + ": %s\n", validCamGridData ? "OK": "NO");        
    }
    
    //4-bis) if the calibration could not be read from the camera, use the calibration from the csv file in calib folder as a FALLBACK
    if (!oCamGridData.hasData() && useFallbackCSVFileInCalibFolder)
    {    
        std::string errString = oCamGridData.loadFromCSV(oCalibFallbackFileInCalibFolder);
        bool validCamGridData = errString.empty();        
        wmLog(eInfo, "CamGridData loaded from fallback file %s: %s (backcompatibility)\n", oCalibFallbackFileInCalibFolder.c_str(), validCamGridData ? "OK": "NO");        
        
        if (validCamGridData && !m_rCalibrationManager.isSimulation())  
        {
            wmLog(eWarning, "Copying calibration fallback file to configuration folder (as required by current version) \n");
            std::string errMsg = oCamGridData.saveToCSV(oCalibFallbackFile);
            if (! errMsg.empty())
            {
                wmLog(eWarning, errMsg);
            }
        }
        
        
    }
    else
    {
        if (Poco::File{oCalibFallbackFileInCalibFolder}.exists())
        {
            wmLog(eInfo, "Ignored fallback file %s from previous version \n", oCalibFallbackFileInCalibFolder.c_str());
        }
    }
        
    
       
    if (!oCamGridData.hasData())
    {
        std::ostringstream oMsg;
        oMsg << "No valid data found in camera"; 
        if (useFallbackCSVFile)
        { 
            oMsg << " or " << oCalibFallbackFile ;
        }
        if (useFallbackCSVFileInCalibFolder)
        {
            oMsg << " or " << oCalibFallbackFileInCalibFolder ;
        }
        oMsg << "\n";
        std::cout << oMsg.str() <<  std::endl;
        wmLog(eError, "No Scheimpflug valid data found\n");  
        wmLog(eDebug, oMsg.str());
        //wmLogTr(eInfo, "QnxMsg.Calib.ImgLoad", "Scheimpflug calibration image file %s %d %d...\n", oCalibImgFile.c_str(), oCalibDataFile.width(), oCalibDataFile.height() );            
    }
    return oCamGridData;
}



//called by CalibrationManager::getOSCalibrationDataFromHW(
//if sensortype is coax, this should be bypassed
//return true if scheimpflug calibration was successful
bool CalibrateIbOpticalSystem::camToCalibData(const int p_oSensorID) // no default to prevent errors by forgetting to set the correct cam/sensorID
{
    wmLog(eDebug, "CalibrateIbOpticalSystem::camToCalibData\n");           
    
    auto& rCalibrationData = m_rCalibrationManager.getCalibrationData(p_oSensorID);
    if ( rCalibrationData.getSensorModel() == SensorModel::eLinearMagnification )
    {
        std::string oMsg("camtocalibdata called, but sensortype is coax\n"); 
        wmLog(eWarning, oMsg.c_str());
        std::cout << oMsg;
        assert(false && "camToCalibData called with Coax");
        return false;                  
    }
    
    //check camgriddata or fallbackfiles, load grid is successful
    system::CamGridData oCamGridData = getCamGridDataFromCamera(p_oSensorID);
    if (!oCamGridData.hasData())
    {
        return false;
    }
    std::cout << "Initialize camgriddata " << std::endl;
    
    //there is valid oCamGridData object, now lets' load it    
    bool ok = rCalibrationData.load3DFieldFromCamGridData(oCamGridData);
        
    if (ok)
    {
        wmLogTr(eInfo, "QnxMsg.Calib.ScheimOK", "Found valid Scheimpflug calibration data.\n");
        assert(rCalibrationData.getSensorModel() == SensorModel::eCalibrationGridOnLaserPlane);
        assert(rCalibrationData.checkCalibrationValuesConsistency(rCalibrationData.CALIB_VALUES_TOL_MIN, eWarning, true));
    }
    
    return ok;
}


//computes values from averaging images
bool CalibrateIbOpticalSystem::evaluateCalibrationLayers(const int p_oSensorID)
{
	if ( !getAvgResultOfImages(m_oGraph, 4, p_oSensorID, m_oCameraRelatedParameters) )
	{
		wmLogTr(eError, "QnxMsg.Calib.BadCalib",  "Calibration failed. Bad lie or illumination of calibration workpiece or improper parameters.\n");
		return false;
	}

	return true;
}

//computes and saves scheimTriangAngle
bool CalibrateIbOpticalSystem::determineTriangulationAngle(const int p_oSensorID, const bool p_oCoax, const math::CameraRelatedParameters & rCameraRelatedParameters,
	LaserLine p_oWhichLine /* = FrontLaserLine */)
{
    m_oCameraRelatedParameters = rCameraRelatedParameters;
    //read rCalibrationDataParams 
	loadCalibrationTargetSize(p_oSensorID);

	
	if (p_oCoax)
	{
		wmLogTr(eWarning, "QnxMsg.Calib.NoScheimSys", "Not a Scheimpflug system or no Scheimpflug calibration data available.\n");
		return false;
	}

	auto& rCalibrationData = m_rCalibrationManager.getCalibrationData(p_oSensorID);
	assert(rCalibrationData.getSensorId() == p_oSensorID); 

	if ( !rCalibrationData.isInitialized())
	{
		wmLog(eError, "CalibrateIbOpticalSystem::determineTriangulationAngle has requested a not instantiated sensor", p_oSensorID);	
		return false;
	}
		
	//check presence scheimpflug system

    if ( rCalibrationData.getSensorModel() != SensorModel::eCalibrationGridOnLaserPlane || !rCalibrationData.hasData() )
    {
        wmLogTr(eWarning, "QnxMsg.Calib.NoScheimSys", "Not a Scheimpflug system or no Scheimpflug calibration data available.\n");
        return false;
    }

    bool useTransposedImage = false; //FIXME
    setFilterParametersLine(m_oGraph, p_oWhichLine, useTransposedImage, rCalibrationData.getParameters());
    bool updatedLayers = evaluateCalibrationLayers(p_oSensorID);
    if (!updatedLayers)
    {
        wmLog(eWarning,"Scheimpflug system  could not evaluate calibration layers");
        wmLogTr(eError, "QnxMsg.Calib.BadCalib",  "Calibration failed. Bad lie or illumination of calibration workpiece or improper parameters.\n");
        return false;
    }


	// get HWRoi Offset
	int oXOffsetHWRoi(0), oYOffsetHWRoi(0);
	try
	{
		if (m_oHasCamera)
		{
			oXOffsetHWRoi = m_rCalibrationManager.getROI_X0(p_oSensorID);
			oYOffsetHWRoi = m_rCalibrationManager.getROI_Y0(p_oSensorID);
		}
	}
	catch(std::exception &p_rExc)
	{
		wmLogTr(eWarning, "QnxMsg.Calib.NoCam", "Cannot determine Hardware Roi. Check camera connection!\n");
	}

	// Start computation

	double oLength = 0.0;

	//std::cout<<"+++++++++++++++++++++++++Strecke Z _Richtung in mm: "<<oLength<<"++++++++++++++++++++++++"<<std::endl;

	double oAngle(0.0);
	double angle_deg(0);

	// todo: key values with sensorID...
		
    //rCalibrationData.setKeyValue("scheimTriangAngle", 0.0); // reset angle
    oAngle = computeTriangAngle(oLength, oXOffsetHWRoi, oYOffsetHWRoi, SensorId(p_oSensorID), p_oCoax);
    angle_deg = oAngle*180/math::pi;

    // Validity checks. All OK? Only in this case we modify the CalibrationData parameters...
    if ( oLength > 0 )
    {
        double scheimAngle = rCalibrationData.getParameters().getDouble("scheimTriangAngle") ;
        double newOrientationAngle =  angle_deg - scheimAngle;


        rCalibrationData.setKeyValue("DpixX", m_oCameraRelatedParameters.m_oDpixX);
        rCalibrationData.setKeyValue("DpixX", m_oCameraRelatedParameters.m_oDpixY);
        printParameterUpdate(newOrientationAngle, rCalibrationData.getParameters().getDouble("scheimOrientationAngle")  , "scheimOrientationAngle", 0.001, eWarning);
        rCalibrationData.setKeyValue("scheimOrientationAngle", newOrientationAngle);
        
        drawAngleInfo(oLength, angle_deg);
        //as in calibrateLine for the coax case, we need to send the calib data changed message
        //todo: check if p_oInit can be  false, because the coordinate in the internal plane dont' change
        m_rCalibrationManager.sendCalibDataChangedSignal(p_oSensorID, true); // transfer xml file
        assert(rCalibrationData.checkCalibrationValuesConsistency());
        wmLog(eDebug,"determineTriangulationAngle: called signalCalibDataChanged(%d) \n", p_oSensorID );
        
        return true;
    }
    else
    {
        wmLogTr(eError, "QnxMsg.Calib.badTriangScheim", "Cannot determine Scheimpflug triangulation angle for sensor %d!\n", p_oSensorID);
        return false; 
    }

} 

bool CalibrateIbOpticalSystem::LayerLines::isInitialized() const
{
    return m_oSlopeHigher != 0.0 || m_oSlopeLower != 0.0 || m_oInterceptHigher != 0.0 || m_oInterceptLower != 0.0;
}


double CalibrateIbOpticalSystem::LayerLines::getYHigher(double x) const
{
    return m_oInterceptHigher + m_oSlopeHigher * x;
}


double CalibrateIbOpticalSystem::LayerLines::getYLower(double x) const
{
    return m_oInterceptLower + m_oSlopeLower * x;
}

}// namespaces
}
