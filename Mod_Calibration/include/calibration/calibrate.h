/*
 * Calibrate.h
 *
 *  Created on: Jun 10, 2013
 *      Author: abeschorner
 */

#ifndef TRIANGULATION_H_
#define TRIANGULATION_H_

#include <vector>
#include <array>
#include <map>
#include <string>

#include "calibrationManager.h"
#include "calibrationProcedure.h"
#include "calibrationGraph.h"
#include "math/mathUtils.h"

using precitec::math::angleUnit;
using precitec::filter::LaserLine;

namespace precitec {

using namespace interface;

/**
 * @brief In this namespace all the calibration related classes are organized - the CalibrationManager is the main class that controls and executes the calibration procedures.
 */

namespace calibration {


/*
 * @brief Base class for any calibration, image based or not.
 *
 */
class CalibrateIb : public CalibrationProcedure {
public:
	explicit CalibrateIb(CalibrationManager &p_rCalibMgr); ///< Explicit constructor utilizing CalibrationManager object
	virtual ~CalibrateIb();                                ///< Destructor

	virtual bool calibrate() { return false; };        ///< Main routine for calibration

	static bool setFilterParametersLine(CalibrationGraph &p_rGraph, const LaserLine p_oWhichLine, bool useTransposedImage, const precitec::math::CalibrationParamMap& rCalibrationData);  ///< Set filter parameters for calibration graph
	static bool computeLineRegression(double &p_rSlope, double &p_rIntercept, math::tVecDouble &p_rPlaneX, math::tVecDouble &p_rPlaneY); ///< Compute linear regression given coordinatex p_rPlaneX and p_rPlaneY.
	void showLivePic(const std::string p_oTitle);
	bool getAvgResultOfImages(CalibrationGraph &p_rGraph, const unsigned int p_oNumImages, const int p_oSensorID, const math::CameraRelatedParameters & rCameraRelatedParameters, 
			const std::string p_oTitle=""); ///< Make p_oNumImages single shots, execute graph and get avg. results

protected:
	virtual void drawInfo(const double, const double) {};
	static bool printParameterUpdate(const double p_newValue, const double p_oldValue, const std::string p_name, const double p_tol, const LogType p_LogLevel );
	double m_toleranceParameterUpdate;
	static bool checkLayerPosition(const bool isTopLayerHigher, const double Yhigher, const double Ylower);

 	BImage m_oLastImage;
	double m_oHWRoiX;                 ///< Hardware ROI x direction
	double m_oHWRoiY;                 ///< Hardware ROI y direction

	math::tVecDouble m_oXHigher;         ///< Vector of line end/start x pixel coordinates of higher level plane of image from calibration workpiece. For magnification computation.
	math::tVecDouble m_oYHigher;         ///< Vector of line end/start y pixel coordinates of higher level plane of image from calibration workpiece
	math::tVecDouble m_oXLower;         ///< Vector of line end/start x pixel coordinates of lower level plane of image from calibration workpiece
	math::tVecDouble m_oYLower;         ///< Vector of line end/start y pixel coordinates of lower level planes of image from calibration workpiece

	math::tVecDouble m_oXPlaneHigher;    ///< Vector of line x pixels of higher level planes of image from calibration workpiece. For laser plane computation.
	math::tVecDouble m_oYPlaneHigher;    ///< Vector of line y pixels of higher level planes of image from calibration workpiece
	math::tVecDouble m_oXPlaneLower;    ///< Vector of line x pixels of lower level planes of image from calibration workpiece
	math::tVecDouble m_oYPlaneLower;    ///< Vector of line y pixels of lower level planes of image from calibration workpiece


	bool m_oisTopLayerHigher;
};


// ------------------------------------ //

/*
 * @brief Image based calibration base class for optical systems; Calibration via given graph.
 *
 * CalibrationData holds two magnification values beta0 and beta0Grid and their z positions (baseCalibPos and beta0PosGrid). The *base* magnification value beta0
 * needs to be calibrated before any measurements are meaningful. Grid calibration should be performed for any Scheimpflug system, as the magnification varies with
 * the vertical position of objects in an image. The image should for grid computation needs to have a distance of at least 40 pixel (down or up) from the base
 * calibration. If so, on success a linearly interpolated grid is computed and CalibrationData::beta0(int ypos) offers a way of returning a beta0 value given
 * the y position. The grid is quantized by m_oGridDelta.
 *
 */
class CalibrateIbOpticalSystem : public CalibrateIb
{
public:
	explicit CalibrateIbOpticalSystem(CalibrationManager &p_rCalibMgr);  ///< Explicit constructor
	virtual ~CalibrateIbOpticalSystem();                                 ///< Destructor
	virtual bool calibrate(){ return false; }                                                       ///< Main calibration routine which has become deprecated
	bool calibrateLine(filter::LaserLine p_oWhichLine, bool useOrientedLine, const math::CameraRelatedParameters & rCameraRelatedParameters);
	bool camToCalibData(const int p_oSensorID); //needed by calibrationmanager                                                      ///< Main calibration routine.
	bool determineTriangulationAngle(const int p_oSensorID, const bool p_oCoax, const math::CameraRelatedParameters & rCameraRelatedParameters,
		LaserLine p_oWhichLine = filter::LaserLine::FrontLaserLine); //called by calibration manager for Scheimpflug case

protected:
	struct LayerLines
	{
		double m_oSlopeHigher = 0.0;               ///< Slope of higher layer regression line
		double m_oSlopeLower = 0.0;               ///< Slope of lower layer regression line
		double m_oInterceptHigher = 0.0;           ///< Intercept of higher layer regression line
		double m_oInterceptLower = 0.0;           ///< Intercept of lower layer regression line
		bool isInitialized() const;
		void reset(math::tVecDouble m_oXPlaneHigher, math::tVecDouble m_oYPlaneHigher,
                   math::tVecDouble m_oXPlaneLower, math::tVecDouble m_oYPlaneLower,
            double m_oHWRoiX, double m_oHWRoiY, bool m_oisTopLayerHigher);
        double getYHigher(double x) const;
        double getYLower(double x) const;

	};
    struct BetaZPaintInfo
    {
        int m_oProjX = 0; // for painting betaZ
        int m_oProjY = 0; // for painting betaZ
        int m_oValX = 0;  // dito
        int m_oValY = 0;  // dito
    };


    void loadCalibrationTargetSize(int pSensorId);
	bool startCalibration(CalibrationGraph &p_rGraph, const math::SensorId pSensorId, const filter::LaserLine p_oWhichLine, bool useOrientedLine);  ///< Subroutine called by calibrate(). Evaluates calibration and saves results on success.
	bool performMagnificationComputation(CalibrationGraph &p_rGraph, const int p_oSensorID, LaserLine p_oWhichLine); ///< Compute beta0 and betaZ magnifications, grid and base.
	double computeBeta0(double &p_rPixDeltaBeta0, const double p_oDpixX) const; ///< Compute beta0, called by performMagnificationComputation.
	double computeBetaZ(double &p_rPixDelta,double &p_rLength, BetaZPaintInfo & p_rPaintInfo, const double p_oDpixY) const; ///< Subroutine for betaZ computation, called by performMagnificationComputation
	double computeTriangAngle(double &p_rLength, const int p_oOffsetX, const int p_oOffsetY, const math::SensorId p_oSensorID,const bool p_oCoax);
	system::CamGridData getCamGridDataFromCamera(const int p_oSensorID); //needed by camToCalibData
    
	bool evaluateCalibrationLayers(const int p_oSensorID);
	void drawAngleInfo(const double p_oLength, const double p_oAngle);
	double projectOntoHigherRegression(const double p_oXPosLower, const double p_oYPosLower,
			int &p_rXHigher, int &p_rYHigher, double &p_rPixDelta) const;
	double projectOntoHigherOffset(const double p_oXSrc, const double p_oYSrc, const int p_oOffsetX, const int p_oOffsetY,
			int &p_rXHigher, int &p_rYHigher, double &p_rPixDelta) const;
	
	CalibrationGraph m_oGraph; ///< Calibration graph.


	LayerLines m_oLayerLines;
	BetaZPaintInfo m_oBetaZPaintInfo;
	math::CoaxTriangulationAngle m_oComputedParams;

	bool m_oHasCamera;
	math::CameraRelatedParameters m_oCameraRelatedParameters;
	double m_oLayerDelta;
	double m_oGapWidth;  
    bool m_oAdditionalDebug;
};


// ------------------------------------ //



} // namespaces
}

#endif /* TRIANGULATION_H_ */
